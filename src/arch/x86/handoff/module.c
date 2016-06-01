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

#include <coreboot_tables.h>
#include <sysinfo.h>

#include "arch/x86/handoff/handoff.h"
#include "base/fwdb.h"
#include "vboot/util/memory.h"

extern uint32_t handoff_parameter;

int cb_parse_arch_specific(struct cb_record *rec, struct sysinfo_t *info)
{
	return 0;
}

void handoff_special(void)
{
	fwdb_use_db((FwdbHeader *)(uintptr_t)handoff_parameter);

	uintptr_t start = (uintptr_t)fwdb_db_pointer();
	uintptr_t end = start + fwdb_db_max_size();
	if (start != end)
		memory_mark_used(start, end);

	// Get information from the coreboot tables if they exist.
	if (cb_parse_header((void *)0, 0x1000, &lib_sysinfo))
		cb_parse_header((void *)0x000f0000, 0x1000, &lib_sysinfo);
}
