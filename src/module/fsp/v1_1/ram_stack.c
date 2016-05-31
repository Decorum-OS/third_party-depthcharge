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
 * but without any warranty; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdlib.h>

#include "arch/x86/temp_stack/util.h"
#include "base/dcdir_structs.h"
#include "module/fsp/fsp.h"
#include "module/fsp/temp_stack.h"
#include "module/fsp/v1_1/fsp.h"

// This should probably be moved into a kconfig setting.
static const int debug_print_hob_list = 0;

void ram_stack_fsp(FspV1_1InformationHeader *header,
		   EfiHobPointers hob_list_ptr)
{
	if (debug_print_hob_list)
		temp_stack_print_hob_list(hob_list_ptr);

	DcDirAnchor *anchor = temp_stack_find_dcdir_anchor();
	if (!anchor)
		halt();
	void *root = (uint8_t *)anchor + sizeof(DcDirAnchor);
	uint32_t base = anchor->root_base;

	void *ro = temp_stack_find_dir_in_dir(
		NULL, &base, "RO", base, root);
	if (!ro) {
		temp_stack_puts("/RO not found.\n");
		halt();
	}

	void *firmware = temp_stack_find_dir_in_dir(
		NULL, &base, "FIRMWARE", base, ro);
	if (!firmware) {
		temp_stack_puts("/RO/FIRMWARE not found.\n");
		halt();
	}

	uint32_t size;
	void *fw_sel = temp_stack_find_region_in_dir(
		&size, &base, "FW SEL", base, firmware);
	if (!fw_sel) {
		temp_stack_puts("/RO/FIRMWARE/FW SEL not found.\n");
		halt();
	}

	halt();
}
