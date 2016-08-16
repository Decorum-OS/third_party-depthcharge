/*
 * Copyright 2014 Google Inc.
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
#include <stdlib.h>

#include "base/xalloc.h"
#include "drivers/board/board.h"
#include "drivers/storage/storage.h"
#include "vboot_api.h"

/*
 * NVRAM storage in flash uses a block of flash memory to represent the NVRAM
 * blob. Typical flash memory allows changing of individual bits from one to
 * zero. Changing bits from zero to one requires an erase operation, which
 * affects entire blocks of storage.
 *
 * In a typical case the last non-erased blob of CONFIG_NVRAM_BLOCK_SIZE bytes
 * in the dedicated block is considered the current NVRAM contents. If there
 * is a need to change the NVRAM contents, the next blob worth of bytes is
 * written. It becomes the last non-erased blob, which is by definition the
 * current NVRAM contents.
 *
 * If the entire dedicated block is empty, the first blob is used as the
 * NVRAM. It will be considered invalid and overwritten by vboot as required.
 *
 * If there is no room in the dedicated block to store a new blob - the NVRAM
 * write operation would fail.
 *
 * This scheme expects the user space application to manage the allocated
 * block, erasing it and initializing the lowest blob with the current NVRAM
 * contents once it gets close to capacity.
 */

// Offset of the actual NVRAM blob offset in the NVRAM block.
static int nvram_blob_offset;

// Local cache of the NVRAM blob.
static uint8_t nvram_cache[VBNV_BLOCK_SIZE];

static StorageOps *nvram_storage;
static int nvram_storage_size;

static int flash_nvram_init(void)
{
	StorageOps *storage = board_storage_vboot_nvstorage();
	nvram_storage_size = storage_size(nvram_storage);
	if (nvram_storage_size < 0)
		return -1;

	uint8_t *data = xmalloc(nvram_storage_size);
	if (storage_read(nvram_storage, data, 0, nvram_storage_size)) {
		free(data);
		return -1;
	}

	// Prepare an empty NVRAM block to compare against.
	uint8_t empty_block[sizeof(nvram_cache)];
	memset(empty_block, 0xff, sizeof(empty_block));

	// Find the first completely empty NVRAM blob. The actual NVRAM
	// blob will be right before it.
	int last_offset = 0;
	const int size = sizeof(nvram_cache);
	for (int offset = size; offset < nvram_storage_size; offset += size) {
		if (!memcmp(data + offset, empty_block, size))
			break;
		last_offset = offset;
	}

	nvram_storage = storage;
	nvram_blob_offset = last_offset;
	memcpy(nvram_cache, data + last_offset, size);
	free(data);
	return 0;
}

VbError_t VbExNvStorageRead(uint8_t *buf)
{
	if (!nvram_storage && flash_nvram_init())
		return VBERROR_UNKNOWN;

	memcpy(buf, nvram_cache, sizeof(nvram_cache));
	return VBERROR_SUCCESS;
}

VbError_t VbExNvStorageWrite(const uint8_t *buf)
{
	const int cache_size = sizeof(nvram_cache);

	if (!nvram_storage && flash_nvram_init())
		return VBERROR_UNKNOWN;

	// Bail out if there have been no changes.
	if (!memcmp(buf, nvram_cache, cache_size))
		return VBERROR_SUCCESS;

	// See if we can overwrite the current blob with the new one.
	int i;
	for (i = 0; i < cache_size; i++)
		if ((nvram_cache[i] & buf[i]) != buf[i])
			break;

	if (i != cache_size) {
		// We won't be able to update the existing entry. Check if
		// the next is available.
		int new_offset = nvram_blob_offset + cache_size;
		if (new_offset >= nvram_storage_size) {
			// Nvram is full, so erase it.
			uint8_t *empty = xmalloc(nvram_storage_size);
			memset(empty, 0xff, nvram_storage_size);
			if (storage_write(nvram_storage, empty, 0,
					  nvram_storage_size)) {
				free(empty);
				return VBERROR_UNKNOWN;
			}
			free(empty);
			new_offset = 0;
		}
		nvram_blob_offset = new_offset;
	}

	if (storage_write(nvram_storage, buf, nvram_blob_offset, cache_size))
		return VBERROR_UNKNOWN;

	memcpy(nvram_cache, buf, cache_size);
	return VBERROR_SUCCESS;
}

int nvstorage_flash_get_offset(void)
{
	return nvram_blob_offset;
}
