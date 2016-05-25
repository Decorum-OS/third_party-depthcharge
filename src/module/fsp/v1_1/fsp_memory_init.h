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

#ifndef __MODULE_FSP_V1_1_FSP_MEMORY_INIT_H__
#define __MODULE_FSP_V1_1_FSP_MEMORY_INIT_H__

#include "module/fsp/v1_1/fsp_init.h"

typedef struct __attribute__((packed))
{
	void *nvs_buffer_ptr;
	FspV1_1InitRtBuffer *rt_buffer_ptr;
	void **hob_list_ptr;
} FspV1_1MemoryInitParams;

typedef uint32_t (*FspV1_1MemoryInit)(FspV1_1MemoryInitParams *params);

#endif /* __MODULE_FSP_V1_1_FSP_MEMORY_INIT_H__ */
