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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/container_of.h"
#include "base/fwdb.h"
#include "base/xalloc.h"
#include "drivers/storage/fwdb.h"

static int fwdb_storage_read(StorageOps *me, void *buffer,
			     uint64_t offset, size_t size)
{
	FwdbStorage *storage = container_of(me, FwdbStorage, ops);

	if (!storage->entry.ptr &&
	    fwdb_access(storage->name, &storage->entry, NULL)) {
		return 1;
	}

	if (offset + size > storage->entry.size) {
		printf("Read beyond the bounds of FWDB entry %s.\n",
		       storage->name);
		return 1;
	}

	memcpy(buffer, (uint8_t *)storage->entry.ptr + offset, size);
	return 0;
}

static int fwdb_storage_write(StorageOps *me, const void *buffer,
			      uint64_t offset, size_t size)
{
	FwdbStorage *storage = container_of(me, FwdbStorage, ops);

	if (!storage->entry.ptr &&
	    fwdb_access(storage->name, &storage->entry, NULL)) {
		return 1;
	}

	if (offset + size > storage->entry.size) {
		printf("Write beyond the bounds of FWDB entry %s.\n",
		       storage->name);
		return 1;
	}

	memcpy((uint8_t *)storage->entry.ptr + offset, buffer, size);
	return 0;
}

static int fwdb_ro_storage_write(StorageOps *me, const void *buffer,
				 uint64_t offset, size_t size)
{
	printf("Write attempted on RO FWDB storage.\n");
	return 1;
}

static int fwdb_storage_size(StorageOps *me)
{
	FwdbStorage *storage = container_of(me, FwdbStorage, ops);

	if (!storage->entry.ptr &&
	    fwdb_access(storage->name, &storage->entry, NULL)) {
		return -1;
	}

	return storage->entry.size;
}

FwdbStorage *new_fwdb_storage(const char *name)
{
	FwdbStorage *storage = xzalloc(sizeof(*storage));

	storage->ops.read = &fwdb_storage_read;
	storage->ops.write = &fwdb_storage_write;
	storage->ops.size = &fwdb_storage_size;

	storage->name = name;

	return storage;
}

FwdbStorage *new_fwdb_ro_storage(const char *name)
{
	FwdbStorage *storage = xzalloc(sizeof(*storage));

	storage->ops.read = &fwdb_storage_read;
	storage->ops.write = &fwdb_ro_storage_write;
	storage->ops.size = &fwdb_storage_size;

	storage->name = name;

	return storage;
}
