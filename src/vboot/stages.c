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
#include <libpayload.h>
#include <stdint.h>
#include <vboot_api.h>
#include <vboot_nvstorage.h>

#include "base/power.h"
#include "base/timestamp.h"
#include "boot/commandline.h"
#include "drivers/flash/flash.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/storage/blockdev.h"
#include "image/fmap.h"
#include "image/index.h"
#include "module/module.h"
#include "module/symbols.h"
#include "vboot/boot.h"
#include "vboot/stages.h"
#include "vboot/crossystem/crossystem.h"
#include "vboot/util/commonparams.h"
#include "vboot/util/ec.h"
#include "vboot/util/flag.h"
#include "vboot/util/memory.h"
#include "vboot/vbnv.h"

enum {
	CmdLineSize = 4096,
	CrosParamSize = 4096,
};

int vboot_init(void)
{
	VbInitParams iparams = {
		.flags = 0
	};

	int dev_switch = flag_fetch(FLAG_DEVSW);
	int rec_switch = flag_fetch(FLAG_RECSW);
	int wp_switch = flag_fetch(FLAG_WPSW);
	int lid_switch = flag_fetch(FLAG_LIDSW);
	int oprom_loaded = 0;
	if (CONFIG_OPROM_MATTERS)
		oprom_loaded = flag_fetch(FLAG_OPROM);


	printf("%s:%d dev %d, rec %d, wp %d, lid %d, oprom %d\n",
	       __func__, __LINE__, dev_switch, rec_switch,
	       wp_switch, lid_switch, oprom_loaded);

	if (dev_switch < 0 || rec_switch < 0 || lid_switch < 0 ||
	    wp_switch < 0 || oprom_loaded < 0) {
		// An error message should have already been printed.
		return 1;
	}

	// Decide what flags to pass into VbInit.
	iparams.flags |= VB_INIT_FLAG_RO_NORMAL_SUPPORT;
	/* Don't fail the boot process when lid switch is closed.
	 * The OS might not have enough time to log success before
	 * shutting down.
	 */
	if (!lid_switch)
		iparams.flags |= VB_INIT_FLAG_NOFAIL_BOOT;
	if (dev_switch)
		iparams.flags |= VB_INIT_FLAG_DEV_SWITCH_ON;
	if (rec_switch)
		iparams.flags |= VB_INIT_FLAG_REC_BUTTON_PRESSED;
	if (wp_switch)
		iparams.flags |= VB_INIT_FLAG_WP_ENABLED;
	if (oprom_loaded)
		iparams.flags |= VB_INIT_FLAG_OPROM_LOADED;
	if (CONFIG_OPROM_MATTERS)
		iparams.flags |= VB_INIT_FLAG_OPROM_MATTERS;
	if (CONFIG_VIRTUAL_DEV_SWITCH)
		iparams.flags |= VB_INIT_FLAG_VIRTUAL_DEV_SWITCH;
	if (CONFIG_EC_SOFTWARE_SYNC)
		iparams.flags |= VB_INIT_FLAG_EC_SOFTWARE_SYNC;
        if (!CONFIG_PHYSICAL_REC_SWITCH)
                iparams.flags |= VB_INIT_FLAG_VIRTUAL_REC_SWITCH;

	if (common_params_init())
		return 1;

	printf("Calling VbInit().\n");
	VbError_t res = VbInit(&cparams, &iparams);
	if (res != VBERROR_SUCCESS) {
		printf("VbInit returned %d, Doing a cold reboot.\n", res);
		if (cold_reboot())
			return 1;
	}

	return vboot_do_init_out_flags(iparams.out_flags);
};

int vboot_do_init_out_flags(uint32_t out_flags)
{
	if (out_flags & VB_INIT_OUT_CLEAR_RAM) {
		if (memory_wipe_unused())
			return 1;
	}
	/*
	 * If in developer mode or recovery mode, assume we're going to need
	 * input. We'll want it up and responsive by the time we present
	 * prompts to the user, so get it going ahead of time.
	 */
	if (out_flags & (VB_INIT_OUT_ENABLE_DEVELOPER |
			 VB_INIT_OUT_ENABLE_RECOVERY))
		keyboard_prepare();

	return 0;
}

int vboot_select_firmware(void)
{
	VbSelectFirmwareParams fparams = {
		.verification_block_A = NULL,
		.verification_block_B = NULL,
		.verification_size_A = 0,
		.verification_size_B = 0
	};

	// Set up the fparams structure.
	FmapArea vblock_a, vblock_b;
	if (fmap_find_area("VBLOCK_A", &vblock_a) ||
	    fmap_find_area("VBLOCK_B", &vblock_b)) {
		printf("Couldn't find one of the vblocks.\n");
		return 1;
	}
	fparams.verification_block_A =
		flash_read(vblock_a.offset, vblock_a.size);
	fparams.verification_size_A = vblock_a.size;
	fparams.verification_block_B =
		flash_read(vblock_b.offset, vblock_b.size);
	fparams.verification_size_B = vblock_b.size;

	if (common_params_init())
		return 1;

	printf("Calling VbSelectFirmware().\n");
	VbError_t res = VbSelectFirmware(&cparams, &fparams);
	if (res != VBERROR_SUCCESS) {
		printf("VbSelectFirmware returned %d, "
		       "Doing a cold reboot.\n", res);
		if (cold_reboot())
			return 1;
	}

	enum VbSelectFirmware_t select = fparams.selected_firmware;

	// If an RW firmware was selected, start it.
	if (select == VB_SELECT_FIRMWARE_A || select == VB_SELECT_FIRMWARE_B) {
		const char *name;
		if (select == VB_SELECT_FIRMWARE_A)
			name = "FW_MAIN_A";
		else
			name = "FW_MAIN_B";

		FmapArea rw_area;
		if (fmap_find_area(name, &rw_area)) {
			printf("Didn't find section %s in the fmap.\n", name);
			return 1;
		}

		uint32_t image_size;
		const void *image = index_subsection(&rw_area, 0, &image_size);
		if (!image)
			return 1;

		if (start_module(image, image_size))
			return 1;
	}

	return 0;
}

int vboot_select_and_load_kernel(void)
{
	static char cmd_line_buf[2 * CmdLineSize];

	VbSelectAndLoadKernelParams kparams = {
		.kernel_buffer = &_kernel_start,
		.kernel_buffer_size = &_kernel_end - &_kernel_start
	};

	if (common_params_init())
		return 1;

	printf("Calling VbSelectAndLoadKernel().\n");
	VbError_t res = VbSelectAndLoadKernel(&cparams, &kparams);
	if (res == VBERROR_EC_REBOOT_TO_RO_REQUIRED) {
		printf("Rebooting the EC to RO.\n");
		reboot_ec_to_ro();
		if (power_off())
			return 1;
	} else if (res == VBERROR_SHUTDOWN_REQUESTED) {
		printf("Powering off.\n");
		if (power_off())
			return 1;
	}
	if (res != VBERROR_SUCCESS) {
		printf("VbSelectAndLoadKernel returned %d, "
		       "Doing a cold reboot.\n", res);
		if (cold_reboot())
			return 1;
	}

	timestamp_add_now(TS_CROSSYSTEM_DATA);

	// The scripts that packaged the kernel assumed it was going to end
	// up at 1MB which is frequently not right. The address of the 
	// "loader", which isn't actually used any more, is set based on that
	// assumption. We have to subtract the 1MB offset from it, and then add
	// the actual load address to figure ou thwere it actually is,
	// or would be if it existed.
	void *kernel = kparams.kernel_buffer;
	void *loader = (uint8_t *)kernel +
		(kparams.bootloader_address - 0x100000);
	void *params = (uint8_t *)loader - CrosParamSize;
	void *orig_cmd_line = (uint8_t *)params - CmdLineSize;

	BlockDev *bdev = (BlockDev *)kparams.disk_handle;

	if (commandline_subst(orig_cmd_line, cmd_line_buf,
			      sizeof(cmd_line_buf), 0,
			      kparams.partition_number + 1,
			      kparams.partition_guid, bdev->external_gpt))
		return 1;

	if (crossystem_setup())
		return 1;

	boot(kernel, cmd_line_buf, params, loader);

	return 1;
}
