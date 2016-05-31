/*
 * Copyright 2013 Google Inc.
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
#include <stdint.h>

#include "module/symbols.h"
#include "module/trampoline/trampoline.h"

// Temporarily pass the coreboot tables (or whatever else was our parameter)
// through to the next module.
extern uint32_t handoff_parameter;

void enter_trampoline(Elf32_Ehdr *ehdr)
{
	uint8_t *estack = &trampoline_stack[TrampolineStackSize - 8];

	__asm__ __volatile__(
		"mov sp, %[new_stack]\n"
		"mov r0, %[ehdr]\n"
		"mov r1, %[handoff_parameter]\n"
		// Need register addressing since the call goes too far
		"bx %[trampoline]\n"
		:: [new_stack]"r"(estack), [ehdr]"r"(ehdr),
		   [zero]"r"(0), [trampoline]"r"(&trampoline),
		   [handoff_parameter]"r"(handoff_parameter)
		: "memory", "r0", "r1", "sp"
	);
}
