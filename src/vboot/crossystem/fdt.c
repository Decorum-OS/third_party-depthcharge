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
#include <endian.h>
#include <gbb_header.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vboot_api.h>
#include <vboot_struct.h>

#include "base/device_tree.h"
#include "base/xalloc.h"
#include "vboot/callbacks/nvstorage_flash.h"
#include "vboot/crossystem/crossystem.h"
#include "vboot/firmware_id.h"
#include "vboot/util/commonparams.h"
#include "vboot/util/gbb.h"

static int install_crossystem_data(DeviceTreeFixup *fixup, DeviceTree *tree)
{
	const char *path[] = { "firmware", "chromeos", NULL };
	DeviceTreeNode *node = dt_find_node(tree->root, path, NULL, NULL, 1);

	dt_add_string_prop(node, "compatible", "chromeos-firmware");

	if (common_params_init())
		return 1;
	dt_add_bin_prop(node, "vboot-shared-data", cparams.shared_data_blob,
			cparams.shared_data_size);
	VbSharedDataHeader *vdat = cparams.shared_data_blob;

	if (CONFIG_NV_STORAGE_CMOS) {
		dt_add_string_prop(node, "nonvolatile-context-storage","nvram");
	} else if (CONFIG_NV_STORAGE_CROS_EC) {
		dt_add_string_prop(node,
				"nonvolatile-context-storage", "cros-ec");
	} else if (CONFIG_NV_STORAGE_DISK) {
		dt_add_string_prop(node, "nonvolatile-context-storage", "disk");
		dt_add_u32_prop(node, "nonvolatile-context-lba",
				CONFIG_NV_STORAGE_DISK_LBA);
		dt_add_u32_prop(node, "nonvolatile-context-offset",
				CONFIG_NV_STORAGE_DISK_OFFSET);
		dt_add_u32_prop(node, "nonvolatile-context-size",
				CONFIG_NV_STORAGE_DISK_SIZE);
	} else if (CONFIG_NV_STORAGE_FLASH) {
		dt_add_string_prop(node, "nonvolatile-context-storage","flash");
		dt_add_u32_prop(node, "nonvolatile-context-offset",
				nvstorage_flash_get_offset());
		dt_add_u32_prop(node, "nonvolatile-context-size",
				VBNV_BLOCK_SIZE);
	}

	int fw_index = vdat->firmware_index;

	size_t fwid_size;
	const char *fwid = firmware_id_for(fw_index, &fwid_size);
	if (fwid == NULL) {
		printf("No firmware ID for %d.\n", fw_index);
		return 1;
	}
	char *fwid_copy = xmalloc(fwid_size);
	memcpy(fwid_copy, fwid, fwid_size);

	dt_add_bin_prop(node, "firmware-version", fwid_copy, fwid_size);

	if (fw_index == VDAT_RECOVERY)
		dt_add_string_prop(node, "firmware-type", "recovery");
	else if (vdat->flags & VBSD_BOOT_DEV_SWITCH_ON)
		dt_add_string_prop(node, "firmware-type", "developer");
	else
		dt_add_string_prop(node, "firmware-type", "normal");

	dt_add_u32_prop(node, "fmap-offset", CONFIG_FMAP_OFFSET);

	fwid_size = 0;
	fwid = firmware_id_for(VDAT_RO, &fwid_size);

	if (fwid_size) {
		fwid_copy = xmalloc(fwid_size);
		memcpy(fwid_copy, fwid, fwid_size);
		dt_add_bin_prop(node, "readonly-firmware-version",
				fwid_copy, fwid_size);
	}

	GoogleBinaryBlockHeader *gbb = cparams.gbb_data;
	if (memcmp(gbb->signature, GBB_SIGNATURE, GBB_SIGNATURE_SIZE)) {
		printf("Bad signature on GBB.\n");
		return 1;
	}
	size_t hwid_size;
	const char *hwid = gbb_read_hwid(&hwid_size);
	if (!hwid)
		return 1;
	void *hwid_copy = xmalloc(hwid_size);
	memcpy(hwid_copy, hwid, hwid_size);
	dt_add_bin_prop(node, "hardware-id", hwid_copy, hwid_size);

	if (CONFIG_EC_SOFTWARE_SYNC) {
		int in_rw = 0;

		if (VbExEcRunningRW(0, &in_rw)) {
			printf("Couldn't tell if the EC firmware is RW.\n");
			return 1;
		}
		dt_add_string_prop(node, "active-ec-firmware",
			    in_rw ? "RW" : "RO");
	}

	return 0;
}

static DeviceTreeFixup crossystem_fixup = {
	.fixup = &install_crossystem_data
};

int crossystem_setup(void)
{
	list_insert_after(&crossystem_fixup.list_node, &device_tree_fixups);
	return 0;
}
