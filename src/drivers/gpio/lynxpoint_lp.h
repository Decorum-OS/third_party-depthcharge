/*
 * Copyright 2013 Google Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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

#ifndef __DRIVERS_GPIO_LYNXPOINT_LP_H__
#define __DRIVERS_GPIO_LYNXPOINT_LP_H__

#include "drivers/gpio/gpio.h"

typedef struct LpPchGpio
{
	GpioOps ops;
	int (*use)(struct LpPchGpio *, unsigned use);
	int num;
	uint16_t addr;
	int dir_set;
} LpPchGpio;

LpPchGpio *new_lp_pch_gpio(unsigned num);
LpPchGpio *new_lp_pch_gpio_input(unsigned num);
LpPchGpio *new_lp_pch_gpio_output(unsigned num);

void init_lp_pch_gpio(LpPchGpio *gpio, unsigned num);
void init_lp_pch_gpio_input(LpPchGpio *gpio, unsigned num);
void init_lp_pch_gpio_output(LpPchGpio *gpio, unsigned num);

// Read the GPIO configuration base address without caching it. Use this
// and the lp_pch_gpio_set_addr function to set up a GPIO before RAM is
// writable.
uint16_t lp_pch_gpio_base_uncached(void);
void lp_pch_gpio_set_addr(LpPchGpio *gpio, uint16_t gpio_base);

#endif /* __DRIVERS_GPIO_LYNXPOINT_LP_H__ */
