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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "base/physmem.h"

uint64_t arch_phys_memset(uint64_t start, int c, uint64_t size)
{
	uint64_t max_addr = (uint64_t)((1ULL << 48) - 1);
	uint64_t end = start + size;
	assert(start <= max_addr);
	assert(end >= start);

	if (end > max_addr)
		size = max_addr - start;

        memset((void *)(uintptr_t)start, c, size);

	return start;
}
