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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "arch/x86/ia32/temp_stack/util.h"
#include "module/fsp/v1_1/board.h"
#include "module/fsp/v1_1/fsp.h"
#include "module/fsp/v1_1/fsp_init.h"
#include "module/fsp/v1_1/fsp_memory_init.h"


void temp_stack_fsp(FspV1_1InformationHeader *header, uintptr_t temp_ram_base,
		    uintptr_t temp_ram_end) __attribute__((noreturn));
void temp_stack_fsp(FspV1_1InformationHeader *header, uintptr_t temp_ram_base,
		    uintptr_t temp_ram_end)
{
	temp_stack_puts("\nPreparing to call FSP memory init.\n");

	uintptr_t vpd_addr = header->image_base + header->cfg_region_offset;
	FspV1_1Vpd *vpd_ptr = (FspV1_1Vpd *)vpd_addr;

	uintptr_t upd_addr = header->image_base +
			     vpd_ptr->pcd_upd_region_offset;

	FspV1_1InitRtCommonBuffer rt_buffer;
	memset(&rt_buffer, 0, sizeof(rt_buffer));
	rt_buffer.stack_top = NULL;
	rt_buffer.boot_mode = FspV1_1BootWithFullConfiguration;
	rt_buffer.upd_data_rgn_ptr = (void *)upd_addr;

	EfiHobPointers hob_list_ptr;

	FspV1_1MemoryInitParams params = {
		.nvs_buffer_ptr = NULL,
		.rt_buffer_ptr = &rt_buffer,
		.hob_list_ptr = (void *)&hob_list_ptr,
	};

	uintptr_t memory_init_addr = header->image_base +
				     header->fsp_memory_init_entry_offset;
	FspV1_1MemoryInit memory_init = (FspV1_1MemoryInit)memory_init_addr;

	uint32_t result = board_fsp_v1_1_memory_init(&params, memory_init);

	temp_stack_puts("FSP memory init returned ");
	if (result != FspSuccess) {
		temp_stack_print_num32(result);
		temp_stack_puts(" (failure).\n");
		halt();
	}
	temp_stack_puts("0 (success).\n");

	uintptr_t new_stack_top = CONFIG_FSP_EARLY_STACK_BASE +
				  CONFIG_FSP_EARLY_STACK_SIZE;

	__asm__ __volatile__(
		"mov %[new_stack], %%esp\n"
		"push %[hob_list_ptr]\n"
		"push %[header]\n"
		"call ram_stack_fsp\n"
	: : [new_stack]"r"(new_stack_top - 8),
	    [header]"r"(header),
	    [hob_list_ptr]"r"(hob_list_ptr.raw)
	: "memory"
	);

	halt();
}
