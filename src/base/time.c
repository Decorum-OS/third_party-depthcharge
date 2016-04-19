/*
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <libpayload.h>
#include <stdio.h>

#include "base/time.h"
#include "drivers/timer/timer.h"

static inline void _delay(uint64_t delta)
{
	uint64_t start = timer_raw_value();
	while (timer_raw_value() - start < delta) ;
}

/**
 * Delay for a specified number of nanoseconds.
 *
 * @param n Number of nanoseconds to delay for.
 */
void ndelay(uint64_t n)
{
	_delay(n * timer_hz() / 1000000000);
}

/**
 * Delay for a specified number of microseconds.
 *
 * @param n Number of microseconds to delay for.
 */
void udelay(uint64_t u)
{
	_delay(u * timer_hz() / 1000000);
}

/**
 * Delay for a specified number of milliseconds.
 *
 * @param m Number of milliseconds to delay for.
 */
void mdelay(uint64_t m)
{
	_delay(m * timer_hz() / 1000);
}

/**
 * Delay for a specified number of seconds.
 *
 * @param s Number of seconds to delay for.
 */
void delay(uint64_t s)
{
	_delay(s * timer_hz());
}

uint64_t time_us(uint64_t base)
{
	static uint64_t hz;

	// Only check timer_hz once. Assume it doesn't change.
	if (hz == 0) {
		hz = timer_hz();
		if (hz < 1000000) {
			printf("Timer frequency %lld is too low, "
			       "must be at least 1MHz.\n", hz);
			halt();
		}
	}

	return (1000000 * timer_raw_value()) / hz - base;
}
