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

#ifndef __MODULE_FSP_V1_1_FSP_INIT_H__
#define __MODULE_FSP_V1_1_FSP_INIT_H__

#include <stdint.h>

enum {
	FspV1_1BootWithFullConfiguration = 0x0,
	FspV1_1BootWithMinimalConfiguration = 0x1,
	FspV1_1BootAssumingNoConfigurationChanges = 0x2,
	FspV1_1BootWithFullConfigurationPlusDiagnostics = 0x3,
	FspV1_1BootWithDefaultSettings = 0x4,
	FspV1_1BootOnS4Resume = 0x5,
	FspV1_1BootOnS5Resume = 0x6,
	FspV1_1BootOnS2Resume = 0x10,
	FspV1_1BootOnS3Resume = 0x11,
	FspV1_1BootOnFlashUpdate = 0x12,
	FspV1_1BootInRecoveryMode = 0x20
};

typedef struct __attribute__((packed))
{
	uint16_t vendor_id;
	uint16_t device_id;
	uint8_t revision_id;
	uint8_t sdi_no;
	uint16_t data_dwords;
	uint32_t reserved;
} FspV1_1AzaliaHeader;

typedef struct __attribute__((packed))
{
	FspV1_1AzaliaHeader header;
	uint32_t *data;
} FspV1_1AudioAzaliaVerbTable;

typedef struct __attribute__((packed))
{
	uint64_t signature;
	uint64_t reserved_1;
	uint8_t unused_upd_space_0[16];
	uint8_t pcd_spd_base_address_0_0;
	uint8_t pcd_spd_base_address_0_1;
	uint8_t pcd_spd_base_address_1_0;
	uint8_t pcd_spd_base_address_1_1;
	uint8_t pcd_tseg_size;
	uint32_t unused_upd_space_1;
	uint8_t pcd_igd_dvmt_50_pre_alloc;
	uint8_t pcd_primary_display;
	uint8_t pcd_internal_gfx;
	uint8_t pcd_mmio_size;
	uint8_t unused_upd_space_2;
	uint8_t pcd_aperture_size;
	uint8_t reserved_upd_space_0[80];
	uint8_t unused_upd_space_3;
	uint8_t pcd_enable_lan;
	uint8_t pcd_enable_sata;
	uint8_t pcd_sata_mode;
	uint8_t pcd_enable_azalia;
	uint8_t pcd_enable_xhci;
	uint8_t pcd_enable_ehci_1;
	uint8_t pcd_enable_ehci_2;
	uint8_t pcd_enable_smbus;
	uint8_t pcd_enable_audio_dsp;
	uint64_t reserved_upd_space_1;
	uint16_t pcd_pm_base;
	uint16_t pcd_pch_pcie_root_port_enable;
	uint16_t pcd_pch_pcie_slot_implemented;
	uint8_t pcd_pch_pcie_root_port_function_swapping_enable;
	uint16_t pcd_gpio_base;
	uint16_t pcd_pch_sata_port_enable;
	uint16_t pcd_pch_sata_solid_state_drive;
	uint16_t pcd_pch_sata_interlock_sw;
	uint16_t pcd_pch_sata_spin_up;
	uint16_t pcd_pch_sata_hot_plug;
	uint8_t unused_upd_space_5[92];
	uint8_t pcd_fast_boot;
	uint8_t pcd_user_crb_board_type;
	uint8_t unused_upd_space_6[130];
	FspV1_1AudioAzaliaVerbTable *azalia_verb_table_ptr;
	uint64_t unused_upd_space7;
	uint8_t dq_dqs_data_effective;
	uint8_t unused_upd_space_8[11];
	uint8_t dq_byte_map[24];
	uint8_t unused_upd_space_9[76];
	uint8_t dqs_map_cpu_2_dram[16];
	uint32_t spd_data_buffer_0_0;
	uint32_t spd_data_buffer_0_1;
	uint32_t spd_data_buffer_1_0;
	uint32_t spd_data_buffer_1_1;
	uint8_t unused_upd_space_10[240];
	uint16_t pcd_region_terminator;
} FspV1_1UpdDataRegion;

typedef struct __attribute__((packed))
{
	uint64_t pcd_vpd_region_sign;
	uint32_t pcd_image_revision;
	uint32_t pcd_upd_region_offset;
	uint8_t unused_vpd_space_0[16];
	uint32_t pcd_fsp_reserved_memory_length;
	uint8_t pcd_port_80_route;
} FspV1_1VpdDataRegion;

typedef struct __attribute__((packed))
{
	uint32_t *stack_top;
	uint32_t boot_mode;
	FspV1_1UpdDataRegion *upd_data_rgn_ptr;
	uint32_t reserved[7];
} FspV1_1InitRtCommonBuffer;

typedef struct __attribute__((packed))
{
	FspV1_1InitRtCommonBuffer common;
} FspV1_1InitRtBuffer;

typedef void (*FspV1_1InitContinuation)(uint32_t status, void *hob_list_ptr);

typedef struct __attribute__((packed))
{
	void *nvs_buffer_ptr;
	FspV1_1InitRtBuffer *rt_buffer_ptr;
	FspV1_1InitContinuation continuation_func;
} FspV1_1InitParams;

typedef uint32_t (*FspV1_1InitFunc)(FspV1_1InitParams *fsp_init_param_ptr);

#endif /* __MODULE_FSP_V1_1_FSP_INIT_H__ */
