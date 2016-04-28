
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

#include <stdio.h>
#include <sysinfo.h>
#include <vboot_struct.h>

#include "base/init_funcs.h"
#include "base/timestamp.h"
#include "drivers/keyboard/keyboard.h"
#include "module/module.h"
#include "vboot/stages.h"
#include "vboot/util/commonparams.h"
#include "vboot/util/flag.h"
#include "vboot/util/vboot_handoff.h"

static int vboot_init_handoff()
{
	struct vboot_handoff *vboot_handoff;

	if (lib_sysinfo.vboot_handoff == NULL) {
		printf("vboot handoff pointer is NULL\n");
		return 1;
	}

	if (lib_sysinfo.vboot_handoff_size != sizeof(struct vboot_handoff)) {
		printf("Unexpected vboot handoff size: %d\n",
		       lib_sysinfo.vboot_handoff_size);
		return 1;
	}

	vboot_handoff = lib_sysinfo.vboot_handoff;

	/* If the lid is closed, don't count down the boot
	 * tries for updates, since the OS will shut down
	 * before it can register success.
	 *
	 * VbInit was already called in coreboot, so we need
	 * to update the vboot internal flags ourself.
	 */
	int lid_switch = flag_fetch(FLAG_LIDSW);
	if (!lid_switch) {
		if (common_params_init())
			return 1;

		VbSharedDataHeader *vdat = cparams.shared_data_blob;

		/* Tell kernel selection to not count down */
		vdat->flags |= VBSD_NOFAIL_BOOT;
	}

	return vboot_do_init_out_flags(vboot_handoff->init_params.out_flags);
}

void module_main(void)
{
	timestamp_add_now(TS_RO_VB_INIT);

	// Set up the common param structure, not clearing shared data.
	if (vboot_init_handoff())
		halt();

	timestamp_add_now(TS_VB_SELECT_AND_LOAD_KERNEL);

	// Select a kernel and boot it.
	if (vboot_select_and_load_kernel())
		halt();
}
