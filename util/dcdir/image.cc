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

#include "image.hh"

#include <endian.h>

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "dcdir_structs.h"
#include "region.hh"

using ::std::cerr;
using ::std::endl;
using ::std::ifstream;
using ::std::string;

namespace dcdir
{

// The image initializes itself with the file at file_name.
ImageBuffer::ImageBuffer(const string &file_name) :
		size_(0), modified_(false),
		major_version_(0), minor_version_(0)
{
	// Read in the image file.
	ifstream image(file_name.c_str(), ifstream::binary);
	if (!image.good()) {
		cerr << "Failed to open image file " << file_name << endl;
		exit(1);
	}
	image.seekg(0, image.end);
	size_ = image.tellg();
	image.seekg(0, image.beg);
	buf_ = std::unique_ptr<uint8_t []>(new uint8_t[size_]);
	image.read((char *)buf_.get(), size_);

	// Find dcdir structures.
	DcDirAnchor *anchor = find_anchor();
	if (!anchor) {
		cerr << "Failed to find depthcharge directory structure."
		     << endl;
		exit(1);
	}
	major_version_ = anchor->major_version;
	minor_version_ = anchor->minor_version;

	uint32_t base = anchor->root_base;
	uint32_t root_offset = offset_of(anchor + 1) - base;
	// We'll just assume the root goes to the end of the image. The size
	// is nominally just used for bounds checking anyway.
	uint32_t root_size = size() - root_offset;

	root_dir_ = std::unique_ptr<Region>(
		new Region(*this, "", base, root_offset, root_size, true));
}

DcDirAnchor *ImageBuffer::find_anchor()
{
	int max_index = size() / sizeof(DcDirAnchor);
	DcDirAnchor *anchor = (DcDirAnchor *)buf();
	for (int i = 0; i < max_index; i++, anchor++) {
		if (memcmp(anchor->signature, DcDirAnchorSignature,
			   DcDirAnchorSignatureSize) != 0)
			continue;
		if (anchor->anchor_offset != offset_of(anchor))
			continue;
		return anchor;
	}
	return NULL;
}

size_t ImageBuffer::storage_overhead() const
{
	size_t overhead = sizeof(DcDirAnchor);
	if (root_dir())
		overhead += root_dir()->storage_overhead();
	return overhead;
}

} // namespace dcdir
