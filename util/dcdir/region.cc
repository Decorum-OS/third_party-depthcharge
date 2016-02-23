/* Copyright 2016 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "region.hh"

#include <endian.h>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

#include "dcdir_structs.h"

#include "image.hh"

using ::std::cerr;
using ::std::endl;
using ::std::string;

namespace dcdir
{

void Region::process_as_directory(const ImageBuffer &image)
{
	uint32_t table_offset = offset() + base();

	// Read in and verify the directory header.
	DcDirDirectoryHeader *header;
	image.pointer_to(table_offset, header);
	if (memcmp(header->signature, DcDirDirectorySignature,
		   DcDirDirectorySignatureSize) != 0) {
		cerr << "Corrupted directory \"" << name() << "\"." << endl;
		exit(1);
	}

	// Compute the directory table size.
	table_size_ = (header->size[0] << 0) |
		      (header->size[1] << 8) |
		      (header->size[2] << 16);
	table_size_ = (table_size_ + 1) * 8;
	assert(base() + table_size_ <= length());
	uint32_t pos = table_offset + sizeof(DcDirDirectoryHeader);

	// Process each entry in this directory.
	const int label_length = 8;
	char clabel[label_length + 1];
	clabel[label_length] = '\0';
	while (pos < table_offset + table_size_) {
		uint64_t *label_ptr;
		image.pointer_to(pos, label_ptr);
		memcpy(clabel, label_ptr, label_length);
		pos += label_length;

		string label(clabel);
		Region *sub_region = new Region(image, label, offset(), pos);
		entries_.push_back(sub_region);
		pos += sub_region->ptr_size();
	}
}

Region::Region(const ImageBuffer &image, const string &new_name,
	       uint32_t new_base, uint32_t new_offset, uint32_t new_length,
	       bool new_directory) :
	name_(new_name), base_(new_base),
	offset_(new_offset), length_(new_length),
	ptr_size_(0), type_(0), directory_(new_directory)
{
	// We don't have to extract any information out of the pointer to this
	// region, but we still need to process its contents if it's a
	// directory.
	if (directory())
		process_as_directory(image);
}

Region::Region(const ImageBuffer &image, const string &new_name,
	       uint32_t dir_offset, uint32_t ptr_offset) :
	name_(new_name), table_size_(0)
{
	DcDirPointer *base_ptr;
	image.pointer_to(ptr_offset, base_ptr);

	// Decode some common fields in the pointer.
	ptr_size_ = (base_ptr->size + 1) * 8;
	directory_ = base_ptr->type & 0x1;
	type_ = base_ptr->type >> 1;

	// Decode fields in the pointer who's structure/location depend on the
	// pointer's type.
	switch (type_)
	{
	case DcDirOffset24Length24:
		{
			DcDirPointerOffset24Length24 *ptr;
			image.pointer_to(ptr_offset, ptr);
			base_ = 0;
			offset_ = (ptr->offset[0] << 0) |
				  (ptr->offset[1] << 8) |
				  (ptr->offset[2] << 16);
			length_ = ((ptr->length[0] << 0) |
				   (ptr->length[1] << 8) |
				   (ptr->length[2] << 16)) + 1;
		}
		break;
	case DcDirBase32Offset32Length32:
		{
			DcDirPointerBase32Offset32Length32 *ptr;
			image.pointer_to(ptr_offset, ptr);
			base_ = le32toh(ptr->base);
			offset_ = le32toh(ptr->offset);
			length_ = le32toh(ptr->length) + 1;
		}
		break;
	default:
		cerr << "Bad pointer type " << type_ << "." << endl;
		exit(1);
	}
	// Make the offset relative to the image rather than the
	// parent directory.
	offset_ += dir_offset;

	// If this region is a directory, process its contents.
	if (directory())
		process_as_directory(image);
}

size_t Region::storage_overhead() const
{
	// Non-directory regions have no intrinsic overhead.
	if (!directory())
		return 0;

	// Directories have overhead from their tables, and also indirectly
	// from the tables in their subdirectories.
	size_t overhead = table_size_;
	for (const auto &entry: entries_) {
		assert(entry);
		overhead += entry->storage_overhead();
	}
	return overhead;
}

} // namespace dcdir
