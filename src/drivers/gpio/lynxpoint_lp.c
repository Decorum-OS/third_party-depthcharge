/*
 * Copyright 2012 Google Inc.
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

#include <assert.h>
#include <libpayload.h>
#include <pci.h>
#include <stdint.h>

#include "base/container_of.h"
#include "base/io.h"
#include "base/xalloc.h"
#include "drivers/gpio/gpio.h"
#include "drivers/gpio/lynxpoint_lp.h"

static inline uint16_t lp_gpio_conf0(unsigned num)
{
	return 0x100 + num * 8;
}

enum {
	LpGpioConf0ModeBit = 1 << 0,
	LpGpioConf0DirBit = 1 << 2,
	LpGpioConf0GpiBit = 1 << 30,
	LpGpioConf0GpoBit = 1 << 31
};

/* Functions for manipulating GPIO regs. */

uint16_t lp_pch_gpio_base_uncached(void)
{
	static const uint32_t dev = PCI_DEV(0, 0x1f, 0);
	static const uint8_t pci_cfg_gpio_base = 0x48;

	uint16_t base = pci_read_config32(dev, pci_cfg_gpio_base);
	// Drop the IO space bit.
	return base & ~0x1;
}

static uint16_t pch_gpio_base(void)
{
	static uint32_t base = ~(uint32_t)0;

	if (base == ~(uint32_t)0)
		base = lp_pch_gpio_base_uncached();

	return base;
}

void lp_pch_gpio_set_addr(LpPchGpio *gpio, uint16_t gpio_base)
{
	gpio->addr = gpio_base + lp_gpio_conf0(gpio->num);
}

static void pch_gpio_set(uint16_t addr, uint32_t bit, int val)
{
	uint32_t conf = inl(addr);
	if (val)
		conf |= bit;
	else
		conf &= ~bit;
	outl(conf, addr);
}

static int pch_gpio_get(uint16_t addr, uint32_t bit)
{
	return !!(inl(addr) & bit);
}


/* Interface functions for manipulating a GPIO. */

static int lp_pch_gpio_get_value(GpioOps *me)
{
	assert(me);
	LpPchGpio *gpio = container_of(me, LpPchGpio, ops);

	if (!gpio->addr)
		gpio->addr = pch_gpio_base() + lp_gpio_conf0(gpio->num);
	if (!gpio->dir_set) {
		pch_gpio_set(gpio->addr, LpGpioConf0DirBit, 1);
		gpio->dir_set = 1;
	}

	return pch_gpio_get(gpio->addr, LpGpioConf0GpiBit);
}

static int lp_pch_gpio_set_value(GpioOps *me, unsigned value)
{
	assert(me);
	LpPchGpio *gpio = container_of(me, LpPchGpio, ops);

	if (!gpio->addr)
		gpio->addr = pch_gpio_base() + lp_gpio_conf0(gpio->num);
	if (!gpio->dir_set) {
		pch_gpio_set(gpio->addr, LpGpioConf0DirBit, 0);
		gpio->dir_set = 1;
	}

	pch_gpio_set(gpio->addr, LpGpioConf0GpoBit, value);

	return 0;
}

static int lp_pch_gpio_use(LpPchGpio *gpio, unsigned use)
{
	assert(gpio);

	if (!gpio->addr)
		gpio->addr = pch_gpio_base() + lp_gpio_conf0(gpio->num);

	pch_gpio_set(gpio->addr, LpGpioConf0ModeBit, use);

	return 0;
}


/* Functions to set up a GPIO structure which has already been allocated. */

void init_lp_pch_gpio(LpPchGpio *gpio, unsigned num)
{
	gpio->use = &lp_pch_gpio_use;
	gpio->num = num;
}

void init_lp_pch_gpio_input(LpPchGpio *gpio, unsigned num)
{
	init_lp_pch_gpio(gpio, num);
	gpio->ops.get = &lp_pch_gpio_get_value;
}

void init_lp_pch_gpio_output(LpPchGpio *gpio, unsigned num)
{
	init_lp_pch_gpio(gpio, num);
	gpio->ops.set = &lp_pch_gpio_set_value;
}


/* Functions to allocate and set up a GPIO structure. */

LpPchGpio *new_lp_pch_gpio(unsigned num)
{
	LpPchGpio *gpio = xzalloc(sizeof(*gpio));
	init_lp_pch_gpio(gpio, num);
	return gpio;
}

LpPchGpio *new_lp_pch_gpio_input(unsigned num)
{
	LpPchGpio *gpio = xzalloc(sizeof(*gpio));
	init_lp_pch_gpio_input(gpio, num);
	return gpio;
}

LpPchGpio *new_lp_pch_gpio_output(unsigned num)
{
	LpPchGpio *gpio = xzalloc(sizeof(*gpio));
	init_lp_pch_gpio_output(gpio, num);
	return gpio;
}
