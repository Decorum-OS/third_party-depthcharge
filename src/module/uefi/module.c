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

#include <assert.h>
#include <elf.h>
#include <stdio.h>

#include "base/container_of.h"
#include "base/die.h"
#include "base/fwdb.h"
#include "base/lzma/lzma.h"
#include "base/xalloc.h"
#include "drivers/storage/storage.h"
#include "module/module.h"
#include "module/uefi/module.h"
#include "uefi/uefi.h"

typedef void (*EntryFunc)(FwdbHeader *) __attribute__((noreturn));

static EntryFunc uefi_load_segment(Elf64_Ehdr *ehdr, Elf64_Phdr *phdr)
{
	EFI_SYSTEM_TABLE *system_table = uefi_system_table_ptr();
	assert(system_table);

	EFI_BOOT_SERVICES *bs = system_table->BootServices;

	uint64_t filesz = phdr->p_filesz;
	uint64_t memsz = phdr->p_memsz;

	EFI_PHYSICAL_ADDRESS pages_addr;
	UINTN pages = (memsz + 4 * 1024 - 1) / (4 * 1024);
	EFI_STATUS status = bs->AllocatePages(AllocateAnyPages, EfiLoaderData,
					      pages, &pages_addr);
	die_if(status != EFI_SUCCESS, "Failed to allocate pages.\n");

	uint8_t *pages_ptr = (uint8_t *)(uintptr_t)pages_addr;
	uint8_t *src = (uint8_t *)ehdr + phdr->p_offset;
	memcpy(pages_ptr, src, filesz);
	memset(pages_ptr + filesz, 0, memsz - filesz);

	uintptr_t entry_addr = (uintptr_t)ehdr->e_entry;
	uintptr_t entry_offset = entry_addr - phdr->p_paddr;
	die_if(entry_addr < phdr->p_paddr || entry_offset > memsz,
	       "Entry point outside of segment.\n");

	return (EntryFunc)(pages_ptr + entry_offset);
}

static void __attribute__((noreturn)) start_uefi_module(
	void *compressed_image, uint32_t compressed_size)
{
	ssize_t size = ulzma_expanded_size(compressed_image, compressed_size);
	assert(size > 0);

	void *image = xmalloc(size);
	assert(ulzman(compressed_image, compressed_size, image, size) == size);

	Elf64_Ehdr *ehdr = image;
	EntryFunc entry = NULL;

	uintptr_t addr = (uintptr_t)ehdr + ehdr->e_phoff;
	uintptr_t step = ehdr->e_phentsize;
	for (int num = ehdr->e_phnum; num; num--) {
		Elf64_Phdr *phdr = (Elf64_Phdr *)addr;
		addr += step;

		if (phdr->p_type == ElfPTypeLoad) {
			die_if(entry, "Too many loadable segments.\n");

			entry = uefi_load_segment(ehdr, phdr);
		}
	}

	FwdbHeader *fwdb = fwdb_db_pointer();
	assert(fwdb);
	entry(fwdb);
}

static int uefi_dc_module_start(DcModuleOps *me)
{
	UefiDcModule *module = container_of(me, UefiDcModule, ops);

	int size = storage_size(module->storage);
	if (size < 0)
		return 1;

	void *image = xmalloc(size);
	if (storage_read(module->storage, image, 0, size)) {
		free(image);
		return 1;
	}

	start_uefi_module(image, size);
}

UefiDcModule *new_uefi_dc_module(StorageOps *storage)
{
	UefiDcModule *module = xzalloc(sizeof(*module));
	module->ops.start = &uefi_dc_module_start;
	module->storage = storage;
	return module;
}
