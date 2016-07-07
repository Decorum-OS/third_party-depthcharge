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

#include <stdio.h>
#include <sys/types.h>

#include "base/fwdb.h"
#include "uefi/uefi.h"

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

	ptr = *(EFI_SYSTEM_TABLE **)entry.ptr;
	return ptr;
}
