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

#include "arch/x86/gdt.h"

union
{
	struct __attribute__((packed))
	{
		uint64_t limit_low : 16;
		uint64_t base_low : 24;
		uint64_t a : 1; // Accessed.
		uint64_t rw : 1; // Read or write, depending on type.
		uint64_t ce : 1; // Conforming or expand-down,
				 // depending on type.
		uint64_t cd : 1; // Code or data.
		uint64_t s : 1; // System (active low).
		uint64_t dpl : 2; // Descriptor priviledge level.
		uint64_t p : 1; // Present.
		uint64_t limit_high : 4;
		uint64_t avl : 1; // Available for software use.
		uint64_t l : 1; // Long mode.
		uint64_t bd : 1; // Default operand size.
		uint64_t g : 1; // Granularity.
		uint64_t base_high : 8;
	} fields;
	uint64_t raw;
} gdt_gdt[] = {
	[0] = {
		.raw = 0
	},
	[GdtCs32Index] = {
		.fields = {
			.limit_low = 0xffff,
			.limit_high = 0xf,
			.base_low = 0x0,
			.base_high = 0x0,
			.a = 1,
			.rw = 1,
			.ce = 0,
			.cd = 1,
			.s = 1,
			.dpl = 0,
			.p = 1,
			.avl = 0,
			.l = 0,
			.bd = 1,
			.g = 1
		}
	},
	[GdtDs32Index] = {
		.fields = {
			.limit_low = 0xffff,
			.limit_high = 0xf,
			.base_low = 0x0,
			.base_high = 0x0,
			.a = 1,
			.rw = 1,
			.ce = 0,
			.cd = 0,
			.s = 1,
			.dpl = 0,
			.p = 1,
			.avl = 0,
			.l = 0,
			.bd = 1,
			.g = 1
		}
	},
};

struct __attribute__((packed))
{
	uint16_t length;
	uint32_t address;
} gdt_ptr = {
	.length = sizeof(gdt_gdt) - 1,
	.address = (uintptr_t)&gdt_gdt,
};
