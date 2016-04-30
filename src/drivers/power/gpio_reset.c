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

#include "base/container_of.h"
#include "base/xalloc.h"
#include "drivers/power/gpio_reset.h"

static int gpio_reboot(PowerOps *me)
{
	GpioResetPowerOps *power = container_of(me, GpioResetPowerOps, ops);
	gpio_set(power->reset_gpio, 1);
	while (1);	// not halt(), it can trigger a GDB entry
	return -1;
}

static int pass_through_power_off(PowerOps *me)
{
	GpioResetPowerOps *power = container_of(me, GpioResetPowerOps, ops);
	return power->power_off_ops->power_off(power->power_off_ops);
}

GpioResetPowerOps *new_gpio_reset_power_ops(PowerOps *power_off_ops,
					    GpioOps *reset_gpio)
{
	GpioResetPowerOps *power = xzalloc(sizeof(*power));
	power->ops.power_off = &pass_through_power_off;
	power->ops.cold_reboot = &gpio_reboot;
	power->power_off_ops = power_off_ops;
	power->reset_gpio = reset_gpio;
	return power;
}
