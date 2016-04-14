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

#include <usb/usb.h>

#include "base/keycodes.h"
#include "base/list.h"
#include "base/xalloc.h"
#include "drivers/bus/usb/usb.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/usb/hid/keyboard.h"
#include "drivers/usb/hid/layouts.h"


enum {
	HidSubclassNone = 0,
	HidSubclassBoot = 1
};

typedef enum {
	HidProtoBoot = 0,
	HidProtoReport = 1
} HidProto;

enum {
	HidBootProtoNone = 0,
	HidBootProtoKeyboard = 1,
	HidBootProtoMouse = 2
};

enum {
	RequestGetReport = 0x1,
	RequestGetIdle = 0x2,
	RequestGetProtocol = 0x3,
	RequestSetReport = 0x9,
	RequestSetIdle = 0xa,
	RequestSetProtocol = 0xb
};

enum {
	KeyboardRepeatMs = 30
};



static void keybuffer_push(UsbKbdBuffer *buf, UsbKeyPress key)
{
	const int buf_size = ARRAY_SIZE(buf->keys);

	// Ignore keys once the buffer is full.
	if (buf->count >= buf_size)
		return;

	int index = (buf->next - buf->count + buf_size) % buf_size;
	buf->keys[index] = key;
	buf->count++;
}

static UsbKeyPress keybuffer_pop(UsbKbdBuffer *buf)
{
	const int buf_size = ARRAY_SIZE(buf->keys);

	// Return -1 if the buffer is empty.
	if (buf->count <= 0)
		return -1;

	buf->count--;
	UsbKeyPress key = buf->keys[buf->next];
	buf->next = (buf->next + 1) % buf_size;
	return key;
}

static int keybuffer_count(UsbKbdBuffer *buf)
{
	return buf->count;
}



const char *countries[36][2] = {
	{ "not supported", "us" },
	{ "Arabic", "ae" },
	{ "Belgian", "be" },
	{ "Canadian-Bilingual", "ca" },
	{ "Canadian-French", "ca" },
	{ "Czech Republic", "cz" },
	{ "Danish", "dk" },
	{ "Finnish", "fi" },
	{ "French", "fr" },
	{ "German", "de" },
	{ "Greek", "gr" },
	{ "Hebrew", "il" },
	{ "Hungary", "hu" },
	{ "International (ISO)", "iso" },
	{ "Italian", "it" },
	{ "Japan (Katakana)", "jp" },
	{ "Korean", "us" },
	{ "Latin American", "us" },
	{ "Netherlands/Dutch", "nl" },
	{ "Norwegian", "no" },
	{ "Persian (Farsi)", "ir" },
	{ "Poland", "pl" },
	{ "Portuguese", "pt" },
	{ "Russia", "ru" },
	{ "Slovakia", "sl" },
	{ "Spanish", "es" },
	{ "Swedish", "se" },
	{ "Swiss/French", "ch" },
	{ "Swiss/German", "ch" },
	{ "Switzerland", "ch" },
	{ "Taiwan", "tw" },
	{ "Turkish-Q", "tr" },
	{ "UK", "uk" },
	{ "US", "us" },
	{ "Yugoslavia", "yu" },
	{ "Turkish-F", "tr" },
	/* 36 - 255: Reserved */
};



static void usb_kbd_event(UsbKeyboard *kbd, const UsbKbdEvent *current)
{
	const int ctrl = ((current->modifiers & 0x11) != 0);
	const int shift = ((current->modifiers & 0x22) != 0);
	const int alt = ((current->modifiers & 0x04) != 0);

	// Did the event change at all?
	if (kbd->last_keypress &&
			!memcmp(current, &kbd->previous, sizeof(*current))) {
		// No. It's a key repeat event then.
		if (kbd->repeat_delay) {
			kbd->repeat_delay--;
		} else {
			keybuffer_push(&kbd->key_buffer, kbd->last_keypress);
			kbd->repeat_delay = 2;
		}

		return;
	}

	kbd->last_keypress = 0;

	for (int i = 0; i < ARRAY_SIZE(current->keys); i++) {
		uint8_t code = current->keys[i];
		// No more keys? skip
		if (code == 0)
			return;

		int skip = 0;
		for (int j = 0; j < ARRAY_SIZE(kbd->previous.keys); j++) {
			if (code == kbd->previous.keys[j]) {
				skip = 1;
				break;
			}
		}
		if (skip)
			continue;


		const UsbLayoutMaps *layout =
			&usb_kbd_layouts[kbd->layout_index];
		int map_index = UsbKbd_Layout_Map_Plain;
		map_index += shift ? UsbKbd_Layout_Map_Shift : 0;
		map_index += alt ? UsbKbd_Layout_Map_Alt : 0;
		UsbKeyPress key = layout->maps[map_index][code];

		if (ctrl) {
			if (key >= 'a' && key <= 'z')
				key &= 0x1f;
			else
				continue;
		}

		if (key == -1) {
			// Debug: Print unknown keys.
			usb_debug("usbhid: <%x> %x [ %x %x %x %x %x %x ] %d\n",
				current->modifiers, current->repeats,
			current->keys[0], current->keys[1],
			current->keys[2], current->keys[3],
			current->keys[4], current->keys[5], i);

			// An unknown key? Try the next one in the queue.
			continue;
		}

		keybuffer_push(&kbd->key_buffer, key);

		// Remember for authentic key repeat.
		kbd->last_keypress = key;
		kbd->repeat_delay = 10;
	}
}

static void usb_kbd_destroy(UsbDev *dev)
{
	UsbKeyboard *kbd = (UsbKeyboard *)dev->data;

	if (kbd->queue) {
		UsbEndpoint *ep = &dev->endpoints[kbd->queue_endpoint_index];
		dev->controller->destroy_intr_queue(ep, kbd->queue);
		kbd->queue = NULL;
	}

	list_remove(&kbd->dynamic.list_node);

	free(dev->data);
}

static void usb_kbd_poll(UsbDev *dev)
{
	UsbKeyboard *kbd = (UsbKeyboard *)dev->data;

	UsbKbdEvent current;
	const uint8_t *buf;
	while ((buf = dev->controller->poll_intr_queue(kbd->queue))) {
		memcpy(&current, buf, sizeof(current));
		usb_kbd_event(kbd, &current);
		kbd->previous = current;
	}
}

static void usb_kbd_set_idle(UsbDev *dev, UsbInterfaceDescriptor *interface,
			     uint16_t duration)
{
	UsbDevReq dr = {
		.data_dir = UsbHostToDevice,
		.req_type = UsbClassType,
		.req_recp = UsbIfaceRecp,
		.bRequest = RequestSetIdle,
		.wValue = (duration >> 2) << 8,
		.wIndex = interface->bInterfaceNumber,
		.wLength = 0
	};
	dev->controller->control(dev, UsbDirOut, sizeof(UsbDevReq), &dr, 0, 0);
}

static void usb_kbd_set_protocol(UsbDev *dev, UsbInterfaceDescriptor *interface,
				 HidProto proto)
{
	UsbDevReq dr = {
		.data_dir = UsbHostToDevice,
		.req_type = UsbClassType,
		.req_recp = UsbIfaceRecp,
		.bRequest = RequestSetProtocol,
		.wValue = proto,
		.wIndex = interface->bInterfaceNumber,
		.wLength = 0
	};
	dev->controller->control(dev, UsbDirOut, sizeof(UsbDevReq), &dr, 0, 0);
}

static int usb_kbd_set_layout(UsbKeyboard *kbd, const char *country)
{
	for (int i = 0; i < usb_kbd_num_layouts; i++) {
		if (strncmp(usb_kbd_layouts[i].country, country,
			    strlen(usb_kbd_layouts[i].country)))
			continue;

		// Found, changing keyboard layout.
		kbd->layout_index = i;
		usb_debug("  Keyboard layout '%s'\n", country);
		return 0;
	}

	usb_debug("  Keyboard layout '%s' not found, using '%s'\n",
			country, usb_kbd_layouts[kbd->layout_index].country);

	// Nothing found, not changed.
	return -1;
}

static int usb_kbd_get_char(KeyboardOps *me)
{
	UsbKeyboard *kbd = container_of(me, UsbKeyboard, dynamic.ops);
	while (1) {
		if (keybuffer_count(&kbd->key_buffer))
			return keybuffer_pop(&kbd->key_buffer);
		usb_poll();
	}
}

static int usb_kbd_have_char(KeyboardOps *me)
{
	UsbKeyboard *kbd = container_of(me, UsbKeyboard, dynamic.ops);
	usb_poll();
	return keybuffer_count(&kbd->key_buffer) != 0;
}

static void usb_kbd_init(UsbDev *dev)
{
	UsbKeyboard *kbd = (UsbKeyboard *)dev->data;

	UsbConfigurationDescriptor *cd =
		(UsbConfigurationDescriptor *)dev->configuration;
	UsbInterfaceDescriptor *interface =
		(UsbInterfaceDescriptor *)((uintptr_t)cd + cd->bLength);

	if (interface->bInterfaceSubClass != HidSubclassBoot)
		// Only the boot subclass is supported.
		return;

	usb_debug("  supports boot interface..\n");
	switch (interface->bInterfaceProtocol) {
		case 0:
			usb_debug("  none\n");
			break;
		case 1:
			usb_debug("  keyboard\n");
			break;
		case 2:
			usb_debug("  mouse\n");
			break;
	}

	if (interface->bInterfaceProtocol != HidBootProtoKeyboard) {
		if (interface->bInterfaceProtocol == HidBootProtoMouse)
			usb_debug("NOTICE: USB mice are not supported.\n");
		return;
	}


	kbd = xzalloc(sizeof(*kbd));
	dev->data = kbd;

	usb_debug("  configuring...\n");
	usb_kbd_set_protocol(dev, interface, HidProtoBoot);
	usb_kbd_set_idle(dev, interface, KeyboardRepeatMs);
	usb_debug("  activating...\n");

	UsbHidDescriptor *desc = malloc(sizeof(*desc));
	if (!desc || usb_get_descriptor(
			dev, usb_gen_bmRequestType(
				UsbDeviceToHost,
				UsbStandardType,
				UsbIfaceRecp),
			0x21, 0, desc,
			sizeof(*desc)) != sizeof(*desc)) {
		usb_debug("usb_get_descriptor(HID) failed\n");
		usb_detach_device(dev->controller, dev->address);
		return;
	}
	kbd->descriptor = desc;

	uint8_t countrycode = desc->bCountryCode;

	if (countrycode >= ARRAY_SIZE(countries))
		countrycode = 0;

	usb_debug("  Keyboard has %s layout "
		  "(country code %02x)\n",
		  countries[countrycode][0], countrycode);

	// Set keyboard layout accordingly.
	usb_kbd_set_layout(kbd, countries[countrycode][1]);

	// Only add here, because we only support boot-keyboard HID devices.
	dev->destroy = &usb_kbd_destroy;
	dev->poll = &usb_kbd_poll;

	int i = 0;
	for (i = 1; i < dev->num_endp; i++) {
		if (dev->endpoints[i].type == UsbEndpTypeInterrupt &&
		    dev->endpoints[i].direction == UsbDirIn)
			break;
	}
	if (i >= dev->num_endp) {
		usb_debug("Could not find HID endpoint\n");
		usb_detach_device(dev->controller, dev->address);
		return;
	}
	usb_debug("  found endpoint %x for interrupt-in\n", i);
	// 20 buffers of 8 bytes, for every 10 msecs.
	kbd->queue = dev->controller->create_intr_queue(&dev->endpoints[i],
							8, 20, 10);
	kbd->queue_endpoint_index = i;

	usb_debug("  configuration done.\n");

	kbd->dynamic.ops.get_char = &usb_kbd_get_char;
	kbd->dynamic.ops.have_char = &usb_kbd_have_char;

	list_insert_after(&kbd->dynamic.list_node,
			  &dynamic_keyboards.keyboards);
}

void usb_hid_init(UsbDev *dev)
{
	usb_kbd_init(dev);
}
