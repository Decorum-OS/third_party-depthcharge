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
#include "module/qemu/x86/elf.inc"

.text
.code32

/*
 * start_elf_module
 * Load an ELF formatted module and start executing it.
 *
 * Inputs:
 * %eax - Pointer to the ELF module in memory/ROM.
 *
 * Clobbers:
 * Doesn't return.
 */
.global start_elf_module
start_elf_module:
	/* Some intructions expect to use %eax, so lets switch to %ebp. */
	mov %eax, %ebp

	/* Make sure what we're looking at is actually an ELF. */
	mov_string_const %ebx, ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3
	cmp %ebx, (E_IDENT + EI_MAG0)(%ebp)
	jne .Lbad_magic_error

	cmpb $ELFCLASS32, (E_IDENT + EI_CLASS)(%ebp)
	jne .Lnot_32_error

	/* Get the offset, size, and count of segments from the header. */
	mov E_PHOFF(%ebp), %edx
	mov $0, %ecx
	mov E_PHNUM(%ebp), %cx

	/* Convert edx from an offset to a pointer to the first header. */
	add %ebp, %edx
	/* Pre-subtract one header size to simplify the loop below. */
	movzwl E_PHENTSIZE(%ebp), %ebx
	sub %ebx, %edx

.Lheader_loop_start:
	/* Check to see if we've gone through all the headers. */
	jecxz .Lheader_loop_end

	/* Advance the counters. */
	dec %ecx
	movzwl E_PHENTSIZE(%ebp), %ebx
	add %ebx, %edx

	/* If this isn't a loadable segment, go to the next one. */
	cmpl $PT_LOAD, P_TYPE(%edx)
	jne .Lheader_loop_start

	/* Put the source and destination addresses in %esi and %edi. */
	mov P_PADDR(%edx), %edi
	mov P_OFFSET(%edx), %esi
	add %ebp, %esi

	/* Tuck %ecx into %ebx so we can use it for string instructions. */
	mov %ecx, %ebx

	/* Copy over the bytes which are actually stored in the ELF. */
	mov P_FILESZ(%edx), %ecx
	cld
	rep movsb

	/* Now zero the rest of the segment. */
	mov P_MEMSZ(%edx), %ecx
	sub P_FILESZ(%edx), %ecx
	cld
	mov $0, %eax
	rep stosb

	/* Restore %ecx. */
	mov %ebx, %ecx

	/* Start the next iteration. */
	jmp .Lheader_loop_start

.Lheader_loop_end:

	/* Now that the ELF is loaded, put its entry point into %eax. */
	mov E_ENTRY(%ebp), %eax

	/* Jump to the newly loaded ELF image! */
	jmp *%eax


.Lbad_magic_error:
	mov $.Lbad_magic_error_msg, %esi
	jmp fatal

.Lnot_32_error:
	mov $.Lnot_32_error_msg, %esi
	jmp fatal

.data
.Lbad_magic_error_msg:
	.asciz "Bad magic value in ELF module.\n"
.Lnot_32_error_msg:
	.asciz "Only 32 bit ELF files are currently supported.\n"
