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

#ifndef __DRIVERS_STORAGE_STORAGE_H__
#define __DRIVERS_STORAGE_STORAGE_H__

#include <stdint.h>

typedef struct StorageOps {
	int (*read)(struct StorageOps *me, void *buffer,
		    uint64_t offset, size_t size);
	int (*write)(struct StorageOps *me, const void *buffer,
		     uint64_t offset, size_t size);
} StorageOps;

static inline int storage_read(StorageOps *me, void *buffer,
			       uint64_t offset, size_t size)
{
	return me->read(me, buffer, offset, size);
}

static inline int storage_write(StorageOps *me, const void *buffer,
				uint64_t offset, size_t size)
{
	return me->write(me, buffer, offset, size);
}

#endif /* __DRIVERS_STORAGE_STORAGE_H__ */
