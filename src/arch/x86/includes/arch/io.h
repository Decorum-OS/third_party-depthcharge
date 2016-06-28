/*
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
 * Copyright (C) 2008 coresystems GmbH
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

#ifndef _ARCH_IO_H
#define _ARCH_IO_H

#include <stdint.h>

static inline __attribute__((always_inline)) uint8_t read8(const volatile void *addr)
{
	return *((volatile uint8_t *)(addr));
}

static inline __attribute__((always_inline)) uint16_t read16(const volatile void *addr)
{
	return *((volatile uint16_t *)(addr));
}

static inline __attribute__((always_inline)) uint32_t read32(const volatile void *addr)
{
	return *((volatile uint32_t *)(addr));
}

static inline __attribute__((always_inline)) void write8(volatile void *addr, uint8_t value)
{
	*((volatile uint8_t *)(addr)) = value;
}

static inline __attribute__((always_inline)) void write16(volatile void *addr, uint16_t value)
{
	*((volatile uint16_t *)(addr)) = value;
}

static inline __attribute__((always_inline)) void write32(volatile void *addr, uint32_t value)
{
	*((volatile uint32_t *)(addr)) = value;
}

static inline uint32_t inl(int port)
{
	uint32_t val;
	__asm__ __volatile__("inl %w1, %0" : "=a"(val) : "Nd"(port));
	return val;
}

static inline uint16_t inw(int port)
{
	uint16_t val;
	__asm__ __volatile__("inw %w1, %w0" : "=a"(val) : "Nd"(port));
	return val;
}

static inline uint8_t inb(int port)
{
	uint8_t val;
	__asm__ __volatile__("inb %w1, %b0" : "=a"(val) : "Nd"(port));
	return val;
}

static inline void outl(uint32_t val, int port)
{
	__asm__ __volatile__("outl %0, %w1" : : "a"(val), "Nd"(port));
}

static inline void outw(uint16_t val, int port)
{
	__asm__ __volatile__("outw %w0, %w1" : : "a"(val), "Nd"(port));
}

static inline void outb(uint8_t val, int port)
{
	__asm__ __volatile__("outb %b0, %w1" : : "a"(val), "Nd"(port));
}

static inline void outsl(int port, const void *addr, uint32_t count)
{
	__asm__ __volatile__("rep; outsl" : "+S"(addr), "+c"(count) : "d"(port));
}

static inline void outsw(int port, const void *addr, uint32_t count)
{
	__asm__ __volatile__("rep; outsw" : "+S"(addr), "+c"(count) : "d"(port));
}

static inline void outsb(int port, const void *addr, uint32_t count)
{
	__asm__ __volatile__("rep; outsb" : "+S"(addr), "+c"(count) : "d"(port));
}

static inline void insl(int port, void *addr, uint32_t count)
{
	__asm__ __volatile__("rep; insl" : "+D"(addr), "+c"(count) : "d"(port)
			     : "memory");
}

static inline void insw(int port, void *addr, uint32_t count)
{
	__asm__ __volatile__("rep; insw" : "+D"(addr), "+c"(count) : "d"(port)
			     : "memory");
}

static inline void insb(int port, void *addr, uint32_t count)
{
	__asm__ __volatile__("rep; insb" : "+D"(addr), "+c"(count) : "d"(port)
			     : "memory");
}

#endif
