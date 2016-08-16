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

#include <stdio.h>

#include "drivers/board/board.h"
#include "drivers/board/board_helpers.h"
#include "drivers/layout/uefi.h"
#include "drivers/storage/dcdir.h"

PRIV_DYN(ro, new_dcdir_storage_root(board__uefi_ro_storage(), 0))
  PRIV_DYN(ro_fwid, new_dcdir_storage(&get_ro()->ops, "FWID"))
  PRIV_DYN(ro_gbb, new_dcdir_storage(&get_ro()->ops, "GBB"))

PRIV_DYN(rw_a, new_dcdir_storage_root(board__uefi_rw_a_storage(), 0))
  PRIV_DYN(rw_a_fwid, new_dcdir_storage(&get_rw_a()->ops, "FWID"))
  PRIV_DYN(rw_a_verified, new_dcdir_storage_dir(&get_rw_a()->ops, "VERIFIED"))
    PRIV_DYN(rw_a_verified_main,
             new_dcdir_storage(&get_rw_a_verified()->dc_ops, "MAIN"))
  PRIV_DYN(rw_a_vblock, new_dcdir_storage(&get_rw_a()->ops, "VBLOCK"))

PRIV_DYN(rw_b, new_dcdir_storage_root(board__uefi_rw_b_storage(), 0))
  PRIV_DYN(rw_b_fwid, new_dcdir_storage(&get_rw_b()->ops, "FWID"))
  PRIV_DYN(rw_b_verified, new_dcdir_storage_dir(&get_rw_b()->ops, "VERIFIED"))
    PRIV_DYN(rw_b_verified_main,
             new_dcdir_storage(&get_rw_b_verified()->dc_ops, "MAIN"))
  PRIV_DYN(rw_b_vblock, new_dcdir_storage(&get_rw_b()->ops, "VBLOCK"))

PUB_DYN(storage_fwid_ro, &get_ro_fwid()->ops)
PUB_DYN(storage_fwid_rwa, &get_rw_a_fwid()->ops)
PUB_DYN(storage_fwid_rwb, &get_rw_b_fwid()->ops)
PUB_DYN(storage_gbb, &get_ro_gbb()->ops)
PUB_DYN(storage_main_fw_a, &get_rw_a_verified_main()->ops)
PUB_DYN(storage_main_fw_b, &get_rw_b_verified_main()->ops)
PUB_DYN(storage_vblock_a, &get_rw_a_vblock()->ops)
PUB_DYN(storage_vblock_b, &get_rw_b_vblock()->ops)
PUB_DYN(storage_verified_a, &get_rw_a_verified()->ops)
PUB_DYN(storage_verified_b, &get_rw_b_verified()->ops)
