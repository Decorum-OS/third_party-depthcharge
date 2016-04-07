/*
 * Copyright 2013 Google Inc.
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
#include "drivers/uart/s5p.h"

typedef struct
{
	uint32_t ulcon;		// line control
	uint32_t ucon;		// control
	uint32_t ufcon;		// FIFO control
	uint32_t umcon;		// modem control
	uint32_t utrstat;	// Tx/Rx status
	uint32_t uerstat;	// Rx error status
	uint32_t ufstat;	// FIFO status
	uint32_t umstat;	// modem status
	uint32_t utxh;		// transmit buffer
	uint32_t urxh;		// receive buffer
	uint32_t ubrdiv;	// baud rate divisor
	uint32_t ufracval;	// divisor fractional value
	uint32_t uintp;		// interrupt pending
	uint32_t uints;		// interrupt source
	uint32_t uintm;		// interrupt mask
} S5pRegs;

static void put_char(UartOps *me, uint8_t c)
{
	const uint32_t TxFifoFullBit = (0x1 << 24);

	UartS5p *uart = container_of(me, UartS5p, ops);
	S5pRegs *regs = (S5pRegs *)uart->base;

	while (readl(&regs->ufstat) & TxFifoFullBit)
	{;}

	writeb(c, &regs->utxh);
	if (c == '\n')
		me->put_char(me, '\r');
}

static int have_char(UartOps *me)
{
	const uint32_t DataReadyMask = (0xf << 0) | (0x1 << 8);

	UartS5p *uart = container_of(me, UartS5p, ops);
	S5pRegs *regs = (S5pRegs *)uart->base;

	return (readl(&regs->ufstat) & DataReadyMask) != 0;
}

static int get_char(UartOps *me)
{
	while (!me->have_char(me))
	{;}

	UartS5p *uart = container_of(me, UartS5p, ops);
	S5pRegs *regs = (S5pRegs *)uart->base;

	return readb(&regs->urxh);
}

UartS5p *new_uart_s5p(uintptr_t base)
{
	UartS5p *uart = xzalloc(sizeof(*uart));

	uart->ops.put_char = &put_char;
	uart->ops.have_char = &have_char;
	uart->ops.get_char = &get_char;

	uart->base = base;

	return uart;
}
