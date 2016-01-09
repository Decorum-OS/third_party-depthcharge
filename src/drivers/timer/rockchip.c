/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2014 Rockchip Electronics
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

struct rk_timer {
	uint32_t timer_load_count0;
	uint32_t timer_load_count1;
	uint32_t timer_curr_value0;
	uint32_t timer_curr_value1;
	uint32_t timer_ctrl_reg;
	uint32_t timer_int_status;
};

uint64_t timer_hz(void)
{
	return 24000000;
}

uint64_t timer_raw_value(void)
{
	uint64_t upper;
	uint64_t lower;
	struct rk_timer *rk_timer;
	rk_timer = (struct rk_timer *)CONFIG_DRIVER_TIMER_ROCKCHIP_ADDRESS;
	lower = (uint64_t)rk_timer->timer_curr_value0;
	upper = (uint64_t)rk_timer->timer_curr_value1;
	return (upper << 32) | lower;
}
