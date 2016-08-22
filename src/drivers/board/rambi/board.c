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

#include "base/die.h"
#include "base/init_funcs.h"
#include "drivers/blockdev/blockdev.h"
#include "drivers/blockdev/sdhci.h"
#include "drivers/board/board.h"
#include "drivers/board/board_helpers.h"
#include "drivers/board/rambi/device_nvs.h"
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

// ECRW GPIO: SCGPIO59
PRIV_DYN(ec_in_rw_gpio, &new_baytrail_gpio_input(59 / 32, 59 % 32)->ops)

PUB_STAT(flag_write_protect, gpio_get(&fwdb_gpio_wpsw.ops))
PUB_STAT(flag_recovery, gpio_get(&fwdb_gpio_recsw.ops))
PUB_STAT(flag_developer_mode, gpio_get(&fwdb_gpio_devsw.ops))
PUB_STAT(flag_option_roms_loaded, gpio_get(&fwdb_gpio_oprom.ops))
PUB_STAT(flag_lid_open, gpio_get(&fwdb_gpio_lidsw.ops))
PUB_STAT(flag_power, gpio_get(&fwdb_gpio_pwrsw.ops))
PUB_STAT(flag_ec_in_rw, gpio_get(get_ec_in_rw_gpio()))

PUB_DYN(_coreboot_storage, new_x86_flash_storage())

static int board_setup(void)
{
	device_nvs_t *nvs = lib_sysinfo.acpi_gnvs + DEVICE_NVS_OFFSET;

	CrosEcLpcBus *cros_ec_lpc_bus =
		new_cros_ec_lpc_bus(CROS_EC_LPC_BUS_GENERIC);
	cros_ec_set_bus(&cros_ec_lpc_bus->ops);

	/* Setup sound components */
	uintptr_t lpe_mmio = nvs->lpe_bar0;
	if (!nvs->lpe_en) {
		pcidev_t lpe_pcidev = PCI_DEV(0, 0x15, 0);
		lpe_mmio = pci_read_config32(lpe_pcidev, PciConfBar0);
	}
	BytI2s *i2s = new_byt_i2s(lpe_mmio, &baytrail_max98090_settings,
				16, 2,4800000, 48000);
	I2sSource *i2s_source = new_i2s_source(&i2s->ops, 48000, 2, 16000);
	SoundRoute *sound_route = new_sound_route(&i2s_source->ops);

	die_if(!nvs->lpss_en[LPSS_NVS_I2C2], "Codec I2C misconfigured\n");
	DesignwareI2c *i2c = new_designware_i2c(
		nvs->lpss_bar0[LPSS_NVS_I2C2], 400000);
	Max98090Codec *codec = new_max98090_codec(
		&i2c->ops, 0x10, 16, 48000, 520, 1);
	list_insert_after(&codec->component.list_node,
			  &sound_route->components);
	sound_set_ops(&sound_route->ops);

	tpm_set_ops(&new_lpc_tpm((void *)0xfed40000)->ops);

	/* Initialize eMMC and SD ports in ACPI or PCI mode */
	SdhciHost *emmc;

	if (nvs->scc_en[SCC_NVS_MMC])
		emmc = new_mem_sdhci_host((void *)nvs->scc_bar0[SCC_NVS_MMC],
					  0, emmc_sd_clock_min, emmc_clock_max);
	else
		emmc = new_pci_sdhci_host(PCI_DEV(0, 23, 0), 0,
					  emmc_sd_clock_min, emmc_clock_max);

	list_insert_after(&emmc->mmc_ctrlr.ctrlr.list_node,
			  &fixed_block_dev_controllers);

	if (nvs->scc_en[SCC_NVS_SD])
		emmc = new_mem_sdhci_host((void *)nvs->scc_bar0[SCC_NVS_SD],
					  1, emmc_sd_clock_min, sd_clock_max);
	else
		emmc = new_pci_sdhci_host(PCI_DEV(0, 18, 0), 1,
					  emmc_sd_clock_min, sd_clock_max);

	list_insert_after(&emmc->mmc_ctrlr.ctrlr.list_node,
			  &removable_block_dev_controllers);

	return 0;
}

PUB_STAT(power, &baytrail_power_ops)

PUB_DYN(debug_uart, &new_uart_8250_io(0x3f8)->uart.ops)

PUB_ARR(trusted_keyboards, &new_ps2_keyboard()->ops)
PUB_ARR(untrusted_keyboards, &dynamic_keyboards.ops)

INIT_FUNC(board_setup);