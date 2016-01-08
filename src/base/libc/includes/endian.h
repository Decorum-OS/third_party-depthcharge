/*
 * This file is part of the libpayload project.
 *
 * Copyright (c) 2012 The Chromium OS Authors.
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

#ifndef _ENDIAN_H_
#define _ENDIAN_H_

#include <arch/io.h>
#include <arch/types.h>

static inline uint16_t swap_bytes16(uint16_t in)
{
	return ((in & 0xFF) << 8) | ((in & 0xFF00) >> 8);
}

static inline uint32_t swap_bytes32(uint32_t in)
{
	return (uint32_t)__builtin_bswap32(in);
}

static inline uint64_t swap_bytes64(uint64_t in)
{
	return (uint64_t)__builtin_bswap64(in);
}

#if CONFIG_BIG_ENDIAN

#define htobe16(in) (in)
#define htobe32(in) (in)
#define htobe64(in) (in)

#define htole16(in) swap_bytes16(in)
#define htole32(in) swap_bytes32(in)
#define htole64(in) swap_bytes64(in)

#elif CONFIG_LITTLE_ENDIAN

#define htobe16(in) swap_bytes16(in)
#define htobe32(in) swap_bytes32(in)
#define htobe64(in) swap_bytes64(in)

#define htole16(in) (in)
#define htole32(in) (in)
#define htole64(in) (in)

#else

#error Cant tell if the CPU is little or big endian.

#endif /* CONFIG_*_ENDIAN */

#define be16toh(in) htobe16(in)
#define be32toh(in) htobe32(in)
#define be64toh(in) htobe64(in)

#define le16toh(in) htole16(in)
#define le32toh(in) htole32(in)
#define le64toh(in) htole64(in)

#define htonw(in) htobe16(in)
#define htonl(in) htobe32(in)
#define htonll(in) htobe64(in)

#define ntohw(in) be16toh(in)
#define ntohl(in) be32toh(in)
#define ntohll(in) be64toh(in)

// Handy bit manipulation functions.

static inline void clrsetbits_le32(void *addr, uint32_t clear, uint32_t set)
{
	writel(htole32((le32toh(readl(addr)) & ~clear) | set), addr);
}
static inline void setbits_le32(void *addr, uint32_t set)
{
	writel(htole32(le32toh(readl(addr)) | set), addr);
}
static inline void clrbits_le32(void *addr, uint32_t clear)
{
	writel(htole32(le32toh(readl(addr)) & ~clear), addr);
}

static inline void clrsetbits_be32(void *addr, uint32_t clear, uint32_t set)
{
	writel(htobe32((be32toh(readl(addr)) & ~clear) | set), addr);
}
static inline void setbits_be32(void *addr, uint32_t set)
{
	writel(htobe32(be32toh(readl(addr)) | set), addr);
}
static inline void clrbits_be32(void *addr, uint32_t clear)
{
	writel(htobe32(be32toh(readl(addr)) & ~clear), addr);
}

#endif /* _ENDIAN_H_ */
