/*
 * Copyright 2013 Google Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <arch/msr.h>
#include <stdio.h>

#include "arch/x86/cpu.h"
#include "base/cleanup_funcs.h"
#include "base/init_funcs.h"
#include "base/io.h"

static int x86_mtrr_cleanup(struct CleanupFunc *cleanup, CleanupType type)
{
	/*
	 * Un-cache the ROM so the kernel has one more MTRR available.
	 * Coreboot should have assigned this to the top available variable
	 * MTRR.
	 */
	uint8_t top_mtrr = (_rdmsr(MTRRcap_MSR) & 0xff) - 1;
	uint8_t top_type = _rdmsr(MTRRphysBase_MSR(top_mtrr)) & 0xff;

	// Make sure this MTRR is the correct Write-Protected type.
	if (top_type == MTRR_TYPE_WP) {
		disable_cache();
		_wrmsr(MTRRphysBase_MSR(top_mtrr), 0);
		_wrmsr(MTRRphysMask_MSR(top_mtrr), 0);
		enable_cache();
	}
	return 0;
}

static int x86_mtrr_cleanup_install(void)
{
	static CleanupFunc dev =
	{
		&x86_mtrr_cleanup,
		CleanupOnHandoff | CleanupOnLegacy,
		NULL
	};

	list_insert_after(&dev.list_node, &cleanup_funcs);
	return 0;
}

INIT_FUNC(x86_mtrr_cleanup_install);

static int coreboot_finalize(struct CleanupFunc *cleanup, CleanupType type)
{
	// Indicate legacy mode for coreboot fixups
	if (type == CleanupOnLegacy)
		outb(0xcc, 0xb2);

	// Issue SMI to Coreboot to lock down ME and registers.
	printf("Finalizing Coreboot\n");
	outb(0xcb, 0xb2);
	return 0;
}

static int coreboot_finalize_install(void)
{
	static CleanupFunc dev =
	{
		&coreboot_finalize,
		CleanupOnHandoff | CleanupOnLegacy,
		NULL
	};

	list_insert_after(&dev.list_node, &cleanup_funcs);
	return 0;
}

INIT_FUNC(coreboot_finalize_install);
