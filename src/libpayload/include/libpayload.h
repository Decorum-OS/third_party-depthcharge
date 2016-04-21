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

#ifndef _LIBPAYLOAD_H
#define _LIBPAYLOAD_H

#include <stddef.h>
#include <stdio.h>
#include <arch/types.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

uint8_t nvram_read(uint8_t addr);
void nvram_write(uint8_t val, uint8_t addr);

int usb_initialize(void);
int usb_exit (void);

int video_init(void);
int video_console_init(void);
void video_get_rows_cols(unsigned int *rows, unsigned int *cols);
void video_console_putchar(unsigned int ch);
void video_console_putc(uint8_t row, uint8_t col, unsigned int ch);
void video_console_clear(void);
void video_console_cursor_enable(int state);
void video_console_get_cursor(unsigned int *x, unsigned int *y, unsigned int *en);
void video_console_set_cursor(unsigned int cursorx, unsigned int cursory);
/*
 * print characters on video console with colors. note that there is a size
 * restriction for the internal buffer. so, output string can be truncated.
 */
enum video_printf_align {
	VIDEO_PRINTF_ALIGN_KEEP = 0,
	VIDEO_PRINTF_ALIGN_LEFT,
	VIDEO_PRINTF_ALIGN_CENTER,
	VIDEO_PRINTF_ALIGN_RIGHT,
};
void video_printf(int foreground, int background, enum video_printf_align align,
		  const char *fmt, ...);

/* Count Leading Zeroes: clz(0) == 32, clz(0xf) == 28, clz(1 << 31) == 0 */
static inline int clz(uint32_t x)
{
	return x ? __builtin_clz(x) : sizeof(x) * 8;
}
/* Integer binary logarithm (rounding down): log2(0) == -1, log2(5) == 2 */
static inline int log2(uint32_t x)
{
	return sizeof(x) * 8 - clz(x) - 1;
}
/* Find First Set: __ffs(0xf) == 0, __ffs(0) == -1, __ffs(1 << 31) == 31 */
static inline int __ffs(uint32_t x)
{
	return log2(x & (uint32_t)(-(int32_t)x));
}

#endif
