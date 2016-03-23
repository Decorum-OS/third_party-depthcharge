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

/* Hard code anchor location for now. */
.set _dcdir_anchor_addr, 0x100000000 - 0x400000

/*
 * mov_string_const
 * Move up to 4 ASCII characters into a register.
 *
 * reg - The register to store the constant in.
 * a, b, c, d - Characters which form the string being put into the register.
 *              Any characters which aren't specified explicitly are stored
 *              as zero.
 */
.macro mov_string_const reg, a=0, b=0, c=0, d=0
	mov $((\a << 0) | (\b << 8) | (\c << 16) | (\d << 24)), \reg
.endm
