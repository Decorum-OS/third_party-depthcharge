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
#include "uefi/edk/Protocol/EfiShellInterface.h"
#include "uefi/edk/Protocol/EfiShellParameters.h"
#include "uefi/edk/Protocol/LoadedImage.h"
#include "uefi/edk/Protocol/SimpleFileSystem.h"
#include "uefi/uefi.h"
#include "vboot/util/memory.h"



extern uint8_t _binary_ro_image_start;
extern uint8_t _binary_ro_image_size;
extern uint8_t ImageBase;



static EFI_GUID shell_parameters_protocol_guid =
	EFI_SHELL_PARAMETERS_PROTOCOL_GUID;
static EFI_GUID shell_protocol_guid = EFI_SHELL_PROTOCOL_GUID;
static EFI_GUID shell_interface_guid = SHELL_INTERFACE_PROTOCOL_GUID;

static EFI_GUID file_info_guid = EFI_FILE_INFO_ID;
static EFI_GUID loaded_image_protocol_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
static EFI_GUID simple_fs_protocol_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;



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

static EFI_SHELL_INTERFACE *get_shell_interface(EFI_BOOT_SERVICES *bs)
{
	EFI_HANDLE handle;
	if (uefi_image_handle(&handle))
		return NULL;

	EFI_SHELL_INTERFACE *shell_int;
	EFI_STATUS status = bs->HandleProtocol(
		handle, &shell_interface_guid, (void **)&shell_int);

	if (status != EFI_SUCCESS) {
		printf("No shell interface found.\n");
		return NULL;
	}

	return shell_int;
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

static int insert_file_name_into_fwdb_from_fs(EFI_FILE_PROTOCOL *root,
					      CHAR16 *file_name,
					      const char *name)
{
	EFI_FILE_PROTOCOL *file;
	EFI_STATUS status = root->Open(root, &file, file_name,
				       EFI_FILE_MODE_READ, 0);
	if (status != EFI_SUCCESS) {
		printf("Failed to open file.\n");
		return 1;
	}

	EFI_FILE_INFO file_info;
	UINTN buf_size = sizeof(file_info);
	status = file->GetInfo(file, &file_info_guid,
			       &buf_size, &file_info);
	if (status != EFI_SUCCESS) {
		file->Close(file);
		printf("Failed to get file size.\n");
		return 1;
	}

	FwdbEntry entry = { .ptr = NULL, .size = file_info.FileSize };
	if (fwdb_access(name, NULL, &entry) ||
	    fwdb_access(name, &entry, NULL)) {
		file->Close(file);
		return 1;
	}

	buf_size = entry.size;
	status = file->Read(file, &buf_size, entry.ptr);
	if (status != EFI_SUCCESS) {
		file->Close(file);
		printf("Failed to read file.\n");
		return 1;
	}

	file->Close(file);
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

	EFI_SHELL_PROTOCOL *shell_prot = get_shell_protocol(bs);
	EFI_SHELL_PARAMETERS_PROTOCOL *shell_params = NULL;
	EFI_SHELL_INTERFACE *shell_int = NULL;

	if (shell_prot) {
		printf("UEFI standard shell protocol found.\n");

		shell_params = get_shell_parameters(bs);
		if (!shell_params)
			return 1;

		if (shell_params->Argc != 3) {
			printf("Bad number of arguments.\n");
			printf("Usage: dc <rwa image> <rwb image>\n");
			return 1;
		}

		if (insert_file_name_into_fwdb(
			shell_prot, shell_params->Argv[1], "uefi_rw_a_image") ||
		    insert_file_name_into_fwdb(
			shell_prot, shell_params->Argv[2], "uefi_rw_b_image")) {
			return 1;
		}
	} else {
		printf("Falling back to non-standard shell "
		       "interface protocol.\n");
		shell_int = get_shell_interface(bs);

		if (shell_int->Argc != 1) {
			printf("Bad number of arguments.\n");
			printf("Usage: dc\n");
			printf("When using the non-standard interface, rwa "
			       "and rwb are assumed to be on \n");
			printf("the same device as the main depthcharge "
			       "executable at the path \n");
			printf("\\depthcharge\\rwa and \\depthcharge\\rwb.\n");
			return 1;
		}

		EFI_HANDLE handle;
		if (uefi_image_handle(&handle))
			return 1;

		EFI_LOADED_IMAGE_PROTOCOL *loaded_image;
		EFI_STATUS status =
			bs->HandleProtocol(handle, &loaded_image_protocol_guid,
					   (void **)&loaded_image);
		if (status != EFI_SUCCESS) {
			printf("Failed to open loaded image protocol.\n");
			return 1;
		}

		EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *simple_fs;
		status = bs->HandleProtocol(loaded_image->DeviceHandle,
					   &simple_fs_protocol_guid,
					   (void **)&simple_fs);
		if (status != EFI_SUCCESS) {
			printf("Failed to open simple fs protocol.\n");
			return 1;
		}

		EFI_FILE_PROTOCOL *root;
		status = simple_fs->OpenVolume(simple_fs, &root);
		if (status != EFI_SUCCESS) {
			printf("Failed to open simple fs root.\n");
			return 1;
		}

		int ret = insert_file_name_into_fwdb_from_fs(
			root, L"depthcharge\\rwa", "uefi_rw_a_image");
		ret = ret || insert_file_name_into_fwdb_from_fs(
			root, L"depthcharge\\rwb", "uefi_rw_b_image");

		status = root->Close(root);
		if (status != EFI_SUCCESS) {
			printf("Failed to close fs root.\n");
			return 1;
		}

		return ret;
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

	if (desc_size < sizeof(EFI_MEMORY_DESCRIPTOR)) {
		printf("Descriptor size is too small?\n");
		free(map);
		return 1;
	}

	int num_descs = size / desc_size;
	int num_ranges = 0;
	for (int i = 0; i < num_descs; i++) {
		EFI_MEMORY_DESCRIPTOR *desc =
			(void *)((uint8_t *)map + i * desc_size);

		if (desc->NumberOfPages == 0)
			continue;

		if (num_ranges >= ARRAY_SIZE(e820->ranges)) {
			printf("Too many memory ranges to fit in the "
			       "FWDB map.\n");
			free(map);
			return 1;
		}
		E820MemRange *range = &e820->ranges[num_ranges];

		range->base = desc->PhysicalStart;
		range->size = desc->NumberOfPages * 4 * 1024;

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
