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
.code16
.global _entry
_entry:
	/* Disable interrupts. */
	cli

	/* Enable protected mode. */
	mov %cr0, %eax
	btsl $0, %eax
	mov %eax, %cr0

	/* Load the GDT pseudo descriptor. */
	addr32 lgdtl %cs:(gdt_ptr - 0xffff0000)

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

.data
.align 8
gdt_ptr:
	.short gdt_end - gdt - 1
	.int gdt

.align 8
gdt:
	.int 0
	.int 0

	/* 32 bit CPL 0 code segment */
	.short 0xffff       /* limit 15:00 */
	.short 0x0000       /* base 15:00 */
	.byte  0x00         /* base 23:16 */
	.byte  0b10011011   /* P(1) DPL(00) S(1) 1 C(0) R(1) A(1) */
	.byte  0b11001111   /* G(1) D(1) 0 0 limit 19:16 */
	.byte  0x0          /* base 31:24 */

	/* 32 bit CPL 0 data segment */
	.short 0xffff       /* limit 15:00 */
	.short 0x0000       /* base 15:00 */
	.byte  0x00         /* base 23:16 */
	.byte  0b10010011   /* P(1) DPL(00) S(1) 0 E(0) W(1) A(1) */
	.byte  0b11001111   /* G(1) B(1) 0 0 limit 19:16 */
	.byte  0x0          /* base 31:24 */
gdt_end:
