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
 *
 */
#ifndef __DRIVERS_KEYBOARD_KEYBOARD_H__
#define __DRIVERS_KEYBOARD_KEYBOARD_H__

typedef struct KeyboardOps {
	int (*get_char)(struct KeyboardOps *me);
	int (*have_char)(struct KeyboardOps *me);
} KeyboardOps;

// Make sure all the keyboards on the system are awake and ready to receive
// keys. This lets us get any delays out of the way ahead of time and avoids
// having them between when we've prompted the user, displayed graphics, etc.,
// and when their input can be accepted.
void keyboard_prepare(void);

#endif /* __DRIVERS_KEYBOARD_KEYBOARD_H__ */
