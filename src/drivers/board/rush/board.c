/*
 * Copyright 2014 Google Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <sysinfo.h>

#include "base/algorithm.h"
#include "base/init_funcs.h"
#include "base/time.h"
#include "base/xalloc.h"
#include "boot/fit.h"
#include "drivers/blockdev/tegra_mmc.h"
#include "drivers/board/board.h"
#include "drivers/board/board_helpers.h"
#include "drivers/bus/spi/tegra.h"
#include "drivers/bus/i2c/tegra.h"
#include "drivers/bus/usb/usb.h"
#include "drivers/dma/tegra_apb.h"
#include "drivers/ec/cros/spi.h"
#include "drivers/flash/spi.h"
#include "drivers/gpio/fwdb.h"
#include "drivers/gpio/gpio.h"
#include "drivers/gpio/tegra.h"
#include "drivers/keyboard/dynamic.h"
#include "drivers/keyboard/mkbp/keyboard.h"
#include "drivers/layout/coreboot.h"
#include "drivers/power/as3722.h"
#include "drivers/power/gpio_reset.h"
#include "drivers/sound/i2s.h"
#include "drivers/sound/max98090.h"
#include "drivers/sound/tegra_ahub.h"
#include "drivers/storage/flash.h"
#include "drivers/tpm/slb9635_i2c.h"
#include "drivers/tpm/tpm.h"
#include "drivers/uart/8250.h"
#include "drivers/video/display.h"
#include "drivers/video/tegra132.h"

enum {
	BOARD_ID_PROTO_0 = 0x0A,
};

enum {
	CLK_RST_BASE = 0x60006000,

	CLK_RST_L_RST_SET = CLK_RST_BASE + 0x300,
	CLK_RST_L_RST_CLR = CLK_RST_BASE + 0x304,
	CLK_RST_H_RST_SET = CLK_RST_BASE + 0x308,
	CLK_RST_H_RST_CLR = CLK_RST_BASE + 0x30c,
	CLK_RST_U_RST_SET = CLK_RST_BASE + 0x310,
	CLK_RST_U_RST_CLR = CLK_RST_BASE + 0x314
};

enum {
	CLK_L_I2C1 = 0x1 << 12,
	CLK_U_I2C3 = 0x1 << 3,
	CLK_H_I2C5 = 0x1 << 15
};

typedef struct VirtualMmcPowerGpio
{
	GpioOps ops;

	GpioOps *gpio;
	As3722Pmic *as3722;
	uint8_t reg, enable_val, disable_val;  // Params for as3722.
} VirtualMmcPowerGpio;

static int virtual_mmc_power_set(GpioOps *me, unsigned value)
{
	VirtualMmcPowerGpio *power = container_of(me, VirtualMmcPowerGpio, ops);
	assert(power->gpio && power->as3722);
	if (gpio_set(power->gpio, value) ||
	    power->as3722->set_reg(power->as3722, power->reg, value ?
				   power->enable_val : power->disable_val)) {
		printf("Failed to enable SD/MMC power.\n");
		return -1;
	}
	return 0;
}

static VirtualMmcPowerGpio *new_virtual_mmc_power(GpioOps *gpio,
						  As3722Pmic *as3722,
						  uint8_t reg,
						  uint8_t enable_val,
						  uint8_t disable_val)
{
	VirtualMmcPowerGpio *power = xzalloc(sizeof(*power));
	power->gpio = gpio;
	power->as3722 = as3722;
	power->reg = reg;
	power->enable_val = enable_val;
	power->disable_val = disable_val;
	power->ops.set = &virtual_mmc_power_set;
	return power;
}

static void choose_devicetree_by_boardid(void)
{
	switch(lib_sysinfo.board_id) {
	case BOARD_ID_PROTO_0:
		fit_set_compat("nvidia,norrin");
		break;
	default:
		printf("Unknown board id: %x\n", lib_sysinfo.board_id);
		break;
	}
}

PRIV_DYN(pwr_i2c, &new_tegra_i2c((void *)0x7000d000, 5,
				 (void *)CLK_RST_H_RST_SET,
				 (void *)CLK_RST_H_RST_CLR,
				 CLK_H_I2C5)->ops)

PRIV_DYN(pmic, new_as3722_pmic(get_pwr_i2c(), 0x40))

PRIV_DYN(reset_gpio, &new_tegra_gpio_output(GPIO(I, 5))->ops)
PRIV_DYN(reset_gpio_n, new_gpio_not(get_reset_gpio()))

PRIV_DYN(lid_gpio, &new_tegra_gpio_input(GPIO(R, 4))->ops)
PRIV_DYN(power_gpio, &new_tegra_gpio_input(GPIO(Q, 0))->ops)

PUB_STAT(flag_write_protect, gpio_get(&fwdb_gpio_wpsw.ops))
PUB_STAT(flag_recovery, gpio_get(&fwdb_gpio_recsw.ops))
PUB_STAT(flag_developer_mode, gpio_get(&fwdb_gpio_devsw.ops))
PUB_STAT(flag_option_roms_loaded, gpio_get(&fwdb_gpio_oprom.ops))
PUB_STAT(flag_lid_open, gpio_get(get_lid_gpio()))
PUB_STAT(flag_power, !gpio_get(get_power_gpio()))
PUB_STAT(flag_ec_in_rw, gpio_get(&fwdb_gpio_ecinrw.ops))

static TegraApbDmaController *build_dma_controller(void)
{
	void *dma_channel_bases[32];
	for (int i = 0; i < ARRAY_SIZE(dma_channel_bases); i++) {
		dma_channel_bases[i] =
			(void *)((uintptr_t)0x60021000 + 0x40 * i);
	}

	return new_tegra_apb_dma((void *)0x60020000, dma_channel_bases,
				 ARRAY_SIZE(dma_channel_bases));
}
PRIV_DYN(dma_controller, build_dma_controller())

PRIV_DYN(spi1, &new_tegra_spi(0x7000d400, get_dma_controller(),
			      APBDMA_SLAVE_SL2B1)->ops)
PRIV_DYN(spi4, &new_tegra_spi(0x7000da00, get_dma_controller(),
			      APBDMA_SLAVE_SL2B4)->ops)

PRIV_DYN(flash, &new_spi_flash(get_spi4())->ops)
PUB_DYN(_coreboot_storage, &new_flash_storage(get_flash())->ops)

static int board_setup(void)
{
        choose_devicetree_by_boardid();

	TegraI2c *cam_i2c = new_tegra_i2c((void *)0x7000c500, 3,
					  (void *)CLK_RST_U_RST_SET,
					  (void *)CLK_RST_U_RST_CLR,
					  CLK_U_I2C3);

	tpm_set_ops(&new_slb9635_i2c(&cam_i2c->ops, 0x20)->base.ops);

	TegraAudioHubXbar *xbar = new_tegra_audio_hub_xbar(0x70300800);
	TegraAudioHubApbif *apbif = new_tegra_audio_hub_apbif(0x70300000, 8);

	TegraI2s *i2s1 = new_tegra_i2s(0x70301100, &apbif->ops, 1, 16, 2,
				       1536000, 48000);
	TegraAudioHub *ahub = new_tegra_audio_hub(xbar, apbif, i2s1);
	I2sSource *i2s_source = new_i2s_source(&i2s1->ops, 48000, 2, 16000);
	SoundRoute *sound_route = new_sound_route(&i2s_source->ops);
	TegraI2c *i2c1 = new_tegra_i2c((void *)0x7000c000, 1,
				       (void *)CLK_RST_L_RST_SET,
				       (void *)CLK_RST_L_RST_CLR,
				       CLK_L_I2C1);
	Max98090Codec *codec = new_max98090_codec(&i2c1->ops, 0x10, 16, 48000, 256, 1);
	list_insert_after(&ahub->component.list_node, &sound_route->components);
	list_insert_after(&codec->component.list_node, &sound_route->components);

	sound_set_ops(&sound_route->ops);

	cros_ec_set_bus(&new_cros_ec_spi_bus(get_spi1())->ops);

	// sdmmc4
	TegraMmcHost *emmc = new_tegra_mmc_host(0x700b0600, 8, 0, NULL, NULL);
	// sdmmc3
	TegraGpio *enable_vdd_sd = new_tegra_gpio_output(GPIO(R, 0));
	// The params in mmc_power set AS3722_LDO6 to 3.3V.
	VirtualMmcPowerGpio *mmc_power = new_virtual_mmc_power(
			&enable_vdd_sd->ops, get_pmic(), 0x16, 0x3F, 0);
	TegraGpio *card_detect = new_tegra_gpio_input(GPIO(V, 2));
	GpioOps *card_detect_ops = &card_detect->ops;
	card_detect_ops = new_gpio_not(card_detect_ops);
	TegraMmcHost *sd_card = new_tegra_mmc_host(0x700b0400, 4, 1,
						   card_detect_ops,
						   &mmc_power->ops);

	list_insert_after(&emmc->mmc.ctrlr.list_node,
			  &fixed_block_dev_controllers);
	list_insert_after(&sd_card->mmc.ctrlr.list_node,
			  &removable_block_dev_controllers);

	/* Careful: the EHCI base is at offset 0x100 from the SoC's IP base */
	UsbHostController *usbd = new_usb_hc(UsbEhci, 0x7d000100);
	/* USB2 is connected to the camera, not needed in firmware */
	UsbHostController *usb3 = new_usb_hc(UsbEhci, 0x7d008100);

	list_insert_after(&usbd->list_node, &usb_host_controllers);
	list_insert_after(&usb3->list_node, &usb_host_controllers);

	return 0;
}

INIT_FUNC(board_setup);

/* Turn on or turn off the backlight */
static int rush_backlight_update(DisplayOps *me, uint8_t enable)
{
	TegraGpio *backlight_vdd_enable = new_tegra_gpio_output(GPIO(P, 2));
	TegraGpio *backlight_enable = new_tegra_gpio_output(GPIO(H, 2));

	gpio_set(&backlight_vdd_enable->ops, enable);
	mdelay(10);
	gpio_set(&backlight_enable->ops, enable);

	return 0;
}

static DisplayOps rush_display_ops = {
	.init = &tegra132_display_init,
	.backlight_update = &rush_backlight_update,
	.stop = &tegra132_display_stop,
};

static int display_setup(void)
{
	if (lib_sysinfo.framebuffer == NULL ||
		lib_sysinfo.framebuffer->physical_address == 0)
		return 0;

	display_set_ops(&rush_display_ops);

	return 0;
}

INIT_FUNC(display_setup);

PUB_DYN(power, &new_gpio_reset_power_ops(&get_pmic()->ops,
					 get_reset_gpio_n())->ops)

PUB_DYN(debug_uart, &new_uart_8250_mem(0x70006000, 1)->uart.ops)

PUB_ARR(trusted_keyboards, &mkbp_keyboard.ops)
PUB_ARR(untrusted_keyboards, &dynamic_keyboards.ops)
