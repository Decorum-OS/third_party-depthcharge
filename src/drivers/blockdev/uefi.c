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

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "base/algorithm.h"
#include "base/xalloc.h"
#include "base/container_of.h"
#include "drivers/blockdev/uefi.h"
#include "uefi/edk/Protocol/BlockIo.h"
#include "uefi/uefi.h"

static EFI_GUID block_io_protocol_guid = EFI_BLOCK_IO_PROTOCOL_GUID;

static lba_t uefi_blockdev_read(BlockDevOps *me, lba_t start, lba_t count,
				void *buffer)
{
	UefiBlockDev *ubdev = container_of(me, UefiBlockDev, dev.ops);

	const size_t block_size = ubdev->dev.block_size;
	size_t buffer_size = block_size * count;
	size_t chunk_size = MIN(64 * 1024, buffer_size);
	// Round chunk_size up to the nearest multiple of block_size;
	chunk_size = ((chunk_size + block_size - 1) / block_size) * block_size;

	void *read_buf;
	if (ubdev->io_align > 1)
		read_buf = memalign(ubdev->io_align, chunk_size);
	else
		read_buf = malloc(chunk_size);

	if (!read_buf) {
		printf("Failed to allocate read buffer.\n");
		return 0;
	}

	while (buffer_size) {
		EFI_STATUS status = ubdev->bio->ReadBlocks(
			ubdev->bio, ubdev->media_id, start,
			chunk_size, read_buf);
		if (status != EFI_SUCCESS) {
			printf("Failed to read blocks from UEFI device.\n");
			free(read_buf);
			return 0;
		}
		memcpy(buffer, read_buf, chunk_size);

		buffer = (uint8_t *)buffer + chunk_size;
		start += chunk_size / block_size;
		buffer_size -= chunk_size;
		chunk_size = MIN(chunk_size, buffer_size);
	}

	free(read_buf);
	return count;
}

static lba_t uefi_blockdev_write(BlockDevOps *me, lba_t start, lba_t count,
				 const void *buffer)
{
	UefiBlockDev *ubdev = container_of(me, UefiBlockDev, dev.ops);

	const size_t block_size = ubdev->dev.block_size;
	size_t buffer_size = block_size * count;
	size_t chunk_size = MIN(64 * 1024, buffer_size);
	// Round chunk_size up to the nearest multiple of block_size;
	chunk_size = ((chunk_size + block_size - 1) / block_size) * block_size;

	void *write_buf;
	if (ubdev->io_align > 1)
		write_buf = memalign(ubdev->io_align, chunk_size);
	else
		write_buf = malloc(chunk_size);
	if (!write_buf) {
		printf("Failed to allocate write buffer.\n");
		return 0;
	}

	while (buffer_size) {
		memcpy(write_buf, buffer, chunk_size);
		EFI_STATUS status = ubdev->bio->WriteBlocks(
			ubdev->bio, ubdev->media_id, start,
			chunk_size, write_buf);
		if (status != EFI_SUCCESS) {
			printf("Failed to write blocks to UEFI device.\n");
			free(write_buf);
			return 0;
		}

		buffer = (uint8_t *)buffer + chunk_size;
		start += chunk_size / block_size;
		buffer_size -= chunk_size;
		chunk_size = MIN(chunk_size, buffer_size);
	}

	free(write_buf);

	if (ubdev->write_caching) {
		EFI_STATUS status = ubdev->bio->FlushBlocks(ubdev->bio);
		if (status != EFI_SUCCESS) {
			printf("Failed to flush blocks to UEFI device.\n");
			return 0;
		}
	}

	return count;
}

static int uefi_blockdev_ctrlr_update(struct BlockDevCtrlrOps *me)
{
	UefiBlockDevCtrlr *ctrlr =
		container_of(me, UefiBlockDevCtrlr, ctrlr.ops);

	// Detach the existing block devices. They'll be re-added if they're
	// still present.
	UefiBlockDev *ubdev;
	UefiBlockDev *last_ubdev = NULL;
	list_for_each(ubdev, ctrlr->uefi_blockdev_list, uefi_blockdev_list) {
		list_remove(&last_ubdev->uefi_blockdev_list);
		list_remove(&last_ubdev->dev.list_node);
		free(last_ubdev);
		last_ubdev = ubdev;
	}
	if (last_ubdev) {
		list_remove(&last_ubdev->uefi_blockdev_list);
		list_remove(&last_ubdev->dev.list_node);
		free(last_ubdev);
	}

	EFI_SYSTEM_TABLE *st = uefi_system_table_ptr();
	if (!st)
		return 1;
	EFI_BOOT_SERVICES *bs = st->BootServices;

	UINTN buf_size = 0;
	EFI_HANDLE dummy;
	EFI_STATUS status = bs->LocateHandle(
		ByProtocol, &block_io_protocol_guid, NULL, &buf_size, &dummy);
	// If there are no handles, we're already done.
	if (status == EFI_SUCCESS)
		return 0;
	if (status != EFI_BUFFER_TOO_SMALL) {
		printf("Error retrieving block io handles.\n");
		return 1;
	}

	EFI_HANDLE *handles = xmalloc(buf_size);

	status = bs->LocateHandle(
		ByProtocol, &block_io_protocol_guid, NULL, &buf_size, handles);
	if (status != EFI_SUCCESS) {
		printf("Failed to retrieve block io handles.\n");
		free(handles);
		return 1;
	}

	int handle_count = buf_size / sizeof(dummy);

	for (int i = 0; i < handle_count; i++) {
		EFI_BLOCK_IO_PROTOCOL *bio;
		status = bs->HandleProtocol(
			handles[i], &block_io_protocol_guid, (void **)&bio);
		if (status != EFI_SUCCESS) {
			printf("Failed to retrieve block io protocol.\n");
			return 1;
		}

		if (!bio->Media->MediaPresent ||
		    bio->Media->LogicalPartition ||
		    !!bio->Media->RemovableMedia != !!ctrlr->removable) {
			continue;
		}

		ubdev = xzalloc(sizeof(*ubdev));

		ubdev->bio = bio;
		ubdev->media_id = bio->Media->MediaId;
		ubdev->io_align = bio->Media->IoAlign;
		ubdev->write_caching = bio->Media->WriteCaching;

		BlockDev *dev = &ubdev->dev;
		dev->ops.read = &uefi_blockdev_read;
		dev->ops.write = &uefi_blockdev_write;

		dev->name = "UEFI block device.\n";
		dev->removable = bio->Media->RemovableMedia;
		dev->block_size = bio->Media->BlockSize;
		dev->block_count = bio->Media->LastBlock + 1;

		list_insert_after(&ubdev->uefi_blockdev_list,
				  &ctrlr->uefi_blockdev_list);
		if (bio->Media->RemovableMedia) {
			list_insert_after(&dev->list_node,
					  &removable_block_devices);
		} else {
			list_insert_after(&dev->list_node,
					  &fixed_block_devices);
		}
	}

	return 0;
}

UefiBlockDevCtrlr *new_uefi_blockdev_ctrlr(int removable)
{
	UefiBlockDevCtrlr *ctrlr = xzalloc(sizeof(*ctrlr));

	ctrlr->ctrlr.ops.update = &uefi_blockdev_ctrlr_update;
	ctrlr->ctrlr.need_update = 1;

	ctrlr->removable = removable;

	return ctrlr;
}
