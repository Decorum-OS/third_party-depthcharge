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

#include <pci.h>
#include <sysinfo.h>

#include "base/init_funcs.h"
#include "drivers/blockdev/blockdev.h"
#include "drivers/blockdev/sdhci.h"
#include "drivers/board/board.h"
#include "drivers/board/board_helpers.h"
#include "drivers/board/strago/device_nvs.h"
#include "drivers/bus/i2c/designware.h"
#include "drivers/bus/i2s/baytrail/max98090.h"
#include "drivers/ec/cros/lpc.h"
#include "drivers/gpio/baytrail.h"
#include "drivers/gpio/fwdb.h"
#include "drivers/gpio/gpio.h"
#include "drivers/keyboard/dynamic.h"
#include "drivers/keyboard/ps2.h"
#include "drivers/layout/coreboot.h"
#include "drivers/power/pch.h"
#include "drivers/sound/i2s.h"
#include "drivers/sound/max98090.h"
#include "drivers/storage/x86_flash.h"
#include "drivers/bus/usb/usb.h"
#include "drivers/tpm/lpc.h"
#include "drivers/uart/8250.h"

/*
 * Clock frequencies for the eMMC and SD ports are defined below. The minimum
 * frequency is the same for both interfaces, the firmware does not run any
 * interface faster than 52 MHz, but defines maximum eMMC frequency as 200 MHz
 * for proper divider settings.
 */
static const int emmc_sd_clock_min = 400 * 1000;
static const int emmc_clock_max = 200 * 1000 * 1000;
static const int sd_clock_max = 52 * 1000 * 1000;

PUB_STAT(flag_write_protect, gpio_get(&fwdb_gpio_wpsw.ops))
PUB_STAT(flag_recovery, gpio_get(&fwdb_gpio_recsw.ops))
PUB_STAT(flag_developer_mode, gpio_get(&fwdb_gpio_devsw.ops))
PUB_STAT(flag_option_roms_loaded, gpio_get(&fwdb_gpio_oprom.ops))
PUB_STAT(flag_lid_open, gpio_get(&fwdb_gpio_lidsw.ops))
PUB_STAT(flag_power, gpio_get(&fwdb_gpio_pwrsw.ops))
PUB_STAT(flag_ec_in_rw, gpio_get(&fwdb_gpio_ecinrw.ops))

PUB_DYN(_coreboot_storage, new_x86_flash_storage())

static int board_setup(void)
{
	device_nvs_t *nvs = lib_sysinfo.acpi_gnvs + DEVICE_NVS_OFFSET;

#if CONFIG_DRIVER_EC_CROS
  #if CONFIG_DRIVER_EC_CROS_LPC
	CrosEcLpcBus *cros_ec_lpc_bus =
		new_cros_ec_lpc_bus(CROS_EC_LPC_BUS_MEC);
	cros_ec_set_bus(&cros_ec_lpc_bus->ops);
  #endif
#endif

	tpm_set_ops(&new_lpc_tpm((void *)0xfed40000)->ops);

	SdhciHost *emmc, *sd;

	if (nvs->scc_en[SCC_NVS_MMC])
		emmc = new_mem_sdhci_host((void *)nvs->scc_bar0[SCC_NVS_MMC],
					  0, emmc_sd_clock_min, emmc_clock_max);
	else
		emmc = new_pci_sdhci_host(PCI_DEV(0, 0x10, 0), 0,
				emmc_sd_clock_min, emmc_clock_max);

	list_insert_after(&emmc->mmc_ctrlr.ctrlr.list_node,
			&fixed_block_dev_controllers);

	if (nvs->scc_en[SCC_NVS_SD])
		sd = new_mem_sdhci_host((void *)nvs->scc_bar0[SCC_NVS_SD],
					  1, emmc_sd_clock_min, sd_clock_max);
	else
		sd = new_pci_sdhci_host(PCI_DEV(0, 0x12, 0), 1,
				emmc_sd_clock_min, sd_clock_max);

	list_insert_after(&sd->mmc_ctrlr.ctrlr.list_node,
			&removable_block_dev_controllers);

	uintptr_t UsbMmioBase = pci_read_config32(PCI_DEV(0, 0x14, 0),
						  PciConfBar0);
	UsbMmioBase &= 0xFFFF0000;	// 32 bits only?
	UsbHostController *usb_host1 = new_usb_hc(UsbXhci, UsbMmioBase );
	list_insert_after(&usb_host1->list_node, &usb_host_controllers);

	return 0;
}

PUB_STAT(power, &baytrail_power_ops)

PUB_DYN(debug_uart, &new_uart_8250_io(0x3f8)->uart.ops)

PUB_ARR(trusted_keyboards, &new_ps2_keyboard()->ops)
PUB_ARR(untrusted_keyboards, &dynamic_keyboards.ops)

INIT_FUNC(board_setup);
