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

#include "module/fsp/temp_stack.h"
#include "module/fsp/v1_1/board.h"
#include "module/fsp/v1_1/fsp.h"
#include "module/fsp/v1_1/fsp_init.h"
#include "module/fsp/v1_1/fsp_memory_init.h"

static void print_num(uint32_t num)
{
	for (int digit_idx = sizeof(num) * 2; digit_idx; digit_idx--) {
		uint32_t digit = (num >> ((digit_idx - 1) * 4)) & 0xf;
		uint32_t character = (digit < 10) ? ('0' + digit) :
						    ('a' + digit - 10);
		__asm__ __volatile__(
			"push %%ebx\n"
			"push %%eax\n"
			"push %%edx\n"
			"push %%ebp\n"
			"pushf\n"
			"mov %0, %%ebx\n"
			"mov %%esp, %%ebp\n"
			"mov $0f, %%esp\n"
			"jmp preram_uart_send_char\n"
			"0:\n"
			"mov %%ebp, %%esp\n"
			"popf\n"
			"pop %%ebp\n"
			"pop %%edx\n"
			"pop %%eax\n"
			"pop %%ebx\n"
		: : "r" (character)
		);
	}
}

void temp_stack_fsp(FspV1_1InformationHeader *header, uintptr_t temp_ram_base,
		    uintptr_t temp_ram_end) __attribute__((noreturn));
void temp_stack_fsp(FspV1_1InformationHeader *header, uintptr_t temp_ram_base,
		    uintptr_t temp_ram_end)
{
	temp_stack_puts("Preparing to call FSP memory init.\n");

	uintptr_t vpd_addr = header->image_base + header->cfg_region_offset;
	FspV1_1Vpd *vpd_ptr = (FspV1_1Vpd *)vpd_addr;

	uintptr_t upd_addr = header->image_base +
			     vpd_ptr->pcd_upd_region_offset;

	FspV1_1InitRtCommonBuffer rt_buffer;
	memset(&rt_buffer, 0, sizeof(rt_buffer));
	rt_buffer.stack_top = NULL;
	rt_buffer.boot_mode = FspV1_1BootWithFullConfiguration;
	rt_buffer.upd_data_rgn_ptr = (void *)upd_addr;

	void *hob_list_ptr;

	FspV1_1MemoryInitParams params = {
		.nvs_buffer_ptr = NULL,
		.rt_buffer_ptr = &rt_buffer,
		.hob_list_ptr = &hob_list_ptr,
	};

	uintptr_t memory_init_addr = header->image_base +
				     header->fsp_memory_init_entry_offset;
	FspV1_1MemoryInit memory_init = (FspV1_1MemoryInit)memory_init_addr;

	uint32_t result = board_fsp_v1_1_memory_init(&params, memory_init);

	temp_stack_puts("FSP memory init returned ");
	if (result != FspSuccess) {
		print_num(result);
		temp_stack_puts(" (failure).\n");
		halt();
	}
	temp_stack_puts("0 (success).\n");

	halt();
}
