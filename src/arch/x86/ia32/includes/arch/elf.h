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

#ifndef __ARCH_ELF_H__
#define __ARCH_ELF_H__

enum {
	R_386_NONE = 0,
	R_386_32 = 1,
	R_386_PC32 = 2,
	R_386_GOT32 = 3,
	R_386_PLT32 = 4,
	R_386_COPY = 5,
	R_386_GLOB_DAT = 6,
	R_386_JMP_SLOT = 7,
	R_386_RELATIVE = 8,
	R_386_GOTOFF = 9,
	R_386_GOTPC = 10,
	R_386_NUM
};

#endif /* __ARCH_ELF_H__ */
