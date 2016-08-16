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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/x86/ia32/temp_stack/util.h"
#include "base/dcdir_structs.h"
#include "drivers/board/samus/fsp.h"
#include "drivers/gpio/lynxpoint_lp.h"
#include "module/fsp/v1_1/board.h"

static uint8_t dq_byte_map[24] = {
	0x0F, 0xF0,
	0x00, 0xF0,
	0x0F, 0xF0,
	0x0F, 0x00,
	0xFF, 0x00,
	0xFF, 0x00,

	0x0F, 0xF0,
	0x00, 0xF0,
	0x0F, 0xF0,
	0x0F, 0x00,
	0xFF, 0x00,
	0xFF, 0x00,
};

static const uint8_t dqs_map_cpu_2_dram[16] = {
	2, 0, 1, 3, 6, 4, 7, 5,
	2, 1, 0, 3, 6, 5, 4, 7,
};

static int read_spd_idx(void)
{
	uint16_t gpio_base = lp_pch_gpio_base_uncached();
	LpPchGpio spd_0, spd_1, spd_2, spd_3;

	init_lp_pch_gpio_input(&spd_0, 69);
	lp_pch_gpio_set_addr(&spd_0, gpio_base);
	spd_0.use(&spd_0, 1);

	init_lp_pch_gpio_input(&spd_1, 68);
	lp_pch_gpio_set_addr(&spd_1, gpio_base);
	spd_1.use(&spd_1, 1);

	init_lp_pch_gpio_input(&spd_2, 67);
	lp_pch_gpio_set_addr(&spd_2, gpio_base);
	spd_2.use(&spd_2, 1);

	init_lp_pch_gpio_input(&spd_3, 65);
	lp_pch_gpio_set_addr(&spd_3, gpio_base);
	spd_3.use(&spd_3, 1);

	int spd_bit_0 = gpio_get(&spd_0.ops);
	int spd_bit_1 = gpio_get(&spd_1.ops);
	int spd_bit_2 = gpio_get(&spd_2.ops);
	int spd_bit_3 = gpio_get(&spd_3.ops);

	if (spd_bit_0 == -1 || spd_bit_1 == -1 ||
	    spd_bit_2 == -1 || spd_bit_2 == -1) {
		return -1;
	}

	return (spd_bit_0 << 0) | (spd_bit_1 << 1) |
	       (spd_bit_2 << 2) | (spd_bit_3 << 3);
}

uint32_t board_fsp_v1_1_memory_init(FspV1_1MemoryInitParams *params,
				    FspV1_1MemoryInit memory_init_func)
{
	// Replace the generic runtime buffer with the broadwell version.
	// These happen to be the same, but we'll still do it as an example
	// for other CPUs.
	BroadwellFspInitRtBuffer rt_buffer;
	memcpy(&rt_buffer.common, params->rt_buffer_ptr,
	       sizeof(rt_buffer.common));
	params->rt_buffer_ptr = &rt_buffer;

	// Copy the upd out, and point to the copy.
	BroadwellUpd upd;
	memcpy(&upd, rt_buffer.common.upd_data_rgn_ptr, sizeof(upd));
	rt_buffer.common.upd_data_rgn_ptr = &upd;

	// Figure out what SPD we're strapped for.
	int spd_idx = read_spd_idx();
	if (spd_idx < 0) {
		temp_stack_puts("Error reading SPD index strapping.\n");
	} else {
		temp_stack_puts("spd_idx = ");
		temp_stack_print_num16(spd_idx);
		temp_stack_puts("\n");
	}

	// Find that SPD in the image.
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

	void *spd_dir = temp_stack_find_dir_in_dir(
		NULL, &base, "SPD", base, ro);
	if (!spd_dir) {
		temp_stack_puts("/RO/SPD not found.\n");
		halt();
	}

	char spd_name[9] = { [sizeof(spd_name) - 1] = '\0' };
	snprintf(spd_name, sizeof(spd_name) - 1, "%x", spd_idx);
	void *spd = temp_stack_find_region_in_dir(
		NULL, &base, spd_name, base, spd_dir);
	if (!spd) {
		temp_stack_puts("RO/SPD/");
		temp_stack_puts(spd_name);
		temp_stack_puts(" not found.\n");
	}

	// Update the upd for samus.
	upd.spd_data_buffer_0_0 = (uintptr_t)spd;
	upd.spd_data_buffer_1_0 = (uintptr_t)spd;

	memcpy(upd.dq_byte_map, dq_byte_map, sizeof(upd.dq_byte_map));
	memcpy(upd.dqs_map_cpu_2_dram, dqs_map_cpu_2_dram,
	       sizeof(upd.dqs_map_cpu_2_dram));
	upd.dq_dqs_data_effective = 1;

	return memory_init_func(params);
}

uint32_t board_fsp_v1_1_temp_ram_exit(FspV1_1TempRamExit temp_ram_exit_func)
{
	return temp_ram_exit_func(NULL);
}
