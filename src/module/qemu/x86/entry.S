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

#include "module/qemu/x86/util.inc"

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

	/*
	 * The dcdir_anchor_addr symbol points to the anchor structure in
	 * the memory mapped image of the ROM. It's computed and injected
	 * when the final firmware image is constructed by build_image.
	 * Use it to load the root directory base into %esi, and put the
	 * address of the root directory table immediately after the anchor
	 * in %edi.
	 */
	mov dcdir_anchor_addr + 12, %esi
	mov $dcdir_anchor_addr + 16, %edi
	mov_string_const %eax, 'R', 'O'
	mov_string_const %edx
	mov $0f, %esp
	jmp find_in_dir
0:
	mov %eax, %ecx
	jecxz .Lfile_not_found
	jnc .Lfile_not_a_directory

	mov %eax, %edi
	mov_string_const %eax, 'F', 'I', 'R', 'M'
	mov_string_const %edx, 'W', 'A', 'R', 'E'
	mov $0f, %esp
	jmp find_in_dir
0:
	mov %eax, %ecx
	jecxz .Lfile_not_found
	jnc .Lfile_not_a_directory

	mov %eax, %edi
	mov_string_const %eax, 'F', 'W', ' ', 'S'
	mov_string_const %edx, 'E', 'L'
	mov $0f, %esp
	jmp find_in_dir
0:
	mov %eax, %ecx
	jecxz .Lfile_not_found
	jc .Lfile_not_a_file

	/* A pointer to the ELF is already in %eax. */
	jmp start_elf_module

	/* We should never get to the end of the module. */
	mov $.Lend_of_module_msg, %esi
	jmp fatal

.Lfile_not_found:
	mov $.Lfile_not_found_msg, %esi
	jmp fatal

.Lfile_not_a_directory:
	mov $.Lfile_not_a_directory_msg, %esi
	jmp fatal

.Lfile_not_a_file:
	mov $.Lfile_not_a_file_msg, %esi
	jmp fatal


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

.Lfile_not_found_msg:
	.asciz "Firmware select module not found.\n"
.Lfile_not_a_directory_msg:
	.asciz "Fould a file when expecting a directory.\n"
.Lfile_not_a_file_msg:
	.asciz "Found a directory when expecting a file.\n"

.Lend_of_module_msg:
	.asciz "Reached the end of the entry module.\n"
