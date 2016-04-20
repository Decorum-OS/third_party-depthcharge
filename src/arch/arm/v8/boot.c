/*
 * Copyright 2013 Google Inc.
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
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <coreboot_tables.h>
#include <libpayload.h>
#include <lzma.h>
#include <lz4.h>
#include <stdlib.h>
#include <sysinfo.h>

#include "arch/arm/boot.h"
#include "base/io.h"
#include "base/physmem.h"
#include "base/ranges.h"
#include "base/timestamp.h"
#include "vboot/boot.h"

static const size_t MaxKernelSize = 64 * MiB;

typedef struct {
	uint32_t code0;
	uint32_t code1;
	uint64_t text_offset;
	uint64_t image_size;
	uint64_t flags;
	uint64_t res2;
	uint64_t res3;
	uint64_t res4;
	uint32_t magic;
	uint32_t res5;
} Arm64KernelHeader;

struct {
	union {
		Arm64KernelHeader header;
		uint8_t raw[sizeof(Arm64KernelHeader) + 0x100];
	};
	uint32_t canary;
} scratch;

static void *get_kernel_reloc_addr(uint32_t load_offset)
{
	for (int i = 0; i < lib_sysinfo.n_memranges; i++) {
		struct memrange *range = &lib_sysinfo.memrange[i];
		if (range->type != CB_MEM_RAM)
			continue;

		uint64_t start = range->base;
		uint64_t end = range->base + range->size;
		uint64_t kstart = ALIGN_UP(start, 2 * MiB) + load_offset;
		uint64_t kend = kstart + MaxKernelSize;

		if (kend > CONFIG_BASE_ADDRESS || kend > CONFIG_KERNEL_START ||
		    kend > CONFIG_KERNEL_FIT_FDT_ADDR) {
			printf("ERROR: Kernel might overlap depthcharge!\n");
			return 0;
		}

		if (kend <= end)
			return (void *)kstart;

		// Should be avoided in practice, that memory might be wasted.
		printf("WARNING: Skipping low memory range [%p:%p]!\n",
			       (void *)start, (void *)end);
	}

	printf("ERROR: Cannot find enough continuous memory for kernel!\n");
	return 0;
}

int boot_arm_linux(void *fdt, FitImageNode *kernel)
{
	static const uint32_t KernelHeaderMagic = 0x644d5241;
	static const uint32_t ScratchCanaryValue = 0xdeadbeef;

	// Partially decompress to get text_offset. Can't check for errors.
	scratch.canary = ScratchCanaryValue;
	switch (kernel->compression) {
	case CompressionNone:
		memcpy(scratch.raw, kernel->data, sizeof(scratch.raw));
		break;
	case CompressionLzma:
		ulzman(kernel->data, kernel->size,
		       scratch.raw, sizeof(scratch.raw));
		break;
	case CompressionLz4:
		ulz4fn(kernel->data, kernel->size,
		       scratch.raw, sizeof(scratch.raw));
		break;
	default:
		printf("ERROR: Unsupported compression algorithm!\n");
		return 1;
	}

	// Should never happen, but if it does we'll want to know.
	if (scratch.canary != ScratchCanaryValue) {
		printf("ERROR: Partial decompression ran over scratchbuf!\n");
		return 1;
	}

	if (scratch.header.magic != KernelHeaderMagic) {
		printf("ERROR: Invalid kernel magic: %#.8x\n != %#.8x\n",
		       scratch.header.magic, KernelHeaderMagic);
		return 1;
	}

	void *reloc_addr = get_kernel_reloc_addr(scratch.header.text_offset);
	if (!reloc_addr)
		return 1;

	size_t true_size = kernel->size;
	switch (kernel->compression) {
	case CompressionNone:
		if (kernel->size > MaxKernelSize) {
			printf("ERROR: Cannot relocate a kernel this large!\n");
			return 1;
		}
		printf("Relocating kernel to %p\n", reloc_addr);
		memmove(reloc_addr, kernel->data, kernel->size);
		break;
	case CompressionLzma:
		printf("Decompressing LZMA kernel to %p\n", reloc_addr);
		true_size = ulzman(kernel->data, kernel->size,
				   reloc_addr, MaxKernelSize);
		if (!true_size) {
			printf("ERROR: LZMA decompression failed!\n");
			return 1;
		}
		break;
	case CompressionLz4:
		printf("Decompressing LZ4 kernel to %p\n", reloc_addr);
		true_size = ulz4fn(kernel->data, kernel->size,
				   reloc_addr, MaxKernelSize);
		if (!true_size) {
			printf("ERROR: LZ4 decompression failed!\n");
			return 1;
		}
		break;
	default:
		return 1;
	}

	printf("jumping to kernel\n");

	timestamp_add_now(TS_START_KERNEL);

	/* Flush dcache and icache to make loaded code visible. */
	arch_program_segment_loaded(reloc_addr, true_size);

	tlb_invalidate_all();

	boot_arm_linux_jump(fdt, reloc_addr);

	return 0;
}
