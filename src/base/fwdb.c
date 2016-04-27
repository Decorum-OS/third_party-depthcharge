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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "base/fwdb.h"

static const uint8_t fwdb_major_version = 1;
static const uint8_t fwdb_minor_version = 0;

typedef struct __attribute__((packed)) FwdbHeader
{
	uint8_t major_version;
	uint8_t minor_version;
	// A signature field to verify that a region actually contains an
	// FWDB. The intention is not for this to be used to locate this
	// header in memory by scanning for a particular value, just so that
	// we can verify that what someone claims is an FWDB actually is.
	uint8_t signature[6];

	// A pointer to the optional index structure which accelerates
	// accesses to this structure.
	uint64_t index_ptr;

	// The maximum size this structure, including its various entries, can
	// grow to.
	uint32_t max_size;
	// The size of this header.
	uint16_t header_size;
	uint16_t reserved;
} FwdbHeader;

// After the above header, entries in the database are composed of three
// fields. The first is a 32 bit size which specifies the size of the entire
// entry in bytes. The second is a name field which is a NULL terminated
// string. Those two fields are padded out so that they together take up a
// multiple of 8 bytes. Finally they are followed by a data field which takes
// the remainder of the entry. The next entry starts at the next 8 byte
// boundary after the end of the previous.
//
// At the end of the sequence of entries, there is a single 32 bit "size" field
// which is set to zero and not followed by the other two fields.

static const uint8_t FwdbSignature[] = { 'D', 'C', 'F', 'W', 'D', 'B' };
static const size_t FwdbSignatureSize = ARRAY_SIZE(FwdbSignature);

static FwdbHeader *fwdb_header;

int fwdb_create_db(void *base, uint32_t max_size)
{
	// Make sure there's at least enough room for the header and the
	// terminating table entry.
	if (max_size < sizeof(FwdbHeader) + sizeof(uint32_t)) {
		printf("The FWDB won't fit in a %d byte region.\n", max_size);
		return 1;
	}

	FwdbHeader *header = (FwdbHeader *)base;
	header->major_version = fwdb_major_version;
	header->minor_version = fwdb_minor_version;

	memcpy(header->signature, FwdbSignature, FwdbSignatureSize);

	header->index_ptr = 0;

	header->max_size = max_size;
	header->header_size = sizeof(*header);

	header->reserved = 0;

	uint32_t *first_size = (uint32_t *)((uintptr_t)base + sizeof(*header));
	*first_size = 0;

	return fwdb_use_db(header);
}

int fwdb_use_db(FwdbHeader *header)
{
	// Verify the integrity of the db.
	if (memcmp(header->signature, FwdbSignature, FwdbSignatureSize)) {
		printf("The FWDB primary table is damaged.\n");
		return 1;
	}

	// Make sure this is a version we know how to parse.
	if (header->major_version != fwdb_major_version) {
		printf("Incompatible FWDB version %d, expected %d.\n",
		       header->major_version, fwdb_major_version);
		return 1;
	}

	// Here, we would look over the index structure and either approve it
	// for use, or blow it away and replace it with something we do want
	// to use. Since no index format is yet defined, just verify that the
	// index pointer is NULL. If not, warn about it and continue.
	if (header->index_ptr != 0)
		printf("Warning: Ignoring index structure.\n");

	// Traverse the table and make sure all the entries are well formed
	// (as far as we can tell), and that the table fits in the prescribed
	// bounds so far.
	uint32_t total_size = header->header_size;
	uintptr_t entry_addr = (uintptr_t)header + total_size;
	while (1) {
		uint32_t *entry_size = (uint32_t *)entry_addr;
		total_size += *entry_size;
		if (total_size >= header->max_size)
			break;

		if (!*entry_size)
			break;

		const char *name_ptr =
			(const char *)(entry_addr + sizeof(*entry_size));
		size_t name_len = strnlen(name_ptr, *entry_size);
		if (name_len == *entry_size) {
			printf("Malformed entry name detected.\n");
			return 1;
		}

		entry_addr += *entry_size;
		entry_addr = ALIGN(entry_addr, sizeof(uint64_t));
	}
	// Add in the final 0 terminating size field.
	total_size += sizeof(uint32_t);

	if (total_size > header->max_size) {
		printf("FWDB takes %d bytes, which is more than %d.\n",
		       total_size, header->max_size);
		return 1;
	}

	fwdb_header = header;
	return 0;
}

FwdbHeader *fwdb_db_pointer(void)
{
	return fwdb_header;
}

uint32_t fwdb_db_max_size(void)
{
	if (!fwdb_header)
		return 0;

	return fwdb_header->max_size;
}

int fwdb_access(const char *name, FwdbEntry *entry, FwdbEntry *new_entry)
{
	if (!fwdb_header) {
		printf("No FWDB has been set up.\n");
		return 1;
	}

	uintptr_t header_addr = (uintptr_t)fwdb_header;
	uintptr_t addr = header_addr + fwdb_header->header_size;

	while (1) {
		uint32_t *entry_size = (uint32_t *)addr;

		// If we've reached the end of the list, stop so we can
		// create our entry, if requested.
		if (!*entry_size)
			break;

		const char *name_ptr =
			(const char *)(addr + sizeof(*entry_size));

		// If an entry with this name already, exists, complain and
		// bail out or return information about it.
		if (!strcmp(name_ptr, name)) {
			if (!entry) {
				printf("An FWDB entry named %s "
				       "already exists.\n", name);
				return 1;
			}

			uintptr_t data_offset =
				sizeof(*entry_size) + strlen(name_ptr) + 1;
			data_offset = ALIGN(data_offset, sizeof(uint64_t));

			entry->ptr = (void *)(addr + data_offset);
			entry->size = *entry_size - data_offset;

			return 0;
		}

		addr += *entry_size;
		addr = ALIGN(addr, sizeof(uint64_t));
	}

	if (!new_entry) {
		printf("FWDB entry %s not found.\n", name);
		return 1;
	}

	uint32_t *new_size_ptr = (uint32_t *)addr;

	uintptr_t name_len = strlen(name) + 1;
	// Figure out how far in the data will be in the new entry.
	uintptr_t data_offset = sizeof(*new_size_ptr) + name_len;
	data_offset = ALIGN(data_offset, sizeof(uint64_t));
	// Calculate the number of zeroes needed after "name".
	int name_pad_len = data_offset - name_len - sizeof(*new_size_ptr);

	// Find the new entry's total size.
	uintptr_t new_size = data_offset + new_entry->size;

	// Figure out where the new end of the table will be.
	uintptr_t end_addr = ALIGN(addr + new_size, sizeof(uint64_t)) +
			     sizeof(*new_size_ptr);

	// If there's not enough room, complain.
	if (end_addr - header_addr > fwdb_header->max_size) {
		printf("Not enough room to add %s to the FWDB.\n", name);
		return 1;
	}

	// Set the size of the new entry.
	*new_size_ptr = new_size;
	// Copy over the name.
	char *new_name = (char *)(new_size_ptr + 1);
	memcpy(new_name, name, name_len);
	// Zero out the unused bytes.
	memset(new_name + name_len, 0, name_pad_len);
	// Initialize the data field.
	uint8_t *data_ptr = (uint8_t *)(addr + data_offset);
	if (new_entry->ptr) {
		memcpy(data_ptr, new_entry->ptr, new_entry->size);
	} else {
		memset(data_ptr, 0, new_entry->size);
	}

	if (entry) {
		entry->ptr = data_ptr;
		entry->size = new_entry->size;
	}

	// Zero the next size field.
	new_size_ptr = (uint32_t *)(ALIGN(new_entry->size, sizeof(uint64_t)) +
				    data_ptr);
	*new_size_ptr = 0;

	return 0;
}
