/*
 * Copyright 2015 Google Inc.
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

#include <stdlib.h>
#include <vboot_struct.h>

#include "base/xalloc.h"
#include "board/board.h"
#include "drivers/storage/storage.h"
#include "vboot/firmware_id.h"
#include "vboot/util/commonparams.h"

const char *firmware_id_for(int index, size_t *size)
{
	static const char *rwa;
	static size_t rwa_size;
	static const char *rwb;
	static size_t rwb_size;
	static const char *ro;
	static size_t ro_size;

	StorageOps *storage;
	const char **id_ptr;
	size_t *size_ptr;
	switch (index) {
	case VDAT_RW_A:
		storage = board_storage_fwid_rwa();
		id_ptr = &rwa;
		size_ptr = &rwa_size;
		break;
	case VDAT_RW_B:
		storage = board_storage_fwid_rwb();
		id_ptr = &rwb;
		size_ptr = &rwb_size;
		break;
	case VDAT_RECOVERY:
		storage = board_storage_fwid_ro();
		id_ptr = &ro;
		size_ptr = &ro_size;
		break;
	default:
		return NULL;
	}

	if (!*id_ptr) {
		int ssize = storage_size(storage);
		if (ssize < 1)
			return NULL;

		void *data = xmalloc(ssize);
		if (storage_read(storage, data, 0, ssize)) {
			free(data);
			return NULL;
		}

		*id_ptr = data;
		*size_ptr = ssize;
	}

	if (size)
		*size = *size_ptr;

	return *id_ptr;
}

const char *firmware_id_active(size_t *size)
{
	if (common_params_init())
		return NULL;

	VbSharedDataHeader *vdat = cparams.shared_data_blob;
	return firmware_id_for(vdat->firmware_index, size);
}
