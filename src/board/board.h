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

#ifndef __BOARD_BOARD_H__
#define __BOARD_BOARD_H__

#include "drivers/keyboard/keyboard.h"
#include "drivers/power/power.h"
#include "drivers/storage/storage.h"
#include "drivers/uart/uart.h"

/*
 * These functions should be prefixed by "board_" and return pointers to
 * drivers which implement a generic interface to some functionality which
 * is provided by a board specific piece of hardware.
 *
 * For instance, the driver which implements a UART may change from board to
 * board, but all boards are assumed to have some sort of UART for debugging
 * purposes, or at least something that can implement that interface. Each
 * board will define a function called board_debug_uart which will return a
 * properly configured driver instance that implements that UART in whatever
 * way makes sense for that board.
 *
 * Boards might also define functions for their own use which return common
 * components which might be needed to construct other drivers. For instance,
 * a TPM driver might need an I2C bus controller driver to communicate over.
 * The convention is for those functions to be prefixed with "get_" and to
 * appear before the "board_*" functions.
 */

UartOps *board_debug_uart(void);

// Trusted keyboards are ones which we believe actually reflect input from the
// user and haven't been faked by a clever attacker.
KeyboardOps **board_trusted_keyboards(void);
// Untrusted keyboards are ones which may have been faked, for instance by an
// inconspicuous USB dongle pretending to be a keyboard plugged into the back
// of the machine.
KeyboardOps **board_untrusted_keyboards(void);

PowerOps *board_power(void);

/*
 * This group of functions gather information about the state of the system
 * which may be physically implemented by GPIOs, information gathered from
 * various components of the system like the EC, representing the state of
 * software, or even just made up entirely.
 *
 * They should return a 1 if the flag is on, enabled, applicable, etc., a 0
 * if not, and a -1 if there was an error retrieving or calculating the flags
 * value.
 */
// Whether write protect was enabled at boot time.
int board_flag_write_protect(void);
// Whether recovery mode was requested.
int board_flag_recovery(void);
// Whether developer mode was requested/enabled.
int board_flag_developer_mode(void);
// Whether video option ROMs have been loaded.
int board_flag_option_roms_loaded(void);
// If the "lid" is open on a laptop like device.
int board_flag_lid_open(void);
// If the power button (or equivalent) is pressed, and the user is requesting
// a shutdown.
int board_flag_power(void);
// Whether the EC is running the RW portion of its firmware.
int board_flag_ec_in_rw(void);

/*
 * This group of functions return storage objects which provide access to
 * different bits of information which are needed during boot. These might
 * be additional pieces of firmware, or blocks of data like the GBB which
 * firmware acts on. The storage objects should be combined together to
 * abstract away the actual media storing the data, and any data structures
 * which map out the media into smaller components.
 */

StorageOps *board_storage_gbb(void);
StorageOps *board_storage_legacy(void);
StorageOps *board_storage_nv_scratch(void);
StorageOps *board_storage_fwid_rwa(void);
StorageOps *board_storage_fwid_rwb(void);
StorageOps *board_storage_fwid_ro(void);

#endif /* __BOARD_BOARD_H__ */
