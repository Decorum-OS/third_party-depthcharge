/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.
 */

#include <libpayload.h>
#include <stdint.h>

#include "base/io.h"

static uint32_t *const mtk_tmrus = (void*)CONFIG_DRIVER_TIMER_MEDIATEK_ADDRESS;

uint64_t timer_hz(void)
{
	return CONFIG_DRIVER_TIMER_MEDIATEK_HZ;
}

uint64_t timer_raw_value(void)
{
	return (uint64_t)readl(mtk_tmrus);
}
