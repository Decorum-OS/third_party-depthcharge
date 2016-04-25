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

//#define USB_DEBUG

#include <usb/usb.h>

#include "base/algorithm.h"
#include "base/time.h"
#include "base/xalloc.h"

#define DR_DESC usb_gen_bmRequestType(UsbDeviceToHost, \
				      UsbStandardType, \
				      UsbDevRecp)

static UsbDevHc *usb_hcs = 0;

UsbDevHc *new_controller(void)
{
	UsbDevHc *controller = xzalloc(sizeof (UsbDevHc));
	controller->next = usb_hcs;
	usb_hcs = controller;
	return controller;
}

void detach_controller(UsbDevHc *controller)
{
	if (controller == NULL)
		return;

	usb_detach_device(controller, 0); // Tear down root hub tree.

	if (usb_hcs == controller) {
		usb_hcs = controller->next;
	} else {
		UsbDevHc *it = usb_hcs;
		while (it != NULL) {
			if (it->next == controller) {
				it->next = controller->next;
				return;
			}
			it = it->next;
		}
	}
}

// Shut down all controllers.
int usb_exit(void)
{
	while (usb_hcs != NULL)
		usb_hcs->shutdown(usb_hcs);
	return 0;
}

// Polls all hubs on all USB controllers, to find out about device changes.
void usb_poll(void)
{
	if (usb_hcs == 0)
		return;
	UsbDevHc *controller = usb_hcs;
	while (controller != NULL) {
		for (int i = 0; i < 128; i++) {
			if (controller->devices[i] != 0) {
				controller->devices[i]->poll(
					controller->devices[i]);
			}
		}
		controller = controller->next;
	}
}

UsbDev *init_device_entry(UsbDevHc *controller, int i)
{
	UsbDev *dev = calloc(1, sizeof(UsbDev));
	if (!dev) {
		usb_debug("no memory to allocate device structure\n");
		return NULL;
	}
	if (controller->devices[i] != 0)
		usb_debug("warning: device %d reassigned?\n", i);
	controller->devices[i] = dev;
	dev->controller = controller;
	dev->address = -1;
	dev->hub = -1;
	dev->port = -1;
	dev->init = usb_nop_init;
	dev->init (controller->devices[i]);
	return dev;
}

int usb_set_feature (UsbDev *dev, int endp, int feature, int rtype)
{
	UsbDevReq dr;

	dr.bmRequestType = rtype;
	dr.data_dir = UsbHostToDevice;
	dr.bRequest = UsbReqSetFeature;
	dr.wValue = feature;
	dr.wIndex = endp;
	dr.wLength = 0;

	return dev->controller->control(dev, UsbDirOut, sizeof (dr), &dr, 0, 0);
}

int usb_get_status(UsbDev *dev, int intf, int rtype, int len, void *data)
{
	UsbDevReq dr;

	dr.bmRequestType = rtype;
	dr.data_dir = UsbDeviceToHost;
	dr.bRequest = UsbReqGetStatus;
	dr.wValue = 0;
	dr.wIndex = intf;
	dr.wLength = len;

	return dev->controller->control(dev, UsbDirIn, sizeof(dr),
					&dr, len, data);
}

int usb_get_descriptor(UsbDev *dev, int rtype, int desc_type, int desc_idx,
		       void *data, size_t len)
{
	UsbDevReq dr;
	int fail_tries = 0;
	int ret = 0;

	/*
	 * Certain Lexar / Micron USB 2.0 disks will fail the
	 * usb_get_descriptor(UsbDescTypeCfg)
	 * call due to timing issues. Work around this by making extra
	 * attempts on failure.
	 */
	while (fail_tries++ < 3) {
		dr.bmRequestType = rtype;
		dr.bRequest = UsbReqGetDescriptor;
		dr.wValue = desc_type << 8 | desc_idx;
		dr.wIndex = 0;
		dr.wLength = len;

		ret = dev->controller->control(
			dev, UsbDirIn, sizeof(dr), &dr, len, data);
		if (ret)
			udelay(10);
		else
			return 0;
	}
	return ret;
}

int usb_set_configuration(UsbDev *dev)
{
	UsbDevReq dr;

	dr.bmRequestType = 0;
	dr.bRequest = UsbReqSetConfiguration;
	dr.wValue = dev->configuration->bConfigurationValue;
	dr.wIndex = 0;
	dr.wLength = 0;

	return dev->controller->control(dev, UsbDirOut, sizeof (dr), &dr, 0, 0);
}

int usb_clear_feature(UsbDev *dev, int endp, int feature, int rtype)
{
	UsbDevReq dr;

	dr.bmRequestType = rtype;
	dr.data_dir = UsbHostToDevice;
	dr.bRequest = UsbReqClearFeature;
	dr.wValue = feature;
	dr.wIndex = endp;
	dr.wLength = 0;

	return dev->controller->control(
		dev, UsbDirOut, sizeof (dr), &dr, 0, 0) < 0;
}

int usb_clear_stall(UsbEndpoint *ep)
{
	int ret = usb_clear_feature(ep->dev, ep->endpoint, UsbEndpointHalt,
				    usb_gen_bmRequestType(UsbHostToDevice,
							  UsbStandardType,
							  UsbEndpRecp));
	ep->toggle = 0;
	return ret;
}

// returns free address or -1.
static int get_free_address(UsbDevHc *controller)
{
	for (int i = controller->latest_address + 1;
		i != controller->latest_address; i++) {
		if (i >= ARRAY_SIZE(controller->devices) || i < 1) {
			usb_debug("WARNING: Device addresses for controller %#x"
				  " wrapped around!\n", controller->reg_base);
			i = 0;
			continue;
		}
		if (controller->devices[i] == 0) {
			controller->latest_address = i;
			return i;
		}
	}
	usb_debug("no free address found\n");
	return -1; // no free address
}

int usb_decode_mps0(UsbSpeed speed, uint8_t bMaxPacketSize0)
{
	switch (speed) {
	case UsbLowSpeed:
		if (bMaxPacketSize0 != 8) {
			usb_debug("Invalid MPS0: 0x%02x\n", bMaxPacketSize0);
			bMaxPacketSize0 = 8;
		}
		return bMaxPacketSize0;
	case UsbFullSpeed:
		switch (bMaxPacketSize0) {
		case 8: case 16: case 32: case 64:
			return bMaxPacketSize0;
		default:
			usb_debug("Invalid MPS0: 0x%02x\n", bMaxPacketSize0);
			return 8;
		}
	case UsbHighSpeed:
		if (bMaxPacketSize0 != 64) {
			usb_debug("Invalid MPS0: 0x%02x\n", bMaxPacketSize0);
			bMaxPacketSize0 = 64;
		}
		return bMaxPacketSize0;
	case UsbSuperSpeed:
		if (bMaxPacketSize0 != 9) {
			usb_debug("Invalid MPS0: 0x%02x\n", bMaxPacketSize0);
			bMaxPacketSize0 = 9;
		}
		return 1 << bMaxPacketSize0;
	default: // GCC is stupid and cannot deal with enums correctly.
		return 8;
	}
}

// Normalize bInterval to log2 of microframes.
static int usb_decode_interval(UsbSpeed speed, const UsbEndpointType type,
			       const unsigned char bInterval)
{
	switch (speed) {
	case UsbLowSpeed:
		switch (type) {
		case UsbEndpTypeIsochronous:
		case UsbEndpTypeInterrupt:
			return LOG2(bInterval) + 3;
		default:
			return 0;
		}
	case UsbFullSpeed:
		switch (type) {
		case UsbEndpTypeIsochronous:
			return (bInterval - 1) + 3;
		case UsbEndpTypeInterrupt:
			return LOG2(bInterval) + 3;
		default:
			return 0;
		}
	case UsbHighSpeed:
		switch (type) {
		case UsbEndpTypeIsochronous:
		case UsbEndpTypeInterrupt:
			return bInterval - 1;
		default:
			return LOG2(bInterval);
		}
	case UsbSuperSpeed:
		switch (type) {
		case UsbEndpTypeIsochronous:
		case UsbEndpTypeInterrupt:
			return bInterval - 1;
		default:
			return 0;
		}
	default:
		return 0;
	}
}

UsbDev *usb_generic_set_address(UsbDevHc *controller, UsbSpeed speed,
				int hubport, int hubaddr)
{
	int adr = get_free_address(controller); // Address to set.
	UsbDevReq dr;

	memset(&dr, 0, sizeof(dr));
	dr.data_dir = UsbHostToDevice;
	dr.req_type = UsbStandardType;
	dr.req_recp = UsbDevRecp;
	dr.bRequest = UsbReqSetAddress;
	dr.wValue = adr;
	dr.wIndex = 0;
	dr.wLength = 0;

	UsbDev *dev = init_device_entry(controller, adr);
	if (!dev)
		return NULL;

	// Dummy values for registering the address.
	dev->address = 0;
	dev->hub = hubaddr;
	dev->port = hubport;
	dev->speed = speed;
	dev->endpoints[0].dev = dev;
	dev->endpoints[0].endpoint = 0;
	dev->endpoints[0].maxpacketsize = 8;
	dev->endpoints[0].toggle = 0;
	dev->endpoints[0].direction = UsbDirSetup;
	dev->endpoints[0].type = UsbEndpTypeControl;
	if (dev->controller->control(dev, UsbDirOut, sizeof(dr),
				     &dr, 0, 0) < 0) {
		usb_debug("set_address failed\n");
		usb_detach_device(controller, adr);
		return NULL;
	}
	mdelay(SET_ADDRESS_MDELAY);

	uint8_t buf[8];
	dev->address = adr;
	if (usb_get_descriptor(dev, DR_DESC, UsbDescTypeDev, 0, buf,
			       sizeof(buf)) != sizeof(buf)) {
		usb_debug("first usb_get_descriptor(UsbDescTypeDev) failed\n");
		usb_detach_device (controller, adr);
		return NULL;
	}
	dev->endpoints[0].maxpacketsize = usb_decode_mps0(speed, buf[7]);

	return dev;
}

static int set_address(UsbDevHc *controller, UsbSpeed speed,
		       int hubport, int hubaddr)
{
	UsbDev *dev = controller->set_address(controller, speed,
					      hubport, hubaddr);
	if (!dev) {
		usb_debug("set_address failed\n");
		return -1;
	}

	dev->descriptor = malloc(sizeof(*dev->descriptor));
	if (!dev->descriptor || usb_get_descriptor(dev, DR_DESC,
						   UsbDescTypeDev, 0,
						   dev->descriptor,
						   sizeof(*dev->descriptor)) !=
	    sizeof(*dev->descriptor)) {
		usb_debug("usb_get_descriptor(UsbDescTypeDev) failed\n");
		usb_detach_device(controller, dev->address);
		return -1;
	}

	usb_debug("* found device (0x%04x:0x%04x, USB %x.%x, MPS0: %d)\n",
		 dev->descriptor->idVendor, dev->descriptor->idProduct,
		 dev->descriptor->bcdUSB >> 8, dev->descriptor->bcdUSB & 0xff,
		 dev->endpoints[0].maxpacketsize);
	dev->quirks = usb_quirk_check(dev->descriptor->idVendor,
				      dev->descriptor->idProduct);

	usb_debug("device has %d configurations\n",
		  dev->descriptor->bNumConfigurations);
	if (dev->descriptor->bNumConfigurations == 0) {
		// Device isn't usable.
		usb_debug("... no usable configuration!\n");
		usb_detach_device (controller, dev->address);
		return -1;
	}

	uint16_t buf[2];
	if (usb_get_descriptor(dev, DR_DESC, UsbDescTypeCfg, 0, buf,
			       sizeof(buf)) != sizeof(buf)) {
		usb_debug("first usb_get_descriptor(UsbDescTypeCfg) failed\n");
		usb_detach_device (controller, dev->address);
		return -1;
	}
	// Workaround for some USB devices: wait until they're ready, or
	// they send a NAK when they're not allowed to do. 1ms is enough.
	mdelay(1);
	dev->configuration = malloc(buf[1]);
	if (!dev->configuration) {
		usb_debug("could not allocate %d bytes for UsbDescTypeCfg\n",
			  buf[1]);
		usb_detach_device(controller, dev->address);
		return -1;
	}
	if (usb_get_descriptor(dev, DR_DESC, UsbDescTypeCfg, 0,
			       dev->configuration, buf[1]) != buf[1]) {
		usb_debug("usb_get_descriptor(UsbDescTypeCfg) failed\n");
		usb_detach_device(controller, dev->address);
		return -1;
	}
	UsbConfigurationDescriptor *cd = dev->configuration;
	if (cd->wTotalLength != buf[1]) {
		usb_debug("configuration descriptor size changed, aborting\n");
		usb_detach_device(controller, dev->address);
		return -1;
	}

	/*
	 * If the device is not well known (ifnum == -1), we use the first
	 * interface we encounter, as there was no need to implement something
	 * else for the time being. If you need it, see the SetInterface and
	 * GetInterface functions in the USB specification and set it yourself.
	 */
	usb_debug("device has %x interfaces\n", cd->bNumInterfaces);
	int ifnum = usb_interface_check(dev->descriptor->idVendor,
					dev->descriptor->idProduct);
	if (cd->bNumInterfaces > 1 && ifnum < 0)
		usb_debug ("NOTICE: Your device has multiple interfaces and\n"
			   "this driver will only use the first one. That may\n"
			   "be the wrong choice and cause the device to not\n"
			   "work correctly. Please report this case\n"
			   "(including the above debugging output) to\n"
			   "coreboot@coreboot.org to have the device added to\n"
			   "the list of well-known quirks.\n");

	uint8_t *end = (void *)dev->configuration + cd->wTotalLength;
	UsbInterfaceDescriptor *intf;
	uint8_t *ptr;

	// Find our interface (or the first good one if we don't know).
	for (ptr = (void *)dev->configuration + sizeof(*cd); ; ptr += ptr[0]) {
		if (ptr + 2 > end || !ptr[0] || ptr + ptr[0] > end) {
			usb_debug("Couldn't find usable UsbDescTypeIntf\n");
			usb_detach_device(controller, dev->address);
			return -1;
		}
		if (ptr[1] != UsbDescTypeIntf)
			continue;
		intf = (void *)ptr;
		if (intf->bLength != sizeof(*intf)) {
			usb_debug("Skipping broken UsbDescTypeIntf\n");
			continue;
		}
		if (ifnum >= 0 && intf->bInterfaceNumber != ifnum)
			continue;
		usb_debug("Interface %d: class 0x%x, sub 0x%x. proto 0x%x\n",
			intf->bInterfaceNumber, intf->bInterfaceClass,
			intf->bInterfaceSubClass, intf->bInterfaceProtocol);
		ptr += sizeof(*intf);
		break;
	}

	// Gather up all endpoints belonging to this inteface.
	dev->num_endp = 1;
	for (; ptr + 2 <= end && ptr[0] && ptr + ptr[0] <= end; ptr += ptr[0]) {
		if (ptr[1] == UsbDescTypeIntf || ptr[1] == UsbDescTypeCfg ||
				dev->num_endp >= ARRAY_SIZE(dev->endpoints))
			break;
		if (ptr[1] != UsbDescTypeEndp)
			continue;

		UsbEndpointDescriptor *desc = (void *)ptr;
		static const char *transfertypes[4] = {
			"control", "isochronous", "bulk", "interrupt"
		};
		usb_debug(" #Endpoint %d (%s), max packet size %x, type %s\n",
			desc->bEndpointAddress & 0x7f,
			(desc->bEndpointAddress & 0x80) ? "in" : "out",
			desc->wMaxPacketSize,
			transfertypes[desc->bmAttributes & 0x3]);

		UsbEndpoint *ep = &dev->endpoints[dev->num_endp++];
		ep->dev = dev;
		ep->endpoint = desc->bEndpointAddress;
		ep->toggle = 0;
		ep->maxpacketsize = desc->wMaxPacketSize;
		ep->direction = (desc->bEndpointAddress & 0x80) ?
				UsbDirIn : UsbDirOut;
		ep->type = desc->bmAttributes & 0x3;
		ep->interval = usb_decode_interval(dev->speed, ep->type,
						   desc->bInterval);
	}

	if ((controller->finish_device_config &&
			controller->finish_device_config(dev)) ||
			usb_set_configuration(dev) < 0) {
		usb_debug("Could not finalize device configuration\n");
		usb_detach_device(controller, dev->address);
		return -1;
	}

	int class = dev->descriptor->bDeviceClass;
	if (class == 0)
		class = intf->bInterfaceClass;

	enum {
		audio_device      = 0x01,
		comm_device       = 0x02,
		hid_device        = 0x03,
		physical_device   = 0x05,
		imaging_device    = 0x06,
		printer_device    = 0x07,
		msc_device        = 0x08,
		hub_device        = 0x09,
		cdc_device        = 0x0a,
		ccid_device       = 0x0b,
		security_device   = 0x0d,
		video_device      = 0x0e,
		healthcare_device = 0x0f,
		diagnostic_device = 0xdc,
		wireless_device   = 0xe0,
		misc_device       = 0xef,
	};
	usb_debug("Class: ");
	switch (class) {
	case audio_device:
		usb_debug("audio\n");
		break;
	case comm_device:
		usb_debug("communication\n");
		break;
	case hid_device:
		usb_debug("HID\n");
#if CONFIG_DRIVER_USB_HID
		dev->init = usb_hid_init;
		return dev->address;
#else
		usb_debug("NOTICE: USB HID support not compiled in\n");
#endif
		break;
	case physical_device:
		usb_debug("physical\n");
		break;
	case imaging_device:
		usb_debug("camera\n");
		break;
	case printer_device:
		usb_debug("printer\n");
		break;
	case msc_device:
		usb_debug("MSC\n");
#if CONFIG_USB_MSC
		dev->init = usb_msc_init;
		return dev->address;
#else
		usb_debug("NOTICE: USB MSC support not compiled in\n");
#endif
		break;
	case hub_device:
		usb_debug("hub\n");
#if CONFIG_USB_HUB
		dev->init = usb_hub_init;
		return dev->address;
#else
		usb_debug("NOTICE: USB hub support not compiled in\n");
#endif
		break;
	case cdc_device:
		usb_debug("CDC\n");
		break;
	case ccid_device:
		usb_debug("smartcard / CCID\n");
		break;
	case security_device:
		usb_debug("content security\n");
		break;
	case video_device:
		usb_debug("video\n");
		break;
	case healthcare_device:
		usb_debug("healthcare\n");
		break;
	case diagnostic_device:
		usb_debug("diagnostic\n");
		break;
	case wireless_device:
		usb_debug("wireless\n");
		break;
	default:
		usb_debug("unsupported class %x\n", class);
		break;
	}
	dev->init = usb_generic_init;
	return dev->address;
}

/*
 * Should be called by the hub drivers whenever a physical detach occurs
 * and can be called by usb class drivers if they are unsatisfied with a
 * malfunctioning device.
 */
void usb_detach_device(UsbDevHc *controller, int devno)
{
	// Check if device exists, as we may have been called yet by the
	// usb class driver.
	if (controller->devices[devno]) {
		controller->devices[devno]->destroy(controller->devices[devno]);
		if (controller->destroy_device)
			controller->destroy_device(controller, devno);
		// Tear down the device itself *after* destroy_device()
		// has had a chance to interoogate it.
		free(controller->devices[devno]);
		controller->devices[devno] = NULL;
	}
}

int usb_attach_device(UsbDevHc *controller, int hubaddress, int port,
		      UsbSpeed speed)
{
	static const char* speeds[] = { "full", "low", "high", "super" };
	usb_debug("%sspeed device\n", (speed < sizeof(speeds) / sizeof(char*))
		? speeds[speed] : "invalid value - no");
	int newdev = set_address(controller, speed, port, hubaddress);
	if (newdev == -1)
		return -1;
	UsbDev *newdev_t = controller->devices[newdev];
	// Determine responsible driver - currently done in set_address.
	newdev_t->init(newdev_t);
	// init() may have called usb_detach_device() yet, so check.
	return controller->devices[newdev] ? newdev : -1;
}

static void usb_generic_destroy(UsbDev *dev)
{
	if (usb_generic_remove)
		usb_generic_remove(dev);
}

void usb_generic_init(UsbDev *dev)
{
	dev->data = NULL;
	dev->destroy = usb_generic_destroy;

	if (usb_generic_create)
		usb_generic_create(dev);

	if (dev->data == NULL) {
		usb_debug("Detaching device not used by payload\n");
		usb_detach_device(dev->controller, dev->address);
	}
}

// Returns the address of the closest USB2.0 hub, which is responsible for
// split transactions, along with the number of the used downstream port.
int usb_closest_usb2_hub(const UsbDev *dev, int *const addr, int *const port)
{
	const UsbDev *usb1dev;

	do {
		usb1dev = dev;
		if ((dev->hub >= 0) && (dev->hub < 128))
			dev = dev->controller->devices[dev->hub];
		else
			dev = NULL;
	} while (dev && (dev->speed < 2));

	if (dev) {
		*addr = usb1dev->hub;
		*port = usb1dev->port;
		return 0;
	}

	usb_debug("Couldn't find closest USB2.0 hub.\n");
	return 1;
}
