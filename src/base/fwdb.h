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

#ifndef __BASE_FWDB_H__
#define __BASE_FWDB_H__

#include <stddef.h>
#include <stdint.h>

#include "base/algorithm.h"

struct FwdbHeader;
typedef struct FwdbHeader FwdbHeader;

// This structure holds information about an fwdb entry in a convenient to use
// bucket, and isn't the actual format used to store it in the db.
typedef struct {
	void *ptr;
	size_t size;
} FwdbEntry;

/*
 * Create a fwdb structure in memory.
 *
 * base points to the start of the region to store the table.
 * max_size is the largest size the table is allowed to expand to.
 *
 * Returns 0 on success, non-zero on failure.
 */
int fwdb_create_db(void *base, uint32_t max_size);

/*
 * Start using an fwdb that already exists, pointed to by header.
 *
 * Returns 0 on success, non-zero on failure.
 */
int fwdb_use_db(FwdbHeader *header);

/*
 * Return a pointer to the currently active firmware database structure.
 */
FwdbHeader *fwdb_db_pointer(void);

/*
 * Return the size of the region allocated to the firmware database.
 */
uint32_t fwdb_db_max_size(void);

/*
 * Locate an entry in the fwdb, and/or create a new entry.
 *
 * The "name" parameter is either the name of the entry you're looking for, or
 * the name of the entry you'll create.
 *
 * The "entry" pointer should point to a structure which will be initialized
 * with a pointer to the data in any entry found with the correct name, or any
 * newly created entry. It should be set to NULL if you know no field with
 * that name exists and you definitely want to create a new entry.
 *
 * The "new_entry" pointer should point to a structure which should already
 * have its "size" field set. If no entry with the specified name exists
 * and this pointer is set, a new entry with that name will be created. If one
 * does exist, its information will be returned through the structure pointed
 * to by "entry", or an error will be reported.
 *
 * If its "ptr" field of "new_entry" is non-NULL, the data it points to will
 * be used to initialize any new entry's data area. If it's NULL, then the
 * area will be initialized with zeroes.
 *
 * If you want to read out an entry which you expect or require to be in the
 * database, you should set the "entry" pointer and not the "new_entry"
 * pointer.
 *
 * If you want to create a new entry which you know doesn't already sxist in
 * the database, you should set "new_entry" and not "entry".
 *
 * If you want to either read out an existing entry if there is one, or create
 * one if there isn't, then you should set both "entry" and "new_entry". This
 * prevents one extra traversal of the database.
 *
 * Returns 0 on success, non-zero on failure.
 */
int fwdb_access(const char *name, FwdbEntry *entry, FwdbEntry *new_entry);

#endif /* __BASE_FWDB_H__ */
