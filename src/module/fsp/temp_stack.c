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
 * but without any warranty; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdint.h>
#include <stdlib.h>

#include "module/fsp/temp_stack.h"

void temp_stack_fsp(void *fsp_info_header, void *temp_ram_base,
		    void *temp_ram_end) __attribute__((noreturn));
void temp_stack_fsp(void *fsp_info_header, void *temp_ram_base,
		    void *temp_ram_end)
{
	temp_stack_puts("Preparing to call FSP memory init.\n");
	halt();
}
