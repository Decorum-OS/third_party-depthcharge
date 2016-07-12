/*
 * Copyright (C) 2011 Samsung Electronics
 * Copyright 2016 Google Inc. All rights reserved.
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
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
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
#include <stdio.h>
#include <sysinfo.h>

#include "base/container_of.h"
#include "base/time.h"
#include "base/xalloc.h"
#include "drivers/bus/spi/spi.h"
#include "drivers/flash/spi.h"

typedef enum {
	ReadCommand = 3,
	ReadSr1Command = 5,
	WriteStatus = 1,
	WriteCommand = 2,
	WriteEnableCommand = 6,
	ReadId = 0x9f
} SpiFlashCommands;



static int spi_flash_start(SpiFlash *flash)
{
	while (time_us(flash->last_command_time) < 10)
	{;}
	return spi_start(flash->spi);
}

static int spi_flash_stop(SpiFlash *flash)
{
	flash->last_command_time = time_us(0);
	return spi_stop(flash->spi);
}



static int spi_flash_command(SpiFlash *flash, void *cmd, uint32_t cmd_size)
{
	if (spi_flash_start(flash))
		return 1;

	if (spi_transfer(flash->spi, NULL, cmd, cmd_size)) {
		spi_flash_stop(flash);
		return 1;
	}

	return spi_flash_stop(flash);
}

static int spi_flash_read_command(SpiFlash *flash,
				  void *cmd, uint32_t cmd_size,
				  void *buf, uint32_t size)
{
	if (spi_flash_start(flash))
		return 1;

	if (spi_transfer(flash->spi, NULL, cmd, cmd_size) ||
	    spi_transfer(flash->spi, buf, NULL, size)) {
		spi_flash_stop(flash);
		return 1;
	}

	return spi_flash_stop(flash);
}

static int spi_flash_write_command(SpiFlash *flash,
				   void *cmd, uint32_t cmd_size,
				   const void *buf, uint32_t size)
{
	if (spi_flash_start(flash))
		return 1;

	if (spi_transfer(flash->spi, NULL, cmd, cmd_size) ||
	    spi_transfer(flash->spi, NULL, buf, size)) {
		spi_flash_stop(flash);
		return 1;
	}

	return spi_flash_stop(flash);
}



static int wait_for_wip(SpiFlash *flash)
{
	const uint32_t FlashStatusWip = 1 << 0;

	uint64_t start_time = time_us(0);
	do {
		uint8_t cmd = ReadSr1Command;
		uint8_t status;
		if (spi_flash_read_command(flash, &cmd, sizeof(cmd),
					   &status, sizeof(status)))
			return 1;

		if (!(status & FlashStatusWip))
			return 0;

		if (time_us(start_time) > 2 * 1000 * 1000) {
			printf("Timeout waiting for WIP to clear.\n");
			return 1;
		}

		udelay(10);
	} while (1);
}



static void *spi_flash_read(FlashOps *me, uint32_t offset, uint32_t size)
{
	SpiFlash *flash = container_of(me, SpiFlash, ops);
	assert(offset + size <= flash->rom_size);

	uint32_t cmd = htobe32((ReadCommand << 24) | offset);
	uint8_t *data = flash->cache + offset;
	if (spi_flash_read_command(flash, &cmd, sizeof(cmd), data, size))
		return NULL;

	return data;
}

static int spi_flash_write(FlashOps *me, const void *buffer,
				uint32_t offset, uint32_t size)
{
	SpiFlash *flash = container_of(me, SpiFlash, ops);
	assert(offset + size <= flash->rom_size);
	const uint32_t PageSize = 256;
	const uint32_t OffsetMask = PageSize - 1;

	const uint8_t *buf8 = buffer;

	while (size) {
		const uint32_t write_size = PageSize - (size & ~OffsetMask);

		uint8_t wen_cmd = WriteEnableCommand;
		uint32_t command = htobe32((WriteCommand << 24) | offset);

		if (spi_flash_command(flash, &wen_cmd, sizeof(wen_cmd)) ||
		    spi_flash_write_command(flash, &command, sizeof(command),
					    buf8, write_size) ||
		    wait_for_wip(flash)) {
			return 1;
		}

		buf8 += write_size;
		offset += write_size;
		size -= write_size;
	}

	return 0;
}

static int spi_flash_erase_size(FlashOps *me)
{
	SpiFlash *flash = container_of(me, SpiFlash, ops);
	return flash->sector_size;
}

static int spi_flash_erase(FlashOps *me, uint32_t start, uint32_t size)
{
	SpiFlash *flash = container_of(me, SpiFlash, ops);
	assert(start + size <= flash->rom_size);
	const uint32_t sector_size = flash->sector_size;

	if ((start % sector_size) || (size % sector_size)) {
		printf("Erase not %d aligned, start = %d, size = %d\n",
		       sector_size, start, size);
		return 1;
	}

	for (int offset = 0; offset < size; offset += sector_size) {
		uint8_t wen_cmd = WriteEnableCommand;
		uint32_t command = htobe32((flash->erase_cmd << 24) | offset);

		if (spi_flash_command(flash, &wen_cmd, sizeof(wen_cmd)) ||
		    spi_flash_command(flash, &command, sizeof(command)) ||
		    wait_for_wip(flash)) {
			return 1;
		}
	}

	return 0;
}

static int spi_flash_size(FlashOps *me)
{
	SpiFlash *flash = container_of(me, SpiFlash, ops);
	return flash->rom_size;
}

SpiFlash *new_spi_flash(SpiOps *spi)
{
	uint32_t rom_size = lib_sysinfo.spi_flash.size;
	uint32_t sector_size = lib_sysinfo.spi_flash.sector_size;
	uint8_t erase_cmd = lib_sysinfo.spi_flash.erase_cmd;

	SpiFlash *flash = xzalloc(sizeof(*flash));
	flash->ops.read = &spi_flash_read;
	flash->ops.write = &spi_flash_write;
	flash->ops.erase_size = &spi_flash_erase_size;
	flash->ops.erase = &spi_flash_erase;
	flash->ops.size = &spi_flash_size;
	flash->spi = spi;
	// Provide sufficient alignment on the cache buffer so that the
	// underlying SPI controllers can perform optimal DMA transfers.
	flash->cache = xmemalign(1 * KiB, rom_size);

	flash->erase_cmd = erase_cmd;
	flash->sector_size = sector_size;
	assert(rom_size == ALIGN_DOWN(rom_size, sector_size));
	flash->rom_size = rom_size;
	return flash;
}
