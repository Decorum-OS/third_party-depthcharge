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

#include <stdlib.h>

#include "arch/x86/amd64/handoff/handoff.h"
#include "base/fwdb.h"
#include "vboot/util/memory.h"

extern uint64_t handoff_parameter;

void handoff_special(void)
{
	fwdb_use_db((FwdbHeader *)(uintptr_t)handoff_parameter);

	uintptr_t start = (uintptr_t)fwdb_db_pointer();
	uintptr_t end = start + fwdb_db_max_size();
	if (start != end)
		memory_mark_used(start, end);
}
