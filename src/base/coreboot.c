/*
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
 * Copyright (C) 2009 coresystems GmbH
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

#include <coreboot_tables.h>
#include <stdint.h>
#include <stdlib.h>
#include <sysinfo.h>

#include "base/fwdb.h"
#include "base/physmem.h"
#include "vboot/util/vboot_handoff.h"

struct sysinfo_t lib_sysinfo;

/*
 * The code in this file applies to all coreboot architectures. Some coreboot
 * table tags are architecture specific, they are handled by their respective
 * cb_parse_arch_specific() functions.
 */

/* === Parsing code === */
/* This is the generic parsing code. */

static void cb_parse_memory(void *ptr, struct sysinfo_t *info)
{
	struct cb_memory *mem = ptr;
	int count = MEM_RANGE_COUNT(mem);

	E820MemRanges *e820_map = get_e820_mem_ranges();
	if (!e820_map)
		return;

	if (count > ARRAY_SIZE(e820_map->ranges))
		count = ARRAY_SIZE(e820_map->ranges);

	e820_map->num_ranges = count;
	for (int i = 0; i < count; i++) {
		struct cb_memory_range *range = MEM_RANGE_PTR(mem, i);
		E820MemRange *e820 = &e820_map->ranges[i];
		e820->base = cb_unpack64(range->start);
		e820->size = cb_unpack64(range->size);

		switch (range->type) {
		case CB_MEM_RAM:
		case CB_MEM_RESERVED:
		case CB_MEM_ACPI:
		case CB_MEM_NVS:
		case CB_MEM_UNUSABLE:
			e820->type = range->type;
			break;
		case CB_MEM_TABLE:
			e820->type = E820MemRange_Reserved;
		default:
			e820->type = E820MemRange_Unusable;
		}
		e820->handoff_tag = range->type;
	}
}

static void cb_parse_vboot_handoff(unsigned char *ptr, struct sysinfo_t *info)
{
	struct lb_range *vbho = (struct lb_range *)ptr;
	struct vboot_handoff *vboot_handoff =
		(struct vboot_handoff *)(uintptr_t)vbho->range_start;

	if (vbho->range_size != sizeof(struct vboot_handoff))
		return;

	FwdbEntry shared_data = {
		.ptr = &vboot_handoff->shared_data[0],
		.size = ARRAY_SIZE(vboot_handoff->shared_data),
	};
	fwdb_access("vboot.shared_data", NULL, &shared_data);

	FwdbEntry init_params = {
		.ptr = &vboot_handoff->init_params,
		.size = sizeof(vboot_handoff->init_params),
	};
	fwdb_access("vboot.handoff.init_params", NULL, &init_params);
}

static void cb_parse_vbnv(unsigned char *ptr, struct sysinfo_t *info)
{
	struct lb_range *vbnv = (struct lb_range *)ptr;

	info->vbnv_start = vbnv->range_start;
	info->vbnv_size = vbnv->range_size;
}

static void cb_parse_gpios(unsigned char *ptr, struct sysinfo_t *info)
{
	struct cb_gpios *gpios = (struct cb_gpios *)ptr;

	const char prefix[] = "gpio.";
	char name_buf[sizeof(prefix) + sizeof(gpios->gpios[0].name)];
	memset(name_buf, 0, sizeof(name_buf));
	memcpy(name_buf, prefix, sizeof(prefix));
	char *suffix_ptr = &name_buf[sizeof(prefix) - 1];

	for (int i = 0; i < gpios->count; i++) {
		struct cb_gpio *gpio = &gpios->gpios[i];

		uint8_t val = gpio->value;
		if (gpio->polarity == CB_GPIO_ACTIVE_LOW)
			val = !val;

		FwdbEntry gpio_entry = {
			.ptr = &val,
			.size = sizeof(val),
		};
		memcpy(suffix_ptr, gpio->name, sizeof(gpio->name));
		fwdb_access(name_buf, NULL, &gpio_entry);
	}
}

static void cb_parse_vdat(unsigned char *ptr, struct sysinfo_t *info)
{
	struct lb_range *vdat = (struct lb_range *) ptr;

	info->vdat_addr = (void *)(uintptr_t)vdat->range_start;
	info->vdat_size = vdat->range_size;
}

static void cb_parse_tstamp(unsigned char *ptr, struct sysinfo_t *info)
{
	struct cb_cbmem_tab *const cbmem = (struct cb_cbmem_tab *)ptr;
	info->tstamp_table = (void *)(uintptr_t)cbmem->cbmem_tab;
}

static void cb_parse_cbmem_cons(unsigned char *ptr, struct sysinfo_t *info)
{
	struct cb_cbmem_tab *const cbmem = (struct cb_cbmem_tab *)ptr;
	info->cbmem_cons = (void *)(uintptr_t)cbmem->cbmem_tab;
}

static void cb_parse_acpi_gnvs(unsigned char *ptr, struct sysinfo_t *info)
{
	struct cb_cbmem_tab *const cbmem = (struct cb_cbmem_tab *)ptr;
	info->acpi_gnvs = (void *)(uintptr_t)cbmem->cbmem_tab;
}

static void cb_parse_board_id(unsigned char *ptr, struct sysinfo_t *info)
{
	struct cb_board_id *const cbbid = (struct cb_board_id *)ptr;
	info->board_id = cbbid->board_id;
}

static void cb_parse_ram_code(unsigned char *ptr, struct sysinfo_t *info)
{
	struct cb_ram_code *const ram_code = (struct cb_ram_code *)ptr;
	info->ram_code = ram_code->ram_code;
}

#if CONFIG_COREBOOT_VIDEO_CONSOLE
static void cb_parse_framebuffer(void *ptr, struct sysinfo_t *info)
{
	/* ptr points to a coreboot table entry and is already virtual */
	info->framebuffer = ptr;
}
#endif

static void cb_parse_ramoops(void *ptr, struct sysinfo_t *info)
{
	struct lb_range *ramoops = (struct lb_range *)ptr;

	info->ramoops_buffer = ramoops->range_start;
	info->ramoops_buffer_size = ramoops->range_size;
}

static void cb_parse_mtc(void *ptr, struct sysinfo_t *info)
{
	struct lb_range *mtc = (struct lb_range *)ptr;

	info->mtc_start = mtc->range_start;
	info->mtc_size = mtc->range_size;
}

static void cb_parse_spi_flash(void *ptr, struct sysinfo_t *info)
{
	struct cb_spi_flash *flash = (struct cb_spi_flash *)ptr;

	info->spi_flash.size = flash->flash_size;
	info->spi_flash.sector_size = flash->sector_size;
	info->spi_flash.erase_cmd = flash->erase_cmd;
}

static void cb_parse_boot_media_params(unsigned char *ptr,
				       struct sysinfo_t *info)
{
	struct cb_boot_media_params *const bmp =
			(struct cb_boot_media_params *)ptr;
	info->cbfs_offset = bmp->cbfs_offset;
	info->cbfs_size = bmp->cbfs_size;
}

int cb_parse_header(void *addr, int len, struct sysinfo_t *info)
{
	struct cb_header *header;
	unsigned char *ptr = addr;
	void *forward;
	int i;

	for (i = 0; i < len; i += 16, ptr += 16) {
		header = (struct cb_header *)ptr;
		if (!strncmp((const char *)header->signature, "LBIO", 4))
			break;
	}

	/* We walked the entire space and didn't find anything. */
	if (i >= len)
		return -1;

	/* Make sure the checksums match. */
	if (ipchecksum((uint16_t *)header, sizeof(*header)) != 0)
		return -1;

	if (!header->table_bytes)
		return 0;

	if (ipchecksum((uint16_t *)(ptr + sizeof(*header)),
		       header->table_bytes) != header->table_checksum)
		return -1;

	info->header = header;

	/*
	 * Board straps represented by numerical values are small numbers.
	 * Preset them to an invalid value in case the firmware does not
	 * supply the info.
	 */
	info->board_id = ~0;
	info->ram_code = ~0;

	/* Now, walk the tables. */
	ptr += header->header_bytes;

	for (i = 0; i < header->table_entries; i++) {
		struct cb_record *rec = (struct cb_record *)ptr;

		/* We only care about a few tags here (maybe more later). */
		switch (rec->tag) {
		case CB_TAG_FORWARD:
			forward = (void *)(uintptr_t)
				((struct cb_forward *)rec)->forward;
			return cb_parse_header(forward, len, info);
			continue;
		case CB_TAG_MEMORY:
			cb_parse_memory(ptr, info);
			break;
#if CONFIG_COREBOOT_VIDEO_CONSOLE
		// FIXME we should warn on serial if coreboot set up a
		// framebuffer buf the payload does not know about it.
		case CB_TAG_FRAMEBUFFER:
			cb_parse_framebuffer(ptr, info);
			break;
#endif
		case CB_TAG_GPIO:
			cb_parse_gpios(ptr, info);
			break;
		case CB_TAG_VDAT:
			cb_parse_vdat(ptr, info);
			break;
		case CB_TAG_VBNV:
			cb_parse_vbnv(ptr, info);
			break;
		case CB_TAG_VBOOT_HANDOFF:
			cb_parse_vboot_handoff(ptr, info);
			break;
		case CB_TAG_TIMESTAMPS:
			cb_parse_tstamp(ptr, info);
			break;
		case CB_TAG_CBMEM_CONSOLE:
			cb_parse_cbmem_cons(ptr, info);
			break;
		case CB_TAG_ACPI_GNVS:
			cb_parse_acpi_gnvs(ptr, info);
			break;
		case CB_TAG_BOARD_ID:
			cb_parse_board_id(ptr, info);
			break;
		case CB_TAG_RAM_CODE:
			cb_parse_ram_code(ptr, info);
			break;
		case CB_TAG_RAM_OOPS:
			cb_parse_ramoops(ptr, info);
			break;
		case CB_TAG_SPI_FLASH:
			cb_parse_spi_flash(ptr, info);
			break;
		case CB_TAG_MTC:
			cb_parse_mtc(ptr, info);
			break;
		case CB_TAG_BOOT_MEDIA_PARAMS:
			cb_parse_boot_media_params(ptr, info);
			break;
		default:
			cb_parse_arch_specific(rec, info);
			break;
		}

		ptr += rec->size;
	}

	return 0;
}
