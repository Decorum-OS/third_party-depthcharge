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

#ifndef __VBOOT_FIRMWARE_ID_H__
#define __VBOOT_FIRMWARE_ID_H__

#include <stddef.h>

#include "drivers/storage/storage.h"

enum {
	VDAT_RW_A = 0x0,
	VDAT_RW_B = 0x1,
	VDAT_RO = 0xFF,
	VDAT_RECOVERY = 0xFF,
	VDAT_UNKNOWN = 0x100,
};

const char *firmware_id_for(int index, size_t *size_ptr);
const char *firmware_id_active(size_t *size_ptr);

#endif /* __VBOOT_FIRMWARE_ID_H__ */
