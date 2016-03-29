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

.text
.code32

/*
 * halt
 * Halt the CPU.
 */
.global halt
halt:
	cli
	hlt
	jmp halt


/*
 * uart_send_char
 * Send a character on the UART, presumed to be at address 0x3fd.
 *
 * Inputs:
 * %bl - The character to send.
 * %esp - The return address.
 *
 * Clobbers:
 * %eax, %edx, %eflags
 */
.global uart_send_char
uart_send_char:
	/* Wait for the last character to be sent. */
	mov $0x3fd, %edx
0:
	in %dx, %al
	bt $0x6, %eax
	jnc 0b

	/* Send the next character. */
	mov %bl, %al
	mov $0x3f8, %edx
	out %al, %dx

	jmp *%esp



/*
 * puts
 * Print a string to the UART.
 *
 * Inputs:
 * %esi - A pointer to the string to print.
 * %esp - The return address.
 *
 * Clobbers:
 * %eax, %ebx, %edx, %eflags (by way of uart_send_char)
 * %ecx
 */
.global puts
puts:
	mov %esp, %ecx

.Lnext_char:
	/* Check to see if we're at the end of the string. */
	cmpb $0, (%esi)
	jne 0f

	/* If we're done, return to the caller. */
	jmp *%ecx

0:
	/* Send the current character. */
	mov $1f, %esp
	mov (%esi), %bl
	jmp uart_send_char
1:

	/* Start again, but one character farther in. */
	inc %esi
	jmp .Lnext_char



/*
 * fatal
 * Print an error message and halt the machine.
 *
 * Inputs:
 * %esi - A pointer to the error message string.
 *
 * Clobbers:
 * We're halting, so it doesn't matter.
 */
.global fatal
fatal:
	mov $halt, %esp
	jmp puts



/*
 * find_dcdir_anchor
 * Search for the dcdir anchor structure. Start at an expected address, and
 * if that fails, scan for it in the rest of the image.
 *
 * The dcdir_anchor_addr symbol points to the anchor structure in the memory
 * mapped image of the ROM. It's computed and injected when the final
 * firmware image is constructed by build_image. rom_image_base is set to the
 * start of the memory mapped ROM image and is set up similarly.
 *
 * Inputs:
 * %esp - The return address.
 *
 * Outputs:
 * %edi - Where the anchor was found, or zero if it wasn't.
 *
 * Clobbers:
 * %eax, %ebx, %ecx, %edx, %esi, %eflags (by way of puts)
 * %ebp, %edi
 */
.global find_dcdir_anchor
find_dcdir_anchor:
	mov %esp, %ebp
	mov $dcdir_anchor_addr, %edi

	mov_string_const %eax, 'D', 'C', ' ', 'D'
	cmp %eax, (%edi)
	jne .Lanchor_not_where_expected
	mov_string_const %eax, 'I', 'R'
	cmp %ax, 4(%edi)
	jne .Lanchor_not_where_expected
	mov %edi, %eax
	sub $rom_image_base, %eax
	cmpl %eax, 8(%edi)
	jne .Lanchor_not_where_expected
	/* The anchor was where we expected. Leave its address in %edi. */
	jmp *%ebp

.Lanchor_not_where_expected:
	mov $.Lanchor_not_where_expected_msg, %esi
	mov $0f, %esp
	jmp puts
0:

	/* Scan for the DCDIR anchor structure. */
	mov_string_const %eax, 'D', 'C', ' ', 'D'
	mov_string_const %edx, 'I', 'R'
	mov $0, %edi

.Lscanning_loop:
	sub $16, %edi
	/* Check to see if we've gone passed the end of the image. */
	cmpl $rom_image_base, %edi
	jge 0f

	/* We're out of spots to look. No anchor was found. */
	mov $0, %edi
	jmp *%ebp

0:
	/* Check whether the anchor magic value is present. */
	cmp %eax, (%edi)
	jne .Lscanning_loop
	cmp %dx, 4(%edi)
	jne .Lscanning_loop

	/* Compute the current offset in the image. */
	mov %edi, %ecx
	sub $rom_image_base, %ecx
	cmp %ecx, 8(%edi)
	jne .Lscanning_loop

	/* The anchor was found! Leave the pointer to it in %edi. */
	jmp *%ebp



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
.global find_in_dir
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

.Lanchor_not_where_expected_msg:
	.asciz "WARNING: The dcdir anchor was not at the expected address.\nScanning...\n"
