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
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */
#ifndef __DRIVERS_KEYBOARD_PS2_H__
#define __DRIVERS_KEYBOARD_PS2_H__

#include "drivers/keyboard/keyboard.h"

typedef enum {
	Ps2KeyboardLayout_Us
} Ps2KeyboardLayout;

typedef enum {
	Ps2KeyboardStatus_Uninitialized = 0,
	Ps2KeyboardStatus_Present,
	Ps2KeyboardStatus_Not_Present
} Ps2KeyboardStatus;

typedef struct {
	KeyboardOps ops;

	uint8_t shift;
	uint8_t alt;
	uint8_t ctrl;
	uint8_t capslock;

	Ps2KeyboardLayout layout;
	Ps2KeyboardStatus status;
} Ps2Keyboard;

Ps2Keyboard *new_ps2_keyboard(void);

#endif /* __DRIVERS_KEYBOARD_KEYBOARD_H__ */
