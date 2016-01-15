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

#ifndef __MODULE_TRAMPOLINE_H__
#define __MODULE_TRAMPOLINE_H__

#include <stdint.h>

#include "base/elf.h"

enum {
	TrampolineStackSize = 0x200
};

extern uint8_t trampoline_stack[TrampolineStackSize];

// Implemented by each architecture to transfer control to the trampoline,
// which generally involves switching the stack over and jumping to the
// trampoline's entry point.
void enter_trampoline(Elf32_Ehdr *ehdr);
// The trampoline's actual entry point which unpacks an ELF and transfers
// control to it. It can't really do anything else including report errors,
// so we have to be very confident it will succeed before calling into it.
void trampoline(Elf32_Ehdr *ehdr, void *param);

#endif /* __MODULE_TRAMPOLINE_H__ */
