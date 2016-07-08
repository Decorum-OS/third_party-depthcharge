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

#include <endian.h>
#include <stdio.h>
#include <string.h>
#include <sysinfo.h>
#include <vboot_api.h>

#include "arch/cache.h"
#include "base/cbfs/cbfs.h"
#include "base/cbfs/ram_media.h"
#include "base/cleanup_funcs.h"
#include "base/lzma/lzma.h"
#include "base/xalloc.h"
#include "board/board.h"
#include "drivers/flash/flash.h"
#include "drivers/storage/storage.h"
#include "image/fmap.h"

static void load_payload_and_run(struct cbfs_payload *payload);

int VbExLegacy(void)
{
	StorageOps *legacy = board_storage_legacy();
	int size = storage_size(legacy);
	if (size < 0)
		return 1;
	void *legacy_buf = xmalloc(size);
	if (storage_read(legacy, legacy_buf, 0, size)) {
		free(legacy_buf);
		return 1;
	}

	struct cbfs_media media;
	if (init_cbfs_ram_media(&media, legacy_buf, size)) {
		printf("Could not initialize legacy cbfs.\n");
		free(legacy_buf);
		return 1;
	}

	struct cbfs_payload *payload = cbfs_load_payload(&media, "payload");

	if (payload == NULL) {
		printf("Could not find payload in legacy cbfs.\n");
		free(legacy_buf);
		return 1;
	}

	load_payload_and_run(payload);

	// Should never return unless there is an error.
	free(legacy_buf);
	return 1;
}

static void load_payload_and_run(struct cbfs_payload *payload)
{
	// This is a minimalistic SELF parser.
	struct cbfs_payload_segment *seg = &payload->segments;
	char *base = (void *)seg;

	while (1) {
		void *src = base + be32toh(seg->offset);
		void *dst = (void *)(unsigned long)be64toh(seg->load_addr);
		uint32_t src_len = be32toh(seg->len);
		uint32_t dst_len = be32toh(seg->mem_len);

		typedef void (*EntryFunc)(void);

		switch (seg->type) {
		case PAYLOAD_SEGMENT_CODE:
		case PAYLOAD_SEGMENT_DATA:
			printf("CODE/DATA: dst=%p dst_len=%d src=%p "
				"src_len=%d compression=%d\n", dst, dst_len,
				src, src_len, be32toh(seg->compression));
			if (be32toh(seg->compression) ==
						CBFS_COMPRESS_NONE) {
				if (dst_len < src_len) {
					printf("Output buffer too small.\n");
					return;
				}
				memcpy(dst, src, src_len);
			} else if (be32toh(seg->compression) ==
						CBFS_COMPRESS_LZMA) {
				if (!ulzman(src, src_len, dst, dst_len)) {
					printf("LZMA: Decompression failed.\n");
					return;
				}
			} else {
				printf("Compression type %x not supported\n",
					be32toh(seg->compression));
				return;
			}
			break;
		case PAYLOAD_SEGMENT_BSS:
			printf("BSS: dst=%p len=%d\n", dst, dst_len);
			memset(dst, 0, dst_len);
			break;
		case PAYLOAD_SEGMENT_PARAMS:
			printf("PARAMS: skipped\n");
			break;
		case PAYLOAD_SEGMENT_ENTRY:
			run_cleanup_funcs(CleanupOnLegacy);
			cache_sync_instructions();
			((EntryFunc)dst)();
		default:
			printf("segment type %x not implemented. Exiting\n",
				seg->type);
			return;
		}
		seg++;
	}
}
