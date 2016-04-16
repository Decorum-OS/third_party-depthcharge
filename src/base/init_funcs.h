/*
 * Copyright 2012 Google Inc.
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

#ifndef __BASE_INIT_FUNCS_H__
#define __BASE_INIT_FUNCS_H__

/*
 * Init functions will execute in priority order, where priorities are sorted
 * alphabetically.
 */

// The priority for functions which make the device twitch so external
// observers can tell it's alive.
#define INIT_FUNC_PRIORITY_ALIVE a_alive
// Consoles should be set up very early so we can see what's happening as
// quickly as possible.
#define INIT_FUNC_PRIORITY_CONSOLE c_console
// Initialize timestamps early so we can account for as much of execution
// as possible.
#define INIT_FUNC_PRIORITY_TIMESTAMP d_timestamp
// Print a banner to the console to say what's running.
#define INIT_FUNC_PRIORITY_BANNER e_banner
// The default priority.
#define INIT_FUNC_PRIORITY_NORMAL n_normal

typedef int (*init_func_t)(void);

// A version that takes the fully expanded priority name.
#define ___INIT_FUNC_PRIORITY(priority, func) \
	init_func_t __init_func_ptr__##func \
		__attribute__((section(".init_funcs." #priority))) = &func;

// A version which only requires the name of the priority level. The extra
// level of nesting is to ensure the priority name gets expanded properly.
#define __INIT_FUNC_PRIORITY(priority, func) \
	___INIT_FUNC_PRIORITY(priority, func)
#define _INIT_FUNC_PRIORITY(priority, func) \
	__INIT_FUNC_PRIORITY(INIT_FUNC_PRIORITY_##priority, func)


// If no priority is specified, default to NORMAL.
#define INIT_FUNC(func) _INIT_FUNC_PRIORITY(NORMAL, func)

// Designate init functions at the various priority levels.
#define INIT_FUNC_ALIVE(func) _INIT_FUNC_PRIORITY(ALIVE, func)
#define INIT_FUNC_CONSOLE(func) _INIT_FUNC_PRIORITY(CONSOLE, func)
#define INIT_FUNC_BANNER(func) _INIT_FUNC_PRIORITY(BANNER, func)
#define INIT_FUNC_TIMESTAMP(func) _INIT_FUNC_PRIORITY(TIMESTAMP, func)
#define INIT_FUNC_NORMAL(func) _INIT_FUNC_PRIORITY(NORMAL, func)

int run_init_funcs(void);

#endif /* __BASE_INIT_FUNCS_H__ */
