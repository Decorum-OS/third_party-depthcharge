/*
 * This file is part of the libpayload project.
 *
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

#ifndef __USB_HUB_H
#define __USB_HUB_H

#include <usb/usb.h>

typedef struct generic_hub_ops {
	/* negative results denote an error */

	/* returns 1 if the hub's status changed since the last call (optional) */
	int (*hub_status_changed)(UsbDev *);
	/* returns 1 if the port's status changed since the last call */
	int (*port_status_changed)(UsbDev *, int port);
	/* returns 1 if something is connected to the port */
	int (*port_connected)(UsbDev *, int port);
	/* returns 1 if port is currently resetting */
	int (*port_in_reset)(UsbDev *, int port);
	/* returns 1 if the port is enabled */
	int (*port_enabled)(UsbDev *, int port);
	/* returns speed if port is enabled, negative value if not */
	UsbSpeed (*port_speed)(UsbDev *, int port);

	/* enables (powers up) a port (optional) */
	int (*enable_port)(UsbDev *, int port);
	/* disables (powers down) a port (optional) */
	int (*disable_port)(UsbDev *, int port);
	/* starts a port reset (required if reset_port is set to a generic one from below) */
	int (*start_port_reset)(UsbDev *, int port);

	/* performs a port reset (optional, generic implementations below) */
	int (*reset_port)(UsbDev *, int port);
} generic_hub_ops_t;

typedef struct generic_hub {
	int num_ports;
	/* port numbers are always 1 based,
	   so we waste one int for convenience */
	int *ports; /* allocated to sizeof(*ports)*(num_ports+1) */
#define NO_DEV -1

	const generic_hub_ops_t *ops;

	void *data;
} generic_hub_t;

void generic_hub_destroy(UsbDev *);
int generic_hub_wait_for_port(UsbDev *const dev, const int port,
			      const int wait_for,
			      int (*const port_op)(UsbDev *, int),
			      int timeout_steps, const int step_us);
int  generic_hub_resetport(UsbDev *, int port);
int  generic_hub_scanport(UsbDev *, int port);
/* the provided generic_hub_ops struct has to be static */
int generic_hub_init(UsbDev *, int num_ports, const generic_hub_ops_t *);

#define GEN_HUB(usbdev) ((generic_hub_t *)(usbdev)->data)

#endif
