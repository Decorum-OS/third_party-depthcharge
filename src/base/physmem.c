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

#include "base/fwdb.h"
#include "base/physmem.h"

static const char e820_fwdb_name[] = "e820 memory ranges";

int prepare_e820_mem_ranges(void)
{
	FwdbEntry entry = {
		.size = sizeof(E820MemRanges),
		.ptr = NULL,
	};

	return fwdb_access(e820_fwdb_name, NULL, &entry);
}

E820MemRanges *get_e820_mem_ranges(void)
{
	static E820MemRanges *ranges;
	if (ranges)
		return ranges;

	FwdbEntry entry;
	if (fwdb_access(e820_fwdb_name, &entry, NULL)) {
		printf("E820 ranges not found in the fwdb.\n");
		return NULL;
	}

	if (entry.size != sizeof(E820MemRanges)) {
		printf("E820 ranges not the expected size.\n");
		return NULL;
	}

	ranges = entry.ptr;
	return ranges;
}
