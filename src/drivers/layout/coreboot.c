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

#include "board/board.h"
#include "board/board_helpers.h"
#include "drivers/layout/coreboot.h"
#include "drivers/storage/fmap.h"
#include "drivers/storage/section_index.h"

PRIV_DYN(fmap_media, new_fmap_storage_media(board__coreboot_storage(),
					    CONFIG_FMAP_OFFSET))

PRIV_DYN(fw_main_a, &new_fmap_storage(get_fmap_media(), "FW_MAIN_A")->ops)
  PRIV_DYN(main_a_index, new_section_index_storage(get_fw_main_a()))
PRIV_DYN(fw_main_b, &new_fmap_storage(get_fmap_media(), "FW_MAIN_B")->ops)
  PRIV_DYN(main_b_index, new_section_index_storage(get_fw_main_b()))

PUB_DYN(storage_gbb, &new_fmap_storage(get_fmap_media(), "GBB")->ops)
PUB_DYN(storage_fwid_ro, &new_fmap_storage(get_fmap_media(), "RO_FRID")->ops)
PUB_DYN(storage_fwid_rwa, &new_fmap_storage(get_fmap_media(), "RW_FWID_A")->ops)
PUB_DYN(storage_fwid_rwb, &new_fmap_storage(get_fmap_media(), "RW_FWID_B")->ops)
PUB_DYN(storage_legacy, &new_fmap_storage(get_fmap_media(), "RW_LEGACY")->ops)
PUB_DYN(storage_main_fw_a,
	&new_section_index_entry_storage(get_main_a_index(), 0)->ops)
PUB_DYN(storage_main_fw_b,
	&new_section_index_entry_storage(get_main_b_index(), 0)->ops)
PUB_DYN(storage_nv_scratch, &new_fmap_storage(get_fmap_media(),
					      "SHARED_DATA")->ops)
PUB_DYN(storage_vblock_a, &new_fmap_storage(get_fmap_media(), "VBLOCK_A")->ops)
PUB_DYN(storage_vblock_b, &new_fmap_storage(get_fmap_media(), "VBLOCK_B")->ops)
PUB_DYN(storage_vboot_nvstorage, &new_fmap_storage(get_fmap_media(),
						   "RW_NVRAM")->ops)
PUB_DYN(storage_verified_a, &get_main_a_index()->ops)
PUB_DYN(storage_verified_b, &get_main_b_index()->ops)
