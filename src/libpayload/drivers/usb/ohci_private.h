/*
 * Copyright (C) 2010 Patrick Georgi
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

#ifndef __OHCI_PRIVATE_H
#define __OHCI_PRIVATE_H

#include <usb/usb.h>

#define MASK(startbit, lenbit) (((1<<(lenbit))-1)<<(startbit))

	// FIXME: fake
	typedef enum { CMD} reg;

	typedef enum {
		NumberDownstreamPorts = 1<<0,
		PowerSwitchingMode = 1<<8,
		NoPowerSwitching = 1<<9,
		DeviceType = 1<<10,
		OverCurrentProtectionMode = 1<<11,
		NoOverCurrentProtection = 1<<12,
		PowerOnToPowerGoodTime = 1<<24
	} HcRhDescriptorAReg;

	typedef enum {
		NumberDownstreamPortsMask = MASK(0, 8),
		PowerOnToPowerGoodTimeMask = MASK(24, 8)
	} HcRhDescriptorAMask;

	typedef enum {
		DeviceRemovable = 1<<0,
		PortPowerControlMask = 1<<16
	} HcRhDescriptorBReg;

	typedef enum {
		CurrentConnectStatus		= 1<<0,
		PortEnableStatus		= 1<<1,
		PortSuspendStatus		= 1<<2,
		PortOverCurrentIndicator	= 1<<3,
		PortResetStatus			= 1<<4,
		PortPowerStatus			= 1<<8,
		LowSpeedDeviceAttached		= 1<<9,
		ConnectStatusChange		= 1<<16,
		PortEnableStatusChange		= 1<<17,
		PortSuspendStatusChange		= 1<<18,
		PortOverCurrentIndicatorChange	= 1<<19,
		PortResetStatusChange		= 1<<20
	} HcRhPortStatusRead;
	typedef enum {
		ClearPortEnable			= 1<<0,
		SetPortEnable			= 1<<1,
		SetPortSuspend			= 1<<2,
		ClearSuspendStatus		= 1<<3,
		SetPortReset			= 1<<4,
		SetPortPower			= 1<<8,
		ClearPortPower			= 1<<9,
	} HcRhPortStatusSet;

	typedef enum {
		LocalPowerStatus = 1<<0,
		OverCurrentIndicator = 1<<1,
		DeviceRemoteWakeupEnable = 1<<15,
		LocalPowerStatusChange = 1<<16,
		OverCurrentIndicatorChange = 1<<17,
		ClearRemoteWakeupEnable = 1<<31
	} HcRhStatusReg;

	typedef enum {
		FrameInterval = 1<<0,
		FSLargestDataPacket = 1<<16,
		FrameIntervalToggle = 1<<31
	} HcFmIntervalOffset;
	typedef enum {
		FrameIntervalMask = MASK(0, 14),
		FSLargestDataPacketMask = MASK(16, 15),
		FrameIntervalToggleMask = MASK(31, 1)
	} HcFmIntervalMask;

	typedef enum {
		ControlBulkServiceRatio = 1<<0,
		PeriodicListEnable = 1<<2,
		IsochronousEnable = 1<<3,
		ControlListEnable = 1<<4,
		BulkListEnable = 1<<5,
		HostControllerFunctionalState = 1<<6,
		InterruptRouting = 1<<8,
		RemoteWakeupConnected = 1<<9,
		RemoteWakeupEnable = 1<<10
	} HcControlReg;

	typedef enum {
		ControlBulkServiceRatioMask = MASK(0, 2),
		HostControllerFunctionalStateMask = MASK(6, 2)
	} HcControlMask;

	enum {
		USBReset = 0*HostControllerFunctionalState,
		USBResume = 1*HostControllerFunctionalState,
		USBOperational = 2*HostControllerFunctionalState,
		USBSuspend = 3*HostControllerFunctionalState
	};

	typedef enum {
		HostControllerReset = 1<<0,
		ControlListFilled = 1<<1,
		BulkListFilled = 1<<2,
		OwnershipChangeRequest = 1<<3,
		SchedulingOverrunCount = 1<<16
	} HcCommandStatusReg;

	typedef enum {
		SchedulingOverrunCountMask = MASK(16, 2)
	} HcCommandStatusMask;

	typedef enum {
		FrameRemaining = 1<<0,
		FrameRemainingToggle = 1<<31
	} HcFmRemainingReg;

	typedef enum {
		SchedulingOverrung = 1<<0,
		WritebackDoneHead = 1<<1,
		StartofFrame = 1<<2,
		ResumeDetected = 1<<3,
		UnrecoverableError = 1<<4,
		FrameNumberOverflow = 1<<5,
		RootHubStatusChange = 1<<6,
		OwnershipChange = 1<<30
	} HcInterruptStatusReg;

     typedef struct {
	// Control and Status Partition
	volatile uint32_t HcRevision;
	volatile uint32_t HcControl;
	volatile uint32_t HcCommandStatus;
	volatile uint32_t HcInterruptStatus;
	volatile uint32_t HcInterruptEnable;
	volatile uint32_t HcInterruptDisable;

	// Memory Pointer Partition
	volatile uint32_t HcHCCA;
	volatile uint32_t HcPeriodCurrentED;
	volatile uint32_t HcControlHeadED;
	volatile uint32_t HcControlCurrentED;
	volatile uint32_t HcBulkHeadED;
	volatile uint32_t HcBulkCurrentED;
	volatile uint32_t HcDoneHead;

	// Frame Counter Partition
	volatile uint32_t HcFmInterval;
	volatile uint32_t HcFmRemaining;
	volatile uint32_t HcFmNumber;
	volatile uint32_t HcPeriodicStart;
	volatile uint32_t HcLSThreshold;

	// Root Hub Partition
	volatile uint32_t HcRhDescriptorA;
	volatile uint32_t HcRhDescriptorB;
	volatile uint32_t HcRhStatus;
	/* all bits in HcRhPortStatus registers are R/WC, so
	   _DO NOT_ use |= to set the bits,
	   this clears the entire state */
	volatile uint32_t HcRhPortStatus[];
     } __attribute__ ((packed)) opreg_t;

	typedef struct { /* should be 256 bytes according to spec */
		uint32_t HccaInterruptTable[32];
		volatile uint16_t HccaFrameNumber;
		volatile uint16_t HccaPad1;
		volatile uint32_t HccaDoneHead;
		uint8_t reserved[116]; /* pad according to spec */
		uint8_t what[4]; /* really pad to 256 as spec only covers 252 */
	} __attribute__ ((packed)) hcca_t;

	typedef volatile struct {
		uint32_t config;
		uint32_t tail_pointer;
		uint32_t head_pointer;
		uint32_t next_ed;
	} __attribute__ ((packed)) ed_t;
#define ED_HALTED 1
#define ED_TOGGLE 2

#define ED_FUNC_SHIFT 0
#define ED_FUNC_MASK MASK(0, 7)
#define ED_EP_SHIFT 7
#define ED_EP_MASK MASK(7, 4)
#define ED_DIR_SHIFT 11
#define ED_DIR_MASK MASK(11, 2)
#define ED_LOWSPEED (1 << 13)
#define ED_MPS_SHIFT 16

	typedef volatile struct {
		uint32_t config;
		uint32_t current_buffer_pointer;
		uint32_t next_td;
		uint32_t buffer_end;
	} __attribute__ ((packed)) td_t;
/*
 * Bits 0 through 17 of .config won't be interpreted by the host controller
 * (HC) and, after processing the TD, the HC has to ensure those bits have
 * the same state as before. So we are free to use those bits for our own
 * purpose.
 */
#define TD_QUEUETYPE_SHIFT	0
#define TD_QUEUETYPE_MASK	MASK(TD_QUEUETYPE_SHIFT, 2)
#define TD_QUEUETYPE_ASYNC	(0 << TD_QUEUETYPE_SHIFT)
#define TD_QUEUETYPE_INTR	(1 << TD_QUEUETYPE_SHIFT)

#define TD_DIRECTION_SHIFT 19
#define TD_DIRECTION_MASK MASK(TD_DIRECTION_SHIFT, 2)
#define TD_DIRECTION_SETUP OHCI_SETUP << TD_DIRECTION_SHIFT
#define TD_DIRECTION_IN OHCI_IN << TD_DIRECTION_SHIFT
#define TD_DIRECTION_OUT OHCI_OUT << TD_DIRECTION_SHIFT
#define TD_DELAY_INTERRUPT_SHIFT	21
#define TD_DELAY_INTERRUPT_MASK		MASK(TD_DELAY_INTERRUPT_SHIFT, 3)
#define TD_DELAY_INTERRUPT_ZERO		0
#define TD_DELAY_INTERRUPT_NOINTR	(7 << TD_DELAY_INTERRUPT_SHIFT)
#define TD_TOGGLE_DATA0 0
#define TD_TOGGLE_DATA1 (1 << 24)
#define TD_TOGGLE_FROM_ED 0
#define TD_TOGGLE_FROM_TD (1 << 25)
#define TD_CC_SHIFT 28
#define TD_CC_MASK MASK(TD_CC_SHIFT, 4)
#define TD_CC_NOERR 0
#define TD_CC_NOACCESS (14 << TD_CC_SHIFT) /* the lower of the two values, so "no access" can be tested with >= */

#define OHCI_INST(controller) ((ohci_t*)((controller)->instance))

	typedef struct ohci {
		opreg_t *opreg;
		hcca_t *hcca;
		UsbDev *roothub;
		ed_t *periodic_ed;
#define DMA_SIZE (64 * 1024)
		void *dma_buffer;
	} ohci_t;

	typedef enum { OHCI_SETUP=0, OHCI_OUT=1, OHCI_IN=2, OHCI_FROM_TD=3 } ohci_pid_t;

#endif
