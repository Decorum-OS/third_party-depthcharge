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

#include <assert.h>
#include <elf.h>
#include <stdio.h>

#include "base/cleanup.h"
#include "module/uefi/exit_bs.h"
#include "uefi/uefi.h"

int exit_boot_services_func(DcEvent *event)
{
	return uefi_exit_boot_services();
}

void install_exit_boot_services_cleanup(void)
{
	static CleanupEvent exit_boot_services_cleanup = {
		.event = {
			.trigger = &exit_boot_services_func,
		},
		.types = CleanupOnHandoff,
	};
	cleanup_add(&exit_boot_services_cleanup);
}
