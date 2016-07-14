/*
 * Copyright 2013 Google Inc.
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

#include <stdint.h>
#include <stdio.h>

#include "base/container_of.h"
#include "base/xalloc.h"
#include "drivers/storage/memory.h"

static int memory_storage_read(StorageOps *me, void *buffer,
			       uint64_t offset, size_t size)
{
	MemoryStorage *storage = container_of(me, MemoryStorage, ops);

	if (offset > storage->size || offset + size > storage->size) {
		printf("Out of bounds memory storage read.\n");
		return 1;
	}

	memcpy(buffer, &storage->data[offset], size);
	return 0;
}

static int memory_storage_write(StorageOps *me, const void *buffer,
				uint64_t offset, size_t size)
{
	MemoryStorage *storage = container_of(me, MemoryStorage, ops);

	if (offset > storage->size || offset + size > storage->size) {
		printf("Out of bounds memory storage write.\n");
		return 1;
	}

	memcpy(&storage->data[offset], buffer, size);
	return 0;
}

static int memory_ro_storage_write(StorageOps *me, const void *buffer,
				   uint64_t offset, size_t size)
{
	printf("Write attempted on RO memory storage.\n");
	return 1;
}

static int memory_storage_size(StorageOps *me)
{
	MemoryStorage *storage = container_of(me, MemoryStorage, ops);
	return storage->size;
}



static MemoryStorage *new_memory_base_storage(void *data, size_t size)
{
	MemoryStorage *storage = xzalloc(sizeof(*storage));

	storage->ops.read = &memory_storage_read;
	storage->ops.size = &memory_storage_size;

	storage->data = data;
	storage->size = size;

	return storage;
}

MemoryStorage *new_memory_ro_storage(void *data, size_t size)
{
	MemoryStorage *storage = new_memory_base_storage(data, size);
	storage->ops.write = &memory_ro_storage_write;
	return storage;
}

MemoryStorage *new_memory_storage(void *data, size_t size)
{
	MemoryStorage *storage = new_memory_base_storage(data, size);
	storage->ops.write = &memory_storage_write;
	return storage;
}
