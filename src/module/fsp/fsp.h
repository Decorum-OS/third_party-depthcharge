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

#ifndef __MODULE_FSP_FSP_H__
#define __MODULE_FSP_FSP_H__

#define _FSP_SUCCESS			0x00000000
#define _FSP_INVALID_PARAMETER		0x80000002
#define _FSP_UNSUPPORTED		0x80000003
#define _FSP_NOT_READY			0x80000006
#define _FSP_DEVICE_ERROR		0x80000007
#define _FSP_OUT_OF_RESOURCES		0x80000009
#define _FSP_VOLUME_CORRUPTED		0x8000000a
#define _FSP_NOT_FOUND			0x8000000e
#define _FSP_TIMEOUT			0x80000012
#define _FSP_ABORTED			0x80000015
#define _FSP_ALREADY_STARTED		0x80000014
#define _FSP_INCOMPATIBLE_VERSION	0x80000010
#define _FSP_SECURITY_VIOLATION		0x8000001a
#define _FSP_CRC_ERROR			0x8000001b

#ifndef __ASSEMBLER__

#include <stdint.h>

enum
{
	FspSuccess = _FSP_SUCCESS,
	FspInvalidParameter = _FSP_INVALID_PARAMETER,
	FspUnsupported = _FSP_UNSUPPORTED,
	FspNotReady = _FSP_NOT_READY,
	FspDeviceError = _FSP_DEVICE_ERROR,
	FspOutOfResources = _FSP_OUT_OF_RESOURCES,
	FspVolumeCorrupted = _FSP_VOLUME_CORRUPTED,
	FspNotFound = _FSP_NOT_FOUND,
	FspTimeout = _FSP_TIMEOUT,
	FspAborted = _FSP_ABORTED,
	FspAlreadyStarted = _FSP_ALREADY_STARTED,
	FspIncompatibleVersion = _FSP_INCOMPATIBLE_VERSION,
	FspSecurityViolation = _FSP_SECURITY_VIOLATION,
	FspCrcError = _FSP_CRC_ERROR
};

typedef struct __attribute__((packed))
{
	uint32_t data1;
	uint16_t data2;
	uint16_t data3;
	uint8_t data4[8];
} EfiGuid;

enum
{
	EfiMemTypeReserved,
	EfiMemTypeLoaderCode,
	EfiMemTypeLoaderData,
	EfiMemTypeServicesCode,
	EfiMemTypeServicesData,
	EfiMemTypeConventional,
	EfiMemTypeUnusable,
	EfiMemTypeAcpiReclaim,
	EfiMemTypeAcpiNvs,
	EfiMemTypeMemMappedIo,
	EfiMemTypeMemMappedIoPortSpace,
	EfiMemTypePalCode,
	EfiMemTypeMax,
};

typedef uint32_t EfiResource;
typedef uint32_t EfiResourceAttribute;

// Values for resource_type in EfiHobResourceDescriptor.
enum
{
	EfiResourceSystemMemory = 0,
	EfiResourceMemoryMappedIo = 1,
	EfiResourceIo = 2,
	EfiResourceFirmwareDevice = 3,
	EfiResourceMemoryMappedIoPort = 4,
	EfiResourceMemoryReserved = 5,
	EfiResourceIoReserved = 6,
	EfiResourceMaxMemoryType = 7,
};

// Resource settings.
enum
{
	EfiResourceAttributePresent = 0x1,
	EfiResourceAttributeInitialized = 0x2,
	EfiResourceAttributeTested = 0x4,
};

// Resource capabilities.
enum
{
	EfiResourceAttributeSingleBitEcc = 0x8,
	EfiResourceAttributeMultipleBitEcc = 0x10,
	EfiResourceAttributeEccReserved_1 = 0x20,
	EfiResourceAttributeEccReserved_2 = 0x40,
	EfiResourceAttributeReadProtected = 0x80,
	EfiResourceAttributeWriteProtected = 0x100,
	EfiResourceAttributeExecutionProtected = 0x200,
	EfiResourceAttributeUncacheable = 0x400,
	EfiResourceAttributeWriteCombineable = 0x800,
	EfiResourceAttributeWriteThroughCacheable = 0x1000,
	EfiResourceAttributeWriteBackCacheable = 0x2000,
	EfiResourceAttribute16BitIo = 0x4000,
	EfiResourceAttribute32BitIo = 0x8000,
	EfiResourceAttribute64BitIo = 0x10000,
	EfiResourceAttributeUncachedExported = 0x20000,
};

// Values for hob_type in EfiHobGenericHeader.
enum
{
	EfiHobTypeHandoff = 0x1,
	EfiHobTypeMemoryAllocation = 0x2,
	EfiHobTypeResourceDescriptor = 0x3,
	EfiHobTypeGuidExtension = 0x4,
	EfiHobTypeFv = 0x5,
	EfiHobTypeCpu = 0x6,
	EfiHobTypeMemoryPool = 0x7,
	EfiHobTypeUnused = 0xfffe,
	EfiHobTypeEndOfHobList = 0xffff,
};

typedef struct __attribute__((packed))
{
	uint16_t hob_type;
	uint16_t hob_length;
	uint32_t reserved;
} EfiHobGenericHeader;

typedef struct __attribute__((packed))
{
	EfiGuid name;
	uint64_t memory_base_address;
	uint64_t memory_length;
	uint32_t memory_type;
	uint8_t _reserved_0[4];
} EfiHobMemoryAllocationHeader;

typedef struct __attribute__((packed))
{
	EfiHobGenericHeader header;
	EfiHobMemoryAllocationHeader alloc_descriptor;
} EfiHobMemoryAllocation;

typedef struct __attribute__((packed))
{
	EfiHobGenericHeader header;
	EfiGuid owner;
	EfiResource resource_type;
	EfiResourceAttribute resource_attribute;
	uint64_t physical_start;
	uint64_t resource_length;
} EfiHobResourceDescriptor;

typedef struct __attribute__((packed))
{
	EfiHobGenericHeader header;
	EfiGuid name;
} EfiHobGuidType;

typedef union
{
	EfiHobGenericHeader *header;
	EfiHobMemoryAllocation *memory_allocation;
	EfiHobResourceDescriptor *resource_descriptor;
	EfiHobGuidType *guid;
	uint8_t *raw;
} EfiHobPointers;

#else /* __ASSEMBLER__ */

#define FSP_SUCCESS			_FSP_SUCCESS
#define FSP_INVALID_PARAMETER		_FSP_INVALID_PARAMETER
#define FSP_UNSUPPORTED			_FSP_UNSUPPORTED
#define FSP_NOT_READY			_FSP_NOT_READY
#define FSP_DEVICE_ERROR		_FSP_DEVICE_ERROR
#define FSP_OUT_OF_RESOURCES		_FSP_OUT_OF_RESOURCES
#define FSP_VOLUME_CORRUPTED		_FSP_VOLUME_CORRUPTED
#define FSP_NOT_FOUND			_FSP_NOT_FOUND
#define FSP_TIMEOUT			_FSP_TIMEOUT
#define FSP_ABORTED			_FSP_ABORTED
#define FSP_ALREADY_STARTED		_FSP_ALREADY_STARTED
#define FSP_INCOMPATIBLE_VERSION	_FSP_INCOMPATIBLE_VERSION
#define FSP_SECURITY_VIOLATION		_FSP_SECURITY_VIOLATION
#define FSP_CRC_ERROR			_FSP_CRC_ERROR

#endif

#endif /* __MODULE_FSP_FSP_H__ */
