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

#ifndef __MODULE_FSP_TEMP_STACK_H__
#define __MODULE_FSP_TEMP_STACK_H__

#include <stdint.h>

#include "module/fsp/fsp.h"

void temp_stack_puts(const char *message);

void temp_stack_print_num(uint64_t num, int width);
void temp_stack_print_num64(uint64_t num);
void temp_stack_print_num32(uint32_t num);
void temp_stack_print_num16(uint16_t num);
void temp_stack_print_num8(uint8_t num);

void temp_stack_print_guid(EfiGuid *guid);

void temp_stack_print_hob_list(EfiHobPointers hob_list_ptr);

void *temp_stack_find_dcdir_anchor(void);
void *temp_stack_find_in_dir(uint32_t *size, uint32_t *new_base, int *is_dir,
			     const char *name, uint32_t base, void *dir);
void *temp_stack_find_dir_in_dir(uint32_t *size, uint32_t *new_base,
				 const char *name, uint32_t base, void *dir);
void *temp_stack_find_region_in_dir(uint32_t *size, uint32_t *new_base,
				    const char *name, uint32_t base, void *dir);

#endif /* __MODULE_FSP_TEMP_STACK_H__ */
