/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2014 Rockchip Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.
 */


#include <usb/usb.h>
#include "generic_hub.h"
#include "dwc2_private.h"
#include "dwc2.h"

#include "base/io.h"
#include "base/time.h"

static int
dwc2_rh_port_status_changed(UsbDev *const dev, const int port)
{
	hprt_t hprt;
	int changed;
	dwc_ctrl_t *const dwc2 = DWC2_INST(dev->controller);

	hprt.d32 = read32(dwc2->hprt0);
	changed = hprt.prtconndet;

	/* Clear connect detect flag */
	if (changed) {
		hprt.d32 &= HPRT_W1C_MASK;
		hprt.prtconndet = 1;
		write32(dwc2->hprt0, hprt.d32);
	}
	return changed;
}

static int
dwc2_rh_port_connected(UsbDev *const dev, const int port)
{
	hprt_t hprt;
	dwc_ctrl_t *const dwc2 = DWC2_INST(dev->controller);

	hprt.d32 = read32(dwc2->hprt0);
	return hprt.prtconnsts;
}

static int
dwc2_rh_port_in_reset(UsbDev *const dev, const int port)
{
	hprt_t hprt;
	dwc_ctrl_t *const dwc2 = DWC2_INST(dev->controller);

	hprt.d32 = read32(dwc2->hprt0);
	return hprt.prtrst;
}

static int
dwc2_rh_port_enabled(UsbDev *const dev, const int port)
{
	hprt_t hprt;
	dwc_ctrl_t *const dwc2 = DWC2_INST(dev->controller);

	hprt.d32 = read32(dwc2->hprt0);
	return hprt.prtena;
}

static UsbSpeed
dwc2_rh_port_speed(UsbDev *const dev, const int port)
{
	hprt_t hprt;
	dwc_ctrl_t *const dwc2 = DWC2_INST(dev->controller);

	hprt.d32 = read32(dwc2->hprt0);
	if (hprt.prtena) {
		switch (hprt.prtspd) {
		case PRTSPD_HIGH:
			return UsbHighSpeed;
		case PRTSPD_FULL:
			return UsbFullSpeed;
		case PRTSPD_LOW:
			return UsbLowSpeed;
		}
	}
	return -1;
}

static int
dwc2_rh_reset_port(UsbDev *const dev, const int port)
{
	hprt_t hprt;
	dwc_ctrl_t *const dwc2 = DWC2_INST(dev->controller);

	hprt.d32 = read32(dwc2->hprt0);
	hprt.d32 &= HPRT_W1C_MASK;
	hprt.prtrst = 1;
	write32(dwc2->hprt0, hprt.d32);

	/* Wait a bit while reset is active. */
	mdelay(50);

	/* Deassert reset. */
	hprt.prtrst = 0;
	write32(dwc2->hprt0, hprt.d32);

	/*
	 * If reset and speed enum success the DWC2 core will set enable bit
	 * after port reset bit is deasserted
	 */
	mdelay(1);
	hprt.d32 = read32(dwc2->hprt0);
	usb_debug("%s reset port ok, hprt = 0x%08x\n", __func__, hprt.d32);

	if (!hprt.prtena) {
		usb_debug("%s enable port fail! hprt = 0x%08x\n",
			  __func__, hprt.d32);
		return -1;
	}

	return 0;
}

static int
dwc2_rh_enable_port(UsbDev *const dev, const int port)
{
	hprt_t hprt;
	dwc_ctrl_t *const dwc2 = DWC2_INST(dev->controller);

	/* Power on the port */
	hprt.d32 = read32(dwc2->hprt0);
	hprt.d32 &= HPRT_W1C_MASK;
	hprt.prtpwr = 1;
	write32(dwc2->hprt0, hprt.d32);
	return 0;
}

static int
dwc2_rh_disable_port(UsbDev *const dev, const int port)
{
	hprt_t hprt;
	dwc_ctrl_t *const dwc2 = DWC2_INST(dev->controller);

	hprt.d32 = read32(dwc2->hprt0);
	hprt.d32 &= HPRT_W1C_MASK;
	/* Disable the port*/
	hprt.prtena = 1;
	/* Power off the port */
	hprt.prtpwr = 0;
	write32(dwc2->hprt0, hprt.d32);
	return 0;
}

static const generic_hub_ops_t dwc2_rh_ops = {
	.hub_status_changed	= NULL,
	.port_status_changed	= dwc2_rh_port_status_changed,
	.port_connected		= dwc2_rh_port_connected,
	.port_in_reset		= dwc2_rh_port_in_reset,
	.port_enabled		= dwc2_rh_port_enabled,
	.port_speed		= dwc2_rh_port_speed,
	.enable_port		= dwc2_rh_enable_port,
	.disable_port		= dwc2_rh_disable_port,
	.start_port_reset	= NULL,
	.reset_port		= dwc2_rh_reset_port,
};

void
dwc2_rh_init(UsbDev *dev)
{
	dwc_ctrl_t *const dwc2 = DWC2_INST(dev->controller);

	/* we can set them here because a root hub _really_ shouldn't
	   appear elsewhere */
	dev->address = 0;
	dev->hub = -1;
	dev->port = -1;

	generic_hub_init(dev, 1, &dwc2_rh_ops);
	usb_debug("dwc2_rh_init HPRT 0x%08x p = %p\n ",
		  read32(dwc2->hprt0), dwc2->hprt0);
	usb_debug("DWC2: root hub init done\n");
}
