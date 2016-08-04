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
#include <stdio.h>
#include <sys/types.h>

#include "base/cleanup.h"
#include "base/event.h"
#include "base/fwdb.h"
#include "base/list.h"
#include "base/xalloc.h"
#include "uefi/uefi.h"

static int exit_boot_services_func(DcEvent *event)
{
	return uefi_exit_boot_services();
}

EFI_SYSTEM_TABLE *uefi_system_table_ptr(void)
{
	static EFI_SYSTEM_TABLE *ptr;
	if (ptr)
		return ptr;

	FwdbEntry entry;
	if (fwdb_access("uefi_system_table", &entry, NULL)) {
		printf("UEFI system table not found in FWDB.\n");
		return NULL;
	}

	if (entry.size != sizeof(ptr)) {
		printf("FWDB entry was not the expected size.\n");
		return NULL;
	}

	static CleanupEvent exit_boot_services_cleanup = {
		.event = {
			.trigger = &exit_boot_services_func,
		},
		.types = CleanupOnHandoff,
	};
	cleanup_add(&exit_boot_services_cleanup);

	ptr = *(EFI_SYSTEM_TABLE **)entry.ptr;
	return ptr;
}

int uefi_image_handle(EFI_HANDLE *handle_ptr)
{
	assert(handle_ptr);

	static EFI_HANDLE handle;
	static int initialized;
	if (initialized) {
		*handle_ptr = handle;
		return 0;
	}

	FwdbEntry entry;
	if (fwdb_access("uefi_image_handle", &entry, NULL)) {
		printf("UEFI image handle not found in FWDB.\n");
		return 1;
	}

	if (entry.size != sizeof(handle)) {
		printf("FWDB entry was not the expected size.\n");
		return 1;
	}

	handle = *(EFI_HANDLE *)entry.ptr;
	initialized = 1;

	*handle_ptr = handle;
	return 0;
}

static ListNode exit_boot_services_events;

int uefi_exit_boot_services(void)
{
	EFI_SYSTEM_TABLE *system_table = uefi_system_table_ptr();
	if (!system_table)
		return 1;

	EFI_BOOT_SERVICES *bs = system_table->BootServices;

	EFI_HANDLE handle;
	if (uefi_image_handle(&handle))
		return 1;

	UINTN size = 0;
	UINTN map_key;
	UINTN desc_size;
	UINT32 desc_ver;
	EFI_STATUS status = bs->GetMemoryMap(&size, NULL, &map_key,
					     &desc_size, &desc_ver);
	if (status != EFI_BUFFER_TOO_SMALL) {
		printf("Failed to retrieve memory map size.\n");
		return 1;
	}
	uint8_t *map_buf = xmalloc(size);
	status = bs->GetMemoryMap(&size, (void *)map_buf, &map_key,
				  &desc_size, &desc_ver);
	free(map_buf);
	if (status != EFI_SUCCESS) {
		printf("Failed to retrieve memory map key.\n");
		return 1;
	}

	if (event_trigger_all(&exit_boot_services_events))
		return 1;

	status = bs->ExitBootServices(handle, map_key);
	if (status != EFI_SUCCESS) {
		printf("Failed to exit boot services.\n");
		return 1;
	} else {
		return 0;
	}
}

void uefi_add_exit_boot_services_event(DcEvent *event)
{
	list_insert_after(&event->list_node, &exit_boot_services_events);
}

void uefi_remove_exit_boot_services_event(DcEvent *event)
{
	list_remove(&event->list_node);
}
