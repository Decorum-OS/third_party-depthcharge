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

#include <string.h>

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
	die("Flash storage writes not implemented!");
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
