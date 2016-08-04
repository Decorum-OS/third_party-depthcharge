/*
 * Copyright 2016 Google Inc.
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

#include "drivers/power/uefi.h"
#include "uefi/uefi.h"

static int uefi_power_cold_reboot(PowerOps *me)
{
	EFI_SYSTEM_TABLE *st = uefi_system_table_ptr();
	if (!st)
		return 1;
	EFI_RUNTIME_SERVICES *rs = st->RuntimeServices;

	rs->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);

	return 0;
}

static int uefi_power_power_off(PowerOps *me)
{
	EFI_SYSTEM_TABLE *st = uefi_system_table_ptr();
	if (!st)
		return 1;
	EFI_RUNTIME_SERVICES *rs = st->RuntimeServices;

	rs->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);

	return 0;
}

PowerOps uefi_power_ops = {
	.cold_reboot = &uefi_power_cold_reboot,
	.power_off = &uefi_power_power_off,
};
