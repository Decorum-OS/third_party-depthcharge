/*
 * Copyright 2016 Google Inc.
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
 * Copyright (C) 2008 Ulf Jordan <jordan@chalmers.se>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdint.h>

#include "arch/io.h"
#include "base/container_of.h"
#include "base/xalloc.h"
#include "drivers/uart/8250.h"

enum {
	THR = 0, // Transmitter holding buffer.
	RBR = 0, // Receiver buffer.
	DLL = 0, // Divisor latch low byte.
	IER = 1, // Interrupt enable register.
	DLH = 1, // Divisor latch high byte.
	IIR = 2, // Interrupt identification register.
	FCR = 2, // FIFO control register.
	LCR = 3, // Line control register.
	MCR = 4, // Modem control register.
	LSR = 5, // Line status register.
	MSR = 6, // Modem status register.
	SR = 7   // Scratch register.
};

enum {
	LSR_DR = 0x1 << 0, // Data ready.
	LSR_OE = 0x1 << 1, // Overrun.
	LSR_PE = 0x1 << 2, // Parity error.
	LSR_FE = 0x1 << 3, // Framing error.
	LSR_BI = 0x1 << 4, // Break.
	LSR_THRE = 0x1 << 5, // Xmit holding register empty.
	LSR_TEMT = 0x1 << 6, // Xmitter empty.
	LSR_ERR = 0x1 << 7 // Error.
};

enum {
	LCR_WORD_LENGTH_MASK = 0x3 << 0,
	LCR_5_BITS = 0x0 << 0,
	LCR_6_BITS = 0x1 << 0,
	LCR_7_BITS = 0x2 << 0,
	LCR_8_BITS = 0x3 << 0,

	LCR_ONE_STOP_BIT = 0x0 << 2,
	LCR_TWO_STOP_BITS = 0x1 << 2,

	LCR_PARITY_MASK = 0x7 << 3,
	LCR_NO_PARITY = 0x0 << 3,
	LCR_ODD_PARITY = 0x1 << 3,
	LCR_EVEN_PARITY = 0x3 << 3,
	LCR_MARK = 0x5 << 3,
	LCR_SPACE = 0x7 << 3,

	LCR_BREAK_ENABLE = 0x1 << 6,
	LCR_DIVISOR_LATCH = 0x1 << 7
};

static void uart8250_init(Uart8250 *uart, int speed, int word_bits,
					  int parity, int stop_bits)
{
	uint8_t reg;

	if (uart->read_reg(uart, LSR) == 0xff &&
	    uart->read_reg(uart, MSR) == 0xff) {
		uart->state = Uart8250NotPresent;
		return;
	} else {
		uart->state = Uart8250Present;
	}

	if (!CONFIG_SERIAL_SET_SPEED)
		return;

	// Disable interrupts.
	uart->write_reg(uart, 0, IER);

	// Assert RTS and DTR.
	uart->write_reg(uart, 3, MCR);

	reg = uart->read_reg(uart, LCR);
	uart->write_reg(uart, reg | LCR_DIVISOR_LATCH, LCR);

	// Write the divisor.
	uint16_t divisor = 115200 / speed;
	uart->write_reg(uart, divisor & 0xFF, DLL);
	uart->write_reg(uart, divisor >> 8, DLH);

	uart->write_reg(uart, (reg & ~LCR_DIVISOR_LATCH) | LCR_8_BITS, LCR);
}


static void put_char(UartOps *me, uint8_t c)
{
	Uart8250 *uart = container_of(me, Uart8250, ops);

	if (uart->state == Uart8250Uninitialized)
		uart8250_init(uart, CONFIG_SERIAL_BAUD_RATE, 8, 0, 1);

	if (uart->state != Uart8250Present)
		return;

	while ((uart->read_reg(uart, LSR) & LSR_THRE) == 0)
	{;}

	uart->write_reg(uart, c, THR);
	if (c == '\n')
		me->put_char(me, '\r');
}

static int have_char(UartOps *me)
{
	Uart8250 *uart = container_of(me, Uart8250, ops);

	if (uart->state == Uart8250Uninitialized)
		uart8250_init(uart, CONFIG_SERIAL_BAUD_RATE, 8, 0, 1);

	if (uart->state != Uart8250Present)
		return 0;

	return uart->read_reg(uart, LSR) & LSR_DR;
}

static int get_char(UartOps *me)
{
	Uart8250 *uart = container_of(me, Uart8250, ops);

	if (uart->state == Uart8250Uninitialized)
		uart8250_init(uart, CONFIG_SERIAL_BAUD_RATE, 8, 0, 1);

	if (uart->state != Uart8250Present)
		return -1;

	while (!me->have_char(me))
	{;}

	return uart->read_reg(uart, RBR);
}


static void uart_8250_fill_in(Uart8250 *uart, Uart8250ReadRegFunc read_reg,
					      Uart8250WriteRegFunc write_reg)
{
	uart->ops.put_char = &put_char;
	uart->ops.have_char = &have_char;
	uart->ops.get_char = &get_char;

	uart->read_reg = read_reg;
	uart->write_reg = write_reg;
}


#if CONFIG_IO_ADDRESS_SPACE
static uint8_t read_reg_io(Uart8250 *me, int index)
{
	Uart8250Io *uart = container_of(me, Uart8250Io, uart);
	return inb(uart->base + index);
}

static void write_reg_io(Uart8250 *me, uint8_t val, int index)
{
	Uart8250Io *uart = container_of(me, Uart8250Io, uart);
	outb(val, uart->base + index);
}

Uart8250Io *new_uart_8250_io(uint16_t base)
{
	Uart8250Io *uart = xzalloc(sizeof(*uart));
	uart_8250_fill_in(&uart->uart, &read_reg_io, &write_reg_io);
	uart->base = base;

	return uart;
}
#endif


static uint8_t read_reg_mem(Uart8250 *me, int index)
{
	Uart8250Mem *uart = container_of(me, Uart8250Mem, uart);
	return readb((uint8_t *)uart->base + index * uart->stride);
}

static void write_reg_mem(Uart8250 *me, uint8_t val, int index)
{
	Uart8250Mem *uart = container_of(me, Uart8250Mem, uart);
	writeb(val, (uint8_t *)uart->base + index * uart->stride);
}

Uart8250Mem *new_uart_8250_mem(uintptr_t base, int stride)
{
	Uart8250Mem *uart = xzalloc(sizeof(*uart));
	uart_8250_fill_in(&uart->uart, &read_reg_mem, &write_reg_mem);
	uart->base = base;
	uart->stride = stride;

	return uart;
}


static uint8_t read_reg_mem32(Uart8250 *me, int index)
{
	Uart8250Mem32 *uart = container_of(me, Uart8250Mem32, uart);
	return (uint8_t)readl((uint8_t *)uart->base + index * sizeof(uint32_t));
}

static void write_reg_mem32(Uart8250 *me, uint8_t val, int index)
{
	Uart8250Mem32 *uart = container_of(me, Uart8250Mem32, uart);
	writel(val, (uint8_t *)uart->base + index * sizeof(uint32_t));
}

Uart8250Mem32 *new_uart_8250_mem32(uintptr_t base)
{
	Uart8250Mem32 *uart = xzalloc(sizeof(*uart));
	uart_8250_fill_in(&uart->uart, &read_reg_mem32, &write_reg_mem32);
	uart->base = base;

	return uart;
}
