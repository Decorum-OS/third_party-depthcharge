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

#include <stdio.h>

#include "base/xalloc.h"
#include "base/fwdb.h"
#include "uefi/edk/Protocol/DevicePath.h"
#include "uefi/edk/Protocol/DevicePathFromText.h"
#include "uefi/edk/Protocol/EfiShellParameters.h"
#include "uefi/uefi.h"
#include "vboot/stages.h"

extern uint8_t _binary_ro_image_start;
extern uint8_t _binary_ro_image_size;
extern uint8_t ImageBase;

static EFI_GUID shell_parameters_protocol_guid =
	EFI_SHELL_PARAMETERS_PROTOCOL_GUID;

static EFI_GUID device_path_from_text_guid =
	EFI_DEVICE_PATH_FROM_TEXT_PROTOCOL_GUID;

static EFI_SHELL_PARAMETERS_PROTOCOL *get_shell_parameters(
	EFI_BOOT_SERVICES *boot_services)
{
	EFI_HANDLE handle;
	if (uefi_image_handle(&handle))
		return NULL;

	EFI_SHELL_PARAMETERS_PROTOCOL *shell_params;
	EFI_STATUS status = boot_services->HandleProtocol(
		handle, &shell_parameters_protocol_guid,
		(void **)&shell_params);

	if (status != EFI_SUCCESS) {
		printf("No shell parameter protocol found.\n");
		return NULL;
	}

	return shell_params;
}

static EFI_DEVICE_PATH_FROM_TEXT_PROTOCOL *get_dp_from_text_protocol(
	EFI_BOOT_SERVICES *boot_services)
{
	UINTN buf_size = 0;
	EFI_HANDLE dummy_handle;
	EFI_STATUS status = boot_services->LocateHandle(
		ByProtocol, &device_path_from_text_guid, NULL,
		&buf_size, &dummy_handle);
	if (status == EFI_NOT_FOUND) {
		printf("No device path from text protocol handles found.\n");
		return NULL;
	}
	if (status != EFI_BUFFER_TOO_SMALL) {
		printf("Error retrieving handles.\n");
		return NULL;
	}

	EFI_HANDLE *handles = xmalloc(buf_size);

	status = boot_services->LocateHandle(
		ByProtocol, &device_path_from_text_guid, NULL,
		&buf_size, handles);
	die_if(status != EFI_SUCCESS, "Failed to retrieve handles.\n");

	EFI_DEVICE_PATH_FROM_TEXT_PROTOCOL *dp_from_text;
	status = boot_services->HandleProtocol(
		handles[0], &device_path_from_text_guid,
		(void **)&dp_from_text);
	free(handles);
	if (status != EFI_SUCCESS) {
		printf("Failed to open device path from text protocol.\n");
		return NULL;
	}
	return dp_from_text;
}

static int insert_dp_from_argv(EFI_BOOT_SERVICES *boot_services,
			       EFI_DEVICE_PATH_FROM_TEXT_PROTOCOL *dp_from_text,
			       EFI_SHELL_PARAMETERS_PROTOCOL *shell_params,
			       int idx, const char *name)
{
	EFI_DEVICE_PATH_PROTOCOL *dp_prot =
		dp_from_text->ConvertTextToDevicePath(shell_params->Argv[idx]);
	if (!dp_prot) {
		printf("Failed to convert argv[%d] to device path.\n", idx);
		return 1;
	}
	FwdbEntry entry = {
		.ptr = dp_prot,
		.size = sizeof(*dp_prot)
	};
	if (fwdb_access(name, NULL, &entry)) {
		boot_services->FreePool(dp_prot);
		return 1;
	}
	boot_services->FreePool(dp_prot);

	return 0;
}

static int prepare_fwdb_storage(void)
{
	FwdbEntry ro_image_entry = {
		.ptr = &_binary_ro_image_start,
		.size = &_binary_ro_image_size - &ImageBase,
	};
	if (fwdb_access("uefi_ro_image", NULL, &ro_image_entry))
		return 1;

	EFI_SYSTEM_TABLE *system_table = uefi_system_table_ptr();
	if (!system_table)
		return 1;
	EFI_BOOT_SERVICES *boot_services = system_table->BootServices;

	EFI_SHELL_PARAMETERS_PROTOCOL *shell_params =
		get_shell_parameters(boot_services);
	EFI_DEVICE_PATH_FROM_TEXT_PROTOCOL *dp_from_text =
		get_dp_from_text_protocol(boot_services);

	if (!shell_params || !dp_from_text)
		return 1;

	if (shell_params->Argc != 3) {
		printf("Bad number of arguments.\n");
		printf("Usage: dc <rwa image> <rwb image>\n");
		return 1;
	}

	if (insert_dp_from_argv(boot_services, dp_from_text, shell_params,
				1, "uefi_rw_a_image") ||
	    insert_dp_from_argv(boot_services, dp_from_text, shell_params,
				2, "uefi_rw_b_image")) {
		return 1;
	}

	return 0;
}



void module_main(void)
{
	if (prepare_fwdb_storage())
		halt();

	if (vboot_init())
		halt();
}
