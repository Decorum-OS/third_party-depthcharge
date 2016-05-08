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

#include "arch/x86/preram/util.inc"

.section .entry
.code16
.global _entry
_entry:
	mov $0f, %esp
	jmp preram_real_to_prot32
.code32
0:

	mov $0f, %esp
	jmp preram_board_init
0:

	/* Output post code 0x23. */
	mov $0x23, %eax
	out %al, $0x80

	/* Find the FSP. */

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
	mov_string_const %eax, 'F', 'S', 'P'
	mov_string_const %edx
	mov $0f, %esp
	jmp preram_find_in_dir
0:
	mov %eax, %ecx
	jecxz .Lfile_not_found
	jc .Lfile_not_a_file

	/* A pointer to the FSP is in %eax. */

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
	.asciz "FSP not found.\n"
.Lfile_not_a_directory_msg:
	.asciz "Fould a file when expecting a directory.\n"
.Lfile_not_a_file_msg:
	.asciz "Found a directory when expecting a file.\n"

.Lend_of_module_msg:
	.asciz "Reached the end of the real mode entry point module.\n"

.Lanchor_not_found_msg:
	.asciz "The dcdir anchor was not found in the image.\n"
