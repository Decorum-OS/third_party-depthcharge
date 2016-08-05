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

#include "base/container_of.h"
#include "base/fwdb.h"
#include "base/init_funcs.h"
#include "base/keycodes.h"
#include "base/list.h"
#include "drivers/console/console.h"
#include "uefi/uefi.h"

#include <stdio.h>
#include <string.h>

EFI_GUID simple_text_input_ex_protocol_guid =
	EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL_GUID;

typedef struct {
	Console console;

	DcEvent exit_bs;

	EFI_KEY_DATA last_key;

	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *con_out;
	EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *con_in;
} UefiConsole;

static void uefi_console_write(ConsoleOutputOps *me,
			       const void *buffer, size_t count)
{
	UefiConsole *uefi = container_of(me, UefiConsole, console.output);

	uint16_t output[0x100];
	const uint8_t *input = buffer;

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
			uefi->con_out->OutputString(uefi->con_out, output);
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
		uefi->con_out->OutputString(uefi->con_out, output);
	}
}

static int uefi_console_have_key(ConsoleInputOps *me)
{
	UefiConsole *uefi = container_of(me, UefiConsole,
					 console.trusted_input);

	if (!uefi->last_key.Key.ScanCode && !uefi->last_key.Key.UnicodeChar) {
		EFI_KEY_DATA key;
		if (uefi->con_in->ReadKeyStrokeEx(uefi->con_in, &key) !=
		    EFI_NOT_READY) {
			memcpy(&uefi->last_key, &key, sizeof(key));
		}
	}

	return uefi->last_key.Key.ScanCode || uefi->last_key.Key.UnicodeChar;
}

static int uefi_console_get_char(ConsoleInputOps *me)
{
	UefiConsole *uefi = container_of(me, UefiConsole,
					 console.trusted_input);

	while (!me->havekey(me))
	{;}

	uint16_t key = 0;
	switch (uefi->last_key.Key.ScanCode) {
	case 0x1:
		key = KEY_UP;
		break;
	case 0x2:
		key = KEY_DOWN;
		break;
	case 0x3:
		key = KEY_RIGHT;
		break;
	case 0x4:
		key = KEY_LEFT;
		break;
	default:
		key = uefi->last_key.Key.UnicodeChar;
	}

	memset(&uefi->last_key, 0, sizeof(uefi->last_key));

	return key;
}

static int uefi_console_disable(DcEvent *event)
{
	UefiConsole *uefi = container_of(event, UefiConsole, exit_bs);
	list_remove(&uefi->console.list_node);
	return 0;
}

static int uefi_console_init(void)
{
	EFI_SYSTEM_TABLE *sys = uefi_system_table_ptr();
	if (!sys)
		return 1;

	static UefiConsole uefi = {
		.console = {
			.output = {
				.write = &uefi_console_write
			},

			// There isn't a way to distinguish where input came
			// from, so lets just trust everything when on UEFI.
			.trusted_input = {
				.havekey = &uefi_console_have_key,
				.getchar = &uefi_console_get_char,
			},
		},
		.exit_bs = {
			.trigger = &uefi_console_disable
		},
	};

	uefi.con_out = sys->ConOut;

	EFI_BOOT_SERVICES *bs = sys->BootServices;
	EFI_STATUS status = bs->HandleProtocol(
		sys->ConsoleInHandle, &simple_text_input_ex_protocol_guid,
		(void **)&uefi.con_in);
	if (status != EFI_SUCCESS) {
		printf("Failed to retrieve console input ex protocol.\n");
		return 1;
	}

	list_insert_after(&uefi.console.list_node, &console_list);
	uefi_add_exit_boot_services_event(&uefi.exit_bs);

	return 0;
}

INIT_FUNC_CONSOLE(uefi_console_init)
