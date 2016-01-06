/*
 *  linux/include/linux/mtd/nand.h
 *
 *  Copyright © 2000-2010 David Woodhouse <dwmw2@infradead.org>
 *                        Steven J. Hill <sjhill@realitydiluted.com>
 *		          Thomas Gleixner <tglx@linutronix.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Info:
 *	Contains standard defines and IDs for NAND flash devices
 *
 * Changelog:
 *	See git changelog.
 */
#ifndef __DRIVERS_STORAGE_MTD_NAND_NAND_H__
#define __DRIVERS_STORAGE_MTD_NAND_NAND_H__

#include "drivers/storage/mtd/mtd.h"

/*
 * Standard NAND flash commands
 */
#define NAND_CMD_READ0		0
#define NAND_CMD_READ1		1
#define NAND_CMD_RNDOUT		5
#define NAND_CMD_PAGEPROG	0x10
#define NAND_CMD_READOOB	0x50
#define NAND_CMD_ERASE1		0x60
#define NAND_CMD_STATUS		0x70
#define NAND_CMD_STATUS_MULTI	0x71
#define NAND_CMD_SEQIN		0x80
#define NAND_CMD_RNDIN		0x85
#define NAND_CMD_READID		0x90
#define NAND_CMD_ERASE2		0xd0
#define NAND_CMD_PARAM		0xec
#define NAND_CMD_RESET		0xff

#define NAND_CMD_LOCK		0x2a
#define NAND_CMD_LOCK_TIGHT	0x2c
#define NAND_CMD_UNLOCK1	0x23
#define NAND_CMD_UNLOCK2	0x24
#define NAND_CMD_LOCK_STATUS	0x7a

/* Extended commands for large page devices */
#define NAND_CMD_READSTART	0x30
#define NAND_CMD_RNDOUTSTART	0xE0
#define NAND_CMD_CACHEDPROG	0x15

#define NAND_CMD_NONE		-1

/* Status bits */
#define NAND_STATUS_FAIL	0x01
#define NAND_STATUS_FAIL_N1	0x02
#define NAND_STATUS_TRUE_READY	0x20
#define NAND_STATUS_READY	0x40
#define NAND_STATUS_WP		0x80

struct nand_onfi_params {
	/* rev info and features block */
	/* 'O' 'N' 'F' 'I'  */
	uint8_t sig[4];
	uint16_t revision;
	uint16_t features;
	uint16_t opt_cmd;
	uint8_t reserved[22];

	/* manufacturer information block */
	char manufacturer[12];
	char model[20];
	uint8_t jedec_id;
	uint16_t date_code;
	uint8_t reserved2[13];

	/* memory organization block */
	uint32_t byte_per_page;
	uint16_t spare_bytes_per_page;
	uint32_t data_bytes_per_ppage;
	uint16_t spare_bytes_per_ppage;
	uint32_t pages_per_block;
	uint32_t blocks_per_lun;
	uint8_t lun_count;
	uint8_t addr_cycles;
	uint8_t bits_per_cell;
	uint16_t bb_per_lun;
	uint16_t block_endurance;
	uint8_t guaranteed_good_blocks;
	uint16_t guaranteed_block_endurance;
	uint8_t programs_per_page;
	uint8_t ppage_attr;
	uint8_t ecc_bits;
	uint8_t interleaved_bits;
	uint8_t interleaved_ops;
	uint8_t reserved3[13];

	/* electrical parameter block */
	uint8_t io_pin_capacitance_max;
	uint16_t async_timing_mode;
	uint16_t program_cache_timing_mode;
	uint16_t t_prog;
	uint16_t t_bers;
	uint16_t t_r;
	uint16_t t_ccs;
	uint16_t src_sync_timing_mode;
	uint16_t src_ssync_features;
	uint16_t clk_pin_capacitance_typ;
	uint16_t io_pin_capacitance_typ;
	uint16_t input_pin_capacitance_typ;
	uint8_t input_pin_capacitance_max;
	uint8_t driver_strenght_support;
	uint16_t t_int_r;
	uint16_t t_ald;
	uint8_t reserved4[7];

	/* vendor */
	uint8_t reserved5[90];

	uint16_t crc;
} __attribute__((packed));

#define ONFI_CRC_BASE	0x4F4E

/**
 * struct nand_flash_dev - NAND Flash Device ID Structure
 * Only newer NAND with the pagesize and erasesize implicit
 * in the ID are allowed.
 * @id:		least significant nibble of device ID code
 * @chipsize:	Total chipsize in Mega Bytes
 */
struct nand_flash_dev {
	int id;
	int chipsize;
};

extern const struct nand_flash_dev nand_flash_ids[];

#endif /* __DRIVERS_STORAGE_MTD_NAND_NAND_H__ */
