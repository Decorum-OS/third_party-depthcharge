/*
 * Copyright 2013 Google Inc.
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

#include "base/init_funcs.h"
#include "boot/fit.h"
#include "drivers/blockdev/blockdev.h"
#include "drivers/blockdev/dw_mmc.h"
#include "drivers/board/board.h"
#include "drivers/board/board_helpers.h"
#include "drivers/bus/i2c/exynos5_usi.h"
#include "drivers/bus/i2s/exynos5/exynos5.h"
#include "drivers/bus/spi/exynos5.h"
#include "drivers/bus/usb/exynos.h"
#include "drivers/bus/usb/usb.h"
#include "drivers/ec/cros/spi.h"
#include "drivers/flash/spi.h"
#include "drivers/gpio/exynos5420.h"
#include "drivers/gpio/fwdb.h"
#include "drivers/gpio/gpio.h"
#include "drivers/keyboard/dynamic.h"
#include "drivers/keyboard/mkbp/keyboard.h"
#include "drivers/layout/coreboot.h"
#include "drivers/power/exynos.h"
#include "drivers/sound/i2s.h"
#include "drivers/sound/route.h"
#include "drivers/sound/sound.h"
#include "drivers/storage/flash.h"
#include "drivers/tpm/slb9635_i2c.h"
#include "drivers/tpm/tpm.h"
#include "drivers/uart/s5p.h"

PRIV_DYN(lid_gpio, &new_exynos5420_gpio_input(GPIO_X, 3, 4)->ops)
PRIV_DYN(power_gpio, &new_exynos5420_gpio_input(GPIO_X, 1, 2)->ops)
PRIV_DYN(ec_in_rw_gpio, &new_exynos5420_gpio_input(GPIO_X, 2, 3)->ops)

PUB_STAT(flag_write_protect, gpio_get(&fwdb_gpio_wpsw.ops))
PUB_STAT(flag_recovery, gpio_get(&fwdb_gpio_recsw.ops))
PUB_STAT(flag_developer_mode, gpio_get(&fwdb_gpio_devsw.ops))
PUB_STAT(flag_option_roms_loaded, gpio_get(&fwdb_gpio_oprom.ops))
PUB_STAT(flag_lid_open, gpio_get(get_lid_gpio()))
PUB_STAT(flag_power, !gpio_get(get_power_gpio()))
PUB_STAT(flag_ec_in_rw, gpio_get(get_ec_in_rw_gpio()))

PRIV_DYN(spi1, &new_exynos5_spi(0x12d30000)->ops)
PRIV_DYN(spi2, &new_exynos5_spi(0x12d40000)->ops)

PRIV_DYN(flash, &new_spi_flash(get_spi1())->ops);
PUB_DYN(_coreboot_storage, &new_flash_storage(get_flash())->ops);

static int board_setup(void)
{
	fit_set_compat("google,pit-rev3");

	Exynos5UsiI2c *i2c9 = new_exynos5_usi_i2c(0x12e10000, 400000);

	tpm_set_ops(&new_slb9635_i2c(&i2c9->ops, 0x20)->base.ops);

	CrosEcSpiBus *cros_ec_spi_bus = new_cros_ec_spi_bus(get_spi2());
	cros_ec_set_bus(&cros_ec_spi_bus->ops);

	Exynos5I2s *i2s0 = new_exynos5_i2s_multi(0x03830000, 16, 2, 256);
	I2sSource *i2s_source = new_i2s_source(&i2s0->ops, 48000, 2, 16000);
	sound_set_ops(&new_sound_route(&i2s_source->ops)->ops);

	DwmciHost *emmc = new_dwmci_host(0x12200000, 100000000, 8, 0, NULL,
					 DWMCI_SET_SAMPLE_CLK(1) |
					 DWMCI_SET_DRV_CLK(3) |
					 DWMCI_SET_DIV_RATIO(3));
	DwmciHost *sd_card = new_dwmci_host(0x12220000, 100000000, 4, 1, NULL,
					    DWMCI_SET_SAMPLE_CLK(1) |
					    DWMCI_SET_DRV_CLK(2) |
					    DWMCI_SET_DIV_RATIO(3));
	list_insert_after(&emmc->mmc.ctrlr.list_node,
			  &fixed_block_dev_controllers);
	list_insert_after(&sd_card->mmc.ctrlr.list_node,
			  &removable_block_dev_controllers);

	UsbHostController *usb_drd0 = new_usb_hc(UsbXhci, 0x12000000);
	UsbHostController *usb_drd1 = new_usb_hc(UsbXhci, 0x12400000);

	set_usb_init_callback(usb_drd0, exynos5420_usbss_phy_tune);
	/* DRD1 port has no SuperSpeed lines anyway */

	list_insert_after(&usb_drd0->list_node, &usb_host_controllers);
	list_insert_after(&usb_drd1->list_node, &usb_host_controllers);

	return 0;
}

PUB_STAT(power, &exynos_power_ops)

PUB_DYN(debug_uart, &new_uart_s5p(0x12c00000 + 3 * 0x10000)->ops)

PUB_ARR(trusted_keyboards, &mkbp_keyboard.ops)
PUB_ARR(untrusted_keyboards, &dynamic_keyboards.ops)

INIT_FUNC(board_setup);
