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

#ifndef __MODULE_FSP_V1_1_FSP_H__
#define __MODULE_FSP_V1_1_FSP_H__

#define _FSP_V1_1_SUCCESS		0x00000000
#define _FSP_V1_1_INVALID_PARAMETER	0x80000002
#define _FSP_V1_1_UNSUPPORTED		0x80000003
#define _FSP_V1_1_NOT_READY		0x80000006
#define _FSP_V1_1_DEVICE_ERROR		0x80000007
#define _FSP_V1_1_OUT_OF_RESOURCES	0x80000009
#define _FSP_V1_1_VOLUME_CORRUPTED	0x8000000a
#define _FSP_V1_1_NOT_FOUND		0x8000000e
#define _FSP_V1_1_TIMEOUT		0x80000012
#define _FSP_V1_1_ABORTED		0x80000015
#define _FSP_V1_1_ALREADY_STARTED	0x80000014
#define _FSP_V1_1_INCOMPATIBLE_VERSION	0x80000010
#define _FSP_V1_1_SECURITY_VIOLATION	0x8000001a
#define _FSP_V1_1_CRC_ERROR		0x8000001b

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
} FspV1_1InformationHeader;

enum {
	FspV1_1Success = _FSP_V1_1_SUCCESS,
	FspV1_1InvalidParameter = _FSP_V1_1_INVALID_PARAMETER,
	FspV1_1Unsupported = _FSP_V1_1_UNSUPPORTED,
	FspV1_1NotReady = _FSP_V1_1_NOT_READY,
	FspV1_1DeviceError = _FSP_V1_1_DEVICE_ERROR,
	FspV1_1OutOfResources = _FSP_V1_1_OUT_OF_RESOURCES,
	FspV1_1VolumeCorrupted = _FSP_V1_1_VOLUME_CORRUPTED,
	FspV1_1NotFound = _FSP_V1_1_NOT_FOUND,
	FspV1_1Timeout = _FSP_V1_1_TIMEOUT,
	FspV1_1Aborted = _FSP_V1_1_ABORTED,
	FspV1_1AlreadyStarted = _FSP_V1_1_ALREADY_STARTED,
	FspV1_1IncompatibleVersion = _FSP_V1_1_INCOMPATIBLE_VERSION,
	FspV1_1SecurityViolation = _FSP_V1_1_SECURITY_VIOLATION,
	FspV1_1CrcError = _FSP_V1_1_CRC_ERROR
};

#else /* __ASSEMBLER__ */

#define FSP_V1_1_SUCCESS		_FSP_V1_1_SUCCESS
#define FSP_V1_1_INVALID_PARAMETER	_FSP_V1_1_INVALID_PARAMETER
#define FSP_V1_1_UNSUPPORTED		_FSP_V1_1_UNSUPPORTED
#define FSP_V1_1_NOT_READY		_FSP_V1_1_NOT_READY
#define FSP_V1_1_DEVICE_ERROR		_FSP_V1_1_DEVICE_ERROR
#define FSP_V1_1_OUT_OF_RESOURCES	_FSP_V1_1_OUT_OF_RESOURCES
#define FSP_V1_1_VOLUME_CORRUPTED	_FSP_V1_1_VOLUME_CORRUPTED
#define FSP_V1_1_NOT_FOUND		_FSP_V1_1_NOT_FOUND
#define FSP_V1_1_TIMEOUT		_FSP_V1_1_TIMEOUT
#define FSP_V1_1_ABORTED		_FSP_V1_1_ABORTED
#define FSP_V1_1_ALREADY_STARTED	_FSP_V1_1_ALREADY_STARTED
#define FSP_V1_1_INCOMPATIBLE_VERSION	_FSP_V1_1_INCOMPATIBLE_VERSION
#define FSP_V1_1_SECURITY_VIOLATION	_FSP_V1_1_SECURITY_VIOLATION
#define FSP_V1_1_CRC_ERROR		_FSP_V1_1_CRC_ERROR

#endif

#endif /* __MODULE_FSP_V1_1_FSP_H__ */
