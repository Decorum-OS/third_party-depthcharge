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
#include <gbb_header.h>
#include <stdint.h>
#include <stdio.h>
#include <sysinfo.h>
#include <vboot_api.h>
#include <vboot_struct.h>

#include "base/algorithm.h"
#include "board/board.h"
#include "vboot/crossystem/crossystem.h"
#include "vboot/firmware_id.h"
#include "vboot/util/acpi.h"
#include "vboot/util/commonparams.h"
#include "vboot/util/gbb.h"
#include "vboot/util/vboot_handoff.h"

static void install_id(void *dest, int dest_sz, const void *src, int src_sz)
{
	if (src_sz)
		memcpy(dest, src, MIN(dest_sz, src_sz));
}

int crossystem_setup(void)
{
	if (common_params_init())
		return 1;

	chromeos_acpi_t *acpi_table = (chromeos_acpi_t *)lib_sysinfo.vdat_addr;
	VbSharedDataHeader *vdat = (VbSharedDataHeader *)&acpi_table->vdat;

	memcpy(vdat, cparams.shared_data_blob, cparams.shared_data_size);

	if (vdat->magic != VB_SHARED_DATA_MAGIC) {
		printf("Bad magic value in vboot shared data header.\n");
		return 1;
	}

	acpi_table->boot_reason = BOOT_REASON_OTHER;

	int fw_index = vdat->firmware_index;
	switch (fw_index) {
	case VDAT_RW_A:
		acpi_table->main_fw = BINF_RW_A;
		break;
	case VDAT_RW_B:
		acpi_table->main_fw = BINF_RW_B;
		break;
	case VDAT_RECOVERY:
		acpi_table->main_fw = BINF_RECOVERY;
		break;
	default:
		printf("Unrecognized firmware index %d.\n", fw_index);
		return 1;
	}

	if (fw_index == VDAT_RECOVERY)
		acpi_table->main_fw_type = FIRMWARE_TYPE_RECOVERY;
	else if (vdat->flags & VBSD_BOOT_DEV_SWITCH_ON)
		acpi_table->main_fw_type = FIRMWARE_TYPE_DEVELOPER;
	else
		acpi_table->main_fw_type = FIRMWARE_TYPE_NORMAL;

	// Use the value set by coreboot if we don't want to change it.
	if (CONFIG_EC_SOFTWARE_SYNC) {
		int in_rw = 0;

		if (VbExEcRunningRW(0, &in_rw)) {
			printf("Couldn't tell if the EC firmware is RW.\n");
			return 1;
		}
		acpi_table->ec_fw = in_rw ? ACTIVE_ECFW_RW : ACTIVE_ECFW_RO;
	}

	uint16_t chsw = 0;
	if (board_flag_write_protect())
		chsw |= CHSW_FIRMWARE_WP_DIS;
	if (board_flag_recovery())
		chsw |= CHSW_RECOVERY_X86;
	if (board_flag_developer_mode())
		chsw |= CHSW_DEVELOPER_SWITCH;
	acpi_table->chsw = chsw;

	size_t size;
	const char *id = gbb_read_hwid(&size);
	if (!id)
		return 1;
	install_id(acpi_table->hwid, sizeof(acpi_table->hwid), id, size);

	id = firmware_id_for(fw_index, &size);
	if (!id)
		return 1;

	size_t smbios_size = MIN(size, strnlen(id, ACPI_FWID_SIZE));
	uint8_t *dest = (uint8_t *)(uintptr_t)acpi_table->fwid_ptr;
	memcpy(dest, id, smbios_size);
	dest[smbios_size] = 0;

	install_id(acpi_table->fwid, sizeof(acpi_table->fwid), id, size);

	id = firmware_id_for(VDAT_RO, &size);
	if (!id)
		return 1;
	install_id(acpi_table->frid, sizeof(acpi_table->frid), id, size);

	acpi_table->recovery_reason = vdat->recovery_reason;

	acpi_table->fmap_base = (uint32_t)(-CONFIG_IMAGE_SIZE_KB * 1024) +
				CONFIG_FMAP_OFFSET;

	return 0;
}

