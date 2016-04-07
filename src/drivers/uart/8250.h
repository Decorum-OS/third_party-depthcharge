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

#ifndef __DRIVERS_UART_8250_H__
#define __DRIVERS_UART_8250_H__

#include "drivers/uart/uart.h"

struct Uart8250;
typedef struct Uart8250 Uart8250;

typedef uint8_t (*Uart8250ReadRegFunc)(Uart8250 *me, int index);
typedef void (*Uart8250WriteRegFunc)(Uart8250 *me, uint8_t val, int index);

typedef enum {
	Uart8250Uninitialized = 0,
	Uart8250Present = 1,
	Uart8250NotPresent = 2
} Uart8250State;

typedef struct Uart8250
{
	UartOps ops;

	Uart8250State state;

	Uart8250ReadRegFunc read_reg;
	Uart8250WriteRegFunc write_reg;
} Uart8250;

typedef struct
{
	Uart8250 uart;

	uint16_t base;
} Uart8250Io;

typedef struct
{
	Uart8250 uart;

	uintptr_t base;
	int stride;
} Uart8250Mem;

typedef struct
{
	Uart8250 uart;

	uintptr_t base;
} Uart8250Mem32;

#if CONFIG_IO_ADDRESS_SPACE
Uart8250Io *new_uart_8250_io(uint16_t base);
#endif
Uart8250Mem *new_uart_8250_mem(uintptr_t base, int stride);
Uart8250Mem32 *new_uart_8250_mem32(uintptr_t base);

#endif /* __DRIVERS_UART_8250_H__ */
