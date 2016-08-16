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

#ifndef __BOARD_BOARD_HELPERS_H__
#define __BOARD_BOARD_HELPERS_H__

#include <string.h>

/*
 * A private, static board component.
 *
 * To define a function "get_foo" which returns a pointer to foo_instance,
 * you would write:
 *
 * PRIV_STAT(foo, &foo_instance)
 */
#define PRIV_STAT(name, val)				\
static inline typeof(val) get_##name(void)		\
{							\
	return (val);					\
}

/*
 * A public, static board component.
 *
 * To define a function "board_foo" which returns a pointer to foo_instance,
 * you would write:
 *
 * PUB_STAT(foo, &foo_instance)
 */
#define PUB_STAT(name, val)				\
typeof(val) board_##name(void)				\
{							\
	return (val);					\
}

/*
 * A private, dynamically generated, cached board component.
 *
 * To define a function "get_foo" which returns the cached return value of
 * a function new_foo_instance(), you would write:
 *
 * PRIV_DYN(foo, new_foo_instance())
 */
#define PRIV_DYN(name, func)				\
static inline typeof(func) get_##name(void)		\
{							\
	static typeof(func) name;			\
	if (!name) {					\
		name = (func);				\
	}						\
	return name;					\
}

/*
 * A public, dynamically generated, cached board component.
 *
 * To define a function "board_foo" which returns the cached return value of
 * a function new_foo_instance(), you would write:
 *
 * PUB_DYN(foo, new_foo_instance())
 */
#define PUB_DYN(name, func)				\
typeof(func) board_##name(void)				\
{							\
	static typeof(func) name;			\
	if (!name) {					\
		name = (func);				\
	}						\
	return name;					\
}

/*
 * A public, dynamically generated, cached array of board components.
 *
 * No private version of this macro exists because there's no obvious use
 * case for it. The array returned by this type of property should be made
 * up of pointers of the appropriate type, terminated by NULL. The type of
 * the pointers is determined by the type of the first component.
 *
 * To define a function "board_foos" which returns an array of pointers which
 * point to the cached return values of the functions new_foo_instance() and
 * new_bar_instance() followed by a NULL pointer, you would write:
 *
 * PUB_ARR(foos, new_foo_instance(), new_bar_instance())
 */
#define PUB_ARR(name, first, ...)			\
typeof(first) *board_##name(void)			\
{							\
	typedef typeof(first) el_type;			\
	static el_type name[				\
		sizeof(					\
			(el_type[]) {			\
				(first),		\
				##__VA_ARGS__,		\
				NULL			\
			}				\
		) / sizeof(el_type)];			\
	if (!name[0]) {					\
		memcpy(name,				\
			(el_type[]) {			\
				(first),		\
				##__VA_ARGS__,		\
				NULL			\
			},				\
			sizeof(name));			\
	}						\
	return name;					\
}

#endif /* __BOARD_BOARD_HELPERS_H__ */
