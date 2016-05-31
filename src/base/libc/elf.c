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

#include <arch/cache.h>
#include <elf.h>
#include <stdio.h>
#include <string.h>

void elf_load(Elf32_Ehdr *ehdr)
{
	uintptr_t base = (uintptr_t)ehdr;
	uintptr_t addr = (uintptr_t)ehdr + ehdr->e_phoff;
	uintptr_t step = ehdr->e_phentsize;

	// Copy over the ELF segments.
	for (int num = ehdr->e_phnum; num; num--) {
		Elf32_Phdr *phdr = (Elf32_Phdr *)addr;
		addr += step;

		if (phdr->p_type != ElfPTypeLoad)
			continue;

		uint8_t *dest = (uint8_t *)(uintptr_t)phdr->p_paddr;
		uint8_t *src = (uint8_t *)(uintptr_t)(base + phdr->p_offset);
		uint32_t filesz = phdr->p_filesz;
		uint32_t memsz = phdr->p_memsz;

		if (filesz)
			memcpy(dest, src, filesz);
		if (memsz > filesz)
			memset(dest + filesz, 0, memsz - filesz);
	}
}

void elf_start(Elf32_Ehdr *ehdr, void *param)
{
	cache_sync_instructions();

	// Go for it!
	typedef void (*entry_func)(void *) __attribute__((noreturn));
	((entry_func)ehdr->e_entry)(param);
}

int elf_check_header(Elf32_Ehdr *elf)
{
	// Check that it's a reasonable ELF image.
	unsigned char *e_ident = (unsigned char *)elf;
	if (e_ident[0] != ElfMag0Val || e_ident[1] != ElfMag1Val ||
		e_ident[2] != ElfMag2Val || e_ident[3] != ElfMag3Val) {
		printf("Bad ELF magic value.\n");
		return -1;
	}
	if (e_ident[EI_Class] != ElfClass32) {
		printf("Only 32 bit ELF files are supported.\n");
		return -1;
	}
	return 0;
}
