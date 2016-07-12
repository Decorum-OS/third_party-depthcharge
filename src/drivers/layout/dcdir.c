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

#include "base/algorithm.h"
#include "base/die.h"
#include "base/xalloc.h"
#include "board/board.h"
#include "board/board_helpers.h"
#include "drivers/layout/dcdir.h"
#include "drivers/storage/dcdir.h"

PRIV_DYN(root, new_dcdir_storage_root(board__dcdir_storage(), 6 * 1024 * 1024))
  PRIV_DYN(rw, new_dcdir_storage_dir(&get_root()->ops, "RW"))
    PRIV_DYN(rw_legacy, new_dcdir_storage(&get_rw()->dc_ops, "LEGACY"))
    PRIV_DYN(rw_scratch, new_dcdir_storage(&get_rw()->dc_ops, "SCRATCH"))
    PRIV_DYN(rw_a, new_dcdir_storage_dir(&get_rw()->dc_ops, "A"))
      PRIV_DYN(rw_a_ec, new_dcdir_storage_dir(&get_rw_a()->dc_ops, "EC"))
      PRIV_DYN(rw_a_fwid, new_dcdir_storage(&get_rw_a()->dc_ops, "FWID"))
      PRIV_DYN(rw_a_verified, new_dcdir_storage_dir(&get_rw_a()->dc_ops,
						    "VERIFIED"))
        PRIV_DYN(rw_a_verified_ec,
                 new_dcdir_storage_dir(&get_rw_a_verified()->dc_ops, "EC"))
        PRIV_DYN(rw_a_verified_main,
                 new_dcdir_storage(&get_rw_a_verified()->dc_ops, "MAIN"))
      PRIV_DYN(rw_a_vblock, new_dcdir_storage(&get_rw_a()->dc_ops, "VBLOCK"))
    PRIV_DYN(rw_b, new_dcdir_storage_dir(&get_rw()->dc_ops, "B"))
      PRIV_DYN(rw_b_ec, new_dcdir_storage_dir(&get_rw_b()->dc_ops, "EC"))
      PRIV_DYN(rw_b_fwid, new_dcdir_storage(&get_rw_b()->dc_ops, "FWID"))
      PRIV_DYN(rw_b_verified, new_dcdir_storage_dir(&get_rw_b()->dc_ops,
						    "VERIFIED"))
        PRIV_DYN(rw_b_verified_ec,
                 new_dcdir_storage_dir(&get_rw_b_verified()->dc_ops, "EC"))
        PRIV_DYN(rw_b_verified_main,
                 new_dcdir_storage(&get_rw_b_verified()->dc_ops, "MAIN"))
      PRIV_DYN(rw_b_vblock, new_dcdir_storage(&get_rw_b()->dc_ops, "VBLOCK"))
  PRIV_DYN(ro, new_dcdir_storage_dir(&get_root()->ops, "RO"))
    PRIV_DYN(ro_fwid, new_dcdir_storage(&get_ro()->dc_ops, "FWID"))
    PRIV_DYN(ro_gbb, new_dcdir_storage(&get_ro()->dc_ops, "GBB"))

PUB_DYN(storage_fwid_ro, &get_ro_fwid()->ops)
PUB_DYN(storage_fwid_rwa, &get_rw_a_fwid()->ops)
PUB_DYN(storage_fwid_rwb, &get_rw_b_fwid()->ops)
PUB_DYN(storage_gbb, &get_ro_gbb()->ops)
PUB_DYN(storage_legacy, &get_rw_legacy()->ops)
PUB_DYN(storage_main_fw_a, &get_rw_a_verified_main()->ops)
PUB_DYN(storage_main_fw_b, &get_rw_b_verified_main()->ops)
PUB_DYN(storage_nv_scratch, &get_rw_scratch()->ops)
PUB_DYN(storage_vblock_a, &get_rw_a_vblock()->ops)
PUB_DYN(storage_vblock_b, &get_rw_b_vblock()->ops)
PUB_DYN(storage_verified_a, &get_rw_a_verified()->ops)
PUB_DYN(storage_verified_b, &get_rw_b_verified()->ops)



typedef StorageOps *EcStorageCache[CONFIG_MAX_EC_DEV_IDX + 1];



static StorageOps *board_storage_ec_hash(EcStorageCache *cache,
					 DcDirStorageDir *dir, int devidx)
{
	die_if(devidx > ARRAY_SIZE(*cache),
	       "EC devidx %d is out of bounds.\n", devidx);

	if (!(*cache)[devidx]) {
		const int name_size = 9;

		char *name = xmalloc(name_size);
		int written = snprintf(name, name_size, "%d", devidx);
		die_if(written < 0 || written >= name_size,
			"Error generating EC storage name.\n");

		(*cache)[devidx] = &new_dcdir_storage(&dir->dc_ops, name)->ops;
	}

	return (*cache)[devidx];
}

StorageOps *board_storage_ec_hash_a(int devidx)
{
	EcStorageCache ec_hashes;
	return board_storage_ec_hash(
		&ec_hashes, get_rw_a_verified_ec(), devidx);
}

StorageOps *board_storage_ec_hash_b(int devidx)
{
	EcStorageCache ec_hashes;
	return board_storage_ec_hash(
		&ec_hashes, get_rw_a_verified_ec(), devidx);
}

StorageOps *board_storage_ec_a(int devidx)
{
	EcStorageCache ecs;
	return board_storage_ec_hash(&ecs, get_rw_a_ec(), devidx);
}

StorageOps *board_storage_ec_b(int devidx)
{
	EcStorageCache ecs;
	return board_storage_ec_hash(&ecs, get_rw_b_ec(), devidx);
}
