/*
 * Copyright 2016 Google Inc.
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

#include "base/init_funcs.h"
#include "base/list.h"
#include "drivers/board/board.h"
#include "drivers/console/console.h"
#include "drivers/uart/uart.h"

static int have_char(ConsoleInputOps *me)
{
	UartOps *uart = board_debug_uart();
	return uart->have_char(uart);
}

static int get_char(ConsoleInputOps *me)
{
	UartOps *uart = board_debug_uart();
	return uart->get_char(uart);
}

static void put_char(ConsoleOutputOps *me, unsigned int c)
{
	UartOps *uart = board_debug_uart();
	return uart->put_char(uart, c);
}

static int serial_console_init(void)
{
	static Console console = {
		.trusted_input = {
			.havekey = &have_char,
			.getchar = &get_char,
		},
		.output = {
			.putchar = &put_char,
		},
	};

	list_insert_after(&console.list_node, &console_list);

	return 0;
}

INIT_FUNC_CONSOLE(serial_console_init)
