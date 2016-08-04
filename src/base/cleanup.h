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

#ifndef __BASE_CLEANUP_H__
#define __BASE_CLEANUP_H__

#include "base/event.h"

typedef enum
{
	CleanupOnReboot = 0x1,
	CleanupOnPoweroff = 0x2,
	CleanupOnHandoff = 0x4,
	CleanupOnLegacy = 0x8,
} CleanupType;

typedef struct
{
	DcEvent event;
	CleanupType types;
} CleanupEvent;

void cleanup_add(CleanupEvent *event);
void cleanup_remove(CleanupEvent *event);

int cleanup_trigger(CleanupType type);

#endif /* __BASE_CLEANUP_H__ */
