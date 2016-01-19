/*
 * This file is part of the libpayload project.
 *
 * Copyright (C) 2008 coresystems GmbH
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

#ifndef __USB_H
#define __USB_H

#include <libpayload.h>
#include <pci.h>
#include <stdint.h>

typedef enum {
	host_to_device = 0,
	device_to_host = 1
} dev_req_dir;

typedef enum {
	standard_type = 0,
	class_type = 1,
	vendor_type = 2,
	reserved_type = 3
} dev_req_type;

typedef enum {
	dev_recp = 0,
	iface_recp = 1,
	endp_recp = 2,
	other_recp = 3
} dev_req_recp;

enum {
	DT_DEV = 1,
	DT_CFG = 2,
	DT_STR = 3,
	DT_INTF = 4,
	DT_ENDP = 5,
};

typedef enum {
	GET_STATUS = 0,
	CLEAR_FEATURE = 1,
	SET_FEATURE = 3,
	SET_ADDRESS = 5,
	GET_DESCRIPTOR = 6,
	SET_DESCRIPTOR = 7,
	GET_CONFIGURATION = 8,
	SET_CONFIGURATION = 9,
	GET_INTERFACE = 10,
	SET_INTERFACE = 11,
	SYNCH_FRAME = 12
} bRequest_Codes;

typedef enum {
	ENDPOINT_HALT = 0,
	DEVICE_REMOTE_WAKEUP = 1,
	TEST_MODE = 2
} feature_selectors;

/* SetAddress() recovery interval (USB 2.0 specification 9.2.6.3 */
#define SET_ADDRESS_MDELAY 2

typedef struct {
	uint8_t bDescLength;
	uint8_t bDescriptorType;
	uint8_t bNbrPorts;
	union {
		struct {
			uint32_t logicalPowerSwitchingMode:2;
			uint32_t isCompoundDevice:1;
			uint32_t overcurrentProtectionMode:2;
			uint32_t ttThinkTime:2;
			uint32_t arePortIndicatorsSupported:1;
			uint32_t:8;
		} __attribute__ ((packed));
		uint16_t wHubCharacteristics;
	} __attribute__ ((packed));
	uint8_t bPowerOn2PwrGood;
	uint8_t bHubContrCurrent;
} __attribute__ ((packed)) hub_descriptor_t;

typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t iManufacturer;
	uint8_t iProduct;
	uint8_t iSerialNumber;
	uint8_t bNumConfigurations;
} __attribute__ ((packed)) device_descriptor_t;

typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t bMaxPower;
} __attribute__ ((packed)) configuration_descriptor_t;

typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
} __attribute__ ((packed)) interface_descriptor_t;

typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress;
	uint8_t bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t bInterval;
} __attribute__ ((packed)) endpoint_descriptor_t;

typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdHID;
	uint8_t bCountryCode;
	uint8_t bNumDescriptors;
	uint8_t bReportDescriptorType;
	uint16_t wReportDescriptorLength;
} __attribute__ ((packed)) hid_descriptor_t;

typedef struct {
	union {
		struct {
			dev_req_recp req_recp:5;
			dev_req_type req_type:2;
			dev_req_dir data_dir:1;
		} __attribute__ ((packed));
		uint8_t bmRequestType;
	} __attribute__ ((packed));
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
} __attribute__ ((packed)) dev_req_t;

struct usbdev_hc;
typedef struct usbdev_hc hci_t;

struct usbdev;
typedef struct usbdev usbdev_t;

typedef enum {
	SETUP,
	IN,
	OUT
} direction_t;

typedef enum {
	CONTROL = 0,
	ISOCHRONOUS = 1,
	BULK = 2,
	INTERRUPT = 3
} endpoint_type;

typedef struct {
	usbdev_t *dev;
	int endpoint;
	direction_t direction;
	int toggle;
	int maxpacketsize;
	endpoint_type type;
	int interval; /* expressed as binary logarithm of the number
			 of microframes (i.e. t = 125us * 2^interval) */
} endpoint_t;

typedef enum {
	FULL_SPEED = 0,
	LOW_SPEED = 1,
	HIGH_SPEED = 2,
	SUPER_SPEED = 3,
} usb_speed;

struct usbdev {
	hci_t *controller;
	endpoint_t endpoints[32];
	int num_endp;
	int address;		// usb address
	int hub;		// hub, device is attached to
	int port;		// port where device is attached
	usb_speed speed;
	uint32_t quirks;	// quirks field. got to love usb
	void *data;
	device_descriptor_t *descriptor;
	configuration_descriptor_t *configuration;
	void (*init)(usbdev_t *dev);
	void (*destroy)(usbdev_t *dev);
	void (*poll)(usbdev_t *dev);
};

typedef enum {
	OHCI = 0,
	UHCI = 1,
	EHCI = 2,
	XHCI = 3,
	DWC2 = 4
} hc_type;

struct usbdev_hc {
	hci_t *next;
	uintptr_t reg_base;
	pcidev_t pcidev; // 0 if not used (eg on ARM)
	hc_type type;
	int latest_address;
	usbdev_t *devices[128];	// dev 0 is root hub, 127 is last addressable

	/* start():     Resume operation. */
	void (*start)(hci_t *controller);
	/* stop():      Stop operation but keep controller initialized. */
	void (*stop)(hci_t *controller);
	/* reset():     Perform a controller reset. The controller needs to
	                be (re)initialized afterwards to work (again). */
	void (*reset)(hci_t *controller);
	/* init():      Initialize a (previously reset) controller
	                to a working state. */
	void (*init)(hci_t *controller);
	/* shutdown():  Stop operation, detach host controller and shutdown
	                this driver instance. After calling shutdown() any
			other usage of this hci_t* is invalid. */
	void (*shutdown)(hci_t *controller);

	int (*bulk)(endpoint_t *ep, int size, uint8_t *data, int finalize);
	int (*control)(usbdev_t *dev, direction_t pid, int dr_length,
		       void *devreq, int data_length, uint8_t *data);
	void* (*create_intr_queue)(endpoint_t *ep, int reqsize, int reqcount,
				   int reqtiming);
	void (*destroy_intr_queue)(endpoint_t *ep, void *queue);
	uint8_t* (*poll_intr_queue)(void *queue);
	void *instance;

	/* set_address():		Tell the usb device its address (xHCI
					controllers want to do this by
					themselves). Also, allocate the usbdev
					structure, initialize enpoint 0
					(including MPS) and return it. */
	usbdev_t *(*set_address)(hci_t *controller, usb_speed speed,
				 int hubport, int hubaddr);
	/* finish_device_config():	Another hook for xHCI,
					returns 0 on success. */
	int (*finish_device_config)(usbdev_t *dev);
	/* destroy_device():		Finally, destroy all structures that
					were allocated during set_address()
					and finish_device_config(). */
	void (*destroy_device)(hci_t *controller, int devaddr);
};

hci_t *usb_add_mmio_hc(hc_type type, void *bar);
hci_t *new_controller(void);
void detach_controller(hci_t *controller);
void usb_poll(void);
usbdev_t *init_device_entry(hci_t *controller, int num);

int usb_decode_mps0(usb_speed speed, uint8_t bMaxPacketSize0);
int set_feature(usbdev_t *dev, int endp, int feature, int rtype);
int get_status(usbdev_t *dev, int endp, int rtype, int len, void *data);
int get_descriptor(usbdev_t *dev, int rtype, int descType, int descIdx,
		   void *data, size_t len);
int set_configuration(usbdev_t *dev);
int clear_feature(usbdev_t *dev, int endp, int feature, int rtype);
int clear_stall(endpoint_t *ep);

void usb_nop_init(usbdev_t *dev);
void usb_hub_init(usbdev_t *dev);
void usb_hid_init(usbdev_t *dev);
void usb_msc_init(usbdev_t *dev);
void usb_generic_init(usbdev_t *dev);

int closest_usb2_hub(const usbdev_t *dev, int *const addr, int *const port);

static inline unsigned char gen_bmRequestType(dev_req_dir dir,
					      dev_req_type type,
					      dev_req_recp recp)
{
	return (dir << 7) | (type << 5) | recp;
}

// Default "set address" handler.
usbdev_t *generic_set_address(hci_t *controller, usb_speed speed,
			      int hubport, int hubaddr);

void usb_detach_device(hci_t *controller, int devno);
int usb_attach_device(hci_t *controller, int hubaddress, int port,
		      usb_speed speed);

uint32_t usb_quirk_check(uint16_t vendor, uint16_t device);
int usb_interface_check(uint16_t vendor, uint16_t device);

enum {
	USB_QUIRK_MSC_FORCE_PROTO_SCSI = 1 << 0,
	USB_QUIRK_MSC_FORCE_PROTO_ATAPI = 1 << 1,
	USB_QUIRK_MSC_FORCE_PROTO_UFI = 1 << 2,
	USB_QUIRK_MSC_FORCE_PROTO_RBC = 1 << 3,
	USB_QUIRK_MSC_FORCE_TRANS_BBB = 1 << 4,
	USB_QUIRK_MSC_FORCE_TRANS_CBI = 1 << 5,
	USB_QUIRK_MSC_FORCE_TRANS_CBI_I = 1 << 6,
	USB_QUIRK_MSC_NO_TEST_UNIT_READY = 1 << 7,
	USB_QUIRK_MSC_SHORT_INQUIRY = 1 << 8,
	USB_QUIRK_TEST = 1 << 31,
	USB_QUIRK_NONE = 0
};

static inline void usb_debug(const char *fmt, ...)
{
#ifdef USB_DEBUG
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
#endif
}

/**
 * To be implemented by libpayload-client. It's called by the USB stack
 * when a new USB device is found which isn't claimed by a built in driver,
 * so the client has the chance to know about it.
 *
 * @param dev descriptor for the USB device
 */
void __attribute__((weak)) usb_generic_create(usbdev_t *dev);

/**
 * To be implemented by libpayload-client. It's called by the USB stack
 * when it finds out that a USB device is removed which wasn't claimed by a
 * built in driver.
 *
 * @param dev descriptor for the USB device
 */
void __attribute__((weak)) usb_generic_remove(usbdev_t *dev);

#endif
