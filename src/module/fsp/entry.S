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

	jmp .Lrun_temp_ram_init

.Lfile_not_found:
	mov $.Lfile_not_found_msg, %esi
	jmp preram_fatal
.Lfile_not_found_msg:
	.asciz "FSP not found.\n"

.Lfile_not_a_directory:
	mov $.Lfile_not_a_directory_msg, %esi
	jmp preram_fatal
.Lfile_not_a_directory_msg:
	.asciz "Fould a file when expecting a directory.\n"

.Lfile_not_a_file:
	mov $.Lfile_not_a_file_msg, %esi
	jmp preram_fatal
.Lfile_not_a_file_msg:
	.asciz "Found a directory when expecting a file.\n"

.Lanchor_not_found_msg:
	.asciz "The dcdir anchor was not found in the image.\n"


.Lrun_temp_ram_init:
	/*
	 * A pointer to the FSP is in %eax. Compute the location of the
	 * TempRamInit entry point.
	 */

	mov %eax, %esp
	mov %eax, %ebp
	/* Check that the Firmware Volume header signature is "_FVH". */
	mov 40(%esp), %eax
	cmp $0x4856465F, %eax
	je 0f
	mov $.Lbad_firmware_volume_header_signature_msg, %esi
	jmp preram_fatal

0:
	/* Move past the standard firmware volume header. */
	movzwl 52(%esp), %eax /* ExtHeaderOffset */
	add %eax, %esp
	/* And then the extended one. */
	mov 16(%esp), %eax /* ExtHeaderSize */
	add %eax, %esp
	/* Finally, align to an 8 byte boundary, relative to the start. */
	mov %ebp, %eax
	and $7, %eax
	sub %eax, %esp
	add $7, %esp
	and $~7, %esp
	add %eax, %esp

	/* Check the FFS "file header" GUID. It starts at offset 0. */
	cmpl $0x912740BE, (%esp)
	jne .Lbad_ffs_guid
	cmpl $0x47342284, 4(%esp)
	jne .Lbad_ffs_guid
	cmpl $0xB08471B9, 8(%esp)
	jne .Lbad_ffs_guid
	cmpl $0x0C3F3527, 12(%esp)
	jne .Lbad_ffs_guid

	/* Move past the header FFS header. */
	add $24, %esp

	/* Verify that the first "section" in the file is type RAW. */
	cmpb $0x19, 3(%esp)
	je 0f
	mov $.Lbad_section_type_msg, %esi
	jmp preram_fatal

0:
	/* Move past the section header */
	add $4, %esp

	/* We're finally at the start of the FSP information header! */

	/* Verify the FSP signature, which should be "FSPH". */
	cmpl $0x48505346, (%esp)
	je 0f
	mov $.Lbad_fsp_signature_msg, %esi
	jmp preram_fatal

0:
	/*
	 * Make sure that the FSP was actually configured to run where it is
	 * right now, in case it was placed incorrectly or moved somehow.
	 */
	cmpl %ebp, 28(%esp)
	je 0f
	mov $.Lfsp_in_the_wrong_place_msg, %esi
	jmp preram_fatal

0:
	/*XXX
	 *XXX Verify that this FSP is for this CPU. Skip for now.
	 *XXX*/

	/* Add the TempRamInitEntry offset to the start of the FSP. */
	mov 48(%esp), %eax
	add %ebp, %eax

	/* Set up the fake stack frame. */
	lea .Lfsp_temp_ram_init_stack_frame, %esp

	/* Finally, actually jump into the FSP. */
	jmp *%eax


.Lfsp_temp_ram_init_params:
	.int microcode_addr
	.int microcode_size
	.int 0xffffffff - 0x200000 + 1 /* code region base */
	.int 0x200000 /* code region size */

.Lfsp_temp_ram_init_stack_frame:
	.int .Lfsp_temp_ram_init_complete
	.int .Lfsp_temp_ram_init_params


.Lfsp_temp_ram_init_complete:

	/* We should never get to the end of the module. */
	mov $.Lend_of_module_msg, %esi
	jmp preram_fatal

.Lend_of_module_msg:
	.asciz "Reached the end of the real mode entry point module.\n"

.Lbad_firmware_volume_header_signature_msg:
	.asciz "Bad signature in the FSP firmware volume header.\n"

.Lbad_ffs_guid:
	mov $.Lbad_ffs_guid_msg, %esi
	jmp preram_fatal
.Lbad_ffs_guid_msg:
	.asciz "Bad GUID in the FSP FFS file header.\n"

.Lbad_section_type_msg:
	.asciz "Bad section type in the FSP raw section.\n"

.Lbad_fsp_signature_msg:
	.asciz "Bad FSP signature.\n"

.Lfsp_in_the_wrong_place_msg:
	.asciz "The FSP wasn't configured to run from its current address.\n"
