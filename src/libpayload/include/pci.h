/*
 * This file is part of the libpayload project.
 *
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
 * Copyright (C) 2008 coresystems GmbH
 * Copyright 2016 Google Inc.
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

#ifndef _PCI_H
#define _PCI_H

#include <arch/types.h>
typedef uint32_t pcidev_t;

/* Device config space registers. */
typedef enum {
	PciConfVendorId = 0x00,
	PciConfDeviceId = 0x02,
	PciConfCommand = 0x04,
	PciConfStatus = 0x06,
	PciConfRevisionId = 0x08,
	PciConfProgIf = 0x09,
	PciConfSubclass = 0x0a,
	PciConfClass = 0x0b,
	PciConfCacheLineSize = 0x0c,
	PciConfLatencyTimer = 0x0d,
	PciConfHeaderType = 0x0e,
	PciConfBist = 0x0f,
	PciConfBar0 = 0x10,
	PciConfBar1 = 0x14,
	PciConfBar2 = 0x18,
	PciConfBar3 = 0x1c,
	PciConfBar4 = 0x20,
	PciConfBar5 = 0x24,
	PciConfCardbusCisPointer = 0x28,
	PciConfSubsysVendorId = 0x2c,
	PciConfSubsysId = 0x2e,
	PciConfDevOpromBase = 0x30,
	PciConfCapPointer = 0x34,
	PciConfInterruptLine = 0x3c,
	PciConfInterruptPin = 0x3d,
	PciConfMinGrant = 0x3e,
	PciConfMaxLatency = 0x3f,

/* Bridge config space registers. */
	PciConfPrimaryBus = 0x18,
	PciConfSecondaryBus = 0x19,
	PciConfSubordinateBus = 0x1a,
	PciConfSecondaryLatency = 0x1b,
	PciConfRegIoBase = 0x1c,
	PciConfRegIoLimit = 0x1d,
	PciConfSecondaryStatus = 0x1e,
	PciConfMemoryBase = 0x20,
	PciConfMemoryLimit = 0x22,
	PciConfPrefetchMemBase = 0x24,
	PriConfPrefetchMemLimit = 0x26,
	PciConfPrefetchBaseUpper = 0x28,
	PciConfPrefetchLimitUpper = 0x2c,
	PciConfIoBaseUpper = 0x30,
	PciConfIoLimitUpper = 0x32,
	PciConfBridgeOpromBase = 0x38,
	PciConfBridgeControl = 0x3c,
} PciConfRegOffsets;

enum {
	PciConfOpromAddrMask = 0x7ff // Bits *not* to use.
};

enum {
	PciConfBarSpaceMask = 0x1,
	PciConfBarSpaceIo = 0x1,
	PciConfBarSpaceMem = 0x0,
	PciConfBarIoMask = 0xf, // Bits *not* to use.
	PciConfBarMemMask = 0x3 // Bits *not* to use.
};

typedef enum {
	PciConfCommandIo = 1 << 0,
	PciConfCommandMem = 1 << 1,
	PciConfCommandBm = 1 << 2
} PciConfCommandBits;

typedef enum {
	PciConfHeaderTypeNormal = 0,
	PciConfHeaderTypeBridge = 1,
	PciConfHeaderTypeCardbus = 2,
	PciConfHeaderTypeMultifunction = 0x80
} PciConfHeaderTypes;

#define PCI_ADDR(_bus, _dev, _fn, _reg) \
(0x80000000 | (_bus << 16) | (_dev << 11) | (_fn << 8) | (_reg & ~3))

#define PCI_DEV(_bus, _dev, _fn) \
(0x80000000 | (_bus << 16) | (_dev << 11) | (_fn << 8))

#define PCI_BUS(_d)  ((_d >> 16) & 0xff)
#define PCI_SLOT(_d) ((_d >> 11) & 0x1f)
#define PCI_FUNC(_d) ((_d >> 8) & 0x7)

uint8_t pci_read_config8(uint32_t device, uint16_t reg);
uint16_t pci_read_config16(uint32_t device, uint16_t reg);
uint32_t pci_read_config32(uint32_t device, uint16_t reg);

void pci_write_config8(uint32_t device, uint16_t reg, uint8_t val);
void pci_write_config16(uint32_t device, uint16_t reg, uint16_t val);
void pci_write_config32(uint32_t device, uint16_t reg, uint32_t val);

int pci_find_device(uint16_t vid, uint16_t did, pcidev_t *dev);
uint32_t pci_read_resource(pcidev_t dev, int bar);

void pci_set_bus_master(pcidev_t dev);

#endif
