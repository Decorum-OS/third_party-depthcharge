/*
 * Copyright 2012 Google Inc.
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

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "base/ranges.h"
#include "base/physmem.h"
#include "module/symbols.h"
#include "vboot/util/memory.h"

static Ranges used;

static void used_list_initialize(void)
{
	static int initialized;
	if (initialized)
		return;

	ranges_init(&used);

	// Add regions depthcharge occupies.
	ranges_add(&used, (uintptr_t)&_start, (uintptr_t)&_end);

	initialized = 1;
}

void memory_mark_used(uint64_t start, uint64_t end)
{
	used_list_initialize();
	ranges_add(&used, start, end);
}


static void unused_memset(uint64_t start, uint64_t end, void *data)
{
	printf("\t[%#016"PRIx64", %#016"PRIx64")\n", start, end);
	arch_phys_memset(start, 0, end - start);
}

static void remove_range(uint64_t start, uint64_t end, void *data)
{
	ranges_sub((Ranges *)data, start, end);
}

int memory_wipe_unused(void)
{
	Ranges ranges;

	// Process the memory map from coreboot.
	ranges_init(&ranges);
	E820MemRanges *e820 = get_e820_mem_ranges();
	if (!e820)
		return 1;
	for (int i = 0; i < e820->num_ranges; i++) {
		E820MemRange *range = &e820->ranges[i];
		uint64_t start = range->base;
		uint64_t end = range->base + range->size;
		switch (range->type) {
		case E820MemRange_Ram:
			ranges_add(&ranges, start, end);
			break;
		case E820MemRange_Reserved:
		case E820MemRange_Acpi:
		case E820MemRange_Nvs:
		case E820MemRange_Unusable:
			ranges_sub(&ranges, start, end);
			break;
		default:
			printf("Unrecognized memory type %d!\n", range->type);
			return 1;
		}
	}

	// Exclude memory that's being used.
	used_list_initialize();
	ranges_for_each(&used, &remove_range, &ranges);

	// Do the wipe.
	printf("Wipe memory regions:\n");
	ranges_for_each(&ranges, &unused_memset, NULL);
	ranges_teardown(&ranges);
	return 0;
}
