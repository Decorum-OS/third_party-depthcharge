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

#ifndef __REGION_HH__
#define __REGION_HH__

#include <assert.h>

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace dcdir
{

class ImageBuffer;

class Region
{
public:
	Region(const ImageBuffer &image, const std::string &new_name,
	       uint32_t new_base, uint32_t new_offset, uint32_t new_length,
	       bool new_directory);

	Region(const ImageBuffer &image, const std::string &new_name,
	       uint32_t dir_offset, uint32_t ptr_offset);

	// Return the name of this region.
	const std::string &name() const
	{
		return name_;
	}

	// Return the "base" of this region, ie. the offset of the directory
	// table within this region. Only meaningful for directories.
	uint32_t base() const
	{
		return base_;
	}

	// Return this region's offset within the image.
	uint32_t offset() const
	{
		return offset_;
	}

	// Return this region's length.
	uint32_t length() const
	{
		return length_;
	}

	// Return the size of the pointer which pointed to this region.
	uint32_t ptr_size() const
	{
		assert(ptr_size_);
		return ptr_size_;
	}

	// Return the type of the pointer to this region.
	uint8_t type() const
	{
		return type_;
	}

	// Return whether this region is a directory.
	bool directory() const
	{
		return directory_;
	}

	std::vector<Region *> &entries()
	{
		return entries_;
	}

	// Return the number of bytes occupied by the dcdir structures
	// themselves.
	size_t storage_overhead() const;

private:
	// Process the content of this region as a directory, and fill in
	// those fields of this object.
	void process_as_directory(const ImageBuffer &image);

	// The name of this region in its parent directory.
	std::string name_;

	// The offset of the directory table in this region, if it's a
	// directory.
	uint64_t base_;
	// The offset of this region relative to its parent directory.
	uint64_t offset_;
	// The length of this region.
	uint64_t length_;
	// The size of the pointer that points to this region.
	uint32_t ptr_size_;
	// The type of the pointer.
	uint8_t type_;
	// Whether this region is a directory.
	bool directory_;
	// The size of the directory table in this region, if any.
	uint32_t table_size_;
	// If this region is a directory, the entries within it.
	std::vector<Region *> entries_;
};

} // namespace dcdir

#endif // __REGION_HH__
