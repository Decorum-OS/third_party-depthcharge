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

PRIV_DYN(root, &new_dcdir_storage_root(board__dcdir_storage(),
				       6 * 1024 * 1024)->ops)
  PRIV_DYN(rw, &new_dcdir_storage_dir(get_root(), "RW")->ops)
    PUB_DYN(storage_nv_scratch, &new_dcdir_storage(get_rw(), "SCRATCH")->ops)
  PRIV_DYN(ro, &new_dcdir_storage_dir(get_root(), "RO")->ops)
    PUB_DYN(storage_gbb, &new_dcdir_storage(get_ro(), "GBB")->ops)
