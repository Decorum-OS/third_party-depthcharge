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

#ifndef __DCDIR_STRUCTS_H__
#define __DCDIR_STRUCTS_H__

#include <stdint.h>

typedef struct __attribute__((packed))
{
	// Signature field which should be set to "DC DIR".
	uint8_t signature[6];
	// Major version which must change when non-backwards compatible
	// changes are made to these structures or their usage. The current
	// major version number is 1.
	uint8_t major_version;
	// Changes which can be consumed transparently as older versions of
	// this structure within the same major version but which are
	// different and consumed differently for code that's aware of them
	// should increment the minor version number.
	uint8_t minor_version;
	// The offset of the anchor structure in the image, used to verify
	// that the signature above is actually part of an anchor and not just
	// that string in the image for unrelated reasons.
	uint32_t anchor_offset;
	// The "base" value for the root directory. Used to work back from the
	// directory table which comes immediately after this structure back
	// to the offset of the directory itself.
	uint32_t root_base;
} DcDirAnchor;

// This has a prefixed name to prevent it from colliding with any ARRAY_SIZE
// macro which has already been defined.
#define DCDIR_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

const uint8_t DcDirAnchorSignature[] = { 'D', 'C', ' ', 'D', 'I', 'R' };
const size_t DcDirAnchorSignatureSize =
	DCDIR_ARRAY_SIZE(DcDirAnchorSignature);

typedef struct __attribute__((packed))
{
	// Signature field of a directory which should be set to "DCDR".
	uint8_t signature[4];
	// Reserved, should be set to 0.
	uint8_t reserved;
	// The size of the directory structure, including labels and pointers,
	// divided by 8 and minus 1.
	uint8_t size[3];
} DcDirDirectoryHeader;

const uint8_t DcDirDirectorySignature[] = { 'D', 'C', 'D', 'R' };
const size_t DcDirDirectorySignatureSize =
	DCDIR_ARRAY_SIZE(DcDirDirectorySignature);

typedef enum
{
	DcDirUndefined_0 = 0x00,
	DcDirOffset24Length24 = 0x01,
	DcDirBase32Offset32Length32 = 0x02,
	DcDirUndefined_7f = 0x7f
} DcDirType;

typedef struct __attribute__((packed))
{
	// The least significant bit of this field says whether the region
	// pointed to is a directory. The other 7 bits specify what type of
	// pointer it is, and what the rest of the structure holds.
	uint8_t type;
	// The size of the pointer structure, divided by 8 and minus 1.
	uint8_t size;
} DcDirPointer;

typedef struct __attribute__((packed))
{
	// The type and size fields below are shared/overlaid with the basic
	// pointer type above.
	uint8_t type; // DcDirOffset24Length24
	uint8_t size;
	// The offset of the region pointed to relative to its parent region.
	uint8_t offset[3];
	// The length of the region in bytes, minus 1.
	uint8_t length[3];
} DcDirPointerOffset24Length24;

typedef struct __attribute__((packed))
{
	// The type and size fields below are shared/overlaid with the basic
	// pointer type above.
	uint8_t type; // DcDirBase32Offset32Length32
	uint8_t size;
	// An unused padding value.
	uint16_t pad;
	// The offset of the directory table within the region itself, if the
	// region is a directory.
	uint32_t base;
	// The offset of the region, relative to its parent region.
	uint32_t offset;
	// The length of the region in bytes, minus 1.
	uint32_t length;
} DcDirPointerBase32Offset32Length32;

#endif // __DCDIR_STRUCTS_H__
