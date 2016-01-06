/*
 * Copyright 2014 Google Inc.
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

#include <endian.h>
#include <libpayload.h>
#include <stdint.h>
#include <string.h>

#include "base/device_tree.h"
#include "base/init_funcs.h"
#include "base/list.h"
#include "boot/ramoops.h"
#include "vboot/util/memory.h"

typedef struct
{
	uint64_t start;
	uint64_t size;
	uint64_t record_size;

	DeviceTreeFixup fixup;
} Ramoops;

/* Undocumented, but linux kernel expects record-size to use only one cell */
static const int RecordSizeCells = 1;

static int ramoops_fixup(DeviceTreeFixup *fixup, DeviceTree *tree)
{
	Ramoops *ramoops = container_of(fixup, Ramoops, fixup);

	// Eliminate any existing ramoops node.
	DeviceTreeNode *node = dt_find_compat(tree->root, "ramoops");
	if (node)
		list_remove(&node->list_node);

	uint32_t addr_cells = 1, size_cells = 1;
	dt_read_cell_props(tree->root, &addr_cells, &size_cells);

	// Create a ramoops node at the root of the tree.
	node = xzalloc(sizeof(*node));
	node->name = "ramoops";
	list_insert_after(&node->list_node, &tree->root->children);

	// Add a compatible property.
	dt_add_string_prop(node, "compatible", "ramoops");

	// Add a reg property.
	dt_add_reg_prop(node, &ramoops->start, &ramoops->size, 1,
			addr_cells, size_cells);

	// Add a record-size property.
	uint8_t *data = xzalloc(size_cells * sizeof(uint32_t));
	dt_write_int(data, ramoops->record_size,
		     RecordSizeCells * sizeof(uint32_t));
	dt_add_bin_prop(node, "record-size", data,
			RecordSizeCells * sizeof(uint32_t));

	// Add the optional dump-oops property.
	dt_add_bin_prop(node, "dump-oops", NULL, 0);

	// Add a memory reservation for the buffer range.
	DeviceTreeReserveMapEntry *reserve = xzalloc(sizeof(*reserve));
	reserve->start = ramoops->start;
	reserve->size = ramoops->size;
	list_insert_after(&reserve->list_node, &tree->reserve_map);

	return 0;
}

void ramoops_buffer(uint64_t start, uint64_t size, uint64_t record_size)
{
	static Ramoops ramoops;

	die_if(ramoops.fixup.fixup, "Only one ramoops buffer allowed.\n");

	ramoops.start = start;
	ramoops.size = size;
	ramoops.record_size = record_size;
	ramoops.fixup.fixup = &ramoops_fixup;
	list_insert_after(&ramoops.fixup.list_node, &device_tree_fixups);

	memory_mark_used(start, start + size);
}

static int ramoops_init(void)
{
	if (lib_sysinfo.ramoops_buffer == 0 ||
	    lib_sysinfo.ramoops_buffer_size == 0)
		return 0;

	ramoops_buffer(lib_sysinfo.ramoops_buffer,
		       lib_sysinfo.ramoops_buffer_size, 0x20000);

	return 0;
}

INIT_FUNC(ramoops_init);
