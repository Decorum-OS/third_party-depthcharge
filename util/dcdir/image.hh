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

#ifndef __IMAGE_HH__
#define __IMAGE_HH__

#include <cassert>
#include <memory>
#include <string>

#include "dcdir_structs.h"

#include "region.hh"

namespace dcdir
{

class ImageBuffer
{
public:
	// The image initializes itself with the file at file_name.
	explicit ImageBuffer(const std::string &file_name);

	// Return a pointer to the buffer holding the image's contents.
	void *buf() const
	{
		return buf_.get();
	}

	// Return the size of the image.
	uint32_t size() const
	{
		return size_;
	}

	// Mark the image as modified in case it needs to be written back.
	void modified(bool new_modified)
	{
		modified_ = new_modified;
	}

	// Return the major version number.
	int major_version() const
	{
		return major_version_;
	}

	// Return the minor version number.
	int minor_version() const
	{
		return minor_version_;
	}

	// Return the number of bytes occupied by the dcdir structures
	// themselves.
	size_t storage_overhead() const;

	// Return a pointer to the root directory in this image.
	dcdir::Region *root_dir() const
	{
		return root_dir_.get();
	}

	/*
	 * Given a pointer into the image's buffer, return the offset of that
	 * data in bytes. Also check that the type pointed to fits entirely
	 * within the image.
	 */
	template<typename T>
	uint32_t offset_of(const T *ptr) const
	{
		uint32_t ret = (const uint8_t *)ptr -
		               (const uint8_t *)buf_.get();
		assert(ret + sizeof(T) <= size_);
		return ret;
	}

	/*
	 * Given an offset into the image, return a pointer to that data. Also
	 * make sure that the type pointed to fits within the buffer. The
	 * pointer is set by reference so that the template knows what type
	 * to use without having to have a clunky <> at each call sight.
	 */
	template<typename T>
	void pointer_to(uint32_t off, T *&ptr) const
	{
		assert(off + sizeof(T) <= size_);
		ptr = (T*)((uint8_t *)buf_.get() + off);
	}

private:
	DcDirAnchor *find_anchor();

	// Buffer of image data.
	std::unique_ptr<uint8_t []> buf_;
	// Size of the image/data.
	uint32_t size_;
	// Whether the image data has been changed and should be written back,
	// if that's appropriate for what the user requested.
	bool modified_;

	// Version numbers of the dcdir structures.
	int major_version_;
	int minor_version_;
	// The root directory in this image.
	std::unique_ptr<Region> root_dir_;
};

} // namespace dcdir

#endif // __IMAGE_HH__
