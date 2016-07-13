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

#include <string.h>

#include "image/fmap.h"

int fmap_check_signature(const Fmap *fmap)
{
	return memcmp(fmap->signature, (const uint8_t *)"__FMAP__",
		      sizeof(fmap->signature));
}

int fmap_find_area(const Fmap *fmap, const char *name, const FmapArea **area)
{
	for (int i = 0; i < fmap->nareas; i++) {
		const FmapArea *cur = &(fmap->areas[i]);
		if (!strncmp(name, (const char *)cur->name,
				sizeof(cur->name))) {
			*area = cur;
			return 0;
		}
	}
	return 1;
}
