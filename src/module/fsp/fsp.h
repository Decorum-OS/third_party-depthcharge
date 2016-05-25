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

typedef struct __attribute__((packed))
{
	uint32_t signature;
	uint32_t header_length;
	uint8_t reserved_1[3];
	uint8_t header_revision;
	struct __attribute__((packed))
	{
		uint8_t minor;
		uint8_t major;
		uint16_t reserved;
	} image_revision;
	uint64_t image_id;
	uint32_t image_size;
	uint32_t image_base;
	uint32_t image_attribute;
	uint32_t cfg_region_offset;
	uint32_t cfg_region_size;
	uint32_t api_entry_num;
	uint32_t temp_ram_init_entry_offset;
	uint32_t fsp_init_entry_offset;
	uint32_t notify_phase_entry_offset;
	uint32_t fsp_memory_init_entry_offset;
	uint32_t temp_ram_exit_entry_offset;
	uint32_t fsp_silicon_init_entry_offset;
} FspInformationHeader;

enum {
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
