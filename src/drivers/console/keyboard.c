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

#include <libpayload.h>

#include "base/init_funcs.h"
#include "board/board.h"
#include "drivers/keyboard/keyboard.h"

static int have_char(void)
{
	KeyboardOps **keyboards = board_keyboards();
	KeyboardOps *keyboard;

	for (keyboard = *keyboards; keyboard;
		keyboard = *keyboards, keyboards++) {
		if (keyboard->have_char(keyboard))
			return 1;
	}

	return 0;
}

static int get_char(void)
{
	KeyboardOps **keyboards = board_keyboards();
	KeyboardOps *keyboard;

	while (1) {
		for (keyboard = *keyboards; keyboard;
			keyboard = *keyboards, keyboards++) {
			if (keyboard->have_char(keyboard))
				return keyboard->get_char(keyboard);
		}
	}
}

static int keyboard_console_init(void)
{
	static struct console_input_driver keyboard_console = {
		.havekey = &have_char,
		.getchar = &get_char
	};

	console_add_input_driver(&keyboard_console);

	return 0;
}

INIT_FUNC(keyboard_console_init)
