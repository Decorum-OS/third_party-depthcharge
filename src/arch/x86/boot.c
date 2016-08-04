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
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <arch/msr.h>
#include <assert.h>
#include <stdio.h>

#include "arch/x86/boot.h"
#include "base/algorithm.h"
#include "base/cleanup.h"
#include "base/physmem.h"
#include "base/timestamp.h"
#include "vboot/boot.h"

static void * const ParamsBuff = (void *)(uintptr_t)0x1000;
static void * const CmdLineBuff = (void *)(uintptr_t)0x2000;

static const uint32_t KernelV2Magic = 0x53726448;
static const uint16_t MinProtocol = 0x0202;

int boot_x86_linux(struct boot_params *boot_params, char *cmd_line, void *entry)
{
	// Move the boot_params structure and the command line to where Linux
	// suggests and to where they'll be safe from being trampled by the
	// kernel as it's decompressed.
	assert((uint8_t *)CmdLineBuff - (uint8_t *)ParamsBuff >=
		sizeof(*boot_params));

	struct setup_header *hdr = &boot_params->hdr;

	if (hdr->header != KernelV2Magic || hdr->version < MinProtocol) {
		printf("Boot protocol is too old.\n");
		return 1;
	}

	E820MemRanges *e820 = get_e820_mem_ranges();
	if (!e820)
		return 1;

	unsigned num_entries = MIN(e820->num_ranges,
				   ARRAY_SIZE(boot_params->e820_map));
	if (num_entries < e820->num_ranges) {
		printf("Warning: Limiting e820 map to %d entries.\n",
			num_entries);
	}
	boot_params->e820_entries = num_entries;

	for (int i = 0; i < num_entries; i++) {
		E820MemRange *range = &e820->ranges[i];
		struct e820entry *entry = &boot_params->e820_map[i];

		entry->addr = range->base;
		entry->size = range->size;
		entry->type = range->type;
	}

	// Loader type is undefined.
	hdr->type_of_loader = 0xFF;

	// Don't reload the data/code segments.
	hdr->loadflags |= KEEP_SEGMENTS;

	hdr->cmd_line_ptr = (uintptr_t)CmdLineBuff;

	cleanup_trigger(CleanupOnHandoff);

	memcpy(ParamsBuff, boot_params, sizeof(*boot_params));
	strcpy(CmdLineBuff, cmd_line);

	puts("\nStarting kernel ...\n\n");
	timestamp_add_now(TS_START_KERNEL);

	boot_x86_linux_start_kernel(ParamsBuff, entry);
}
