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

#include <assert.h>
#include <libpayload.h>
#include <stdio.h>
#include <stdlib.h>

#include "base/container_of.h"
#include "base/die.h"
#include "base/xalloc.h"
#include "drivers/storage/bdev_stream.h"

static uint64_t bdev_stream_read(StreamOps *me, uint64_t count, void *buffer)
{
	BdevStream *stream = container_of(me, BdevStream, stream);
	unsigned block_size = stream->bdev->block_size;

	// TODO(dehrenberg): implement buffering so that unaligned reads are
	// possible because alignment constraints are obscure.
	if (count & (block_size - 1))
		die("read_stream_simple(%lld) not LBA multiple\n", count);

	uint64_t sectors = count / block_size;
	if (sectors > stream->end_sector - stream->current_sector)
		die("read_stream_simple past the end, "
		    "end_sector=%lld, current_sector=%lld, sectors=%lld\n",
		    stream->end_sector, stream->current_sector, sectors);

	int ret = stream->bdev->ops.read(&stream->bdev->ops,
					 stream->current_sector, sectors,
					 buffer);

	if (ret != sectors)
		return ret;

	stream->current_sector += sectors;
	return count;
}

static void bdev_stream_close(StreamOps *me)
{
	free(container_of(me, BdevStream, stream));
}

StreamOps *new_bdev_stream(BlockDev *bdev, lba_t start, lba_t count)
{
	BdevStream *stream = xzalloc(sizeof(*stream));

	stream->bdev = bdev;
	stream->current_sector = start;
	stream->end_sector = start + count;
	stream->stream.read = bdev_stream_read;
	stream->stream.close = bdev_stream_close;
	// Check that block size is a power of 2.
	assert((bdev->block_size & (bdev->block_size - 1)) == 0);

	return &stream->stream;
}
