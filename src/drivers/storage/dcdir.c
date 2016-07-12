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

#include <stdint.h>
#include <stdio.h>

#include "base/xalloc.h"
#include "base/container_of.h"
#include "base/dcdir.h"
#include "drivers/storage/dcdir.h"

static int dcdir_storage_init_root(DcDirStorageRoot *root)
{
	if (dcdir_open_root(&root->root_handle, root->media,
			    root->anchor_offset)) {
		return 1;
	}

	root->opened = 1;
	return 0;
}

static int dcdir_storage_root_init_dir(DcDirStorageOps *me, StorageOps **media,
				       DcDirStorageDir *dir)
{
	DcDirStorageRoot *root = container_of(me, DcDirStorageRoot, ops);
	*media = root->media;

	if (!root->opened && dcdir_storage_init_root(root))
		return 1;

	if (dcdir_open_dir_raw(&dir->dir_handle, &dir->raw_region, *media,
			       &root->root_handle, dir->name)) {
		return 1;
	}

	return 0;
}

static int dcdir_storage_root_init_region(DcDirStorageOps *me,
					  StorageOps **media,
					  DcDirStorage *region)
{
	DcDirStorageRoot *root = container_of(me, DcDirStorageRoot, ops);
	*media = root->media;

	if (!root->opened && dcdir_storage_init_root(root))
		return 1;

	if (dcdir_open_region(&region->region_handle, *media,
			      &root->root_handle, region->name)) {
		return 1;
	}

	return 0;
}

DcDirStorageRoot *new_dcdir_storage_root(StorageOps *media,
					 uint32_t anchor_offset)
{
	DcDirStorageRoot *root = xzalloc(sizeof(*root));

	root->ops.open_dir = &dcdir_storage_root_init_dir;
	root->ops.open_region = &dcdir_storage_root_init_region;

	root->media = media;
	root->anchor_offset = anchor_offset;

	return root;
}



static int dcdir_storage_init_dir(DcDirStorageDir *dir, StorageOps **media)
{
	if (dir->parent->open_dir(dir->parent, media, dir))
		return 1;

	dir->media = *media;
	return 0;
}

static int dcdir_storage_dir_init_dir(DcDirStorageOps *me, StorageOps **media,
				      DcDirStorageDir *dir)
{
	DcDirStorageDir *this_dir = container_of(me, DcDirStorageDir, dc_ops);

	if (!this_dir->media && dcdir_storage_init_dir(this_dir, media))
		return 1;

	if (dcdir_open_dir_raw(&dir->dir_handle, &dir->raw_region, *media,
			       &this_dir->dir_handle, dir->name)) {
		return 1;
	}

	return 0;
}

static int dcdir_storage_dir_init_region(DcDirStorageOps *me,
					 StorageOps **media,
					 DcDirStorage *region)
{
	DcDirStorageDir *this_dir = container_of(me, DcDirStorageDir, dc_ops);

	if (!this_dir->media && dcdir_storage_init_dir(this_dir, media))
		return 1;

	if (dcdir_open_region(&region->region_handle, *media,
			      &this_dir->dir_handle, region->name)) {
		return 1;
	}

	return 0;
}

static int dcdir_storage_dir_read(StorageOps *me, void *buffer,
				  uint64_t offset, size_t size)
{
	DcDirStorageDir *dir = container_of(me, DcDirStorageDir, ops);

	if (!dir->media && dcdir_storage_init_dir(dir, &dir->media))
		return 1;

	if (offset + size > dir->raw_region.size) {
		printf("Read beyond the bounds of dcdir directory.\n");
		return 1;
	}

	return storage_read(dir->media, buffer,
			    dir->raw_region.offset + offset, size);
}

static int dcdir_storage_dir_write(StorageOps *me, const void *buffer,
				   uint64_t offset, size_t size)
{
	printf("Writing to a dcdir directory is not supported.\n");
	return 1;
}

static int dcdir_storage_dir_size(StorageOps *me)
{
	DcDirStorageDir *dir = container_of(me, DcDirStorageDir, ops);

	if (!dir->media && dcdir_storage_init_dir(dir, &dir->media))
		return -1;

	return dir->raw_region.size;
}

DcDirStorageDir *new_dcdir_storage_dir(DcDirStorageOps *parent,
				       const char *name)
{
	DcDirStorageDir *dir = xzalloc(sizeof(*dir));

	dir->dc_ops.open_dir = &dcdir_storage_dir_init_dir;
	dir->dc_ops.open_region = &dcdir_storage_dir_init_region;

	dir->ops.read = &dcdir_storage_dir_read;
	dir->ops.write = &dcdir_storage_dir_write;
	dir->ops.size = &dcdir_storage_dir_size;

	dir->parent = parent;
	dir->name = name;

	return dir;
}




static int dcdir_storage_init(DcDirStorage *storage, StorageOps **media)
{
	return storage->parent->open_region(storage->parent, media, storage);
}

static int dcdir_storage_read(StorageOps *me, void *buffer,
			      uint64_t offset, size_t size)
{
	DcDirStorage *storage = container_of(me, DcDirStorage, ops);

	if (!storage->media && dcdir_storage_init(storage, &storage->media))
		return 1;

	if (offset + size > storage->region_handle.size) {
		printf("Read beyond the bounds of dcdir region.\n");
		return 1;
	}

	return storage_read(storage->media, buffer,
			    storage->region_handle.offset + offset, size);
}

static int dcdir_storage_write(StorageOps *me, const void *buffer,
			       uint64_t offset, size_t size)
{
	DcDirStorage *storage = container_of(me, DcDirStorage, ops);

	if (!storage->media && dcdir_storage_init(storage, &storage->media))
		return 1;

	if (offset + size > storage->region_handle.size) {
		printf("Read beyond the bounds of dcdir region.\n");
		return 1;
	}

	return storage_write(storage->media, buffer,
			     storage->region_handle.offset + offset, size);
}

static int dcdir_storage_size(StorageOps *me)
{
	DcDirStorage *storage = container_of(me, DcDirStorage, ops);

	if (!storage->media && dcdir_storage_init(storage, &storage->media))
		return 1;

	return storage->region_handle.size;
}

DcDirStorage *new_dcdir_storage(DcDirStorageOps *parent, const char *name)
{
	DcDirStorage *storage = xzalloc(sizeof(*storage));

	storage->ops.read = &dcdir_storage_read;
	storage->ops.write = &dcdir_storage_write;
	storage->ops.size = &dcdir_storage_size;

	storage->parent = parent;
	storage->name = name;

	return storage;
}
