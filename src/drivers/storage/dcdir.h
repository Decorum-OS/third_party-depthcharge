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

#ifndef __DRIVERS_STORAGE_DCDIR_H__
#define __DRIVERS_STORAGE_DCDIR_H__

#include <stdint.h>

#include "base/dcdir.h"
#include "drivers/storage/storage.h"

struct DcDirStorageDir;
struct DcDirStorage;

typedef struct DcDirStorageOps {
	int (*open_dir)(struct DcDirStorageOps *me, StorageOps **media,
			struct DcDirStorageDir *dir);
	int (*open_region)(struct DcDirStorageOps *me, StorageOps **media,
			   struct DcDirStorage *region);
} DcDirStorageOps;

typedef struct {
	DcDirStorageOps ops;

	StorageOps *media;
	uint32_t anchor_offset;

	DcDir root_handle;
	int opened;
} DcDirStorageRoot;

struct DcDirStorageDir {
	StorageOps ops;
	DcDirStorageOps dc_ops;

	DcDirStorageOps *parent;
	const char *name;

	StorageOps *media;
	DcDir dir_handle;
	DcDirRegion raw_region;
};
typedef struct DcDirStorageDir DcDirStorageDir;

struct DcDirStorage {
	StorageOps ops;

	DcDirStorageOps *parent;
	const char *name;

	StorageOps *media;
	DcDirRegion region_handle;
};
typedef struct DcDirStorage DcDirStorage;

DcDirStorageRoot *new_dcdir_storage_root(StorageOps *media,
					 uint32_t anchor_offset);
DcDirStorageDir *new_dcdir_storage_dir(DcDirStorageOps *parent,
				       const char *name);
DcDirStorage *new_dcdir_storage(DcDirStorageOps *parent, const char *name);

#endif /* __DRIVERS_STORAGE_DCDIR_H__ */
