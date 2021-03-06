/*
 * Copyright 2016 Google Inc.
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

#ifndef __UEFI_UEFI_H__
#define __UEFI_UEFI_H__

#include "base/event.h"
#include "uefi/edk.h"

EFI_SYSTEM_TABLE *uefi_system_table_ptr(void);
int uefi_image_handle(EFI_HANDLE *handle);

int uefi_get_memory_map(unsigned *size, EFI_MEMORY_DESCRIPTOR **map,
			unsigned *desc_size, uint32_t *desc_version);

int uefi_exit_boot_services(void);

void uefi_add_exit_boot_services_event(DcEvent *event);
void uefi_remove_exit_boot_services_event(DcEvent *event);

#endif /* __UEFI_UEFI_H__ */
