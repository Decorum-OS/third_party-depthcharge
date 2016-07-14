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
#include <stdio.h>
#include <vboot_api.h>

#include "base/algorithm.h"
#include "base/time.h"
#include "base/timestamp.h"
#include "base/xalloc.h"
#include "board/board.h"
#include "drivers/ec/cros/ec.h"

int VbExTrustEC(int devidx)
{
	int val;

	if (devidx != 0)
		return 0;

	val = board_flag_ec_in_rw();
	if (val < 0) {
		printf("Couldn't tell if the EC is running RW firmware.\n");
		return 0;
	}
	// Trust the EC if it's NOT in its RW firmware.
	return !val;
}

VbError_t VbExEcRunningRW(int devidx, int *in_rw)
{
	enum ec_current_image image;

	if (cros_ec_read_current_image(devidx, &image) < 0) {
		printf("Failed to read current EC image.\n");
		return VBERROR_UNKNOWN;
	}
	switch (image) {
	case EC_IMAGE_RO:
		*in_rw = 0;
		break;
	case EC_IMAGE_RW:
		*in_rw = 1;
		break;
	default:
		printf("Unrecognized EC image type %d.\n", image);
		return VBERROR_UNKNOWN;
	}

	return VBERROR_SUCCESS;
}

VbError_t VbExEcJumpToRW(int devidx)
{
	if (cros_ec_reboot(devidx, EC_REBOOT_JUMP_RW, 0) < 0) {
		printf("Failed to make the EC jump to RW.\n");
		return VBERROR_UNKNOWN;
	}

	return VBERROR_SUCCESS;
}

VbError_t VbExEcDisableJump(int devidx)
{
	if (cros_ec_reboot(devidx, EC_REBOOT_DISABLE_JUMP, 0) < 0) {
		printf("Failed to make the EC disable jumping.\n");
		return VBERROR_UNKNOWN;
	}

	return VBERROR_SUCCESS;
}

VbError_t VbExEcHashRW(int devidx, const uint8_t **hash, int *hash_size)
{
	static struct ec_response_vboot_hash resp;

	if (cros_ec_read_hash(devidx, &resp) < 0) {
		printf("Failed to read EC hash.\n");
		return VBERROR_UNKNOWN;
	}

	/*
	 * TODO (rspangler@chromium.org): the code below isn't very tolerant
	 * of errors.
	 *
	 * If the EC is busy calculating a hash, we should wait and retry
	 * reading the hash status.
	 *
	 * If the hash is unavailable, the wrong type, or covers the wrong
	 * offset/size (which we need to get from the FDT, since it's
	 * board-specific), we should request a new hash and wait for it to
	 * finish.  Also need a flag to force it to rehash, which we'll use
	 * after doing a firmware update.
	 */
	if (resp.status != EC_VBOOT_HASH_STATUS_DONE) {
		printf("EC hash wasn't finished.\n");
		return VBERROR_UNKNOWN;
	}
	if (resp.hash_type != EC_VBOOT_HASH_TYPE_SHA256) {
		printf("EC hash was the wrong type.\n");
		return VBERROR_UNKNOWN;
	}

	*hash = resp.hash_digest;
	*hash_size = resp.digest_size;

	return VBERROR_SUCCESS;
}

VbError_t VbExEcGetExpectedRW(int devidx, enum VbSelectFirmware_t select,
			      const uint8_t **image, int *image_size)
{
	typedef struct {
		const uint8_t *image;
		int size;
	} Ec;
	typedef Ec EcCache[CONFIG_MAX_EC_DEV_IDX + 1];
	static EcCache cache_a, cache_b;

	if (devidx > ARRAY_SIZE(cache_a)) {
		printf("EC devidx %d is greater than the max of %d.\n",
		       devidx, CONFIG_MAX_EC_DEV_IDX);
		return VBERROR_UNKNOWN;
	}

	StorageOps *ec;
	EcCache *cache;
	if (select == VB_SELECT_FIRMWARE_A) {
		ec = board_storage_ec_a(devidx);
		cache = &cache_a;
	} else if (select == VB_SELECT_FIRMWARE_B) {
		ec = board_storage_ec_b(devidx);
		cache = &cache_b;
	}

	if (!cache) {
		printf("Unrecognized EC has select value %d.\n", select);
		return VBERROR_UNKNOWN;
	}

	Ec *vals = &(*cache)[devidx];

	if (!vals->image) {
		int size = storage_size(ec);
		if (size < 0)
			return VBERROR_UNKNOWN;

		void *data = xmalloc(size);
		if (storage_read(ec, data, 0, size)) {
			free(data);
			return VBERROR_UNKNOWN;
		}

		vals->size = size;
		vals->image = data;
	}

	*image = vals->image;
	*image_size = vals->size;

	return VBERROR_SUCCESS;
}

static VbError_t ec_protect_rw(int devidx, int protect)
{
	struct ec_response_flash_protect resp;
	uint32_t mask = EC_FLASH_PROTECT_ALL_NOW | EC_FLASH_PROTECT_ALL_AT_BOOT;

	/* Update protection */
	if (cros_ec_flash_protect(devidx, mask,
				  protect ? mask : 0, &resp) < 0) {
		printf("Failed to update EC flash protection.\n");
		return VBERROR_UNKNOWN;
	}

	if (!protect) {
		/* If protection is still enabled, need reboot */
		if (resp.flags & EC_FLASH_PROTECT_ALL_NOW)
			return VBERROR_EC_REBOOT_TO_RO_REQUIRED;

		return VBERROR_SUCCESS;
	}

	/*
	 * If write protect and ro-at-boot aren't both asserted, don't expect
	 * protection enabled.
	 */
	if ((~resp.flags) & (EC_FLASH_PROTECT_GPIO_ASSERTED |
			     EC_FLASH_PROTECT_RO_AT_BOOT))
		return VBERROR_SUCCESS;

	/* If flash is protected now, success */
	if (resp.flags & EC_FLASH_PROTECT_ALL_NOW)
		return VBERROR_SUCCESS;

	/* If RW will be protected at boot but not now, need a reboot */
	if (resp.flags & EC_FLASH_PROTECT_ALL_AT_BOOT)
		return VBERROR_EC_REBOOT_TO_RO_REQUIRED;

	/* Otherwise, it's an error */
	return VBERROR_UNKNOWN;
}

VbError_t VbExEcGetExpectedRWHash(int devidx, enum VbSelectFirmware_t select,
				  const uint8_t **hash, int *hash_size)
{
	typedef struct {
		const uint8_t *hash;
		int size;
	} EcHash;
	typedef EcHash EcHashCache[CONFIG_MAX_EC_DEV_IDX + 1];
	static EcHashCache cache_a, cache_b;

	if (devidx > ARRAY_SIZE(cache_a)) {
		printf("EC devidx %d is greater than the max of %d.\n",
		       devidx, CONFIG_MAX_EC_DEV_IDX);
		return VBERROR_UNKNOWN;
	}

	StorageOps *ec_hash;
	EcHashCache *cache;
	if (select == VB_SELECT_FIRMWARE_A) {
		ec_hash = board_storage_ec_hash_a(devidx);
		cache = &cache_a;
	} else if (select == VB_SELECT_FIRMWARE_B) {
		ec_hash = board_storage_ec_hash_b(devidx);
		cache = &cache_b;
	}

	if (!cache) {
		printf("Unrecognized EC has select value %d.\n", select);
		return VBERROR_UNKNOWN;
	}

	EcHash *hash_vals = &(*cache)[devidx];

	if (!hash_vals->hash) {
		int size = storage_size(ec_hash);
		if (size < 0)
			return VBERROR_UNKNOWN;

		void *data = xmalloc(size);
		if (storage_read(ec_hash, data, 0, size)) {
			free(data);
			return VBERROR_UNKNOWN;
		}

		hash_vals->size = size;
		hash_vals->hash = data;
	}

	*hash = hash_vals->hash;
	*hash_size = hash_vals->size;

	printf("Hash = ");
	for (int i = 0; i < *hash_size; i++)
		printf("%02x", (*hash)[i]);
	printf("\n");

	return VBERROR_SUCCESS;
}

VbError_t VbExEcUpdateRW(int devidx, const uint8_t *image, int image_size)
{
	int rv;

	rv = ec_protect_rw(devidx, 0);
	if (rv == VBERROR_EC_REBOOT_TO_RO_REQUIRED || rv != VBERROR_SUCCESS)
		return rv;

	if (cros_ec_flash_update_rw(devidx, image, image_size)) {
		printf("Failed to update EC RW flash.\n");
		return VBERROR_UNKNOWN;
	}

	return VBERROR_SUCCESS;
}

VbError_t VbExEcProtectRW(int devidx)
{
	return ec_protect_rw(devidx, 1);
}

VbError_t VbExEcEnteringMode(int devidx, enum VbEcBootMode_t mode)
{
	switch(mode) {
	case VB_EC_RECOVERY:
		return cros_ec_entering_mode(devidx, VBOOT_MODE_RECOVERY);
	case VB_EC_DEVELOPER:
		return cros_ec_entering_mode(devidx, VBOOT_MODE_DEVELOPER);
	case VB_EC_NORMAL:
	default :
		return cros_ec_entering_mode(devidx, VBOOT_MODE_NORMAL);
	}
}

/* Wait 3 seconds after software sync for EC to clear the limit power flag. */
#define LIMIT_POWER_WAIT_TIMEOUT 3000
/* Check the limit power flag every 50 ms while waiting. */
#define LIMIT_POWER_POLL_SLEEP 50

VbError_t VbExEcVbootDone(int in_recovery)
{
	int limit_power;
	int limit_power_wait_time = 0;
	int message_printed = 0;

	/* Ensure we have enough power to continue booting */
	while(1) {
		if (cros_ec_read_limit_power_request(&limit_power)) {
			printf("Failed to check EC limit power flag.\n");
			return VBERROR_UNKNOWN;
		}

		/*
		 * Do not wait for the limit power flag to be cleared in
		 * recovery mode since we didn't just sysjump.
		 */
		if (!limit_power || in_recovery ||
		    limit_power_wait_time > LIMIT_POWER_WAIT_TIMEOUT)
			break;

		if (!message_printed) {
			printf("Waiting for EC to clear limit power flag.\n");
			message_printed = 1;
		}

		mdelay(LIMIT_POWER_POLL_SLEEP);
		limit_power_wait_time += LIMIT_POWER_POLL_SLEEP;
	}

	if (limit_power) {
		printf("EC requests limited power usage. Request shutdown.\n");
		return VBERROR_SHUTDOWN_REQUESTED;
	}

	timestamp_add_now(TS_VB_EC_VBOOT_DONE);
	return VBERROR_SUCCESS;
}
