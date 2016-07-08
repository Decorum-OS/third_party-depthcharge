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

#include <pci.h>

#include "base/init_funcs.h"
#include "base/list.h"
#include "board/board.h"
#include "board/board_helpers.h"
#include "drivers/blockdev/ahci.h"
#include "drivers/blockdev/blockdev.h"
#include "drivers/flash/flash.h"
#include "drivers/flash/memmapped.h"
#include "drivers/gpio/fwdb.h"
#include "drivers/gpio/gpio.h"
#include "drivers/gpio/lynxpoint_lp.h"
#include "drivers/keyboard/dynamic.h"
#include "drivers/keyboard/ps2.h"
#include "drivers/layout/coreboot.h"
#include "drivers/power/pch.h"
#include "drivers/sound/pcat_beep.h"
#include "drivers/sound/sound.h"
#include "drivers/storage/flash.h"
#include "drivers/tpm/lpc.h"
#include "drivers/tpm/tpm.h"
#include "drivers/uart/8250.h"

PRIV_DYN(recovery_gpio, &new_lp_pch_gpio_input(12)->ops)

PUB_STAT(flag_write_protect, gpio_get(&fwdb_gpio_wpsw.ops))
PUB_STAT(flag_recovery, !gpio_get(get_recovery_gpio()))
PUB_STAT(flag_developer_mode, gpio_get(&fwdb_gpio_devsw.ops))
PUB_STAT(flag_option_roms_loaded, gpio_get(&fwdb_gpio_oprom.ops))
PUB_STAT(flag_lid_open, gpio_get(&fwdb_gpio_lidsw.ops))
PUB_STAT(flag_power, gpio_get(&fwdb_gpio_pwrsw.ops))
PUB_STAT(flag_ec_in_rw, gpio_get(&fwdb_gpio_ecinrw.ops))

PRIV_DYN(flash, &new_mem_mapped_flash(0xff800000, 0x800000)->ops);
PUB_DYN(_coreboot_storage, &new_flash_storage(get_flash())->ops);

static int board_setup(void)
{
	flash_set_ops(get_flash());

	sound_set_ops(&new_pcat_beep()->ops);

	AhciCtrlr *ahci = new_ahci_ctrlr(PCI_DEV(0, 31, 2));
	list_insert_after(&ahci->ctrlr.list_node, &fixed_block_dev_controllers);

	tpm_set_ops(&new_lpc_tpm((void *)(uintptr_t)0xfed40000)->ops);

	return 0;
}

PUB_STAT(power, &pch_power_ops)

PUB_DYN(debug_uart, &new_uart_8250_io(0x3f8)->uart.ops)

PUB_ARR(trusted_keyboards, &new_ps2_keyboard()->ops)
PUB_ARR(untrusted_keyboards, &dynamic_keyboards.ops)

INIT_FUNC(board_setup);
