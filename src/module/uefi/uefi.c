/*
 * Copyright 2016 Google Inc.
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
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include "drivers/board/board.h"
#include "module/module.h"
#include "module/uefi/fwdb.h"
#include "module/uefi/module.h"
#include "vboot/stages.h"

void module_main(void)
{
	if (uefi_prepare_fwdb_storage())
		halt();

	if (uefi_prepare_fwdb_e820_map())
		halt();

	if (vboot_init())
		halt();

	UefiDcModule *rwa = new_uefi_dc_module(board_storage_main_fw_a());
	UefiDcModule *rwb = new_uefi_dc_module(board_storage_main_fw_b());
	if (vboot_select_firmware(&rwa->ops, &rwb->ops))
		halt();

	if (vboot_select_and_load_kernel())
		halt();
}
