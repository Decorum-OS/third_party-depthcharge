/*
 * Copyright 2012 Google Inc.
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

#include <elf.h>
#include <lzma.h>
#include <stdio.h>

#include "module/module.h"
#include "module/symbols.h"
#include "module/trampoline/trampoline.h"

int start_module(const void *compressed_image, uint32_t size)
{
	// Put the decompressed module at the end of the trampoline.
	Elf32_Ehdr *elf = (Elf32_Ehdr *)&_tramp_end;

	//XXX This is a hack for now which assumes the decompression area
	// ends at the end of the kernel area. Once some sort of global
	// memory allocator exists which can keep track of these things
	// explicitly (as opposed to by convention) then this can go away.
	uint8_t *decomp_end = (uint8_t *)(uintptr_t)(CONFIG_KERNEL_START +
						     CONFIG_KERNEL_SIZE);

	// Decompress the trampoline itself.
	uint32_t out_size = ulzman(&_binary_trampoline_start, size,
				   (unsigned char *)elf,
				   decomp_end - &_tramp_end);
	if (!out_size) {
		printf("Error decompressing trampoline.\n");
		return -1;
	}

	// Expand the trampoline into place.
	if (elf_check_header(elf))
		return -1;
	elf_load(elf);

	// Decompress the target image.
	out_size = ulzman(compressed_image, size, (unsigned char *)elf,
			  decomp_end - &_tramp_end);
	if (!out_size) {
		printf("Error decompressing module.\n");
		return -1;
	}

	// Do some basic checks on the headers.
	if (elf_check_header(elf))
		return -1;

	enter_trampoline(elf);

	// We should never actually reach the end of this function.
	return 0;
}
