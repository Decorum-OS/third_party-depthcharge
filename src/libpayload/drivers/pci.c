/*
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
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

#include <pci.h>
#include <stdint.h>

#include "base/io.h"

uint8_t pci_read_config8(pcidev_t device, uint16_t reg)
{
	outl(device | (reg & ~3), 0xCF8);
	return inb(0xCFC + (reg & 3));
}

uint16_t pci_read_config16(pcidev_t device, uint16_t reg)
{
	outl(device | (reg & ~3), 0xCF8);
	return inw(0xCFC + (reg & 3));
}

uint32_t pci_read_config32(pcidev_t device, uint16_t reg)
{
	outl(device | (reg & ~3), 0xCF8);
	return inl(0xCFC + (reg & 3));
}

void pci_write_config8(pcidev_t device, uint16_t reg, uint8_t val)
{
	outl(device | (reg & ~3), 0xCF8);
	outb(val, 0xCFC + (reg & 3));
}

void pci_write_config16(pcidev_t device, uint16_t reg, uint16_t val)
{
	outl(device | (reg & ~3), 0xCF8);
	outw(val, 0xCFC + (reg & 3));
}

void pci_write_config32(pcidev_t device, uint16_t reg, uint32_t val)
{
	outl(device | (reg & ~3), 0xCF8);
	outl(val, 0xCFC + (reg & 3));
}

static int find_on_bus(int bus, unsigned short vid, unsigned short did,
		       pcidev_t * dev)
{
	int devfn;
	uint32_t val;
	unsigned char hdr;

	for (devfn = 0; devfn < 0x100; devfn++) {
		int func = devfn & 0x7;
		int slot = (devfn >> 3) & 0x1f;

		val = pci_read_config32(PCI_DEV(bus, slot, func),
					PciConfVendorId);

		if (val == 0xffffffff || val == 0x00000000 ||
		    val == 0x0000ffff || val == 0xffff0000)
			continue;

		if (val == ((did << 16) | vid)) {
			*dev = PCI_DEV(bus, slot, func);
			return 1;
		}

		hdr = pci_read_config8(PCI_DEV(bus, slot, func),
				       PciConfHeaderType);
		hdr &= 0x7F;

		if (hdr == PciConfHeaderTypeBridge ||
				hdr == PciConfHeaderTypeCardbus) {
			unsigned int busses;
			busses = pci_read_config32(PCI_DEV(bus, slot, func),
						   PciConfPrimaryBus);
			busses = (busses >> 8) & 0xFF;

			/* Avoid recursion if the new bus is the same as
			 * the old bus (insert lame The Who joke here) */

			if ((busses != bus) &&
			    find_on_bus(busses, vid, did, dev))
				return 1;
		}
	}

	return 0;
}

int pci_find_device(uint16_t vid, uint16_t did, pcidev_t * dev)
{
	return find_on_bus(0, vid, did, dev);
}

uint32_t pci_read_resource(pcidev_t dev, int bar)
{
	return pci_read_config32(dev, 0x10 + (bar * 4));
}

void pci_set_bus_master(pcidev_t dev)
{
	uint16_t val = pci_read_config16(dev, PciConfCommand);
	val |= PciConfCommandBm;
	pci_write_config16(dev, PciConfCommand, val);
}
