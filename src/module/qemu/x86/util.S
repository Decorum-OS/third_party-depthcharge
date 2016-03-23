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
	/* Check to see if we're at the end of the string. */
	cmpb $0, (%esi)

	/* If we are, then halt. */
	je halt

	/* We're not, so we need to send a character. */
	mov $0f, %esp
	mov (%esi), %bl
	jmp uart_send_char
0:

	/* Start again, but one character farther in. */
	inc %esi
	jmp fatal



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
	mov $0, %eax
	mov 12(%edi), %al
	shl $8, %eax
	mov 11(%edi), %al
	shl $8, %eax
	mov 10(%edi), %al
	/* Add the base pointer to it. */
	add %esi, %eax

	/* The new base value for this type is always zero. */
	mov $0, %esi

	/* Load the size into %edx. */
	mov $0, %edx
	mov 15(%edi), %dl
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
	mov $0, %ebx
	mov 9(%edi), %bl
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
