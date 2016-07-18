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
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include "base/fwdb.h"
#include "uefi/uefi.h"
#include "vboot/stages.h"

extern uint8_t _binary_ro_image_start;
extern uint8_t _binary_ro_image_size;
extern uint8_t ImageBase;

static int prepare_fwdb_storage(void)
{
	FwdbEntry new_entry = {
		.ptr = &_binary_ro_image_start,
		.size = &_binary_ro_image_size - &ImageBase,
	};
	if (fwdb_access("uefi_ro_image", NULL, &new_entry))
		return 1;

	return 0;
}

void module_main(void)
{
	if (prepare_fwdb_storage())
		halt();

	if (vboot_init())
		halt();
}
