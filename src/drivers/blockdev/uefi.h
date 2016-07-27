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

#ifndef __DRIVERS_BLOCKDEV_UEFI_H__
#define __DRIVERS_BLOCKDEV_UEFI_H__

#include "drivers/blockdev/blockdev.h"

struct _EFI_BLOCK_IO_PROTOCOL;
typedef struct _EFI_BLOCK_IO_PROTOCOL EFI_BLOCK_IO_PROTOCOL;

typedef struct {
	BlockDev dev;

	EFI_BLOCK_IO_PROTOCOL *bio;
	uint32_t media_id;
	uint32_t io_align;
	int write_caching;

	ListNode uefi_blockdev_list;
} UefiBlockDev;

typedef struct {
	BlockDevCtrlr ctrlr;

	int removable;

	ListNode uefi_blockdev_list;
} UefiBlockDevCtrlr;

UefiBlockDevCtrlr *new_uefi_blockdev_ctrlr(int removable);

#endif /* __DRIVERS_BLOCKDEV_UEFI_H__ */
