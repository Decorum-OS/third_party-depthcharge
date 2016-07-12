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
#include "base/xalloc.h"
#include "drivers/storage/section_index.h"
#include "image/index.h"

static int section_index_storage_find_index(SectionIndexStorage *storage)
{
	SectionIndex *index;

	uint32_t count;
	if (storage_read(storage->base, &count, 0, sizeof(count)))
		return 1;

	uint32_t size = sizeof(count) + count * sizeof(SectionIndexEntry);
	index = xmalloc(size);

	index->count = count;
	if (storage_read(storage->base, index->entries,
			 offsetof(SectionIndex, entries),
			 count * sizeof(SectionIndexEntry))) {
		free(index);
		return 1;
	}

	storage->index = index;
	storage->total_size = sizeof(count);
	for (uint64_t i = 0; i < count; i++) {
		storage->total_size += sizeof(SectionIndexEntry);
		storage->total_size += (index->entries[i].size + 3) & ~3;
	}
	return 0;
}

static int section_index_storage_read(StorageOps *me, void *buffer,
				      uint64_t offset, size_t size)
{
	SectionIndexStorage *storage =
		container_of(me, SectionIndexStorage, ops);

	if (!storage->index && section_index_storage_find_index(storage))
		return 1;

	if (offset + size > storage->total_size) {
		printf("Read beyond the end of the section index.\n");
		return 1;
	}

	return storage_read(storage->base, buffer, offset, size);
}

static int section_index_storage_write(StorageOps *me, const void *buffer,
				       uint64_t offset, size_t size)
{
	printf("Writing into a section index is not supported.\n");
	return 1;
}

static int section_index_storage_size(StorageOps *me)
{
	SectionIndexStorage *storage =
		container_of(me, SectionIndexStorage, ops);
	return storage->total_size;
}

SectionIndexStorage *new_section_index_storage(StorageOps *base)
{
	SectionIndexStorage *storage = xzalloc(sizeof(*storage));

	storage->ops.read = &section_index_storage_read;
	storage->ops.write = &section_index_storage_write;
	storage->ops.size = &section_index_storage_size;

	storage->base = base;
	return storage;
}



static int section_index_storage_find_entry(SectionIndexEntryStorage *storage)
{
	if (!storage->parent->index &&
	    section_index_storage_find_index(storage->parent)) {
		return 1;
	}

	storage->entry = &storage->parent->index->entries[storage->index];
	return 0;
}

static int section_index_entry_storage_read(StorageOps *me, void *buffer,
					    uint64_t offset, size_t size)
{
	SectionIndexEntryStorage *storage =
		container_of(me, SectionIndexEntryStorage, ops);

	if (!storage->entry && section_index_storage_find_entry(storage))
		return 1;

	if (offset + size > storage->entry->size) {
		printf("Read beyond the bounds of section index entry.\n");
		return 1;
	}

	return storage_read(storage->parent->base, buffer,
			    offset + storage->entry->offset, size);
}

static int section_index_entry_storage_write(StorageOps *me, const void *buffer,
					     uint64_t offset, size_t size)
{
	SectionIndexEntryStorage *storage =
		container_of(me, SectionIndexEntryStorage, ops);

	if (!storage->entry && section_index_storage_find_entry(storage))
		return 1;

	if (offset + size > storage->entry->size) {
		printf("Write beyond the bounds of section index entry.\n");
		return 1;
	}

	return storage_write(storage->parent->base, buffer,
			     offset + storage->entry->offset, size);
}

static int section_index_entry_storage_size(StorageOps *me)
{
	SectionIndexEntryStorage *storage =
		container_of(me, SectionIndexEntryStorage, ops);

	if (!storage->entry && section_index_storage_find_entry(storage))
		return 1;

	return storage->entry->size;
}

SectionIndexEntryStorage *new_section_index_entry_storage(
	SectionIndexStorage *parent, uint32_t index)
{
	SectionIndexEntryStorage *storage = xzalloc(sizeof(*storage));

	storage->ops.read = &section_index_entry_storage_read;
	storage->ops.write = &section_index_entry_storage_write;
	storage->ops.size = &section_index_entry_storage_size;

	storage->parent = parent;
	storage->index = index;

	return storage;
}
