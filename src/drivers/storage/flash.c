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

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "base/algorithm.h"
#include "base/container_of.h"
#include "base/die.h"
#include "base/xalloc.h"
#include "drivers/storage/flash.h"

static int flash_storage_read(StorageOps *me, void *buffer,
			      uint64_t offset, size_t size)
{
	FlashStorage *storage = container_of(me, FlashStorage, ops);

	void *result = storage->flash->read(storage->flash, offset, size);
	if (!result)
		return 1;

	memcpy(buffer, result, size);
	return 0;
}

static int flash_storage_write(StorageOps *me, const void *buffer,
			       uint64_t offset, size_t size)
{
	FlashStorage *storage = container_of(me, FlashStorage, ops);
	if (offset + size > storage->flash->size(storage->flash)) {
		printf("Write beyond end of flash storage.\n");
		return 1;
	}

	const int64_t erase_size = storage->flash->erase_size(storage->flash);
	if (erase_size < 0)
		return 1;
	// Make sure the erase size is a power of 2.
	assert(((erase_size - 1) & erase_size) == 0);
	const uint64_t erase_mask = ~((uint64_t)erase_size - 1);

	// Round the original offset down to an erase boundary.
	uint64_t aligned_offset = offset & erase_mask;
	// Calculate how much below the original offset that is.
	uint64_t extra_low = offset - aligned_offset;
	// Add that into the original size.
	uint64_t aligned_size = size + extra_low;
	// Now round the size up so that it's a multiple of the erase size.
	aligned_size = (aligned_size + ~erase_mask) & erase_mask;

	uint64_t input_pos = 0;

	// Process one flash sector at a time.
	for (uint64_t pos = aligned_offset;
	     pos < aligned_offset + aligned_size;
	     pos += erase_size) {

		// Read the existing contents.
		uint8_t *existing =
			storage->flash->read(storage->flash, pos, erase_size);
		if (!existing)
			return 1;
		// Set up a pointer to data to write to this sector.
		uint8_t *new_data = (uint8_t *)buffer + input_pos;
		// Figure out how many bytes long that is.
		int bytes = MIN(erase_size - extra_low, size - input_pos);

		// Loop until we would hit the end of either buffer.
		for (int i = 0; i < bytes; i++) {
			// We'll assume bits can change from 1 to 0 but not the
			// other way.
			uint8_t original = existing[i + extra_low];
			uint8_t new = new_data[i];
			if ((original & new) != new) {
				// A straight overwrite isn't possible, so
				// we need to erase this sector.
				if (storage->flash->erase(storage->flash,
							  pos, erase_size))
					return 1;
				// We already erased, so we can stop now.
				break;
			}
		}

		// Prepare the chunk of data to write.
		memcpy(existing + extra_low, new_data, bytes);
		uint8_t *write_buf = existing;
		int write_size = erase_size;
		uint64_t write_pos = pos;

		// We can skip over bytes that are all ones on either end.
		while (write_size && write_buf[write_size - 1] == 0xff)
			write_size--;
		while (write_size && write_buf[0] == 0xff) {
			write_buf++;
			write_size--;
			write_pos++;
		}

		// Actually do the write.
		if (storage->flash->write(storage->flash, write_buf,
					  write_pos, write_size)) {
			return 1;
		}

		// Update our position in the input buffer.
		input_pos += bytes;
		// The start is no longer misaligned.
		extra_low = 0;
	}

	return 0;
}

static int flash_storage_size(StorageOps *me)
{
	FlashStorage *storage = container_of(me, FlashStorage, ops);
	return storage->flash->size(storage->flash);
}

FlashStorage *new_flash_storage(FlashOps *flash)
{
	FlashStorage *storage = xzalloc(sizeof(*storage));

	storage->ops.read = &flash_storage_read;
	storage->ops.write = &flash_storage_write;
	storage->ops.size = &flash_storage_size;

	storage->flash = flash;

	return storage;
}
