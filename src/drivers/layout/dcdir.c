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
#include "drivers/layout/dcdir.h"
#include "drivers/storage/dcdir.h"

PRIV_DYN(root, new_dcdir_storage_root(board__dcdir_storage(), 6 * 1024 * 1024))
  PRIV_DYN(rw, new_dcdir_storage_dir(&get_root()->ops, "RW"))
    PRIV_DYN(rw_legacy, new_dcdir_storage(&get_rw()->dc_ops, "LEGACY"))
    PRIV_DYN(rw_scratch, new_dcdir_storage(&get_rw()->dc_ops, "SCRATCH"))
    PRIV_DYN(rw_a, new_dcdir_storage_dir(&get_rw()->dc_ops, "A"))
      PRIV_DYN(rw_a_fwid, new_dcdir_storage(&get_rw_a()->dc_ops, "FWID"))
    PRIV_DYN(rw_b, new_dcdir_storage_dir(&get_rw()->dc_ops, "B"))
      PRIV_DYN(rw_b_fwid, new_dcdir_storage(&get_rw_b()->dc_ops, "FWID"))
  PRIV_DYN(ro, new_dcdir_storage_dir(&get_root()->ops, "RO"))
    PRIV_DYN(ro_fwid, new_dcdir_storage(&get_ro()->dc_ops, "FWID"))
    PRIV_DYN(ro_gbb, new_dcdir_storage(&get_ro()->dc_ops, "GBB"))

PUB_DYN(storage_fwid_ro, &get_ro_fwid()->ops)
PUB_DYN(storage_fwid_rwa, &get_rw_a_fwid()->ops)
PUB_DYN(storage_fwid_rwb, &get_rw_b_fwid()->ops)
PUB_DYN(storage_gbb, &get_ro_gbb()->ops)
PUB_DYN(storage_legacy, &get_rw_legacy()->ops)
PUB_DYN(storage_nv_scratch, &get_rw_scratch()->ops)
