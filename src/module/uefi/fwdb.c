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

#include <inttypes.h>
#include <stdio.h>

#include "base/xalloc.h"
#include "base/fwdb.h"
#include "base/physmem.h"
#include "module/uefi/fwdb.h"
#include "uefi/edk/Protocol/EfiShell.h"
#include "uefi/edk/Protocol/EfiShellParameters.h"
#include "uefi/edk/Protocol/SimpleFileSystem.h"
#include "uefi/uefi.h"
#include "vboot/util/memory.h"



extern uint8_t _binary_ro_image_start;
extern uint8_t _binary_ro_image_size;
extern uint8_t ImageBase;



static EFI_GUID shell_parameters_protocol_guid =
	EFI_SHELL_PARAMETERS_PROTOCOL_GUID;

static EFI_GUID shell_protocol_guid = EFI_SHELL_PROTOCOL_GUID;



static EFI_SHELL_PARAMETERS_PROTOCOL *get_shell_parameters(
	EFI_BOOT_SERVICES *bs)
{
	EFI_HANDLE handle;
	if (uefi_image_handle(&handle))
		return NULL;

	EFI_SHELL_PARAMETERS_PROTOCOL *shell_params;
	EFI_STATUS status = bs->HandleProtocol(
		handle, &shell_parameters_protocol_guid,
		(void **)&shell_params);

	if (status != EFI_SUCCESS) {
		printf("No shell parameter protocol found.\n");
		return NULL;
	}

	return shell_params;
}

static EFI_SHELL_PROTOCOL *get_shell_protocol(EFI_BOOT_SERVICES *bs)
{
	EFI_SHELL_PROTOCOL *shell_prot;

	UINTN buf_size = 0;
	EFI_HANDLE dummy;
	EFI_STATUS status = bs->LocateHandle(
		ByProtocol, &shell_protocol_guid, NULL, &buf_size, &dummy);
	if (status == EFI_NOT_FOUND) {
		printf("No shell protocol found.\n");
		return NULL;
	}
	if (status != EFI_BUFFER_TOO_SMALL) {
		printf("Error retrieving shell protocol handles.\n");
		return NULL;
	}

	EFI_HANDLE *handles = xmalloc(buf_size);

	status = bs->LocateHandle(ByProtocol, &shell_protocol_guid,
				  NULL, &buf_size, handles);
	if (status != EFI_SUCCESS) {
		printf("Failed to retrieve shell protocol handles.\n");
		return NULL;
	}

	int handle_count = buf_size / sizeof(dummy);
	if (handle_count > 1)
		printf("More than one shell found?\n");
	EFI_HANDLE handle = handles[0];
	free(handles);

	status = bs->HandleProtocol(handle, &shell_protocol_guid,
				    (void **)&shell_prot);
	if (status != EFI_SUCCESS) {
		printf("Failed to retrieve shell protocol.\n");
		return NULL;
	}

	return shell_prot;
}



static int insert_file_into_fwdb(EFI_SHELL_PROTOCOL *shell_prot,
				 SHELL_FILE_HANDLE file, const char *name)
{
	UINT64 size;
	EFI_STATUS status = shell_prot->GetFileSize(file, &size);
	if (status != EFI_SUCCESS) {
		printf("Failed to get file size.\n");
		return 1;
	}

	FwdbEntry entry = { .ptr = NULL, .size = size };
	if (fwdb_access(name, NULL, &entry) || fwdb_access(name, &entry, NULL))
		return 1;

	UINTN buffer_size = size;
	status = shell_prot->ReadFile(file, &buffer_size, entry.ptr);
	if (status != EFI_SUCCESS) {
		printf("Failed to read file.\n");
		return 1;
	}

	return 0;
}

static int insert_file_name_into_fwdb(EFI_SHELL_PROTOCOL *shell_prot,
				      CHAR16 *file_name, const char *name)
{
	SHELL_FILE_HANDLE file;
	EFI_STATUS status = shell_prot->OpenFileByName(
		file_name, &file, EFI_FILE_MODE_READ);
	if (status != EFI_SUCCESS) {
		printf("Failed to open read/write image.\n");
		return 1;
	}
	int ret = insert_file_into_fwdb(shell_prot, file, name);
	shell_prot->CloseFile(file);
	return ret;
}

int uefi_prepare_fwdb_storage(void)
{
	FwdbEntry ro_image_entry = {
		.ptr = &_binary_ro_image_start,
		.size = &_binary_ro_image_size - &ImageBase,
	};
	if (fwdb_access("uefi_ro_image", NULL, &ro_image_entry))
		return 1;

	EFI_SYSTEM_TABLE *st = uefi_system_table_ptr();
	if (!st)
		return 1;
	EFI_BOOT_SERVICES *bs = st->BootServices;

	EFI_SHELL_PARAMETERS_PROTOCOL *shell_params = get_shell_parameters(bs);
	EFI_SHELL_PROTOCOL *shell_prot = get_shell_protocol(bs);

	if (!shell_params || !shell_prot)
		return 1;

	if (shell_params->Argc != 3) {
		printf("Bad number of arguments.\n");
		printf("Usage: dc <rwa image> <rwb image>\n");
		return 1;
	}

	if (insert_file_name_into_fwdb(shell_prot, shell_params->Argv[1],
				       "uefi_rw_a_image") ||
	    insert_file_name_into_fwdb(shell_prot, shell_params->Argv[2],
				       "uefi_rw_b_image")) {
		return 1;
	}

	return 0;
}


int uefi_prepare_fwdb_e820_map(void)
{
	if (prepare_e820_mem_ranges())
		return 1;

	E820MemRanges *e820 = get_e820_mem_ranges();
	if (!e820)
		return 1;

	unsigned size = 0;
	EFI_MEMORY_DESCRIPTOR *map;
	unsigned desc_size;
	uint32_t desc_ver;

	if (uefi_get_memory_map(&size, &map, &desc_size, &desc_ver))
		return 1;

	int num_descs = size / desc_size;
	if (num_descs > ARRAY_SIZE(e820->ranges)) {
		printf("Too many memory ranges to fit in the FWDB map.\n");
		free(map);
		return 1;
	}

	if (desc_size < sizeof(EFI_MEMORY_DESCRIPTOR)) {
		printf("Descriptor size is too small?\n");
		free(map);
		return 1;
	}

	int num_ranges = 0;
	for (int i = 0; i < num_descs; i++) {
		EFI_MEMORY_DESCRIPTOR *desc =
			(void *)((uint8_t *)map + i * desc_size);
		E820MemRange *range = &e820->ranges[num_ranges];

		range->base = desc->PhysicalStart;
		range->size = desc->NumberOfPages * 4 * 1024;

		if (range->size == 0)
			continue;
		num_ranges++;

		switch (desc->Type) {
		case EfiLoaderCode:
		case EfiLoaderData:
		case EfiBootServicesCode:
		case EfiBootServicesData:
			memory_mark_used(range->base,
					 range->base + range->size);
		case EfiConventionalMemory:
			range->type = E820MemRange_Ram;
			break;
		case EfiReservedMemoryType:
		case EfiRuntimeServicesCode:
		case EfiRuntimeServicesData:
		case EfiPalCode:
		case EfiACPIMemoryNVS:
			range->type = E820MemRange_Reserved;
			break;
		case EfiACPIReclaimMemory:
			range->type = E820MemRange_Acpi;
			break;
		case EfiPersistentMemory:
			range->type = E820MemRange_Nvs;
			break;
		case EfiUnusableMemory:
			range->type = E820MemRange_Unusable;
			break;
		default:
			printf("Warning: Memory range of type %#"PRIx32" "
			       "marked as reserved.\n", desc->Type);
			range->type = E820MemRange_Reserved;
		}

		range->handoff_tag = desc->Type;
	}

	e820->num_ranges = num_ranges;
	free(map);
	return 0;
}
