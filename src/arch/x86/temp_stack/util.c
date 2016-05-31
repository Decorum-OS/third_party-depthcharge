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
#include <string.h>

#include "arch/x86/temp_stack/util.h"

void temp_stack_puts(const char *message)
{
	__asm__ __volatile__(
		"push %%ebp\n"
		"pushf\n"

		"mov %%esp, %%dr0\n"
		"mov $0f, %%esp\n"
		"jmp preram_puts\n"
		"0:\n"
		"mov %%dr0, %%esp\n"

		"popf\n"
		"pop %%ebp\n"
	: : "S"(message) : "eax", "ebx", "ecx", "edx", "edi"
	);
}

void temp_stack_print_num(uint64_t num, int width)
{
	for (int digit_idx = width; digit_idx; digit_idx--) {
		uint64_t digit = (num >> ((digit_idx - 1) * 4)) & 0xf;
		uint32_t character = (digit < 10) ? ('0' + digit) :
						    ('a' + digit - 10);
		__asm__ __volatile__(
			"push %%ebp\n"
			"pushf\n"

			"mov %%esp, %%dr0\n"
			"mov $0f, %%esp\n"
			"jmp preram_uart_send_char\n"
			"0:\n"
			"mov %%dr0, %%esp\n"

			"popf\n"
			"pop %%ebp\n"
		: : "b"(character) : "eax", "ecx", "edx", "esi", "edi"
		);
	}
}

void *temp_stack_find_dcdir_anchor(void)
{
	void *anchor;
	__asm__ __volatile__(
		"push %%ebp\n"
		"pushf\n"

		"mov %%esp, %%dr0\n"
		"mov $0f, %%esp\n"
		"jmp preram_find_dcdir_anchor\n"
		"0:\n"
		"mov %%dr0, %%esp\n"

		"popf\n"
		"pop %%ebp\n"
	: "=D"(anchor) : : "eax", "ebx", "ecx", "edx", "esi"
	);
	return anchor;
}

void *temp_stack_find_in_dir(uint32_t *size, uint32_t *new_base, int *is_dir,
			     const char *name, uint32_t base, void *dir)
{
	uint32_t name_buf[2];
	memset(name_buf, 0, sizeof(name_buf));
	strncpy((void *)name_buf, name, sizeof(name_buf));

	uint32_t _size, _new_base, _is_dir;

	void *child;
	__asm__ __volatile__(
		"push %%ebp\n"
		"pushf\n"

		"mov %%esp, %%dr0\n"
		"mov $0f, %%esp\n"
		"jmp preram_find_in_dir\n"
		"0:\n"
		"mov %%dr0, %%esp\n"

		"pushf\n"
		"pop %%edi\n"
		"and $1, %%edi\n"

		"popf\n"
		"pop %%ebp\n"
	: "=a"(child), "=d"(_size), "=S"(_new_base), "=D"(_is_dir)
	: "a"(name_buf[0]), "d"(name_buf[1]), "S"(base), "D"(dir)
	: "ebx", "ecx"
	);

	if (size)
		*size = _size;
	if (new_base)
		*new_base = _new_base;
	if (is_dir)
		*is_dir = _is_dir;

	return child;
}

void *temp_stack_find_dir_in_dir(uint32_t *size, uint32_t *new_base,
				 const char *name, uint32_t base, void *dir)
{
	int is_dir;
	void *res = temp_stack_find_in_dir(size, new_base, &is_dir,
					   name, base, dir);
	if (!is_dir) {
		temp_stack_puts(name);
		temp_stack_puts(" is not a directory.\n");
		return NULL;
	}
	return res;
}

void *temp_stack_find_region_in_dir(uint32_t *size, uint32_t *new_base,
				    const char *name, uint32_t base, void *dir)
{
	int is_dir;
	void *res = temp_stack_find_in_dir(size, new_base, &is_dir,
					   name, base, dir);
	if (is_dir) {
		temp_stack_puts(name);
		temp_stack_puts(" is a directory.\n");
		return NULL;
	}
	return res;
}

void temp_stack_print_num64(uint64_t num)
{
	temp_stack_print_num(num, sizeof(num) * 2);
}

void temp_stack_print_num32(uint32_t num)
{
	temp_stack_print_num(num, sizeof(num) * 2);
}

void temp_stack_print_num16(uint16_t num)
{
	temp_stack_print_num(num, sizeof(num) * 2);
}

void temp_stack_print_num8(uint8_t num)
{
	temp_stack_print_num(num, sizeof(num) * 2);
}
