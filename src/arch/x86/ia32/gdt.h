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

#ifndef __ARCH_X86_IA32_GDT_H__
#define __ARCH_X86_IA32_GDT_H__

#define _GDT_CS32_IDX_ 1
#define _GDT_DS32_IDX_ 2

#define gdt_idx_to_sel(index, rpl) ((((index) << 3) | (rpl & 0x3)) & 0xffff)

#ifndef __ASSEMBLER__

enum {
	GdtCs32Index = _GDT_CS32_IDX_,
	GdtDs32Index = _GDT_DS32_IDX_,
};

#else /* !__ASSEMBLER__ */

#define GDT_CS32_IDX _GDT_CS32_IDX_
#define GDT_DS32_IDX _GDT_DS32_IDX_

#endif

#endif /* __ARCH_X86_IA32_GDT_H__ */
