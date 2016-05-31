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
#include <string.h>

#include "module/fsp/temp_stack.h"
#include "module/fsp/fsp.h"

void temp_stack_print_guid(EfiGuid *guid)
{
	temp_stack_print_num32(guid->data1);
	temp_stack_puts("-");
	temp_stack_print_num16(guid->data2);
	temp_stack_puts("-");
	temp_stack_print_num16(guid->data3);
	for (int i = 0; i < 8; i++) {
		temp_stack_puts("-");
		temp_stack_print_num8(guid->data4[i]);
	}
}

void temp_stack_print_hob_list(EfiHobPointers hob_list_ptr)
{
	while (hob_list_ptr.header->hob_type != EfiHobTypeEndOfHobList) {
		switch (hob_list_ptr.header->hob_type) {
		// Ignore these types of HOBs.
		case EfiHobTypeHandoff:
		case EfiHobTypeFv:
		case EfiHobTypeCpu:
		case EfiHobTypeMemoryPool:
		case EfiHobTypeUnused:
			break;
		case EfiHobTypeMemoryAllocation:
		{
			temp_stack_puts("Hob type = memory allocation\n");
			EfiHobMemoryAllocation *mem_alloc =
				hob_list_ptr.memory_allocation;

			temp_stack_puts("  Name = ");
			temp_stack_print_guid(
				&mem_alloc->alloc_descriptor.name);
			temp_stack_puts("\n");

			temp_stack_puts("  base address = ");
			temp_stack_print_num64(mem_alloc->alloc_descriptor.
					       memory_base_address);
			temp_stack_puts("\n");

			temp_stack_puts("  length = ");
			temp_stack_print_num64(mem_alloc->alloc_descriptor.
					       memory_length);
			temp_stack_puts("\n");

			temp_stack_puts("  type = ");
			switch (mem_alloc->alloc_descriptor.memory_type) {
			case EfiMemTypeReserved:
				temp_stack_puts("reserved\n");
				break;
			case EfiMemTypeLoaderCode:
				temp_stack_puts("loader code\n");
				break;
			case EfiMemTypeLoaderData:
				temp_stack_puts("loader data\n");
				break;
			case EfiMemTypeServicesCode:
				temp_stack_puts("services code\n");
				break;
			case EfiMemTypeServicesData:
				temp_stack_puts("services data\n");
				break;
			case EfiMemTypeConventional:
				temp_stack_puts("conventional\n");
				break;
			case EfiMemTypeUnusable:
				temp_stack_puts("unusable\n");
				break;
			case EfiMemTypeAcpiReclaim:
				temp_stack_puts("acpi reclaim\n");
				break;
			case EfiMemTypeAcpiNvs:
				temp_stack_puts("acpi nvs\n");
				break;
			case EfiMemTypeMemMappedIo:
				temp_stack_puts("memory mapped io\n");
				break;
			case EfiMemTypeMemMappedIoPortSpace:
				temp_stack_puts("memory mapped io port\n");
				break;
			case EfiMemTypePalCode:
				temp_stack_puts("pal code\n");
				break;
			}
			break;
		}
		case EfiHobTypeResourceDescriptor:
		{
			temp_stack_puts("Hob type = resource descriptor\n");
			EfiHobResourceDescriptor *res_desc =
				hob_list_ptr.resource_descriptor;

			temp_stack_puts("  owner = ");
			temp_stack_print_guid(&res_desc->owner);
			temp_stack_puts("\n");

			temp_stack_puts("  resource_type = ");
			switch (res_desc->resource_type) {
			case EfiResourceSystemMemory:
				temp_stack_puts("system memory\n");
				break;
			case EfiResourceMemoryMappedIo:
				temp_stack_puts("memory mapped io\n");
				break;
			case EfiResourceIo:
				temp_stack_puts("io\n");
				break;
			case EfiResourceFirmwareDevice:
				temp_stack_puts("firmware device\n");
				break;
			case EfiResourceMemoryMappedIoPort:
				temp_stack_puts("memory mapped io port\n");
				break;
			case EfiResourceMemoryReserved:
				temp_stack_puts("reserved\n");
				break;
			case EfiResourceIoReserved:
				break;
			}

			temp_stack_puts("  Attributes:\n");
			if (res_desc->resource_attribute &
			    EfiResourceAttributePresent)
				temp_stack_puts("    present\n");
			if (res_desc->resource_attribute &
			    EfiResourceAttributeInitialized)
				temp_stack_puts("    initialized\n");
			if (res_desc->resource_attribute &
			    EfiResourceAttributeTested)
				temp_stack_puts("    tested\n");
			if (res_desc->resource_attribute &
			    EfiResourceAttributeSingleBitEcc)
				temp_stack_puts("    single bit ecc\n");
			if (res_desc->resource_attribute &
			    EfiResourceAttributeMultipleBitEcc)
				temp_stack_puts("    multiple bit ecc\n");
			if (res_desc->resource_attribute &
			    EfiResourceAttributeEccReserved_1)
				temp_stack_puts("    ecc reserved 1\n");
			if (res_desc->resource_attribute &
			    EfiResourceAttributeEccReserved_2)
				temp_stack_puts("    ecc reserved 2\n");
			if (res_desc->resource_attribute &
			    EfiResourceAttributeReadProtected)
				temp_stack_puts("    read protected\n");
			if (res_desc->resource_attribute &
			    EfiResourceAttributeWriteProtected)
				temp_stack_puts("    write protected\n");
			if (res_desc->resource_attribute &
			    EfiResourceAttributeExecutionProtected)
				temp_stack_puts("    execution protected\n");
			if (res_desc->resource_attribute &
			    EfiResourceAttributeUncacheable)
				temp_stack_puts("    uncacheable\n");
			if (res_desc->resource_attribute &
			    EfiResourceAttributeWriteCombineable)
				temp_stack_puts("    write combineable\n");
			if (res_desc->resource_attribute &
			    EfiResourceAttributeWriteThroughCacheable)
				temp_stack_puts("    write through"
						" cacheable\n");
			if (res_desc->resource_attribute &
			    EfiResourceAttributeWriteBackCacheable)
				temp_stack_puts("    write back cacheable\n");
			if (res_desc->resource_attribute &
			    EfiResourceAttribute16BitIo)
				temp_stack_puts("    16 bit io\n");
			if (res_desc->resource_attribute &
			    EfiResourceAttribute32BitIo)
				temp_stack_puts("    32 bit io\n");
			if (res_desc->resource_attribute &
			    EfiResourceAttribute64BitIo)
				temp_stack_puts("    64 bit io\n");
			if (res_desc->resource_attribute &
			    EfiResourceAttributeUncachedExported)
				temp_stack_puts("    uncached exported\n");

			temp_stack_puts("  physical start = ");
			temp_stack_print_num64(res_desc->physical_start);
			temp_stack_puts("\n");

			temp_stack_puts("  resource length = ");
			temp_stack_print_num64(res_desc->resource_length);
			temp_stack_puts("\n");
			break;
		}
		case EfiHobTypeGuidExtension:
		{
			temp_stack_puts("Hob type = guid extension\n");
			EfiHobGuidType *guid = hob_list_ptr.guid;

			temp_stack_puts("  Name = ");
			temp_stack_print_guid(&guid->name);
			temp_stack_puts("\n");
			break;
		}
		default:
			temp_stack_puts("Hob type = unknown? (");
			temp_stack_print_num32(hob_list_ptr.header->hob_type);
			temp_stack_puts(")\n");
			break;
		};

		hob_list_ptr.raw += hob_list_ptr.header->hob_length;
	};
}
