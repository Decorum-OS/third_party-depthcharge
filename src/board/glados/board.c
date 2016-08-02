/*
 * Copyright (C) 2015 Google Inc.
 * Copyright (C) 2015 Intel Corporation
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
#include <stdint.h>

#include "base/init_funcs.h"
#include "base/list.h"
#include "board/board.h"
#include "board/board_helpers.h"
#include "drivers/blockdev/blockdev.h"
#include "drivers/blockdev/sdhci.h"
#include "drivers/bus/i2c/designware.h"
#include "drivers/bus/i2c/i2c.h"
#include "drivers/ec/cros/lpc.h"
#include "drivers/gpio/fwdb.h"
#include "drivers/gpio/gpio.h"
#include "drivers/gpio/skylake.h"
#include "drivers/keyboard/dynamic.h"
#include "drivers/keyboard/ps2.h"
#include "drivers/layout/coreboot.h"
#include "drivers/power/pch.h"
#include "drivers/sound/gpio_pdm.h"
#include "drivers/sound/route.h"
#include "drivers/sound/ssm4567.h"
#include "drivers/storage/x86_flash.h"
#include "drivers/tpm/lpc.h"
#include "drivers/tpm/tpm.h"
#include "drivers/uart/8250.h"

/*
 * Clock frequencies for the eMMC and SD ports are defined below. The minimum
 * frequency is the same for both interfaces, the firmware does not run any
 * interface faster than 52 MHz, but defines maximum eMMC frequency as 200 MHz
 * for proper divider settings.
 */
#define EMMC_SD_CLOCK_MIN	400000
#define EMMC_CLOCK_MAX		200000000
#define SD_CLOCK_MAX		52000000

PRIV_DYN(ec_in_rw_gpio, &new_skylake_gpio_input(GPP_C6)->ops);

PUB_STAT(flag_write_protect, gpio_get(&fwdb_gpio_wpsw.ops))
PUB_STAT(flag_recovery, gpio_get(&fwdb_gpio_recsw.ops))
PUB_STAT(flag_developer_mode, gpio_get(&fwdb_gpio_devsw.ops))
PUB_STAT(flag_option_roms_loaded, gpio_get(&fwdb_gpio_oprom.ops))
PUB_STAT(flag_lid_open, gpio_get(&fwdb_gpio_lidsw.ops))
PUB_STAT(flag_power, gpio_get(&fwdb_gpio_pwrsw.ops))
PUB_STAT(flag_ec_in_rw, gpio_get(get_ec_in_rw_gpio()))

PUB_DYN(_coreboot_storage, new_x86_flash_storage());

static int board_setup(void)
{
	/* MEC1322 Chrome EC */
	CrosEcLpcBus *cros_ec_lpc_bus =
		new_cros_ec_lpc_bus(CROS_EC_LPC_BUS_MEC);
	cros_ec_set_bus(&cros_ec_lpc_bus->ops);

	/* SPI TPM memory mapped to act like LPC TPM */
	tpm_set_ops(&new_lpc_tpm((void *)(uintptr_t)0xfed40000)->ops);

	/* eMMC */
	SdhciHost *emmc = new_pci_sdhci_host(PCI_DEV(0, 0x1e, 4),
			SDHCI_PLATFORM_NO_EMMC_HS200,
			EMMC_SD_CLOCK_MIN, EMMC_CLOCK_MAX);
	list_insert_after(&emmc->mmc_ctrlr.ctrlr.list_node,
			&fixed_block_dev_controllers);

	/* SD Card */
	SdhciHost *sd = new_pci_sdhci_host(PCI_DEV(0, 0x1e, 6), 1,
			EMMC_SD_CLOCK_MIN, SD_CLOCK_MAX);
	list_insert_after(&sd->mmc_ctrlr.ctrlr.list_node,
				&removable_block_dev_controllers);

	/* Speaker Amp Codec is on I2C4 */
	DesignwareI2c *i2c4 =
		new_pci_designware_i2c(PCI_DEV(0, 0x19, 2), 400000);
	ssm4567Codec *speaker_amp_left =
		new_ssm4567_codec(&i2c4->ops, 0x34, SSM4567_MODE_PDM);

	/* Use GPIO to bit-bang PDM to the codec */
	GpioCfg *i2s2_sclk = new_skylake_gpio_output(GPP_F0, 0);
	GpioCfg *i2s2_txd  = new_skylake_gpio_output(GPP_F2, 0);
	GpioPdm *pdm = new_gpio_pdm(&i2s2_sclk->ops,	/* PDM Clock GPIO */
				    &i2s2_txd->ops,	/* PDM Data GPIO */
				    85000,		/* Clock Start */
				    16000,		/* Sample Rate */
				    2,			/* Channels */
				    1000);		/* Volume */

	/* Connect the Codec to the PDM source */
	SoundRoute *sound = new_sound_route(&pdm->ops);
	list_insert_after(&speaker_amp_left->component.list_node,
			  &sound->components);
	sound_set_ops(&sound->ops);

	return 0;
}

PUB_STAT(power, &skylake_power_ops)

PUB_DYN(debug_uart, &new_uart_8250_mem32(0xfe034000)->uart.ops)

PUB_ARR(trusted_keyboards, &new_ps2_keyboard()->ops)
PUB_ARR(untrusted_keyboards, &dynamic_keyboards.ops)

INIT_FUNC(board_setup);
