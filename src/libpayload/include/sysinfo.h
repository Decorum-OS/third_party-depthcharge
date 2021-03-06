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

#ifndef _SYSINFO_H
#define _SYSINFO_H

/* Maximum number of memory range definitions. */
#define SYSINFO_MAX_MEM_RANGES 32

#include <coreboot_tables.h>

struct sysinfo_t {
	int n_memranges;

	uint32_t vbnv_start;
	uint32_t vbnv_size;

	struct cb_framebuffer *framebuffer;

	struct cb_header *header;

	void		*vdat_addr;
	uint32_t	vdat_size;

	void		*tstamp_table;
	void		*cbmem_cons;
	void		*acpi_gnvs;
	uint32_t	board_id;
	uint32_t	ram_code;
	uint64_t	ramoops_buffer;
	uint32_t	ramoops_buffer_size;
	struct {
		uint32_t size;
		uint32_t sector_size;
		uint32_t erase_cmd;
	} spi_flash;
	uint64_t cbfs_offset;
	uint64_t cbfs_size;
	uint64_t mtc_start;
	uint32_t mtc_size;
};

extern struct sysinfo_t lib_sysinfo;

/*
 * Check if this is an architecture specific coreboot table record and process
 * it, if it is. Return 1 if record type was recognized, 0 otherwise.
 */
int cb_parse_arch_specific(struct cb_record *rec, struct sysinfo_t *info);

/*
 * Check if the region in range addr..addr+len contains a 16 byte aligned
 * coreboot table. If it does - process the table filling up the sysinfo
 * structure with information from the table. Return 0 on success and -1 on
 * failure.
 */
int cb_parse_header(void *addr, int len, struct sysinfo_t *info);

#endif
