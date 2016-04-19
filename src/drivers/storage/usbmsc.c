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

#include <endian.h>
#include <stdint.h>
#include <usb/usb.h>

#include "base/die.h"
#include "drivers/storage/usbdisk.h"
#include "drivers/storage/usbmsc.h"
#include "drivers/timer/timer.h"

enum {
	msc_subclass_rbc = 0x1,
	msc_subclass_mmc2 = 0x2,
	msc_subclass_qic157 = 0x3,
	msc_subclass_ufi = 0x4,
	msc_subclass_sff8070i = 0x5,
	msc_subclass_scsitrans = 0x6
};

static const char *decode_subclass_desc(uint8_t subclass)
{
	switch (subclass) {
		case 0x0: return "(none)";
		case 0x1: return "RBC";
		case 0x2: return "MMC-2";
		case 0x3: return "QIC-157";
		case 0x4: return "UFI";
		case 0x5: return "SFF-8070i";
		case 0x6: return "SCSI transparent";
		default: return NULL;
	}
};

enum {
	msc_proto_cbi_wcomp = 0x0,
	msc_proto_cbi_wocomp = 0x1,
	msc_proto_bulk_only = 0x50
};

static const char *decode_protocol_desc(uint8_t protocol)
{
	switch (protocol) {
	case 0x00:
		return "Control/Bulk/Interrupt protocol "
		       "(with command completion interrupt)";
	case 0x01:
		return "Control/Bulk/Interrupt protocol "
		       "(with no command completion interrupt)";
	case 0x51:
		return "Bulk-Only Transport";
	default:
		return NULL;
	}
};

static void usb_msc_create_disk(UsbDev *dev)
{
	if (usbdisk_create) {
		usbdisk_create(dev);
		MSC_INST(dev)->usbdisk_created = 1;
	}
}

static void usb_msc_remove_disk(UsbDev *dev)
{
	if (MSC_INST(dev)->usbdisk_created && usbdisk_remove)
		usbdisk_remove(dev);
}

static void usb_msc_destroy(UsbDev *dev)
{
	if (dev->data) {
		usb_msc_remove_disk(dev);
		free(dev->data);
	}
	dev->data = 0;
}

// Many USB3 devices do not work with large transfer requests.
// Limit the request size to 64KB chunks to ensure maximum compatibility.
static const int MAX_CHUNK_BYTES = 1024 * 64;

typedef struct {
	uint32_t dCBWSignature;
	uint32_t dCBWTag;
	uint32_t dCBWDataTransferLength;
	uint8_t bmCBWFlags;
	uint32_t bCBWLUN:4;
	uint32_t:4;
	uint32_t bCBWCBLength:5;
	uint32_t:3;
	uint8_t CBWCB[31 - 15];
} __attribute__((packed)) cbw_t;

typedef struct {
	uint32_t dCSWSignature;
	uint32_t dCSWTag;
	uint32_t dCSWDataResidue;
	uint8_t bCSWStatus;
} __attribute__((packed)) csw_t;

enum {
	/*
	 * MSC commands can be
	 *   successful,
	 *   fail with proper response or
	 *   fail totally, which results in detaching of the usb device
	 *   and immediate cleanup of the UsbDev structure.
	 * In the latter case the caller has to make sure that they won't
	 * use the device any more.
	 */
	MSC_COMMAND_OK,
	MSC_COMMAND_FAIL,
	MSC_COMMAND_DETACHED
};

static int request_sense(UsbDev *dev);
static int request_sense_no_media(UsbDev *dev);
static void usb_msc_poll(UsbDev *dev);

static int reset_transport(UsbDev *dev)
{
	UsbDevReq dr;
	memset(&dr, 0, sizeof (dr));
	dr.bmRequestType = 0;
	dr.data_dir = UsbHostToDevice;
	dr.req_type = UsbClassType;
	dr.req_recp = UsbIfaceRecp;
	dr.bRequest = 0xff; // Device reset.
	dr.wValue = 0;
	dr.wIndex = 0;
	dr.wLength = 0;

	// If any of these fails, detach device, as we are lost.
	if (dev->controller->control(dev, UsbDirOut, sizeof(dr),
				     &dr, 0, 0) < 0 ||
			usb_clear_stall(MSC_INST(dev)->bulk_in) ||
			usb_clear_stall(MSC_INST(dev)->bulk_out)) {
		usb_debug("Detaching unresponsive device.\n");
		usb_detach_device(dev->controller, dev->address);
		return MSC_COMMAND_DETACHED;
	}
	// Return fail as we are only called in case of failure.
	return MSC_COMMAND_FAIL;
}

// Device may stall this command, so beware!
static void initialize_luns(UsbDev *dev)
{
	usbmsc_inst_t *msc = MSC_INST(dev);
	UsbDevReq dr;
	dr.bmRequestType = 0;
	dr.data_dir = UsbDeviceToHost;
	dr.req_type = UsbClassType;
	dr.req_recp = UsbIfaceRecp;
	dr.bRequest = 0xfe; // Get the maximum LUN.
	dr.wValue = 0;
	dr.wIndex = 0;
	dr.wLength = 1;
	if (dev->controller->control(dev, UsbDirIn, sizeof(dr), &dr,
			sizeof(msc->num_luns), &msc->num_luns) < 0)
		msc->num_luns = 0;	// Assume only 1 lun if req fails.
	msc->num_luns++;	// Get Max LUN returns number of last LUN.
	msc->lun = 0;
}

static unsigned int usbmsc_tag;

static void wrap_cbw(cbw_t *cbw, int datalen, cbw_direction dir,
		     const uint8_t *cmd, int cmdlen, uint8_t lun)
{
	memset(cbw, 0, sizeof(cbw_t));

	// Commands are typically shorter, but we don't want overflows.
	if (cmdlen > sizeof(cbw->CBWCB))
		cmdlen = sizeof(cbw->CBWCB);

	cbw->dCBWSignature = 0x43425355;
	cbw->dCBWTag = ++usbmsc_tag;
	cbw->bCBWLUN = lun;

	cbw->dCBWDataTransferLength = datalen;
	cbw->bmCBWFlags = dir;
	memcpy(cbw->CBWCB, cmd, cmdlen);
	cbw->bCBWCBLength = cmdlen;
}

static int get_csw(UsbEndpoint *ep, csw_t *csw)
{
	if (ep->dev->controller->bulk(ep, sizeof(csw_t),
				      (uint8_t *)csw, 1) < 0) {
		usb_clear_stall(ep);
		if (ep->dev->controller->bulk(ep, sizeof(csw_t),
					      (uint8_t *)csw, 1) < 0)
			return reset_transport(ep->dev);
	}
	if (csw->dCSWTag != usbmsc_tag)
		return reset_transport(ep->dev);
	return MSC_COMMAND_OK;
}

static int execute_command(UsbDev *dev, cbw_direction dir, const uint8_t *cb,
			   int cblen, uint8_t *buf, int buflen, int residue_ok)
{
	cbw_t cbw;
	csw_t csw;

	int always_succeed = 0;
	if ((cb[0] == 0x1b) && (cb[4] == 1)) // Start command, always succeed.
		always_succeed = 1;

	wrap_cbw(&cbw, buflen, dir, cb, cblen, MSC_INST(dev)->lun);
	if (dev->controller->bulk(MSC_INST(dev)->bulk_out, sizeof(cbw),
			(uint8_t *)&cbw, 0) < 0) {
		return reset_transport(dev);
	}
	if (buflen > 0) {
		if (dir == cbw_direction_data_in) {
			if (dev->controller->bulk(MSC_INST(dev)->bulk_in,
					buflen, buf, 0) < 0)
				usb_clear_stall(MSC_INST(dev)->bulk_in);
		} else {
			if (dev->controller->bulk(MSC_INST(dev)->bulk_out,
					buflen, buf, 0) < 0)
				usb_clear_stall(MSC_INST (dev)->bulk_out);
		}
	}

	int ret = get_csw(MSC_INST(dev)->bulk_in, &csw);
	if (ret) {
		return ret;
	} else if (always_succeed == 1) {
		// Return success, regardless of message.
		return MSC_COMMAND_OK;
	} else if (csw.bCSWStatus == 2) {
		// Phase error, reset transport.
		return reset_transport(dev);
	} else if (csw.bCSWStatus == 0) {
		if (csw.dCSWDataResidue == 0 || residue_ok)
			// No error, exit.
			return MSC_COMMAND_OK;
		else
			// Missed some bytes.
			return MSC_COMMAND_FAIL;
	} else {
		if (cb[0] == 0x03) {
			// Requesting sense failed, that's bad.
			return MSC_COMMAND_FAIL;
		} else if (cb[0] == 0) {
			// If command was TEST UNIT READY, determine if the
			// device is of removable type indicating no media
			// found.
			return request_sense_no_media(dev);
		}
		// Error "check condition" or reserved error.
		ret = request_sense(dev);
		// Return fail or the status of request_sense if it's worse.
		return ret ? ret : MSC_COMMAND_FAIL;
	}
}

typedef struct {
	uint8_t command;	//0
	uint8_t res1;		//1
	uint32_t block;		//2-5
	uint8_t res2;		//6
	uint16_t numblocks;	//7-8
	uint8_t control;	//9 - the block is 10 bytes long
} __attribute__ ((packed)) cmdblock_t;

typedef struct {
	uint8_t command;	//0
	uint8_t res1;		//1
	uint8_t res2;		//2
	uint8_t res3;		//3
	union {			//4
		struct {
			uint32_t start:1; // for START STOP UNIT
			uint32_t:7;
		};
		uint8_t length;		// for REQUEST SENSE
	};
	uint8_t control;	//5
} __attribute__ ((packed)) cmdblock6_t;

/**
 * Like readwrite_blocks, but for soft-sectors of 512b size. Converts the
 * start and count from 512b units.
 * Start and count must be aligned so that they match the native
 * sector size.
 *
 * @param dev device to access
 * @param start first sector to access
 * @param n number of sectors to access
 * @param dir direction of access: cbw_direction_data_in == read, cbw_direction_data_out == write
 * @param buf buffer to read into or write from. Must be at least n*512 bytes
 * @return 0 on success, 1 on failure
 */
int readwrite_blocks_512(UsbDev *dev, int start, int n,
			 cbw_direction dir, uint8_t *buf)
{
	int blocksize_divider = MSC_INST(dev)->blocksize / 512;
	return readwrite_blocks(dev, start / blocksize_divider,
		n / blocksize_divider, dir, buf);
}

/**
 * Reads or writes a number of sequential blocks on a USB storage device.
 * As it uses the READ(10) SCSI-2 command, it's limited to storage devices
 * of at most 2TB. It assumes sectors of 512 bytes.
 *
 * @param dev device to access
 * @param start first sector to access
 * @param n number of sectors to access
 * @param dir direction of access: cbw_direction_data_in == read, cbw_direction_data_out == write
 * @param buf buffer to read into or write from. Must be at least n*sectorsize bytes
 * @return 0 on success, 1 on failure
 */
static int readwrite_chunk(UsbDev *dev, int start, int n,
			   cbw_direction dir, uint8_t *buf)
{
	enum {
		READ_COMMAND = 0x28,
		WRITE_COMMAND = 0x2a
	};

	cmdblock_t cb;
	memset(&cb, 0, sizeof (cb));
	if (dir == cbw_direction_data_in)
		cb.command = READ_COMMAND;
	else
		cb.command = WRITE_COMMAND;
	cb.block = htonl(start);
	cb.numblocks = htonw(n);

	return execute_command(dev, dir, (uint8_t *)&cb, sizeof(cb), buf,
			       n * MSC_INST(dev)->blocksize, 0)
		!= MSC_COMMAND_OK ? 1 : 0;
}

/**
 * Reads or writes a number of sequential blocks on a USB storage device
 * that is split into MAX_CHUNK_BYTES size requests.
 *
 * As it uses the READ(10) SCSI-2 command, it's limited to storage devices
 * of at most 2TB. It assumes sectors of 512 bytes.
 *
 * @param dev device to access
 * @param start first sector to access
 * @param n number of sectors to access
 * @param dir direction of access: cbw_direction_data_in == read,
 *                                 cbw_direction_data_out == write
 * @param buf buffer to read into or write from.
 *            Must be at least n*sectorsize bytes
 * @return 0 on success, 1 on failure
 */
int readwrite_blocks(UsbDev *dev, int start, int n,
		     cbw_direction dir, uint8_t *buf)
{
	int chunk_size = MAX_CHUNK_BYTES / MSC_INST(dev)->blocksize;
	int chunk;

	// Read as many full chunks as needed.
	for (chunk = 0; chunk < (n / chunk_size); chunk++) {
		if (readwrite_chunk(dev, start + (chunk * chunk_size),
				    chunk_size, dir,
				    buf + (chunk * MAX_CHUNK_BYTES))
		    != MSC_COMMAND_OK)
			return 1;
	}

	// Read any remaining partial chunk at the end.
	if (n % chunk_size) {
		if (readwrite_chunk(dev, start + (chunk * chunk_size),
				    n % chunk_size, dir,
				    buf + (chunk * MAX_CHUNK_BYTES))
		    != MSC_COMMAND_OK)
			return 1;
	}

	return 0;
}

// Only request it, we don't interpret it.
// On certain errors, that's necessary to get devices out of
// a special state called "Contingent Allegiance Condition".
static int request_sense(UsbDev *dev)
{
	uint8_t buf[19];
	cmdblock6_t cb;
	memset(&cb, 0, sizeof (cb));
	cb.command = 0x3;
	cb.length = sizeof(buf);

	return execute_command(dev, cbw_direction_data_in, (uint8_t *)&cb,
			       sizeof(cb), buf, sizeof(buf), 1);
}

static int request_sense_no_media(UsbDev *dev)
{
	uint8_t buf[19];
	int ret;
	cmdblock6_t cb;
	memset(&cb, 0, sizeof(cb));
	cb.command = 0x3;
	cb.length = sizeof(buf);

	ret = execute_command(dev, cbw_direction_data_in, (uint8_t *)&cb,
			      sizeof(cb), buf, sizeof(buf), 1);

	if (ret)
		return ret;

	// Check if sense key is set to NOT READY.
	if ((buf[2] & 0xf) != 2)
		return MSC_COMMAND_FAIL;

	// Check if additional sense code is 0x3a.
	if (buf[12] != 0x3a)
		return MSC_COMMAND_FAIL;

	// No media is present. Return MSC_COMMAND_OK while marking the disk
	// not ready.
	usb_debug("Empty media found.\n");
	MSC_INST(dev)->ready = USB_MSC_NOT_READY;
	return MSC_COMMAND_OK;
}

static int test_unit_ready(UsbDev *dev)
{
	cmdblock6_t cb;
	memset(&cb, 0, sizeof (cb));	// Full initialization for T-U-R.
	return execute_command(dev, cbw_direction_data_out, (uint8_t *)&cb,
			       sizeof(cb), 0, 0, 0);
}

static int spin_up(UsbDev *dev)
{
	cmdblock6_t cb;
	memset(&cb, 0, sizeof(cb));
	cb.command = 0x1b;
	cb.start = 1;
	return execute_command(dev, cbw_direction_data_out, (uint8_t *)&cb,
			       sizeof(cb), 0, 0, 0);
}

static int read_capacity(UsbDev *dev)
{
	cmdblock_t cb;
	memset(&cb, 0, sizeof (cb));
	cb.command = 0x25;	// Read capacity
	uint32_t buf[2];

	usb_debug("Reading capacity of mass storage device.\n");
	int count = 0, ret;
	while (count++ < 20) {
		switch (ret = execute_command(dev, cbw_direction_data_in,
					      (uint8_t *)&cb, sizeof(cb),
					      (uint8_t *)buf, 8, 0)) {
		case MSC_COMMAND_OK:
			break;
		case MSC_COMMAND_FAIL:
			continue;
		default: // If it's worse, return.
			return ret;
		}
		break;
	}
	if (count >= 20) {
		// Still not successful, assume 2tb in 512byte sectors, which
		// is just the same garbage as any other number, but probably
		// more usable.
		usb_debug("  assuming 2 TB with 512-byte sectors as READ "
			  "CAPACITY didn't answer.\n");
		MSC_INST(dev)->numblocks = 0xffffffff;
		MSC_INST(dev)->blocksize = 512;
	} else {
		MSC_INST(dev)->numblocks = ntohl(buf[0]) + 1;
		MSC_INST(dev)->blocksize = ntohl(buf[1]);
	}
	usb_debug("  %d %d-byte sectors (%d MB)\n", MSC_INST(dev)->numblocks,
		MSC_INST(dev)->blocksize,
		// Round down high block counts to avoid integer overflow.
		MSC_INST(dev)->numblocks > 1000000 ?
			((MSC_INST(dev)->numblocks / 1000) *
			 MSC_INST(dev)->blocksize / 1000) :
			(MSC_INST(dev)->numblocks *
			 MSC_INST(dev)->blocksize / 1000 / 1000));
	return MSC_COMMAND_OK;
}

static int usb_msc_test_unit_ready(UsbDev *dev)
{
	// SCSI/ATA specs say we have to wait up to 30s, but most devices
	// are ready much sooner. Use a 5 sec timeout to better accomodate
	// devices which fail to respond.
	const int timeout_secs = 5;

	usb_debug("  Waiting for device to become ready...");

	// Initially mark the device ready.
	MSC_INST(dev)->ready = USB_MSC_READY;

	uint64_t start_time = timer_us(0);
	while (1) {
		if (timer_us(start_time) > timeout_secs * 1000000) {
			usb_debug("timeout. Device not ready.\n");
			MSC_INST(dev)->ready = USB_MSC_NOT_READY;
			// Don't bother spinning up the stroage device if the
			// device is not ready. This can happen when empty
			// card readers are present. Polling will pick it
			// back up if readiness changes.
			return 0;
		}

		switch (test_unit_ready(dev)) {
		case MSC_COMMAND_OK:
			break;
		case MSC_COMMAND_FAIL:
			mdelay(100);
			usb_debug(".");
			continue;
		default:
			// Device detached, return immediately.
			return USB_MSC_DETACHED;
		}
		break;
	}

	usb_debug("ok.\n");

	usb_debug("  spin up");
	for (int i = 0; i < 30; i++) {
		usb_debug(".");
		switch (spin_up(dev)) {
		case MSC_COMMAND_OK:
			usb_debug(" OK.");
			break;
		case MSC_COMMAND_FAIL:
			mdelay(100);
			continue;
		default:
			// Device detached, return immediately.
			return USB_MSC_DETACHED;
		}
		break;
	}
	usb_debug("\n");

	if (read_capacity(dev) == MSC_COMMAND_DETACHED)
		return USB_MSC_DETACHED;

	return MSC_INST(dev)->ready;
}

void usb_msc_init(UsbDev *dev)
{
	int i;

	// Init .data before setting .destroy.
	dev->data = NULL;

	dev->destroy = usb_msc_destroy;
	dev->poll = usb_msc_poll;

	UsbConfigurationDescriptor *cd =
		(UsbConfigurationDescriptor *)dev->configuration;
	UsbInterfaceDescriptor *interface =
		(UsbInterfaceDescriptor *)(((uint8_t *)cd) + cd->bLength);

	usb_debug("  it uses %s command set\n",
		decode_subclass_desc(interface->bInterfaceSubClass));
	usb_debug("  it uses %s protocol\n",
		decode_protocol_desc(interface->bInterfaceProtocol));


	if (interface->bInterfaceProtocol != 0x50) {
		usb_debug("  Protocol not supported.\n");
		usb_detach_device(dev->controller, dev->address);
		return;
	}

	if ((interface->bInterfaceSubClass != 2) &&	// ATAPI 8020
		(interface->bInterfaceSubClass != 5) &&	// ATAPI 8070
		(interface->bInterfaceSubClass != 6)) {	// SCSI
		// Other protocols such as ATAPI don't seem to be very
		// popular. Looks like ATAPI would be really easy to add,
		// if necessary.
		usb_debug("  Interface SubClass not supported.\n");
		usb_detach_device(dev->controller, dev->address);
		return;
	}

	dev->data = malloc(sizeof (usbmsc_inst_t));
	if (!dev->data)
		die("Not enough memory for USB MSC device.\n");

	MSC_INST(dev)->bulk_in = 0;
	MSC_INST(dev)->bulk_out = 0;
	MSC_INST(dev)->usbdisk_created = 0;

	for (i = 1; i <= dev->num_endp; i++) {
		if (dev->endpoints[i].endpoint == 0)
			continue;
		if (dev->endpoints[i].type != UsbEndpTypeBulk)
			continue;
		if ((dev->endpoints[i].direction == UsbDirIn) &&
		    (MSC_INST(dev)->bulk_in == 0))
			MSC_INST(dev)->bulk_in = &dev->endpoints[i];
		if ((dev->endpoints[i].direction == UsbDirOut) &&
		    (MSC_INST(dev)->bulk_out == 0))
			MSC_INST(dev)->bulk_out = &dev->endpoints[i];
	}

	if (MSC_INST(dev)->bulk_in == 0) {
		usb_debug("couldn't find bulk-in endpoint.\n");
		usb_detach_device(dev->controller, dev->address);
		return;
	}
	if (MSC_INST(dev)->bulk_out == 0) {
		usb_debug("couldn't find bulk-out endpoint.\n");
		usb_detach_device(dev->controller, dev->address);
		return;
	}
	usb_debug("  using endpoint %x as in, %x as out\n",
		MSC_INST(dev)->bulk_in->endpoint,
		MSC_INST(dev)->bulk_out->endpoint);

	// Some sticks need a little more time to get ready after SET_CONFIG.
	udelay(50);

	initialize_luns(dev);
	usb_debug("  has %d luns\n", MSC_INST(dev)->num_luns);

	// Test if unit is ready (nothing to do if it isn't).
	if (usb_msc_test_unit_ready(dev) != USB_MSC_READY)
		return;

	// Create the disk.
	usb_msc_create_disk(dev);
}

static void usb_msc_poll(UsbDev *dev)
{
	usbmsc_inst_t *msc = MSC_INST(dev);
	int prev_ready = msc->ready;

	if (usb_msc_test_unit_ready(dev) == USB_MSC_DETACHED)
		return;

	if (!prev_ready && msc->ready) {
		usb_debug("usb msc: not ready -> ready (lun %d)\n", msc->lun);
		usb_msc_create_disk(dev);
	} else if (prev_ready && !msc->ready) {
		usb_debug("usb msc: ready -> not ready (lun %d)\n", msc->lun);
		usb_msc_remove_disk(dev);
	} else if (!prev_ready && !msc->ready) {
		uint8_t new_lun = (msc->lun + 1) % msc->num_luns;
		usb_debug("usb msc: not ready (lun %d) -> lun %d\n", msc->lun,
			  new_lun);
		msc->lun = new_lun;
	}
}
