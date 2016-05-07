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

#include "arch/x86/gdt.h"

.section .text.preram_real_to_prot32
.code16
.global preram_real_to_prot32
preram_real_to_prot32:
	/* Disable interrupts. */
	cli

	/* Enable protected mode. */
	mov %cr0, %eax
	btsl $0, %eax
	mov %eax, %cr0

	/* Load the GDT pseudo descriptor. */
	addr32 lgdtl %cs:(gdt_ptr - 0xffff0000)

	/* Do a long jump to switch to 32 bit protected mode. */
	ljmpl $gdt_idx_to_sel(GDT_CS32_IDX, 0), $0f

.code32
0:

	/* Load protected mode data segments. */
	mov $gdt_idx_to_sel(GDT_DS32_IDX, 0), %eax
	mov %eax, %ds
	mov %eax, %es
	mov %eax, %fs
	mov %eax, %gs
	mov %eax, %ss

	jmp *%esp
