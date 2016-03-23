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

/* Hard code anchor location for now. */
.set _dcdir_anchor_addr, 0x100000000 - 0x400000

/*
 * mov_string_const
 * Move up to 4 ASCII characters into a register.
 *
 * reg - The register to store the constant in.
 * a, b, c, d - Characters which form the string being put into the register.
 *              Any characters which aren't specified explicitly are stored
 *              as zero.
 */
.macro mov_string_const reg, a=0, b=0, c=0, d=0
	mov $((\a << 0) | (\b << 8) | (\c << 16) | (\d << 24)), \reg
.endm


.align 16

/*
 * The entry point, which starts in 16 bit real mode with standard but odd
 * segment configuration. The base of most segments is 0, but the base of the
 * code segment, cs, is effectively 0xffff0000. All limits are set to 0xffff.
 *
 * This code is presumably executing from ROM and shouldn't write locally. It
 * can write to RAM once that's going.
 */
.section .entry
.code16
	/* Disable interrupts. */
	cli

	/* Enable protected mode. */
	mov %cr0, %eax
	btsl $0, %eax
	mov %eax, %cr0

	/* Load the GDT pseudo descriptor. */
	addr32 lgdtl %cs:(.Lgdt_ptr - 0xffff0000)

	/* Do a long jump to switch to 32 bit protected mode. */
	ljmpl $0x8, $0f

.code32
0:

	/* Load protected mode data segments. */
	mov $0x10, %eax
	mov %eax, %ds
	mov %eax, %es
	mov %eax, %fs
	mov %eax, %gs
	mov %eax, %ss

	/* Look up the next stage of firmware to run in memory. */

	/* Load the dcdir root base into esi. */
	mov _dcdir_anchor_addr + 12, %esi
	mov $_dcdir_anchor_addr + 16, %edi
	mov_string_const %eax, 'R', 'O'
	mov_string_const %edx
	mov $0f, %esp
	jmp find_in_dir
0:
	/* "RO" should be a directory. */
	jnc halt
	/* Make sure something was found. */
	cmp $0, %eax
	je halt

	mov %eax, %edi
	mov_string_const %eax, 'F', 'I', 'R', 'M'
	mov_string_const %edx, 'W', 'A', 'R', 'E'
	mov $0f, %esp
	jmp find_in_dir
0:
	jnc halt
	cmp $0, %eax
	je halt

	mov %eax, %edi
	mov_string_const %eax, 'F', 'W', ' ', 'S'
	mov_string_const %edx, 'E', 'L'
	mov $0f, %esp
	jmp find_in_dir
0:
	/* The region should be a file this time. */
	jc halt
	cmp $0, %eax
	je halt

halt:
	hlt
	jmp halt



/*
 * find_in_dir
 * Search a dcdir directory for an entry with a particular name.
 *
 * Inputs:
 * %eax - First part of the name to search for.
 * %edx - Second part of the name to search for.
 * %esi - Base value.
 * %edi - Pointer to Start of directory table.
 * %esp - Return address.
 *
 * Outputs:
 * %eax - Zero if the child wasn't found, or a pointer to it if it was.
 * %edx - The new region's size in bytes.
 * %esi - The new base value.
 * carry flag - The directory bit.
 *
 * Clobbers:
 * %ecx, %ebx, %eflags
 */
find_in_dir:

	/* Make %esi point to the start of the region. */
	sub %edi, %esi
	neg %esi

	/* Load the size of the table into %ecx. */
	mov 4(%edi), %ecx
	shr $8, %ecx
	inc %ecx
	shl $3, %ecx

	/* Move past the header and to the actual entries. */
	add $8, %edi
	sub $8, %ecx
	jecxz .Lnot_found

.Lnext_pointer:
	/* Compare the name to what we're searching for. */
	cmp (%edi), %eax
	jne .Lnot_yet
	cmp 4(%edi), %edx
	jne .Lnot_yet

	/* Ok, we found one with a matching name. Extract the pointer. */
	mov 8(%edi), %al
	shr $1, %al
	cmp $0x1, %al
	jne .Lnot_type_1
	/* The pointer is type 1. */

	/* Move the offset into %eax. */
	movzbl 12(%edi), %eax
	shl $8, %eax
	mov 11(%edi), %al
	shl $8, %eax
	mov 10(%edi), %al
	/* Add the base pointer to it. */
	add %esi, %eax

	/* The new base value for this type is always zero. */
	mov $0, %esi

	/* Load the size into %edx. */
	movzbl 15(%edi), %edx
	shl $8, %edx
	mov 14(%edi), %dl
	shl $8, %edx
	mov 13(%edi), %dl
	inc %edx

	/* Figure out if this was a directory. */
	bt $0, 8(%edi)

	/* Return our answer. */
	jmp *%esp

.Lnot_type_1:
	cmp $0x2, %al
	jne .Lnot_type_2
	/* The pointer is type 2. */

	/* Move the offset into %eax. */
	mov 16(%edi), %eax
	/* Add the base pointer to it. */
	add %esi, %eax

	/* Load the new base value into %esi. */
	mov 12(%edi), %esi

	/* Load the size into %edx. */
	mov 20(%edi), %edx
	inc %edx

	/* Figure out if this was a directory. */
	bt $0, 8(%edi)

	/* Return our answer. */
	jmp *%esp

.Lnot_type_2:
	/* We're out of types we understand. */

.Lnot_yet:
	/* Compute the size of the pointer in %ebx. */
	movzbl 9(%edi), %ebx
	inc %ebx
	shl $3, %ebx
	/* Account for the label size. */
	add $8, %ebx
	/* Adjust the pointer and the running size. */
	add %ebx, %edi
	sub %ebx, %ecx

	/* Check if we're out of pointers. */
	jecxz .Lnot_found
	jmp .Lnext_pointer

.Lnot_found:
	mov $0, %eax
	jmp *%esp



.data
.align 8
.Lgdt_ptr:
	.short .Lgdt_end - .Lgdt - 1
	.int .Lgdt

.align 8
.Lgdt:
	.int 0
	.int 0

	/* 32 bit CPL 0 code segment */
	.short 0xffff       /* limit 15:00 */
	.short 0x0000       /* base 15:00 */
	.byte  0x00         /* base 23:16 */
	.byte  0b10011010   /* P(1) DPL(00) S(1) 1 C(0) R(1) A(0) */
	.byte  0b11001111   /* G(1) D(1) 0 0 limit 19:16 */
	.byte  0x0          /* base 31:24 */

	/* 32 bit CPL 0 data segment */
	.short 0xffff       /* limit 15:00 */
	.short 0x0000       /* base 15:00 */
	.byte  0x00         /* base 23:16 */
	.byte  0b10010010   /* P(1) DPL(00) S(1) 0 E(0) W(1) A(0) */
	.byte  0b11001111   /* G(1) B(1) 0 0 limit 19:16 */
	.byte  0x0          /* base 31:24 */
.Lgdt_end:
