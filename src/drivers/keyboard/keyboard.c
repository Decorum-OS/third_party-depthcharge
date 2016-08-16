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

#include "drivers/board/board.h"
#include "drivers/keyboard/keyboard.h"

void keyboard_prepare(void)
{
	KeyboardOps *keyboard;
	KeyboardOps **keyboards;

	keyboards = board_trusted_keyboards();
	while ((keyboard = *keyboards++))
		keyboard->have_char(keyboard);

	keyboards = board_untrusted_keyboards();
	while ((keyboard = *keyboards++))
		keyboard->have_char(keyboard);
}
