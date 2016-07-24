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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <vboot_api.h>

#include "base/algorithm.h"
#include "base/xalloc.h"
#include "board/board.h"
#include "drivers/storage/storage.h"

VbError_t VbExHashFirmwareBody(VbCommonParams *cparams,
			       uint32_t firmware_index)
{
	StorageOps *fw;

	if (firmware_index == VB_SELECT_FIRMWARE_A) {
		fw = board_storage_verified_a();
	} else if (firmware_index == VB_SELECT_FIRMWARE_B) {
		fw = board_storage_verified_b();
	} else {
		printf("Unrecognized firmware index %d.\n", firmware_index);
		return VBERROR_UNKNOWN;
	}

	int size = storage_size(fw);
	if (size < 0)
		return VBERROR_UNKNOWN;

	size_t chunk_size = MIN(64 * 1024, size);
	void *data = xmalloc(chunk_size);

	uint64_t offset = 0;
	while (size) {
		if (storage_read(fw, data, offset, chunk_size)) {
			free(data);
			return VBERROR_UNKNOWN;
		}

		VbUpdateFirmwareBodyHash(cparams, data, chunk_size);

		offset += chunk_size;
		size -= chunk_size;
		// Process as much as we did last time, or whatever is left.
		chunk_size = MIN(chunk_size, size);
	}

	free(data);
	return VBERROR_SUCCESS;
}
