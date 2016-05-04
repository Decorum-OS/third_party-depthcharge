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
#include <string.h>

#include "base/algorithm.h"
#include "base/dcdir.h"
#include "base/dcdir_structs.h"
#include "drivers/flash/flash.h"

int dcdir_open_root(DcDir *dcdir, uint32_t anchor_offset)
{
	DcDirAnchor *anchor = flash_read(anchor_offset, sizeof(DcDirAnchor));

	if (memcmp(anchor->signature, DcDirAnchorSignature,
		   DcDirAnchorSignatureSize)) {
		printf("DcDir anchor signature mismatch.\n");
		return 1;
	}

	if (anchor->major_version != 1) {
		printf("Incompatible DcDir version %d.\n",
		       anchor->major_version);
		return 1;
	}

	if (anchor->anchor_offset != anchor_offset) {
		printf("DcDir anchor offset mismatch. "
		       "Expected %#x, found %#x.\n",
		       anchor_offset, anchor->anchor_offset);
		return 1;
	}

	dcdir->offset = anchor_offset + sizeof(DcDirAnchor);
	dcdir->base = anchor->root_base;

	return 0;
}

static DcDirPointer *dcdir_find_in_dir(DcDir *dir, const char *name)
{
	DcDirDirectoryHeader *header =
		flash_read(dir->offset, sizeof(*header));

	if (memcmp(header->signature, DcDirDirectorySignature,
		   DcDirDirectorySignatureSize)) {
		printf("DcDir directory signature mismatch.\n");
		return NULL;
	}

	int size = ((header->size[0] << 0) +
		    (header->size[1] << 8) +
		    (header->size[2] << 16) + 1) * 8;

	size -= sizeof(*header);

	if (size < 0) {
		printf("Malformed DcDir directory found when looking up %s.\n",
		       name);
		return NULL;
	}

	void *pointers = flash_read(dir->offset + sizeof(*header), size);

	while (size > 0) {
		uint64_t *ptr_name = pointers;
		pointers = (uint8_t *)pointers + sizeof(*ptr_name);

		DcDirPointer *gen_ptr = pointers;
		int ptr_size = (gen_ptr->size + 1) * 8;

		size_t name_len = MIN(strlen(name), sizeof(*ptr_name));

		if (!memcmp(name, ptr_name, name_len))
			return gen_ptr;

		pointers = (uint8_t *)pointers + ptr_size;
		size -= ptr_size;
	}

	return NULL;
}

int dcdir_open_dir(DcDir *dcdir, DcDir *parent_dir, const char *name)
{
	DcDirPointer *ptr_ptr = dcdir_find_in_dir(parent_dir, name);

	if (!ptr_ptr)
		return 1;

	int directory = ptr_ptr->type & 0x1;
	int type = ptr_ptr->type >> 1;

	if (!directory) {
		printf("DcDir region is not a directory.\n");
		return 1;
	}

	uint32_t parent_offset = parent_dir->offset - parent_dir->base;

	switch (type) {
	case DcDirOffset24Length24:
	{
		DcDirPointerOffset24Length24 *ptr1 =
			(DcDirPointerOffset24Length24 *)ptr_ptr;

		dcdir->offset = parent_offset +
			(ptr1->offset[0] << 0) +
			(ptr1->offset[1] << 8) +
			(ptr1->offset[2] << 16);
		dcdir->base = 0;
	}
	break;
	case DcDirBase32Offset32Length32:
	{
		DcDirPointerBase32Offset32Length32 *ptr2 =
			(DcDirPointerBase32Offset32Length32 *)ptr_ptr;

		dcdir->offset = parent_offset + ptr2->offset + ptr2->base;
		dcdir->base = ptr2->base;
	}
	break;
	default:
		printf("Unrecognized dcdir pointer type.");
		return 1;
	}

	return 0;
}

int dcdir_open_region(DcDirRegion *region, DcDir *parent_dir, const char *name)
{
	DcDirPointer *ptr_ptr = dcdir_find_in_dir(parent_dir, name);

	if (!ptr_ptr)
		return 1;

	int directory = ptr_ptr->type & 0x1;
	int type = ptr_ptr->type >> 1;

	if (directory) {
		printf("DcDir region is a directory.\n");
		return 1;
	}

	uint32_t parent_offset = parent_dir->offset - parent_dir->base;

	switch (type) {
	case 0x1:
	{
		DcDirPointerOffset24Length24 *ptr1 =
			(DcDirPointerOffset24Length24 *)ptr_ptr;

		region->offset = parent_offset +
			(ptr1->offset[0] << 0) +
			(ptr1->offset[1] << 8) +
			(ptr1->offset[2] << 16);
		region->size =
			(ptr1->length[0] << 0) +
			(ptr1->length[1] << 8) +
			(ptr1->length[2] << 16) + 1;
	}
	break;
	case 0x2:
	{
		DcDirPointerBase32Offset32Length32 *ptr2 =
			(DcDirPointerBase32Offset32Length32 *)ptr_ptr;

		region->offset = parent_offset + ptr2->offset;
		region->size = ptr2->length + 1;
	}
	break;
	default:
		printf("Unrecognized dcdir pointer type.");
		return 1;
	}

	return 0;
}
