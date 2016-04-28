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

#include <gbb_header.h>
#include <stddef.h>
#include <stdio.h>

#include "drivers/flash/flash.h"
#include "image/fmap.h"
#include "vboot/util/gbb.h"

const char *gbb_read_hwid(size_t *hwid_size)
{
	static const char *hwid;
	if (hwid)
		return hwid;

	FmapArea area;
	if (fmap_find_area("GBB", &area)) {
		printf("Couldn't find the GBB.\n");
		return NULL;
	}

	typedef member_typeof(GoogleBinaryBlockHeader, hwid_offset)
		hwid_offset_t;
	size_t hwid_offset_offset =
		offsetof(GoogleBinaryBlockHeader, hwid_offset) + area.offset;
	hwid_offset_t *offset_ptr =
		flash_read(hwid_offset_offset, sizeof(*offset_ptr));
	if (!offset_ptr) {
		printf("Failed to read hwid offset.\n");
		return NULL;
	}
	hwid_offset_t offset = *offset_ptr;

	typedef member_typeof(GoogleBinaryBlockHeader, hwid_size)
		hwid_size_t;
	size_t hwid_size_offset =
		offsetof(GoogleBinaryBlockHeader, hwid_size) + area.offset;
	hwid_size_t *size_ptr =
		flash_read(hwid_size_offset, sizeof(*size_ptr));
	if (!size_ptr) {
		printf("Failed to read hwid size.\n");
		return NULL;
	}
	hwid_size_t size = *size_ptr;

	if (hwid_size)
		*hwid_size = size;

	return flash_read(area.offset + offset, size);
}
