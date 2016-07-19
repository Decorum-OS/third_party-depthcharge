/*
 * Copyright 2016 Google Inc.
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

#include <base/exception.h>
#include <stdio.h>
#include <stdlib.h>

#include "arch/x86/amd64/handoff/handoff.h"
#include "base/init_funcs.h"
#include "module/module.h"

void handoff_common(void)
{
	// Do handoff method specific work, if any.
	handoff_special();

	// Run any generic initialization functions that are compiled in.
	if (run_init_funcs())
		halt();

	module_main();
	printf("Returned from module_main.\n");
	halt();
}
