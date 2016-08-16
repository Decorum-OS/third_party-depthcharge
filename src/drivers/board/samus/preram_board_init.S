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

.section .text.preram_board_init
.code32

/*
 * Route POST codes to the EC on the LPC bus, and do enough configuration to
 * get a serial console going.
 */
.global preram_board_init
preram_board_init:
	/*
	 * The code below establishes communication with the outside world
	 * through very basic serial output and POST codes. It will help debug
	 * this early code during bringup before the FSP is working and
	 * cache-as-ram is available, but is specific to samus and
	 * shouldn't be in a more general module like this. Once the FSP and
	 * CAR are working, it should be deleted.
	 */

	/* Set the PCI config addr to the RCBA. */
	mov $0xcf8, %edx
	mov $(0x01 << 31 | /* enable */    \
	      0x00 << 16 | /* bus */       \
	      0x1f << 11 | /* device */    \
	      0x00 <<  8 | /* function */  \
	      0xf0 <<  0 | /* register */  \
	      0), %eax
	out %eax, %dx

	/*
	 * Set the RCBA to the (probably arbitrary) address stolen
	 * from coreboot, with the enable bit turned on.
	 */
	mov $0xcfc, %edx
	mov $(0xfed1c000 | 1), %eax
	out %eax, %dx

	/*
	 * Clear bit 2 in the General Control and Status (GCS) register
	 * which routes POST code writes to port 0x80 to PCI.
	 */
	btr $2, (0xfed1c000 + 0x3410)

	/* Set the PCI config addr to LPC_EN. */
	mov $0xcf8, %edx
	mov $(0x01 << 31 | /* enable */    \
	      0x00 << 16 | /* bus */       \
	      0x1f << 11 | /* device */    \
	      0x00 <<  8 | /* function */  \
	      0x80 <<  0 | /* register */  \
	      0), %eax
	out %eax, %dx

	/* Enable some legacy decode ranges. */
	mov $0xcfe, %edx
	mov $(1 << 12 | /* CNF1_LPC_EN */  \
	      1 << 13 | /* CNF2_LPC_EN */  \
	      1 << 8  | /* GAMEL_LPC_EN */ \
	      1 << 0  | /* COMA_LPC_EN */  \
	      1 << 10 | /* KBC_LPC_EN */   \
	      1 << 11 | /* MC_LPC_EN */    \
	      0), %eax
	out %ax, %dx

	/* Set LCR to 3 for 8N1. */
	mov $0x3, %eax
	mov $0x3fb, %edx
	out %al, %dx

	jmp *%esp
