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

#ifndef __MODULE_SYMBOLS_H__
#define __MODULE_SYMBOLS_H__

#include <stdint.h>

// C level variable definitions for symbols defined in the linker script.

extern uint8_t _start;
extern uint8_t _end;
extern uint8_t _tramp_start;
extern uint8_t _tramp_end;
extern uint8_t _init_funcs_start;
extern uint8_t _init_funcs_end;

extern uint8_t _binary_trampoline_start;
extern uint8_t _binary_trampoline_end;
extern uint8_t _binary_trampoline_size;

#endif /* __MODULE_SYMBOLS_H__ */
