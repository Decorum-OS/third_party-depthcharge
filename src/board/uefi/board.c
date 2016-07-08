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

#include <stdint.h>

#include "board/board.h"
#include "board/board_helpers.h"
#include "drivers/flash/memmapped.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/layout/coreboot.h"
#include "drivers/power/power.h"
#include "drivers/storage/flash.h"

// Force these to acceptable values for now.
PUB_STAT(flag_write_protect, 1)
PUB_STAT(flag_recovery, 0)
PUB_STAT(flag_developer_mode, 1)
PUB_STAT(flag_lid_open, 1)
PUB_STAT(flag_power, 0)

PUB_ARR(trusted_keyboards, (KeyboardOps *)NULL)
PUB_ARR(untrusted_keyboards, (KeyboardOps *)NULL)

PRIV_DYN(flash, &new_mem_mapped_flash(0xff800000, 0x800000)->ops);
PUB_DYN(_coreboot_storage, &new_flash_storage(get_flash())->ops);

PUB_STAT(power, (PowerOps *)NULL)
