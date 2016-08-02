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
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include "arch/x86/amd64/gdt.h"

.global boot_x86_linux_start_kernel
.section .text.boot_x86_linux_start_kernel
.align 4

.code64
boot_x86_linux_start_kernel:
	// On entry, %rdi points to the boot parameter structure, and %rsi
	// points to the entry point.
	cli

	// Switch to our own GDT, if we haven't already.
	lgdt gdt_ptr(%rip)

	// Do a long ret to switch to compatibility mode.
	mov $gdt_idx_to_sel(GDT_CS32_IDX, 0), %rax
	push %rax
	lea 0f(%rip), %rax
	push %rax
	lretq
.code32
0:

	// Switch to 32 bit data segments.
	mov $gdt_idx_to_sel(GDT_DS32_IDX, 0), %eax
	mov %eax, %ds
	mov %eax, %es
	mov %eax, %fs
	mov %eax, %gs
	mov %eax, %ss

	// Disable paging to switch from compatibility mode to legacy mode.
	mov %cr0, %eax
	btr $31, %eax
	mov %eax, %cr0

	// Turn of the "long mode enable" bit in the EFER.
	mov $0xc0000080, %ecx
	rdmsr
	btr $8, %eax
	wrmsr

	/* We're now all the way back to legacy protected mode. */

	// We need %esi for other purposes. Move the entry point into %eax.
	mov %esi, %eax
	// %esi needs to point to the boot parameter structure.
	mov %edi, %esi
	// These registers should be zero.
	movl $0, %edi
	movl $0, %ebp
	movl $0, %ebx
	jmp *%eax
