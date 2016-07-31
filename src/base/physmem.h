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

#ifndef __BASE_PHYSMEM_H__
#define __BASE_PHYSMEM_H__

#include <stdint.h>

/*
 * These functions work like memset but operate on physical memory which may
 * not be accessible directly.
 *
 * @param s	The physical address to start setting memory at.
 * @param c	The character to set each byte of the region to.
 * @param n	The number of bytes to set.
 *
 * @return	The physical address of the memory which was set.
 */
uint64_t arch_phys_memset(uint64_t s, int c, uint64_t n);

enum {
	E820MemRange_Ram = 1,
	E820MemRange_Reserved = 2,
	E820MemRange_Acpi = 3,
	E820MemRange_Nvs = 4,
	E820MemRange_Unusable = 5,
};

typedef struct {
	uint64_t base;
	uint64_t size;
	uint32_t type;
	uint32_t handoff_tag;
} E820MemRange;

typedef struct {
	uint64_t num_ranges;
	E820MemRange ranges[CONFIG_MAX_MEM_RANGES];
} E820MemRanges;

int prepare_e820_mem_ranges(void);
E820MemRanges *get_e820_mem_ranges(void);

#endif /* __BASE_PHYSMEM_H__ */
