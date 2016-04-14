/*
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
 * Copyright 2016 Google Inc.
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

#include <libpayload.h>
#include <stdint.h>

#include "arch/io.h"
#include "base/cleanup_funcs.h"
#include "base/keycodes.h"
#include "base/xalloc.h"
#include "drivers/keyboard/ps2.h"

enum {
	Ps2Layout_Map_Plain = 0,
	Ps2Layout_Map_Shift = 1,
	Ps2Layout_Map_Alt = 2,
	Ps2Layout_Map_Alt_Shift = Ps2Layout_Map_Alt | Ps2Layout_Map_Shift,
	Ps2Layout_NumMaps
};

enum {
	DataPort = 0x60,
	CommandPort = 0x64
};

typedef const uint16_t Ps2Layout[Ps2Layout_NumMaps][0x57];

static Ps2Layout layouts[] = {
	[Ps2KeyboardLayout_Us] = {
		[Ps2Layout_Map_Plain] = {
	0x00, 0x1B, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
	0x37, 0x38, 0x39, 0x30, 0x2D, 0x3D, 0x08, 0x09,
	0x71, 0x77, 0x65, 0x72, 0x74, 0x79, 0x75, 0x69,
	0x6F, 0x70, 0x5B, 0x5D, 0x0A, 0x00, 0x61, 0x73,
	0x64, 0x66, 0x67, 0x68, 0x6A, 0x6B, 0x6C, 0x3B,
	0x27, 0x60, 0x00, 0x5C, 0x7A, 0x78, 0x63, 0x76,
	0x62, 0x6E, 0x6D, 0x2C, 0x2E, 0x2F, 0x00, 0x2A,
	0x00, 0x20, 0x00, KEY_F(1), KEY_F(2), KEY_F(3), KEY_F(4), KEY_F(5),
	KEY_F(6), KEY_F(7), KEY_F(8), KEY_F(9), KEY_F(10), 0x00, 0x00, KEY_HOME,
	KEY_UP, KEY_NPAGE, 0x00, KEY_LEFT, 0x00, KEY_RIGHT, 0x00, KEY_END,
	KEY_DOWN, KEY_PPAGE, 0x00, KEY_DC, 0x00, 0x00, 0x00
		},
		[Ps2Layout_Map_Shift] = {
	0x00, 0x1B, 0x21, 0x40, 0x23, 0x24, 0x25, 0x5E,
	0x26, 0x2A, 0x28, 0x29, 0x5F, 0x2B, 0x08, 0x00,
	0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49,
	0x4F, 0x50, 0x7B, 0x7D, 0x0A, 0x00, 0x41, 0x53,
	0x44, 0x46, 0x47, 0x48, 0x4A, 0x4B, 0x4C, 0x3A,
	0x22, 0x7E, 0x00, 0x7C, 0x5A, 0x58, 0x43, 0x56,
	0x42, 0x4E, 0x4D, 0x3C, 0x3E, 0x3F, 0x00, 0x2A,
	0x00, 0x20, 0x00, KEY_F(1), KEY_F(2), KEY_F(3), KEY_F(4), KEY_F(5),
	KEY_F(6), KEY_F(7), KEY_F(8), KEY_F(9), KEY_F(10), 0x00, 0x00, KEY_HOME,
	KEY_UP, KEY_NPAGE, 0x00, KEY_LEFT, 0x00, KEY_RIGHT, 0x00, KEY_END,
	KEY_DOWN, KEY_PPAGE, 0x00, KEY_DC, 0x00, 0x00, 0x00
		},
		[Ps2Layout_Map_Alt] = {
	0x00, 0x1B, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
	0x37, 0x38, 0x39, 0x30, 0x2D, 0x3D, 0x08, 0x09,
	0x71, 0x77, 0x65, 0x72, 0x74, 0x79, 0x75, 0x69,
	0x6F, 0x70, 0x5B, 0x5D, 0x0A, 0x00, 0x61, 0x73,
	0x64, 0x66, 0x67, 0x68, 0x6A, 0x6B, 0x6C, 0x3B,
	0x27, 0x60, 0x00, 0x5C, 0x7A, 0x78, 0x63, 0x76,
	0x62, 0x6E, 0x6D, 0x2C, 0x2E, 0x2F, 0x00, 0x2A,
	0x00, 0x20, 0x00, KEY_F(1), KEY_F(2), KEY_F(3), KEY_F(4), KEY_F(5),
	KEY_F(6), KEY_F(7), KEY_F(8), KEY_F(9), KEY_F(10), 0x00, 0x00, KEY_HOME,
	KEY_UP, KEY_NPAGE, 0x00, KEY_LEFT, 0x00, KEY_RIGHT, 0x00, KEY_END,
	KEY_DOWN, KEY_PPAGE, 0x00, KEY_DC, 0x00, 0x00, 0x00
		},
		[Ps2Layout_Map_Alt_Shift] = {
	0x00, 0x1B, 0x21, 0x40, 0x23, 0x24, 0x25, 0x5E,
	0x26, 0x2A, 0x28, 0x29, 0x5F, 0x2B, 0x08, 0x00,
	0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49,
	0x4F, 0x50, 0x7B, 0x7D, 0x0A, 0x00, 0x41, 0x53,
	0x44, 0x46, 0x47, 0x48, 0x4A, 0x4B, 0x4C, 0x3A,
	0x22, 0x7E, 0x00, 0x7C, 0x5A, 0x58, 0x43, 0x56,
	0x42, 0x4E, 0x4D, 0x3C, 0x3E, 0x3F, 0x00, 0x2A,
	0x00, 0x20, 0x00, KEY_F(1), KEY_F(2), KEY_F(3), KEY_F(4), KEY_F(5),
	KEY_F(6), KEY_F(7), KEY_F(8), KEY_F(9), KEY_F(10), 0x00, 0x00, KEY_HOME,
	KEY_UP, KEY_NPAGE, 0x00, KEY_LEFT, 0x00, KEY_RIGHT, 0x00, KEY_END,
	KEY_DOWN, KEY_PPAGE, 0x00, KEY_DC, 0x00, 0x00, 0x00
		}
	}
};

enum {
	Ps2ModShift = 1 << 0,
	Ps2ModCtrl = 1 << 1,
	Ps2ModCapslock = 1 << 2,
	Ps2ModAlt = 1 << 3
};

static void ps2_empty_buffer(Ps2Keyboard *keyboard)
{
	while (keyboard->ops.have_char(&keyboard->ops))
		keyboard->ops.get_char(&keyboard->ops);
}

static int ps2_disconnect(struct CleanupFunc *cleanup, CleanupType type)
{
	Ps2Keyboard *keyboard = cleanup->data;

	ps2_empty_buffer(keyboard);

	// Send keyboard disconnect command.
	outb(0xad, CommandPort);
	// Wait for the command to complete.
	int retries = 10000;
	while (retries-- && (inb(CommandPort) & 0x02))
		udelay(50);

	ps2_empty_buffer(keyboard);

	return 0;
}

static void ps2_init(Ps2Keyboard *keyboard)
{
	// If the command port returns 0xff, there's no keyboard controller.
	if (inb(CommandPort) == 0xFF) {
		keyboard->status = Ps2KeyboardStatus_Not_Present;
		return;
	}
	keyboard->status = Ps2KeyboardStatus_Present;

	ps2_empty_buffer(keyboard);

	CleanupFunc *disconnect = xzalloc(sizeof(*disconnect));
	*disconnect = (CleanupFunc) {
		.cleanup = &ps2_disconnect,
		.types = CleanupOnHandoff | CleanupOnLegacy,
		.data = keyboard
	};

	list_insert_after(&disconnect->list_node, &cleanup_funcs);
}

static int ps2_have_char(KeyboardOps *me)
{
	Ps2Keyboard *keyboard = container_of(me, Ps2Keyboard, ops);

	if (keyboard->status == Ps2KeyboardStatus_Uninitialized)
		ps2_init(keyboard);

	if (keyboard->status != Ps2KeyboardStatus_Present)
		return 0;

	uint8_t c = inb(CommandPort);
	return (c == 0xff) ? 0 : c & 1;
}

static int ps2_get_char(KeyboardOps *me)
{
	Ps2Keyboard *keyboard = container_of(me, Ps2Keyboard, ops);

	if (keyboard->status == Ps2KeyboardStatus_Uninitialized)
		ps2_init(keyboard);

	if (keyboard->status != Ps2KeyboardStatus_Present)
		return -1;

	while (!ps2_have_char(me))
	{;}

	uint8_t scan_code = inb(DataPort);

	switch (scan_code) {
	case 0x36:
	case 0x2a:
		keyboard->shift = 1;
		break;
	case 0x80 | 0x36:
	case 0x80 | 0x2a:
		keyboard->shift = 0;
		break;
	case 0x38:
		keyboard->alt = 1;
		break;
	case 0x80 | 0x38:
		keyboard->alt = 0;
		break;
	case 0x1d:
		keyboard->ctrl = 1;
		break;
	case 0x80 | 0x1d:
		keyboard->ctrl = 0;
		break;
	case 0x3a:
		keyboard->capslock = !keyboard->capslock;

		while (inb(CommandPort) & 2)
		{;}
		outb(0xed, DataPort);
		mdelay(20);

		while (inb(CommandPort) & 2)
		{;}
		outb(keyboard->capslock << 2, DataPort);
		mdelay(20);

		break;
	}

	if (!(scan_code & 0x80) && scan_code < 0x57) {
		int map = keyboard->alt ? Ps2Layout_Map_Alt :
					  Ps2Layout_Map_Plain;
		map |= keyboard->shift ^ keyboard->capslock;

		int key = layouts[keyboard->layout][map][scan_code];
		if (keyboard->ctrl) {
			if (key >= 'a' && key <= 'z')
				return key & 0x1f;
			else
				return 0;
		}
		return key;
	}

	return 0;
}

Ps2Keyboard *new_ps2_keyboard(void)
{
	Ps2Keyboard *keyboard = xzalloc(sizeof(*keyboard));

	keyboard->ops.get_char = &ps2_get_char;
	keyboard->ops.have_char = &ps2_have_char;
	keyboard->layout = Ps2KeyboardLayout_Us;

	return keyboard;
}
