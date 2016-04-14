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

#include "base/list.h"
#include "base/xalloc.h"
#include "drivers/bus/usb/usb.h"
#include "drivers/keyboard/dynamic.h"
#include "drivers/keyboard/keyboard.h"

static int get_char(KeyboardOps *me)
{
	DynamicKeyboards *keyboards = container_of(me, DynamicKeyboards, ops);

	if (CONFIG_DRIVER_USB_HID)
		dc_usb_initialize();

	while (1) {
		if (CONFIG_DRIVER_USB_HID)
			usb_poll();

		DynamicKeyboard *kb;
		list_for_each(kb, keyboards->keyboards, list_node) {
			if (kb->ops.have_char(&kb->ops))
				return kb->ops.get_char(&kb->ops);
		}
	}
}

static int have_char(KeyboardOps *me)
{
	DynamicKeyboards *keyboards = container_of(me, DynamicKeyboards, ops);

	if (CONFIG_DRIVER_USB_HID) {
		dc_usb_initialize();
		usb_poll();
	}

	DynamicKeyboard *kb;
	list_for_each(kb, keyboards->keyboards, list_node) {
		if (kb->ops.have_char(&kb->ops))
			return 1;
	}

	return 0;
}

DynamicKeyboards dynamic_keyboards = {
	.ops = {
		.get_char = &get_char,
		.have_char = &have_char
	}
};
