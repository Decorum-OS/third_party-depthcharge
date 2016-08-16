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

#include <assert.h>
#include <stdio.h>

#include "base/cleanup.h"
#include "drivers/board/board.h"
#include "drivers/power/power.h"

int cold_reboot(void)
{
	PowerOps *power_ops = board_power();
	assert(power_ops->cold_reboot);

	if (cleanup_trigger(CleanupOnReboot))
		return -1;

	printf("Rebooting...\n");
	return power_ops->cold_reboot(power_ops);
}

int power_off(void)
{
	PowerOps *power_ops = board_power();
	assert(power_ops->power_off);

	if (cleanup_trigger(CleanupOnPoweroff))
		return -1;

	return power_ops->power_off(power_ops);
}
