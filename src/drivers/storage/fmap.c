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
#include "drivers/storage/fmap.h"

static int fmap_storage_find_fmap(FmapStorageMedia *media)
{
	size_t size = sizeof(Fmap);
	Fmap *fmap;
	fmap = xmalloc(size);

	if (storage_read(media->base, fmap,
			 media->fmap_offset, size)) {
		free(fmap);
		printf("Failed to read FMAP.\n");
		return 1;
	}

	if (memcmp(fmap->signature, (uint8_t *)FMAP_SIGNATURE,
		   sizeof(fmap->signature))) {
		free(fmap);
		printf("Bad signature on the FMAP.\n");
		return 1;
	}

	size += fmap->nareas * sizeof(FmapArea);
	Fmap *fmap_orig = fmap;
	fmap = xmalloc(size);
	memcpy(fmap, fmap_orig, sizeof(*fmap));
	free(fmap_orig);

	if (storage_read(media->base, fmap + 1,
			 media->fmap_offset + sizeof(*fmap),
			 size - sizeof(*fmap))) {
		free(fmap);
		printf("Failed to read FMAP.\n");
		return 1;
	}

	media->fmap = fmap;
	return 0;
}

static int fmap_storage_find_area(FmapStorage *storage)
{
	if (!storage->media->fmap && fmap_storage_find_fmap(storage->media))
		return 1;

	const Fmap *fmap = storage->media->fmap;
	for (int i = 0; i < fmap->nareas; i++) {
		const FmapArea *cur = &(fmap->areas[i]);
		if (!strncmp(storage->name, (const char *)cur->name,
			     sizeof(cur->name))) {
			storage->area = cur;
			return 0;
		}
	}

	printf("Couldn't find FMAP area %s.\n", storage->name);
	return 1;
}

static int fmap_storage_read(StorageOps *me, void *buffer,
			     uint64_t offset, size_t size)
{
	FmapStorage *storage = container_of(me, FmapStorage, ops);

	if (!storage->area && fmap_storage_find_area(storage))
		return 1;

	if (offset + size > storage->area->size) {
		printf("Read beyond the bounds of FMAP region.\n");
		return 1;
	}

	return storage_read(storage->media->base, buffer,
			    offset + storage->area->offset, size);
}

static int fmap_storage_write(StorageOps *me, const void *buffer,
			      uint64_t offset, size_t size)
{
	FmapStorage *storage = container_of(me, FmapStorage, ops);

	if (!storage->area && fmap_storage_find_area(storage))
		return 1;

	if (offset + size > storage->area->size) {
		printf("Write beyond the bounds of FMAP region.\n");
		return 1;
	}

	return storage_write(storage->media->base, buffer,
			     offset + storage->area->offset, size);
}

static int fmap_storage_size(StorageOps *me)
{
	FmapStorage *storage = container_of(me, FmapStorage, ops);

	if (!storage->area && fmap_storage_find_area(storage))
		return 1;

	return storage->area->size;
}

FmapStorageMedia *new_fmap_storage_media(StorageOps *base,
					 uint32_t fmap_offset)
{
	FmapStorageMedia *media = xzalloc(sizeof(*media));

	media->base = base;
	*(uint32_t *)&media->fmap_offset = fmap_offset;

	return media;
}

FmapStorage *new_fmap_storage(FmapStorageMedia *media, const char *name)
{
	FmapStorage *storage = xzalloc(sizeof(*storage));

	storage->ops.read = &fmap_storage_read;
	storage->ops.write = &fmap_storage_write;
	storage->ops.size = &fmap_storage_size;

	storage->media = media;
	storage->name = name;

	return storage;
}
