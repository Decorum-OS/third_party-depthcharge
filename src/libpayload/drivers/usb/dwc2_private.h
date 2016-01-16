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

#ifndef __DWC2_REGS_H__
#define __DWC2_REGS_H__

#include "libpayload/drivers/usb/dwc2_registers.h"

typedef struct dwc_ctrl {
#define DMA_SIZE (64 * 1024)
	void *dma_buffer;
	uint32_t *hprt0;
	uint32_t frame;
} dwc_ctrl_t;

typedef struct {
	uint8_t *data;
	endpoint_t *endp;
	int reqsize;
	uint32_t reqtiming;
	uint32_t timestamp;
} intr_queue_t;

typedef struct {
	int hubaddr;
	int hubport;
} split_info_t;

#define DWC2_INST(controller) ((dwc_ctrl_t *)((controller)->instance))
#define DWC2_REG(controller) ((dwc2_reg_t *)((controller)->reg_base))

typedef enum {
	HCSTAT_DONE = 0,
	HCSTAT_XFERERR,
	HCSTAT_BABBLE,
	HCSTAT_STALL,
	HCSTAT_ACK,
	HCSTAT_NAK,
	HCSTAT_NYET,
	HCSTAT_UNKNOW,
	HCSTAT_TIMEOUT,
	HCSTAT_DISCONNECTED,
} hcstat_t;
#endif
