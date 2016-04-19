/*
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
	UsbHostToDevice = 0,
	UsbDeviceToHost = 1
} UsbDevReqDir;

typedef enum {
	UsbStandardType = 0,
	UsbClassType = 1,
	UsbVendorType = 2,
	UsbReservedType = 3
} UsbDevReqType;

typedef enum {
	UsbDevRecp = 0,
	UsbIfaceRecp = 1,
	UsbEndpRecp = 2,
	UsbOtherRecp = 3
} UsbDevReqRecp;

enum {
	UsbDescTypeDev = 1,
	UsbDescTypeCfg = 2,
	UsbDescTypeStr = 3,
	UsbDescTypeIntf = 4,
	UsbDescTypeEndp = 5,
};

typedef enum {
	UsbReqGetStatus = 0,
	UsbReqClearFeature = 1,
	UsbReqSetFeature = 3,
	UsbReqSetAddress = 5,
	UsbReqGetDescriptor = 6,
	UsbReqSetDescriptor = 7,
	UsbReqGetConfiguration = 8,
	UsbReqSetConfiguration = 9,
	UsbReqGetInterface = 10,
	UsbReqSetInterface = 11,
	UsbReqSynchFrame = 12
} bRequestCodes;

// Feature selectors.
enum {
	UsbEndpointHalt = 0,
	UsbDeviceRemoteWakeup = 1,
	UsbTestMode = 2
};

// SetAddress() recovery interval (USB 2.0 specification 9.2.6.3).
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
} __attribute__ ((packed)) UsbHubDescriptor;

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
} __attribute__ ((packed)) UsbDeviceDescriptor;

typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t bMaxPower;
} __attribute__ ((packed)) UsbConfigurationDescriptor;

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
} __attribute__ ((packed)) UsbInterfaceDescriptor;

typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress;
	uint8_t bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t bInterval;
} __attribute__ ((packed)) UsbEndpointDescriptor;

typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdHID;
	uint8_t bCountryCode;
	uint8_t bNumDescriptors;
	uint8_t bReportDescriptorType;
	uint16_t wReportDescriptorLength;
} __attribute__ ((packed)) UsbHidDescriptor;

typedef struct {
	union {
		struct {
			UsbDevReqRecp req_recp:5;
			UsbDevReqType req_type:2;
			UsbDevReqDir data_dir:1;
		} __attribute__ ((packed));
		uint8_t bmRequestType;
	} __attribute__ ((packed));
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
} __attribute__ ((packed)) UsbDevReq;

struct UsbDevHc;
typedef struct UsbDevHc UsbDevHc;

struct UsbDev;
typedef struct UsbDev UsbDev;

typedef enum {
	UsbDirSetup,
	UsbDirIn,
	UsbDirOut
} UsbDirection;

typedef enum {
	UsbEndpTypeControl = 0,
	UsbEndpTypeIsochronous = 1,
	UsbEndpTypeBulk = 2,
	UsbEndpTypeInterrupt = 3
} UsbEndpointType;

typedef struct {
	UsbDev *dev;
	int endpoint;
	UsbDirection direction;
	int toggle;
	int maxpacketsize;
	UsbEndpointType type;
	int interval; // Expressed as binary logarithm of the number
		      // of microframes (i.e. t = 125us * 2 ^ interval).
} UsbEndpoint;

typedef enum {
	UsbFullSpeed = 0,
	UsbLowSpeed = 1,
	UsbHighSpeed = 2,
	UsbSuperSpeed = 3,
} UsbSpeed;

struct UsbDev {
	UsbDevHc *controller;
	UsbEndpoint endpoints[32];
	int num_endp;
	int address;		// usb address
	int hub;		// hub, device is attached to
	int port;		// port where device is attached
	UsbSpeed speed;
	uint32_t quirks;	// quirks field. got to love usb
	void *data;
	UsbDeviceDescriptor *descriptor;
	UsbConfigurationDescriptor *configuration;
	void (*init)(UsbDev *dev);
	void (*destroy)(UsbDev *dev);
	void (*poll)(UsbDev *dev);
};

typedef enum {
	UsbOhci = 0,
	UsbUhci = 1,
	UsbEhci = 2,
	UsbXhci = 3,
	UsbDwc2 = 4
} UsbHcType;

struct UsbDevHc {
	UsbDevHc *next;
	uintptr_t reg_base;
	pcidev_t pcidev; // 0 if not used (eg on ARM)
	UsbHcType type;
	int latest_address;
	UsbDev *devices[128];	// dev 0 is root hub, 127 is last addressable

	/* start():     Resume operation. */
	void (*start)(UsbDevHc *controller);
	/* stop():      Stop operation but keep controller initialized. */
	void (*stop)(UsbDevHc *controller);
	/* reset():     Perform a controller reset. The controller needs to
	                be (re)initialized afterwards to work (again). */
	void (*reset)(UsbDevHc *controller);
	/* init():      Initialize a (previously reset) controller
	                to a working state. */
	void (*init)(UsbDevHc *controller);
	/* shutdown():  Stop operation, detach host controller and shutdown
	                this driver instance. After calling shutdown() any
			other usage of this UsbDevHc* is invalid. */
	void (*shutdown)(UsbDevHc *controller);

	int (*bulk)(UsbEndpoint *ep, int size, uint8_t *data, int finalize);
	int (*control)(UsbDev *dev, UsbDirection pid, int dr_length,
		       void *devreq, int data_length, uint8_t *data);
	void* (*create_intr_queue)(UsbEndpoint *ep, int reqsize, int reqcount,
				   int reqtiming);
	void (*destroy_intr_queue)(UsbEndpoint *ep, void *queue);
	uint8_t* (*poll_intr_queue)(void *queue);
	void *instance;

	/* set_address():		Tell the usb device its address (xHCI
					controllers want to do this by
					themselves). Also, allocate the usbdev
					structure, initialize enpoint 0
					(including MPS) and return it. */
	UsbDev *(*set_address)(UsbDevHc *controller, UsbSpeed speed,
				 int hubport, int hubaddr);
	/* finish_device_config():	Another hook for xHCI,
					returns 0 on success. */
	int (*finish_device_config)(UsbDev *dev);
	/* destroy_device():		Finally, destroy all structures that
					were allocated during set_address()
					and finish_device_config(). */
	void (*destroy_device)(UsbDevHc *controller, int devaddr);
};

UsbDevHc *usb_add_mmio_hc(UsbHcType type, void *bar);
UsbDevHc *new_controller(void);
void detach_controller(UsbDevHc *controller);
void usb_poll(void);
UsbDev *init_device_entry(UsbDevHc *controller, int num);

int usb_decode_mps0(UsbSpeed speed, uint8_t bMaxPacketSize0);
int usb_set_feature(UsbDev *dev, int endp, int feature, int rtype);
int usb_get_status(UsbDev *dev, int endp, int rtype, int len, void *data);
int usb_get_descriptor(UsbDev *dev, int rtype, int descType, int descIdx,
		       void *data, size_t len);
int usb_set_configuration(UsbDev *dev);
int usb_clear_feature(UsbDev *dev, int endp, int feature, int rtype);
int usb_clear_stall(UsbEndpoint *ep);

void usb_nop_init(UsbDev *dev);
void usb_hub_init(UsbDev *dev);
void usb_hid_init(UsbDev *dev);
void usb_msc_init(UsbDev *dev);
void usb_generic_init(UsbDev *dev);

int usb_closest_usb2_hub(const UsbDev *dev, int *const addr, int *const port);

static inline unsigned char usb_gen_bmRequestType(UsbDevReqDir dir,
						  UsbDevReqType type,
						  UsbDevReqRecp recp)
{
	return (dir << 7) | (type << 5) | recp;
}

// Default "set address" handler.
UsbDev *usb_generic_set_address(UsbDevHc *controller, UsbSpeed speed,
				int hubport, int hubaddr);

void usb_detach_device(UsbDevHc *controller, int devno);
int usb_attach_device(UsbDevHc *controller, int hubaddress, int port,
		      UsbSpeed speed);

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
void __attribute__((weak)) usb_generic_create(UsbDev *dev);

/**
 * To be implemented by libpayload-client. It's called by the USB stack
 * when it finds out that a USB device is removed which wasn't claimed by a
 * built in driver.
 *
 * @param dev descriptor for the USB device
 */
void __attribute__((weak)) usb_generic_remove(UsbDev *dev);

#endif
