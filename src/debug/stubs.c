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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdint.h>

/*
 * These stubs are linked for debug-related functions in non-developer builds
 * to ensure that those features do not work. Developer builds will override
 * them with the definitions from dev.c.
 */

void gdb_enter(void) __attribute__((weak));
void gdb_enter(void) { /* Do nothing. */ }

void gdb_exit(int8_t exit_code) __attribute__((weak));
void gdb_exit(int8_t exit_code) { /* Do nothing. */ }

void dc_dev_netboot(void) __attribute__((weak));
void dc_dev_netboot(void) { /* Do nothing. */ }
