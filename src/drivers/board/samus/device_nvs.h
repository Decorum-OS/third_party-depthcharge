/*
 * Copyright (C) 2014 Google Inc.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _BROADWELL_DEVICE_NVS_H_
#define _BROADWELL_DEVICE_NVS_H_

#include <stdint.h>

/* Offset in Global NVS where this structure lives */
#define DEVICE_NVS_OFFSET	0x1000

#define SIO_NVS_DMA		0
#define SIO_NVS_I2C0		1
#define SIO_NVS_I2C1		2
#define SIO_NVS_SPI0		3
#define SIO_NVS_SPI1		4
#define SIO_NVS_UART0		5
#define SIO_NVS_UART1		6
#define SIO_NVS_SDIO		7
#define SIO_NVS_ADSP		8

typedef struct {
	uint8_t enable[9];
	uint32_t bar0[9];
	uint32_t bar1[9];
} __attribute__((packed)) device_nvs_t;

#endif
