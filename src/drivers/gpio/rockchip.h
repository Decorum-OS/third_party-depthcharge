/*
 * Copyright 2014 Rockchip Electronics Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __RK3288_GPIO_H
#define __RK3288_GPIO_H

#include "drivers/gpio/gpio.h"

#define GPIO(p, b, i) ((RkGpioSpec){.port = p, .bank = GPIO_##b, .idx = i})

enum {
	GPIO_A = 0,
	GPIO_B,
	GPIO_C,
	GPIO_D,
};

typedef struct {
	uint32_t swporta_dr;
	uint32_t swporta_ddr;
	uint32_t reserved0[(0x30 - 0x08) / 4];
	uint32_t inten;
	uint32_t intmask;
	uint32_t inttype_level;
	uint32_t int_polarity;
	uint32_t int_status;
	uint32_t int_rawstatus;
	uint32_t debounce;
	uint32_t porta_eoi;
	uint32_t ext_porta;
	uint32_t reserved1[(0x60 - 0x54) / 4];
	uint32_t ls_sync;
} RkGpioRegs;

// This structure must be kept in sync with coreboot's GPIO implementation!
typedef union {
	uint32_t raw;
	struct {
		uint16_t port;
		union {
			struct {
				uint16_t num:5;
				uint16_t :11;
			};
			struct {
				uint16_t idx:3;
				uint16_t bank:2;
				uint16_t :11;
			};
		};
	};
} RkGpioSpec;

typedef struct RkGpio {
	GpioOps ops;
	RkGpioSpec gpioindex;
} RkGpio;

GpioOps *new_rk_gpio_input_from_coreboot(uint32_t port);
RkGpio *new_rk_gpio_output(RkGpioSpec gpioindex);
RkGpio *new_rk_gpio_input(RkGpioSpec gpioindex);

#endif
