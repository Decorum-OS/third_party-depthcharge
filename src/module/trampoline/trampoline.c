/*
 * Copyright 2012 Google Inc.
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

#include <elf.h>
#include <string.h>

#include "arch/io.h"
#include "module/trampoline/trampoline.h"

uint8_t trampoline_stack[TrampolineStackSize] __attribute__((aligned(16)));

void trampoline(Elf32_Ehdr *ehdr, void *param)
{
	elf_load(ehdr);

	elf_start(ehdr, param);
}
