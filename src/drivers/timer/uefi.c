/*
 * Copyright 2016 Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <assert.h>
#include <stdint.h>

#include "base/die.h"
#include "base/xalloc.h"
#include "drivers/timer/timer.h"
#include "uefi/edk/Protocol/Timestamp.h"
#include "uefi/uefi.h"

static EFI_GUID uefi_timer_timestamp_guid = EFI_TIMESTAMP_PROTOCOL_GUID;

static void uefi_timer_get_prot(EFI_HANDLE handle,
				EFI_TIMESTAMP_PROTOCOL **prot)
{
	EFI_SYSTEM_TABLE *system_table = uefi_system_table_ptr();
	assert(system_table);
	EFI_BOOT_SERVICES *boot_services = system_table->BootServices;

	EFI_STATUS status = boot_services->HandleProtocol(
		handle, &uefi_timer_timestamp_guid, (void **)prot);
	die_if(status != EFI_SUCCESS, "Failed to get timestamp protocol.\n");
}

static void uefi_timer_get_props(EFI_TIMESTAMP_PROTOCOL *prot,
				 EFI_TIMESTAMP_PROPERTIES *props)
{
	EFI_STATUS status = prot->GetProperties(props);
	die_if(status != EFI_SUCCESS, "Failed to get timestamp properties.\n");
}

static uint64_t uefi_timer_calc_rollover(EFI_TIMESTAMP_PROTOCOL *prot)
{
	EFI_TIMESTAMP_PROPERTIES properties;
	uefi_timer_get_props(prot, &properties);
	return properties.EndValue / properties.Frequency;
}

static EFI_TIMESTAMP_PROTOCOL *timer_uefi_timestamp_protocol(void)
{
	static EFI_TIMESTAMP_PROTOCOL *timestamp_prot;

	if (timestamp_prot)
		return timestamp_prot;

	EFI_SYSTEM_TABLE *system_table = uefi_system_table_ptr();
	assert(system_table);
	EFI_BOOT_SERVICES *boot_services = system_table->BootServices;

	UINTN buf_size = 0;
	EFI_HANDLE dummy_handle;
	EFI_STATUS status = boot_services->LocateHandle(
		ByProtocol, &uefi_timer_timestamp_guid, NULL,
		&buf_size, &dummy_handle);
	die_if(status == EFI_NOT_FOUND,
	       "No timestamp protocol handles found.\n");
	die_if(status != EFI_BUFFER_TOO_SMALL,
	       "Error retrieving timestamp protocol handles.\n");

	EFI_HANDLE *handles = xmalloc(buf_size);

	status = boot_services->LocateHandle(
		ByProtocol, &uefi_timer_timestamp_guid, NULL,
		&buf_size, handles);
	die_if(status != EFI_SUCCESS,
	       "Failed to retrieve timestamp protocol handles.\n");

	int handle_count = buf_size / sizeof(dummy_handle);
	EFI_TIMESTAMP_PROTOCOL *best_prot;
	uefi_timer_get_prot(handles[0], &best_prot);
	uint64_t best_roll_over = uefi_timer_calc_rollover(best_prot);

	if (handle_count > 1) {
		printf("Multiple timestamp sources found.\n"
		       "Picking the one that will take the "
		       "longest to rollover.\n");
	}

	for (int i = 1; i < handle_count; i++) {
		EFI_TIMESTAMP_PROTOCOL *prot;
		uefi_timer_get_prot(handles[i], &prot);
		uint64_t roll_over = uefi_timer_calc_rollover(prot);
		if (roll_over > best_roll_over) {
			best_roll_over = roll_over;
			best_prot = prot;
		}
	}

	free(handles);

	timestamp_prot = best_prot;
	return timestamp_prot;
}

uint64_t timer_hz(void)
{
	static uint64_t frequency;

	if (!frequency) {
		EFI_TIMESTAMP_PROPERTIES properties;
		EFI_TIMESTAMP_PROTOCOL *prot = timer_uefi_timestamp_protocol();
		uefi_timer_get_props(prot, &properties);
		frequency = properties.Frequency;
	}

	return frequency;
}

uint64_t timer_raw_value(void)
{
	EFI_TIMESTAMP_PROTOCOL *prot = timer_uefi_timestamp_protocol();
	return prot->GetTimestamp();
}
