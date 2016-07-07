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
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdlib.h>

#include "arch/x86/amd64/handoff/handoff.h"
#include "base/fwdb.h"
#include "uefi/edk.h"

extern EFI_SYSTEM_TABLE *_uefi_handoff_system_table;
extern EFI_HANDLE _uefi_handoff_image_handle;

void handoff_special(void)
{
	EFI_BOOT_SERVICES *boot_services =
		_uefi_handoff_system_table->BootServices;

	EFI_ALLOCATE_PAGES allocate_pages = boot_services->AllocatePages;

	// Since we're throwing away the FWDB at boot, we don't need to worry
	// about where it gets stuck, except that it can't overlay where the
	// OS expects to go. If it does, we can move it after exiting boot
	// services later on.
	EFI_PHYSICAL_ADDRESS fwdb_addr = 0;

	EFI_STATUS status = allocate_pages(
		AllocateAnyPages, EfiLoaderData,
		CONFIG_UEFI_HANDOFF_FWDB_4K_PAGES, &fwdb_addr);

	// If something bad happens, we're not yet in a state to deal with
	// it. For now we'll halt to prevent anything bad from happening later.

	if (status != EFI_SUCCESS)
		halt();

	if (fwdb_create_db((void *)(uintptr_t)fwdb_addr,
			   CONFIG_UEFI_HANDOFF_FWDB_4K_PAGES * 4096))
		halt();

	FwdbEntry system_table_entry = {
		.size = sizeof(_uefi_handoff_system_table),
		.ptr = &_uefi_handoff_system_table,
	};
	if (fwdb_access("uefi_system_table", NULL, &system_table_entry))
		halt();

	FwdbEntry image_handle_entry = {
		.size = sizeof(_uefi_handoff_image_handle),
		.ptr = &_uefi_handoff_image_handle,
	};
	if (fwdb_access("uefi_image_handle", NULL, &image_handle_entry))
		halt();
}
