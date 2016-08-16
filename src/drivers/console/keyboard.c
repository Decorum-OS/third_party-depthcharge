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
#include "drivers/board/board.h"
#include "drivers/console/console.h"
#include "drivers/keyboard/keyboard.h"

static int _have_char(KeyboardOps **keyboards)
{
	KeyboardOps *keyboard;

	for (keyboard = *keyboards; keyboard;
		keyboard = *keyboards, keyboards++) {
		if (keyboard->have_char && keyboard->have_char(keyboard))
			return 1;
	}

	return 0;
}

static int have_char(ConsoleInputOps *me)
{
	return _have_char(board_untrusted_keyboards());
}

static int have_trusted_char(ConsoleInputOps *me)
{
	return _have_char(board_trusted_keyboards());
}

static int _get_char(KeyboardOps **keyboards)
{
	KeyboardOps *keyboard;

	while (1) {
		for (keyboard = *keyboards; keyboard;
			keyboard = *keyboards, keyboards++) {
			if (keyboard->have_char && keyboard->get_char &&
			    keyboard->have_char(keyboard)) {
				return keyboard->get_char(keyboard);
			}
		}
	}
}

static int get_char(ConsoleInputOps *me)
{
	return _get_char(board_untrusted_keyboards());
}

static int get_trusted_char(ConsoleInputOps *me)
{
	return _get_char(board_trusted_keyboards());
}

static int keyboard_console_init(void)
{
	static Console keyboard_console = {
		.trusted_input = {
			.havekey = &have_trusted_char,
			.getchar = &get_trusted_char
		},
		.input = {
			.havekey = &have_char,
			.getchar = &get_char
		}
	};

	list_insert_after(&keyboard_console.list_node, &console_list);

	return 0;
}

INIT_FUNC_CONSOLE(keyboard_console_init)
