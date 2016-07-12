/*
 * Copyright 2013 Google Inc.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA, 02110-1301 USA
 */

#ifndef __DRIVERS_BUS_SPI_SPI_H__
#define __DRIVERS_BUS_SPI_SPI_H__

#include <stdint.h>

typedef struct SpiOps
{
	int (*start)(struct SpiOps *me);
	int (*transfer)(struct SpiOps *me, void *in, const void *out,
			uint32_t size);
	int (*stop)(struct SpiOps *me);
} SpiOps;

static inline int spi_start(SpiOps *me)
{
	return me->start(me);
}

static inline int spi_transfer(SpiOps *me, void *in, const void *out,
			       uint32_t size)
{
	return me->transfer(me, in, out, size);
}

static inline int spi_stop(SpiOps *me)
{
	return me->stop(me);
}

#endif /* __DRIVERS_BUS_SPI_SPI_H__ */
