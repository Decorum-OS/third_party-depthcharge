/*
 * Copyright 2014 Google Inc.
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

#ifndef __DRIVERS_POWER_GPIO_RESET_H__
#define __DRIVERS_POWER_GPIO_RESET_H__

#include "drivers/gpio/gpio.h"
#include "drivers/power/power.h"

typedef struct {
	PowerOps ops;
	PowerOps *power_off_ops;
	GpioOps *reset_gpio;
} GpioResetPowerOps;

GpioResetPowerOps *new_gpio_reset_power_ops(PowerOps *power_off_ops,
					    GpioOps *reset_gpio);

#endif /* __DRIVERS_POWER_GPIO_RESET_H__ */
