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
#include "drivers/console/console.h"
#include "uefi/Uefi.h"

extern EFI_SYSTEM_TABLE *_uefi_handoff_system_table;

static void uefi_console_write(ConsoleOutputOps *me,
			      const void *buffer, size_t count)
{
	uint16_t output[0x100];
	const uint8_t *input = buffer;

	EFI_SYSTEM_TABLE *sys = _uefi_handoff_system_table;

        SIMPLE_TEXT_OUTPUT_INTERFACE *con_out = sys->ConOut;

	const size_t space = 0x100;
	size_t out = 0;
	size_t in = 0;
	while (in < count) {
		// If we're out of room, flush the output buffer. Never start
		// converting a character with only one or two output slots
		// left. If it's a newline and we need to inject a carriage
		// return, and we need a place for the NULL terminator.
		if (space - out < 3) {
			output[out++] = '\0';
			con_out->OutputString(con_out, output);
			out = 0;
		}

		// Grab a character from the input buffer.
		uint8_t in_char = input[in++];

		// Stuff it into the output buffer.
		output[out++] = in_char;
		// If it was a newline, add a carriage return.
		if (in_char == '\n')
			output[out++] = '\r';
	}
	// Flush any leftovers from the output buffer.
	if (out) {
		output[out++] = '\0';
		con_out->OutputString(con_out, output);
	}
}

static int uefi_console_init(void)
{
	static Console console = {
		.output = {
			.write = &uefi_console_write
		}
	};
	list_insert_after(&console.list_node, &console_list);

	return 0;
}

INIT_FUNC_CONSOLE(uefi_console_init)
