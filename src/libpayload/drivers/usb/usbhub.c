/*
 * Copyright (C) 2013 secunet Security Networks AG
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

//#define USB_DEBUG

#include <usb/usb.h>
#include "generic_hub.h"

/* assume that UsbHostToDevice is overwritten if necessary */
#define DR_PORT usb_gen_bmRequestType(UsbHostToDevice, UsbClassType, UsbOtherRecp)
/* status (and status change) bits */
#define PORT_CONNECTION 0x1
#define PORT_ENABLE 0x2
#define PORT_RESET 0x10
/* feature selectors (for setting / clearing features) */
#define SEL_PORT_RESET 0x4
#define SEL_PORT_POWER 0x8
#define SEL_C_PORT_CONNECTION 0x10
/* request type (USB 3.0 hubs only) */
#define SET_HUB_DEPTH 12

static int
usb_hub_port_status_changed(UsbDev *const dev, const int port)
{
	unsigned short buf[2];
	int ret = usb_get_status (dev, port, DR_PORT, sizeof(buf), buf);
	if (ret >= 0) {
		ret = buf[1] & PORT_CONNECTION;
		if (ret)
			usb_clear_feature(dev, port, SEL_C_PORT_CONNECTION,
					  DR_PORT);
	}
	return ret;
}

static int
usb_hub_port_connected(UsbDev *const dev, const int port)
{
	unsigned short buf[2];
	int ret = usb_get_status (dev, port, DR_PORT, sizeof(buf), buf);
	if (ret >= 0)
		ret = buf[0] & PORT_CONNECTION;
	return ret;
}

static int
usb_hub_port_in_reset(UsbDev *const dev, const int port)
{
	unsigned short buf[2];
	int ret = usb_get_status (dev, port, DR_PORT, sizeof(buf), buf);
	if (ret >= 0)
		ret = buf[0] & PORT_RESET;
	return ret;
}

static int
usb_hub_port_enabled(UsbDev *const dev, const int port)
{
	unsigned short buf[2];
	int ret = usb_get_status (dev, port, DR_PORT, sizeof(buf), buf);
	if (ret >= 0)
		ret = buf[0] & PORT_ENABLE;
	return ret;
}

static UsbSpeed
usb_hub_port_speed(UsbDev *const dev, const int port)
{
	unsigned short buf[2];
	int ret = usb_get_status (dev, port, DR_PORT, sizeof(buf), buf);
	if (ret >= 0 && (buf[0] & PORT_ENABLE)) {
		/* SuperSpeed hubs can only have SuperSpeed devices. */
		if (dev->speed == UsbSuperSpeed)
			return UsbSuperSpeed;

		/*[bit] 10  9  (USB 2.0 port status word)
		 *      0   0  full speed
		 *      0   1  low speed
		 *      1   0  high speed
		 *      1   1  invalid
		 */
		ret = (buf[0] >> 9) & 0x3;
		if (ret != 0x3)
			return ret;
	}
	return -1;
}

static int
usb_hub_enable_port(UsbDev *const dev, const int port)
{
	return usb_set_feature(dev, port, SEL_PORT_POWER, DR_PORT);
}

static int
usb_hub_start_port_reset(UsbDev *const dev, const int port)
{
	return usb_set_feature (dev, port, SEL_PORT_RESET, DR_PORT);
}

static void usb_hub_set_hub_depth(UsbDev *const dev)
{
	UsbDevReq dr = {
		.bmRequestType = usb_gen_bmRequestType(UsbHostToDevice,
						   UsbClassType, UsbDevRecp),
		.bRequest = SET_HUB_DEPTH,
		.wValue = 0,
		.wIndex = 0,
		.wLength = 0,
	};
	UsbDev *parent = dev;
	while (parent->hub > 0) {
		parent = dev->controller->devices[parent->hub];
		dr.wValue++;
	}
	int ret = dev->controller->control(dev, UsbDirOut, sizeof(dr),
					   &dr, 0, NULL);
	if (ret < 0)
		usb_debug("Failed SET_HUB_DEPTH(%d) on hub %d: %d\n",
			  dr.wValue, dev->address, ret);
}

static const generic_hub_ops_t usb_hub_ops = {
	.hub_status_changed	= NULL,
	.port_status_changed	= usb_hub_port_status_changed,
	.port_connected		= usb_hub_port_connected,
	.port_in_reset		= usb_hub_port_in_reset,
	.port_enabled		= usb_hub_port_enabled,
	.port_speed		= usb_hub_port_speed,
	.enable_port		= usb_hub_enable_port,
	.disable_port		= NULL,
	.start_port_reset	= usb_hub_start_port_reset,
	.reset_port		= generic_hub_resetport,
};

void
usb_hub_init(UsbDev *const dev)
{
	int type = dev->speed == UsbSuperSpeed ? 0x2a : 0x29; /* similar enough */
	UsbHubDescriptor desc;	/* won't fit the whole thing, we don't care */
	if (usb_get_descriptor(dev, usb_gen_bmRequestType(UsbDeviceToHost,
						      UsbClassType,
						      UsbDevRecp),
			       type, 0, &desc, sizeof(desc)) != sizeof(desc)) {
		usb_debug("get_descriptor(HUB) failed\n");
		usb_detach_device(dev->controller, dev->address);
		return;
	}

	if (dev->speed == UsbSuperSpeed)
		usb_hub_set_hub_depth(dev);
	generic_hub_init(dev, desc.bNbrPorts, &usb_hub_ops);
}
