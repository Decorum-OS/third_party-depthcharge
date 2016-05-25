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

#include "module/fsp/fsp.h"

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

typedef struct __attribute__((packed))
{
	uint64_t pcd_vpd_region_sign;
	uint32_t pcd_image_revision;
	uint32_t pcd_upd_region_offset;
	uint32_t pcd_upd_region_size;
	uint8_t _reserved_0[12];
} FspV1_1Vpd;

typedef struct __attribute__((packed))
{
	uint64_t signature;
	uint64_t _reserved_0;
} FspV1_1Upd;

#endif /* __MODULE_FSP_V1_1_FSP_H__ */
