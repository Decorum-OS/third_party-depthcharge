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

// A private, dynamically generated, cached board component.
#define PRIV_DYN(name, func)				\
static inline typeof(func) get_##name(void)		\
{							\
	static typeof(func) name;			\
	if (!name) {					\
		name = (func);				\
	}						\
	return name;					\
}

// A public, dynamically generated, cached board component.
#define PUB_DYN(name, func)				\
typeof(func) board_##name(void)				\
{							\
	static typeof(func) name;			\
	if (!name) {					\
		name = (func);				\
	}						\
	return name;					\
}

// A private, static board component.
#define PRIV_STAT(name, val)				\
static inline typeof(val) get_##name(void)		\
{							\
	return (val);					\
}

// A public, static board component.
#define PUB_STAT(name, val)				\
typeof(val) board_##name(void)				\
{							\
	return (val);					\
}

#endif /* __BOARD_BOARD_HELPERS_H__ */
