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

KeyboardOps **board_keyboards(void);

PowerOps *board_power(void);

#endif /* __BOARD_BOARD_H__ */
