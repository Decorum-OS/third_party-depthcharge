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

#include "arch/x86/ia32/preram/util.inc"

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
.global _entry
_entry:
	mov $0f, %esp
	jmp preram_real_to_prot32
.code32
0:

	/* Look up the next stage of firmware to run in memory. */

	mov $0f, %esp
	jmp preram_find_dcdir_anchor
0:
	cmp $0, %edi
	jne 0f

	mov $.Lanchor_not_found_msg, %esi
	jmp preram_fatal

0:
	/*
	 * %edi should currently point to the location of the anchor
	 * structure. Load the root directory base into %esi, and put the
	 * address of the root directory table immediately after the anchor
	 * in %edi.
	 */
	mov 12(%edi), %esi
	add $16, %edi

	mov_string_const %eax, 'R', 'O'
	mov_string_const %edx
	mov $0f, %esp
	jmp preram_find_in_dir
0:
	mov %eax, %ecx
	jecxz .Lfile_not_found
	jnc .Lfile_not_a_directory

	mov %eax, %edi
	mov_string_const %eax, 'F', 'I', 'R', 'M'
	mov_string_const %edx, 'W', 'A', 'R', 'E'
	mov $0f, %esp
	jmp preram_find_in_dir
0:
	mov %eax, %ecx
	jecxz .Lfile_not_found
	jnc .Lfile_not_a_directory

	mov %eax, %edi
	mov_string_const %eax, 'F', 'W', ' ', 'S'
	mov_string_const %edx, 'E', 'L'
	mov $0f, %esp
	jmp preram_find_in_dir
0:
	mov %eax, %ecx
	jecxz .Lfile_not_found
	jc .Lfile_not_a_file

	/* A pointer to the ELF is already in %eax. */
	jmp preram_start_elf_module

	/* We should never get to the end of the module. */
	mov $.Lend_of_module_msg, %esi
	jmp preram_fatal

.Lfile_not_found:
	mov $.Lfile_not_found_msg, %esi
	jmp preram_fatal

.Lfile_not_a_directory:
	mov $.Lfile_not_a_directory_msg, %esi
	jmp preram_fatal

.Lfile_not_a_file:
	mov $.Lfile_not_a_file_msg, %esi
	jmp preram_fatal

.Lfile_not_found_msg:
	.asciz "Firmware select module not found.\n"
.Lfile_not_a_directory_msg:
	.asciz "Fould a file when expecting a directory.\n"
.Lfile_not_a_file_msg:
	.asciz "Found a directory when expecting a file.\n"

.Lend_of_module_msg:
	.asciz "Reached the end of the entry module.\n"

.Lanchor_not_found_msg:
	.asciz "The dcdir anchor was not found in the image.\n"
