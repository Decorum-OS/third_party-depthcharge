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
output_loop:
	mov $0x23, %eax
	out %al, $0x80

	/* Wait for the last character to be transmitted. */
0:
	mov $0x3fd, %edx
	in %dx, %al
	bt $0x6, %eax
	jnc 0b

	/* Output # to the UART. */
	mov $0x23, %eax
	mov $0x3f8, %edx
	out %al, %dx

	/* Wait for the last character to be transmitted. */
0:
	mov $0x3fd, %edx
	in %dx, %al
	bt $0x6, %eax
	jnc 0b

	/* Output a line feed. */
	mov $0xa, %eax
	mov $0x3f8, %edx
	out %al, %dx

	/* Wait for the last character to be transmitted. */
0:
	mov $0x3fd, %edx
	in %dx, %al
	bt $0x6, %eax
	jnc 0b

	/* Output a carriage return. */
	mov $0xd, %eax
	mov $0x3f8, %edx
	out %al, %dx

	/* Wait for a while to avoid DOSing the EC. */
	rdtsc
	mov %eax, %edi
	mov %edx, %esi
timer_loop:
	rdtsc
	sub %edi, %eax
	sbb %esi, %edx
	cmp $0x20000, %eax
	jle timer_loop

	/* Do it again. */
	jmp output_loop
