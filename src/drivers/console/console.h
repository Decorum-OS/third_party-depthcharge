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

#ifndef __DRIVERS_CONSOLE_CONSOLE_H__
#define __DRIVERS_CONSOLE_CONSOLE_H__

#include <stdint.h>

#include "base/list.h"

typedef struct ConsoleInputOps {
	int (*havekey)(struct ConsoleInputOps *me);
	int (*getchar)(struct ConsoleInputOps *me);
} ConsoleInputOps;

typedef struct ConsoleOutputOps {
	void (*putchar)(struct ConsoleOutputOps *me, unsigned int);
	void (*write)(struct ConsoleOutputOps *me, const void *, size_t);
} ConsoleOutputOps;

typedef struct {
	ConsoleInputOps input;
	ConsoleInputOps trusted_input;

	ConsoleOutputOps output;

	ListNode list_node;
} Console;

extern ListNode console_list;

void console_write(const void *buffer, size_t count);
ConsoleInputOps *console_has_key(int trusted);

#endif /* __DRIVERS_CONSOLE_CONSOLE_H__ */
