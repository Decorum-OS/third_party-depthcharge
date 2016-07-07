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
#include <stdlib.h>

#include "base/xalloc.h"
#include "board/board.h"
#include "drivers/storage/storage.h"
#include "vboot/util/gbb.h"

const char *gbb_read_hwid(size_t *hwid_size_ptr)
{
	static const char *hwid;
	if (hwid)
		return hwid;

	StorageOps *gbb = board_storage_gbb();

	typedef member_typeof(GoogleBinaryBlockHeader, hwid_offset)
		hwid_offset_t;
	typedef member_typeof(GoogleBinaryBlockHeader, hwid_size)
		hwid_size_t;
	size_t hwid_offset_offset =
		offsetof(GoogleBinaryBlockHeader, hwid_offset);
	size_t hwid_size_offset =
		offsetof(GoogleBinaryBlockHeader, hwid_size);

	hwid_offset_t hwid_offset;
	if (storage_read(gbb, &hwid_offset, hwid_offset_offset,
			 sizeof(hwid_offset))) {
		printf("Failed to read HWID offset.\n");
		return NULL;
	}

	hwid_size_t hwid_size;
	if (storage_read(gbb, &hwid_size, hwid_size_offset,
			 sizeof(hwid_size))) {
		printf("Failed to read HWID size.\n");
		return NULL;
	}

	if (hwid_size_ptr)
		*hwid_size_ptr = hwid_size;

	hwid = xmalloc(hwid_size);
	if (storage_read(gbb, (void *)hwid, hwid_offset, hwid_size)) {
		printf("Failed to read HWID from the GBB.\n");
		free((void *)hwid);
		return NULL;
	}

	return hwid;
}
