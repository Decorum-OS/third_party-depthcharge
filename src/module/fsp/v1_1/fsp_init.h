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

#ifndef __MODULE_FSP_V1_1_FSP_INIT_H__
#define __MODULE_FSP_V1_1_FSP_INIT_H__

#include <stdint.h>

enum {
	FspV1_1BootWithFullConfiguration = 0x0,
	FspV1_1BootWithMinimalConfiguration = 0x1,
	FspV1_1BootAssumingNoConfigurationChanges = 0x2,
	FspV1_1BootWithFullConfigurationPlusDiagnostics = 0x3,
	FspV1_1BootWithDefaultSettings = 0x4,
	FspV1_1BootOnS4Resume = 0x5,
	FspV1_1BootOnS5Resume = 0x6,
	FspV1_1BootOnS2Resume = 0x10,
	FspV1_1BootOnS3Resume = 0x11,
	FspV1_1BootOnFlashUpdate = 0x12,
	FspV1_1BootInRecoveryMode = 0x20
};

typedef struct __attribute__((packed))
{
	uint32_t *stack_top;
	uint32_t boot_mode;
	void *upd_data_rgn_ptr;
	uint32_t reserved[7];
} FspV1_1InitRtCommonBuffer;

typedef void (*FspV1_1InitContinuation)(uint32_t status, void *hob_list_ptr);

typedef struct __attribute__((packed))
{
	void *nvs_buffer_ptr;
	void *rt_buffer_ptr;
	FspV1_1InitContinuation continuation_func;
} FspV1_1InitParams;

typedef uint32_t (*FspV1_1InitFunc)(FspV1_1InitParams *fsp_init_param_ptr);

#endif /* __MODULE_FSP_V1_1_FSP_INIT_H__ */
