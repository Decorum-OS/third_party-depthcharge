/*
 * Copyright 2012 Google Inc.
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

#include <gbb_header.h>
#include <stdio.h>

#include "base/fwdb.h"
#include "drivers/flash/flash.h"
#include "image/fmap.h"
#include "module/symbols.h"
#include "vboot/util/commonparams.h"

VbCommonParams cparams;

int common_params_init(void)
{
	if (cparams.shared_data_blob)
		return 0;

	// Set up the common param structure.
	memset(&cparams, 0, sizeof(cparams));

	FwdbEntry entry;
	FwdbEntry new_entry = {
		.size = VB_SHARED_DATA_REC_SIZE
	};
	if (fwdb_access("vboot.shared_data", &entry, &new_entry))
		return 1;

	cparams.shared_data_blob = entry.ptr;
	cparams.shared_data_size = entry.size;

	return 0;
}
