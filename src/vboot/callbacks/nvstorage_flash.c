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

#include "board/board.h"
#include "drivers/flash/flash.h"
#include "image/fmap.h"
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

static uint32_t nvram_area_size;
static uint32_t nvram_area_offset;

static int nvram_initialized;

static int flash_nvram_init(void)
{
	FmapArea fmap_area;
	if (fmap_find_area("RW_NVRAM", &fmap_area)) {
		printf("%s: failed to find NVRAM area\n", __func__);
		return -1;
	}
	nvram_area_size = fmap_area.size;
	nvram_area_offset = fmap_area.offset;

	uint8_t *data = flash_read(nvram_area_offset, nvram_area_size);
	if (!data) {
		printf("%s: failed to read NVRAM area\n", __func__);
		return -1;
	}

	// Prepare an empty NVRAM block to compare against.
	uint8_t empty_block[sizeof(nvram_cache)];
	memset(empty_block, 0xff, sizeof(empty_block));

	// Find the first completely empty NVRAM blob. The actual NVRAM
	// blob will be right below it.
	int last_offset = 0;
	const int size = sizeof(nvram_cache);
	for (int offset = size; offset < nvram_area_size; offset += size) {
		if (!memcmp(data + offset, empty_block, size))
			break;
		last_offset = offset;
	}

	memcpy(nvram_cache, data + last_offset, size);
	nvram_blob_offset = last_offset;
	nvram_initialized = 1;
	return 0;
}

VbError_t VbExNvStorageRead(uint8_t *buf)
{
	if (!nvram_initialized && flash_nvram_init())
		return VBERROR_UNKNOWN;

	memcpy(buf, nvram_cache, sizeof(nvram_cache));
	return VBERROR_SUCCESS;
}

static int erase_nvram(void)
{
	if (flash_erase(nvram_area_offset, nvram_area_size) != nvram_area_size)
		return 1;
	return 0;
}

VbError_t VbExNvStorageWrite(const uint8_t *buf)
{
	const int cache_size = sizeof(nvram_cache);

	if (!nvram_initialized && flash_nvram_init())
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
		if (new_offset >= nvram_area_size) {
			printf("Nvram is full, so erase it.\n");
			if (erase_nvram())
				return VBERROR_UNKNOWN;
			new_offset = 0;
		}
		nvram_blob_offset = new_offset;
	}

	if (flash_write(nvram_area_offset + nvram_blob_offset,
			cache_size, buf) != cache_size)
		return VBERROR_UNKNOWN;

	memcpy(nvram_cache, buf, cache_size);
	return VBERROR_SUCCESS;
}

int nvstorage_flash_get_offset(void)
{
	return nvram_blob_offset;
}
