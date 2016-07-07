/*
 * Copyright 2014 Rockchip Electronics Co., Ltd.
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

#include <stdio.h>
#include <sysinfo.h>

#include "base/container_of.h"
#include "base/init_funcs.h"
#include "base/io.h"
#include "base/xalloc.h"
#include "board/board.h"
#include "board/board_helpers.h"
#include "boot/fit.h"
#include "boot/ramoops.h"
#include "drivers/blockdev/dw_mmc.h"
#include "drivers/blockdev/rk_mmc.h"
#include "drivers/bus/i2c/rockchip.h"
#include "drivers/bus/i2s/rockchip.h"
#include "drivers/bus/spi/rockchip.h"
#include "drivers/bus/usb/usb.h"
#include "drivers/flash/spi.h"
#include "drivers/gpio/fwdb.h"
#include "drivers/gpio/gpio.h"
#include "drivers/gpio/rockchip.h"
#include "drivers/keyboard/dynamic.h"
#include "drivers/power/gpio_reset.h"
#include "drivers/power/rk808.h"
#include "drivers/tpm/slb9635_i2c.h"
#include "drivers/tpm/tpm.h"
#include "drivers/uart/8250.h"
#include "drivers/video/display.h"

typedef struct {
	DisplayOps ops;

	GpioOps *ready;		/* Wireless icon, white */
	GpioOps *ready2;	/* Wireless icon, orange */
	GpioOps *syncing;	/* Refresh icon, white */
	GpioOps *error;		/* Warning icon, orange */
} RialtoDisplayOps;

static int rialto_leds_display_screen(DisplayOps *me,
				      enum VbScreenType_t screen_type) {
	RialtoDisplayOps *leds = container_of(me, RialtoDisplayOps, ops);

	switch(screen_type) {
	case VB_SCREEN_BLANK:
		gpio_set(leds->ready,   1);
		gpio_set(leds->ready2,  1);
		gpio_set(leds->syncing, 1);
		gpio_set(leds->error,   1);
		break;

	case VB_SCREEN_DEVELOPER_WARNING:
		gpio_set(leds->ready,   0);
		gpio_set(leds->ready2,  0);
		gpio_set(leds->syncing, 1);
		gpio_set(leds->error,   1);
		break;

	case VB_SCREEN_RECOVERY_REMOVE:
		gpio_set(leds->ready,   0);
		gpio_set(leds->ready2,  1);
		gpio_set(leds->syncing, 0);
		gpio_set(leds->error,   1);
		break;

	case VB_SCREEN_RECOVERY_INSERT:
		gpio_set(leds->ready,   0);
		gpio_set(leds->ready2,  0);
		gpio_set(leds->syncing, 0);
		gpio_set(leds->error,   1);
		break;

	case VB_SCREEN_DEVELOPER_TO_NORM:
		/* fall through */
	case VB_SCREEN_RECOVERY_TO_DEV:
		gpio_set(leds->ready,   1);
		gpio_set(leds->ready2,  0);
		gpio_set(leds->syncing, 1);
		gpio_set(leds->error,   1);
		break;

	case VB_SCREEN_TO_NORM_CONFIRMED:
		/* same as blank */
		gpio_set(leds->ready,   1);
		gpio_set(leds->ready2,  1);
		gpio_set(leds->syncing, 1);
		gpio_set(leds->error,   1);
		break;

	case VB_SCREEN_RECOVERY_NO_GOOD:
		gpio_set(leds->ready,   1);
		gpio_set(leds->ready2,  1);
		gpio_set(leds->syncing, 0);
		gpio_set(leds->error,   1);
		break;

	case VB_SCREEN_DEVELOPER_EGG:
		/* fall through */
	default:
		printf("%s: no led pattern defined for screen %d\n",
			__func__, screen_type);
		return -1;
	}
	return 0;
}

RialtoDisplayOps *new_rialto_leds(void) {
	RialtoDisplayOps *leds = xzalloc(sizeof(*leds));

	/* The following are active high */
	leds->ready   = &new_rk_gpio_output(GPIO(7, A, 0))->ops; /* LED_READY */
	leds->ready2  = &new_rk_gpio_output(GPIO(7, B, 5))->ops; /* Ready2_LED */
	leds->syncing = &new_rk_gpio_output(GPIO(7, B, 3))->ops; /* LED_SYNCING */
	leds->error   = &new_rk_gpio_output(GPIO(7, B, 7))->ops; /* LED_ERROR */

	leds->ops.display_screen = rialto_leds_display_screen;

	return leds;
}

PRIV_DYN(i2c0, &new_rockchip_i2c((void *)0xff650000)->ops)

PRIV_DYN(pmic, &new_rk808_pmic(get_i2c0(), 0x1b)->ops)

PRIV_DYN(recovery_gpio, &new_rk_gpio_input(GPIO(7, B, 1))->ops)
PRIV_DYN(power_gpio, &new_rk_gpio_input(GPIO(0, A, 5))->ops)
PRIV_DYN(ec_in_rw_gpio, &new_rk_gpio_input(GPIO(0, A, 7))->ops)
PRIV_DYN(reset_gpio, &new_rk_gpio_output(GPIO(0, B, 5))->ops)

PUB_STAT(flag_write_protect, gpio_get(&fwdb_gpio_wpsw.ops))
PUB_STAT(flag_recovery, !gpio_get(get_recovery_gpio()))
PUB_STAT(flag_developer_mode, gpio_get(&fwdb_gpio_devsw.ops))
PUB_STAT(flag_option_roms_loaded, gpio_get(&fwdb_gpio_oprom.ops))
// Lid always open for now.
PUB_STAT(flag_lid_open, 1)
PUB_STAT(flag_power, !gpio_get(get_power_gpio()))
PUB_STAT(flag_ec_in_rw, gpio_get(get_ec_in_rw_gpio()))

static int board_setup(void)
{
	RialtoDisplayOps *leds;

	RkSpi *spi2 = new_rockchip_spi(0xff130000);
	flash_set_ops(&new_spi_flash(&spi2->ops)->ops);

	RkI2c *i2c1 = new_rockchip_i2c((void *)0xff140000);
	tpm_set_ops(&new_slb9635_i2c(&i2c1->ops, 0x20)->base.ops);

	DwmciHost *emmc = new_rkdwmci_host(0xff0f0000, 594000000, 8, 0, NULL);
	list_insert_after(&emmc->mmc.ctrlr.list_node,
			  &fixed_block_dev_controllers);

	leds = new_rialto_leds();
	display_set_ops(&(leds->ops));

	UsbHostController *usb_host1 = new_usb_hc(UsbDwc2, 0xff540000);
	list_insert_after(&usb_host1->list_node, &usb_host_controllers);

	UsbHostController *usb_otg = new_usb_hc(UsbDwc2, 0xff580000);
	list_insert_after(&usb_otg->list_node, &usb_host_controllers);

	ramoops_buffer(0x31f00000, 0x100000, 0x20000);

	return 0;
}

PUB_DYN(power, &new_gpio_reset_power_ops(get_pmic(), get_reset_gpio())->ops)

PUB_DYN(debug_uart, &new_uart_8250_mem32(0xff690000)->uart.ops)

PUB_ARR(trusted_keyboards, (KeyboardOps *)NULL)
PUB_ARR(untrusted_keyboards, &dynamic_keyboards.ops)

INIT_FUNC(board_setup);
