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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <vboot_api.h>

#include "uefi/uefi.h"

static EFI_GUID nvstorage_vendor_guid =
{
	0x52bcc60a,
	0x91c7,
	0x4602,
	{ 0xa8, 0x7c, 0xc3, 0xbc, 0x7f, 0x89, 0x59, 0x9a }
};

static CHAR16 *nvstorage_var_name = L"vboot_nv_storage";

VbError_t VbExNvStorageRead(uint8_t* buf)
{
	EFI_SYSTEM_TABLE *st = uefi_system_table_ptr();
	if (!st)
		return VBERROR_UNKNOWN;
	EFI_RUNTIME_SERVICES *rs = st->RuntimeServices;

	UINTN size = VBNV_BLOCK_SIZE;
	EFI_STATUS status = rs->GetVariable(nvstorage_var_name,
					    &nvstorage_vendor_guid,
					    NULL, &size, buf);
	if (status == EFI_NOT_FOUND || status == EFI_BUFFER_TOO_SMALL) {
		printf("NV storage not found or too large.\n");
		printf("Returning all zeroes.\n");
		memset(buf, 0, VBNV_BLOCK_SIZE);
		return VBERROR_SUCCESS;
	} else if (status != EFI_SUCCESS) {
		printf("Error reading NV storage.\n");
		return VBERROR_UNKNOWN;
	}

	return VBERROR_SUCCESS;
}

VbError_t VbExNvStorageWrite(const uint8_t* buf)
{
	EFI_SYSTEM_TABLE *st = uefi_system_table_ptr();
	if (!st)
		return VBERROR_UNKNOWN;
	EFI_RUNTIME_SERVICES *rs = st->RuntimeServices;

	UINTN attributes =
		EFI_VARIABLE_NON_VOLATILE |
		EFI_VARIABLE_BOOTSERVICE_ACCESS |
		EFI_VARIABLE_RUNTIME_ACCESS;

	// Write with size 0 to erase any existing version of this variable.
	EFI_STATUS status = rs->SetVariable(nvstorage_var_name,
					    &nvstorage_vendor_guid, 0, 0,
					    (void *)buf);
	if (status != EFI_SUCCESS && status != EFI_NOT_FOUND) {
		printf("Failed to erase previous version of nvstorage.\n");
		return VBERROR_UNKNOWN;
	}
	status = rs->SetVariable(nvstorage_var_name, &nvstorage_vendor_guid,
				 attributes, VBNV_BLOCK_SIZE, (void *)buf);
	if (status != EFI_SUCCESS) {
		printf("Error writing NV storage.\n");
		return VBERROR_UNKNOWN;
	}

	return VBERROR_SUCCESS;
}
