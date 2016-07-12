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

#ifndef __BASE_DCDIR_H__
#define __BASE_DCDIR_H__

#include <stdint.h>

#include "drivers/storage/storage.h"

/*
 * This structure is supposed to be an opaque handle to a directory in a
 * DcDir. For practical reasons the information is visible, but it's only for
 * the dcdir's internal bookkeeping and shouldn't be examined or modified.
 */
typedef struct
{
	// The offset of the directory table within the directory it describes.
	uint32_t base;
	// The offset of directory table within the image.
	uint32_t offset;
} DcDir;

/*
 * This structure is filled in by the dcdir code to return information about
 * a requested region in a DcDir.
 */
typedef struct
{
	uint32_t offset;
	uint32_t size;
} DcDirRegion;

/*
 * Open the root directory of a dcdir.
 *
 * The structure pointed to by dcdir will be filled in with information
 * pertaining to the root directory. It should be passed verbatim to other
 * functions to access the contents of the root directory, and should be
 * treated as if it was opaque. storage should provide at least a "read"
 * function to access the media with the dcdir on it. anchor_offset should be
 * set to the offset of the "anchor" strcture within the medium described by
 * the dcdir.
 *
 * Returns 0 on success, non-zero on failure.
 */
int dcdir_open_root(DcDir *dcdir, StorageOps *storage, uint32_t anchor_offset);

/*
 * Open a subdirectory within a given parent directory.
 *
 * The structure pointed to by dcdir will be filled in with information
 * pertaining to the directory with the name "name". It should be passed
 * verbatim to other functions to access the contents of that directory, and
 * should be treated as if it was opaque. storage should provide at least a
 * "read" function to access the media with the dcdir on it. parent_dir
 * should be set to point to the parent directory which was filled in by some
 * other dcdir function.
 *
 * The "_raw" version of this function works the same way, but it also
 * records the offset and size of the sub directory region itself in the
 * DcDirRegion structure pointed to by raw_region.
 *
 * Returns 0 on success, non-zero on failure.
 */
int dcdir_open_dir(DcDir *dcdir, StorageOps *storage, DcDir *parent_dir,
		   const char *name);
int dcdir_open_dir_raw(DcDir *dcdir, DcDirRegion *raw_region,
		       StorageOps *storage, DcDir *parent_dir,
		       const char *name);

/*
 * Open a region within a given parent directory.
 *
 * The structure pointed to by "region" will be filled in with information
 * pertaining to the region with the name "name". storage should provide at
 * least a "read" function to access the media with the dcdir on it.
 * parent_dir should be set to point to the parent directory which was filled
 * in by some other dcdir function.
 *
 * Returns 0 on success, non-zero on failure.
 */
int dcdir_open_region(DcDirRegion *region, StorageOps *storage,
		      DcDir *parent_dir, const char *name);

#endif /* __BASE_DCDIR_H__ */
