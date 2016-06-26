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
/*
 * Copyright (C) 2008-2010 coresystems GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "base/algorithm.h"
#include "base/keycodes.h"
#include "drivers/usb/hid/layouts.h"

const UsbLayoutMaps usb_kbd_layouts[] = {
	{ .country = "us",
	  .maps = {
		[UsbKbd_Layout_Map_Plain] = {
			-1, -1, -1, -1, 'a', 'b', 'c', 'd',
			'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
			'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
			'u', 'v', 'w', 'x', 'y', 'z', '1', '2',
			'3', '4', '5', '6', '7', '8', '9', '0',
			'\n', '\e', '\b', '\t', ' ', '-', '=', '[',
			']', '\\', -1, ';', '\'', '`', ',', '.',
			'/', -1 /* CapsLk */, KEY_F(1), KEY_F(2), KEY_F(3),
				KEY_F(4), KEY_F(5), KEY_F(6),
			KEY_F(7), KEY_F(8), KEY_F(9), KEY_F(10), KEY_F(11),
				KEY_F(12), KEY_PRINT, -1 /* ScrLk */,
			KEY_BREAK, KEY_IC, KEY_HOME, KEY_PPAGE, KEY_DC,
				KEY_END, KEY_NPAGE, KEY_RIGHT,
			KEY_LEFT, KEY_DOWN, KEY_UP, -1 /*NumLck*/, '/', '*',
				'-' /* = ? */, '+',
			KEY_ENTER, KEY_END, KEY_DOWN, KEY_NPAGE, KEY_LEFT,
				-1, KEY_RIGHT, KEY_HOME,
			KEY_UP, KEY_PPAGE, -1, KEY_DC, -1 /* < > | */,
				-1 /* Win Key Right */, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1,
		},
		[UsbKbd_Layout_Map_Shift] = {
			-1, -1, -1, -1, 'A', 'B', 'C', 'D',
			'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
			'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
			'U', 'V', 'W', 'X', 'Y', 'Z', '!', '@',
			'#', '$', '%', '^', '&', '*', '(', ')',
			'\n', '\e', '\b', '\t', ' ', '_', '+', '[',
			']', '\\', -1, ':', '\'', '`', ',', '.',
			'/', -1 /* CapsLk */, KEY_F(1), KEY_F(2), KEY_F(3),
				KEY_F(4), KEY_F(5), KEY_F(6),
			KEY_F(7), KEY_F(8), KEY_F(9), KEY_F(10), KEY_F(11),
				KEY_F(12), KEY_PRINT, -1 /* ScrLk */,
			KEY_BREAK, KEY_IC, KEY_HOME, KEY_PPAGE, KEY_DC, 
				KEY_END, KEY_NPAGE, KEY_RIGHT,
			KEY_LEFT, KEY_DOWN, KEY_UP, -1 /*NumLck*/, '/', '*',
				'-' /* = ? */, '+',
			KEY_ENTER, KEY_END, KEY_DOWN, KEY_NPAGE, KEY_LEFT, -1,
				KEY_RIGHT, KEY_HOME,
			KEY_UP, KEY_PPAGE, -1, KEY_DC, -1 /* < > | */,
				-1 /* Win Key Right */, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1,
		},
		[UsbKbd_Layout_Map_Alt] = {
			-1, -1, -1, -1, 'a', 'b', 'c', 'd',
			'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
			'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
			'u', 'v', 'w', 'x', 'y', 'z', '1', '2',
			'3', '4', '5', '6', '7', '8', '9', '0',
			'\n', '\e', '\b', '\t', ' ', '-', '=', '[',
			']', '\\', -1, ';', '\'', '`', ',', '.',
			'/', -1 /* CapsLk */, KEY_F(1), KEY_F(2), KEY_F(3),
				KEY_F(4), KEY_F(5), KEY_F(6),
			KEY_F(7), KEY_F(8), KEY_F(9), KEY_F(10), KEY_F(11),
				KEY_F(12), KEY_PRINT, -1 /* ScrLk */,
			KEY_BREAK, KEY_IC, KEY_HOME, KEY_PPAGE, KEY_DC,
				KEY_END, KEY_NPAGE, KEY_RIGHT,
			KEY_LEFT, KEY_DOWN, KEY_UP, -1 /*NumLck*/, '/', '*',
				'-' /* = ? */, '+',
			KEY_ENTER, KEY_END, KEY_DOWN, KEY_NPAGE, KEY_LEFT, -1,
				KEY_RIGHT, KEY_HOME,
			KEY_UP, KEY_PPAGE, -1, KEY_DC, -1 /* < > | */,
				-1 /* Win Key Right */, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1,
		},
		[UsbKbd_Layout_Map_Alt_Shift] = {
			-1, -1, -1, -1, 'A', 'B', 'C', 'D',
			'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
			'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
			'U', 'V', 'W', 'X', 'Y', 'Z', '!', '@',
			'#', '$', '%', '^', '&', '*', '(', ')',
			'\n', '\e', '\b', '\t', ' ', '-', '=', '[',
			']', '\\', -1, ':', '\'', '`', ',', '.',
			'/', -1 /* CapsLk */, KEY_F(1), KEY_F(2), KEY_F(3),
				KEY_F(4), KEY_F(5), KEY_F(6),
			KEY_F(7), KEY_F(8), KEY_F(9), KEY_F(10), KEY_F(11),
				KEY_F(12), KEY_PRINT, -1 /* ScrLk */,
			KEY_BREAK, KEY_IC, KEY_HOME, KEY_PPAGE, KEY_DC,
				KEY_END, KEY_NPAGE, KEY_RIGHT,
			KEY_LEFT, KEY_DOWN, KEY_UP, -1 /*NumLck*/, '/', '*',
				'-' /* = ? */, '+',
			KEY_ENTER, KEY_END, KEY_DOWN, KEY_NPAGE, KEY_LEFT, -1,
				KEY_RIGHT, KEY_HOME,
			KEY_UP, KEY_PPAGE, -1, KEY_DC, -1 /* < > | */,
				-1 /* Win Key Right */, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1,
	 	}
	  }
	},
};

const int usb_kbd_num_layouts = ARRAY_SIZE(usb_kbd_layouts);
