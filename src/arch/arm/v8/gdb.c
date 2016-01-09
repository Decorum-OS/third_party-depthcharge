/*
 * Copyright 2014 Google Inc.
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
 * Foundation, Inc.
 */

#include <exception.h>
#include <libpayload.h>

#include "debug/gdb/gdb.h"

struct gdb_regs
{
	uint64_t x[32];
	struct fp_reg
	{
		uint64_t quad[2];
	} __attribute__((packed)) f[32];
	uint32_t fpcr;
	uint32_t fpsr;
	uint32_t spsr;
} __attribute__((packed));

static const uint8_t type_to_signal[] = {
	[EXC_SYNC_SP0] = GDB_SIGTRAP,
	[EXC_IRQ_SP0] = GDB_SIGSEGV,
	[EXC_FIQ_SP0] = GDB_SIGSEGV,
	[EXC_SERROR_SP0] = GDB_SIGSEGV,
	[EXC_SYNC_SPX] = GDB_SIGTRAP,
	[EXC_IRQ_SPX] = GDB_SIGSEGV,
	[EXC_FIQ_SPX] = GDB_SIGSEGV,
	[EXC_SERROR_SPX] = GDB_SIGSEGV,
	[EXC_SYNC_ELX_64] = GDB_SIGTRAP,
	[EXC_IRQ_ELX_64] = GDB_SIGSEGV,
	[EXC_FIQ_ELX_64] = GDB_SIGSEGV,
	[EXC_SERROR_ELX_64] = GDB_SIGSEGV,
	[EXC_SYNC_ELX_32] = GDB_SIGTRAP,
	[EXC_IRQ_ELX_32] = GDB_SIGSEGV,
	[EXC_FIQ_ELX_32] = GDB_SIGSEGV,
	[EXC_SERROR_ELX_32] = GDB_SIGSEGV
};

static int gdb_exception_hook(uint32_t type)
{
	return -1;
}

void gdb_arch_init(void)
{
	exception_install_hook(&gdb_exception_hook);
}

void gdb_arch_enter(void)
{
}

int gdb_arch_set_single_step(int on)
{
	/* GDB seems to only need this on x86, ARM works fine without it. */
	return -1;
}

void gdb_arch_encode_regs(struct gdb_message *message)
{
}

void gdb_arch_decode_regs(int offset, struct gdb_message *message)
{
}
