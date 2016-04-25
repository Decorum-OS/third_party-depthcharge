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

#ifndef __BASE_ALGORITHM_H__
#define __BASE_ALGORITHM_H__

#include <stdint.h>

#define _MIN_IMPL(a, b, suffix) ({ \
	typeof(a) _min_impl_a_##suffix = (a); \
	typeof(b) _min_impl_b_##suffix = (b); \
	_min_impl_a_##suffix < _min_impl_b_##suffix ? \
	_min_impl_a_##suffix : _min_impl_b_##suffix; \
})
#define MIN(a, b) _MIN_IMPL(a, b, __COUNTER__)

#define _MAX_IMPL(a, b, suffix) ({ \
	typeof(a) _max_impl_a_##suffix = (a); \
	typeof(b) _max_impl_b_##suffix = (b); \
	_max_impl_a_##suffix > _max_impl_b_##suffix ? \
	_max_impl_a_##suffix : _max_impl_b_##suffix; \
})
#define MAX(a, b) _MAX_IMPL(a, b, __COUNTER__)

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

// Count Leading Zeroes: CLZ(0) == bits in x, CLZ(0xf) == bits in x - 4,
// CLZ(1 << ({bits in x} - 1)) == 0.
#define _CLZ_IMPL(x, suffix) ({ \
	typeof(x) _clz_impl_x_##suffix = (x); \
	_clz_impl_x_##suffix ? __builtin_clz(_clz_impl_x_##suffix) : \
	sizeof(_clz_impl_x_##suffix) * 8; \
})
#define CLZ(x) _CLZ_IMPL(x, __COUNTER__)

// Integer binary logarithm (rounding down): LOG2(0) == -1, LOG2(5) == 2.
#define _LOG2_IMPL(x, suffix) ({ \
	typeof(x) _log2_impl_x_##suffix = (x); \
	sizeof(_log2_impl_x_##suffix) * 8 - CLZ(_log2_impl_x_##suffix) - 1; \
})
#define LOG2(x) _LOG2_IMPL(x, __COUNTER__)

#endif /* __BASE_ALGORITHM_H__ */
