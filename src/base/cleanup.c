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

#include "base/cleanup.h"
#include "base/container_of.h"

static ListNode cleanup_events;

void cleanup_add(CleanupEvent *event)
{
	list_insert_after(&event->event.list_node, &cleanup_events);
}

void cleanup_remove(CleanupEvent *event)
{
	list_remove(&event->event.list_node);
}

int cleanup_trigger(CleanupType type)
{
	int ret = 0;

	CleanupEvent *cleanup;
	list_for_each(cleanup, cleanup_events, event.list_node)
		if (cleanup->types & type)
			ret = event_trigger(&cleanup->event) || ret;

	return ret;
}
