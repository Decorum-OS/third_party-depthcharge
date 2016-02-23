# Copyright 2016 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from Area import Area
from imagelib.util import Buffer
from imagelib.util import CStruct

import collections
import struct

class Region(Area):
    """A region with a name in the directory which holds raw data."""

    def __init__(self, region_name, *args, **kwargs):
        """If you don't want a Region to appear in the directory, set its name
           to None. You might do this if you're recreating a part of the Area
           hierarchy to compute a signature, for instance, and don't want the
           copy to clash with the original in their parent directory.
        """
        super(Region, self).__init__(*args, **kwargs)
        # The name of this region in its parent directory, or None if it
        # shouldn't get an entry there.
        self._region_name = region_name
        # Whether this Region needs a big pointer with an explicit base and
        # 32 bit offset and length values.
        self._big_pointer = False
        # The type of pointer structure to use.
        self.pointer_type = DcDirRegionPointerOffset24Length24
        # What the base value is, if any.
        self._base = 0
        # Whether this Region is actualy a directory, vs. raw unformatted data.
        self._is_directory = False

    def big_pointer(self):
        """Tell build_image that this pointer needs to use the big style
           pointers. It's much easier to make this explicit and the
           responsibility of the layout than to try to figure it out ourselves.
        """
        self._big_pointer = True
        self.pointer_type = DcDirRegionPointerBase32Offset32Length32
        return self

    # Internal functions used by Region, it's subclasses, and DirectoryTable.

    def _get_region_name(self):
        """Return the region name."""
        return self._region_name

    def _set_base(self, _base):
        """Set the "base" value for this region. This is only meaningful when
           used on a directory.
        """
        self._base = _base
        if _base != 0:
            # If the base value is non-zero, we need a big pointer to hold it.
            assert self._big_pointer

    def _set_is_directory(self, _is_directory):
        """Set whether this region is a directory."""
        self._is_directory = _is_directory

    def _get_pointer_size(self):
        """Return how much space a pointer to this region will take up."""
        return self.pointer_type.struct_len

    def _get_pointer(self, parent_offset):
        """Construct a pointer to this region."""
        relative_offset = self.placed_offset - parent_offset

        pointer = self.pointer_type()

        if self._big_pointer:
            pointer.base = self._base
        else:
            # The offset and length fields are only 24 bits in the smaller
            # pointer type. The length field actually stores the length minus
            # 1.
            assert relative_offset < (1 << 24)
            assert self.placed_size <= (1 << 24)

        pointer.offset = relative_offset
        pointer.length = self.placed_size
        pointer.directory = self._is_directory
        return pointer.pack()

class DirectoryTable(Area):
    """A class which represents a table which maps the contents of this region
       as a directory.
    """
    LABEL_FORMAT = "8s"
    LABEL_SIZE = 8

    def __init__(self):
        super(DirectoryTable, self).__init__()
        # A dictionary which maps region names to the regions themselves.
        self._regions = None
        # Whether this is the root directory which needs an anchor structure.
        self._is_root = False
        # A pointer to the directory this table is describing.
        self._directory = None
        self.shrink()

    # Internal functions called by the Directory or RootDirectory classes.

    def _set_regions(self, regions):
        """Set the regions this table will map. "regions" should be a dict
           which maps from region names to Regions. They will be stored in the
           table in sorted order.
        """
        self._regions = regions

    def _set_is_root(self, _is_root):
        """Configure whether this directory is the root of the hierarchy and
           therefore needs to be prefixed with an anchor structure.
        """
        self._is_root = _is_root

    def _set_directory(self, _directory):
        """Set a reference back to the Directory region this table refers to."""
        self._directory = _directory

    # Functions which hook into the generic Area infrastructure.

    def compute_min_size_content(self):
        """This area will optionally have an anchor in it if it's the root. It
           will also have an 8 byte header, and an 8 byte label and a pointer
           for each region that is mapped.
        """
        size = 0

        # Anchor.
        if self._is_root:
            size += DcDirAnchor.struct_len
        # Directory header.
        size += DcDirDirectory.struct_len
        # Labels.
        size += len(self._regions) * self.LABEL_SIZE
        # Pointers.
        size += sum(region._get_pointer_size() for
                    region in self._regions.itervalues())

        return size

    def write(self):
        # A convenient alias for this value.
        dir_offset = self._directory.placed_offset

        buf = Buffer(self)
        # Our current position in the output buffer, relative to the beginning
        # of the entire image.
        pos = self.placed_offset

        if self._is_root:
            # If we're the root directory, start with an anchor structure.
            anchor = DcDirAnchor()
            anchor_size = DcDirAnchor.struct_len
            anchor.anchor_offset = self.placed_offset
            # The "base" value for the directory we go with is the offset of
            # the standard table structure from the base of the region it
            # describes. The start of the table is our current position, plus
            # the size of the anchor structure.
            anchor.root_base = pos + anchor_size - dir_offset
            buf.inject(anchor.pack(), pos, anchor_size)
            pos += anchor_size

        # Output the directory header.
        header = DcDirDirectory()
        # The directory table size is the remaining space in this Area.
        header.size = self.placed_size - (pos - self.placed_offset)
        fixed_header_size = DcDirDirectory.struct_len
        buf.inject(header.pack(), pos, fixed_header_size)
        pos += fixed_header_size

        # For each region within the directory...
        cmp_func = lambda (a, x), (b, y): cmp(x.placed_offset, y.placed_offset)
        for name, region in sorted(self._regions.iteritems(), cmp=cmp_func):
            # Add a label with the region's name.
            assert len(name) <= self.LABEL_SIZE
            label = struct.pack(self.LABEL_FORMAT, name)
            buf.inject(label, pos, self.LABEL_SIZE)
            pos += self.LABEL_SIZE

            # Output a pointer to the region.
            pointer_size = region._get_pointer_size()
            buf.inject(region._get_pointer(self._directory.placed_offset),
                       pos, pointer_size)
            pos += pointer_size

        return buf.data()


class DirectoryBase(Region):
    """Base class for directories which does most of the work."""

    def __init__(self, is_root, region_name, *contents):
        super(DirectoryBase, self).__init__(region_name, *contents)
        self._is_root = is_root
        self._set_is_directory(True)

    def post_config_hook(self):
        """Find regions and the table within this directories area. This
           should be right all children are assigned.
        """
        # Find the regions this directory needs to map, and where it's table
        # area is if one was put somewhere explicitly.
        regions = {}
        table = None

        # Prime the list of children to search with the Areas within this
        # Directory container.
        children = collections.deque(self.children)
        # While we still have children to look at...
        while len(children):
            child = children.pop()
            # If this is where the table is supposed to go, grab a reference
            # to it.
            if isinstance(child, DirectoryTable):
                assert table is None
                table = child
            # If not and this is a Region with a name, put it in our list of
            # regions to map.
            elif isinstance(child, Region):
                name = child._get_region_name()
                if name:
                    regions[name] = child
            # Otherwise, add this child's children to our Queue of Areas to
            # iterate over. Regions at the same level of the hierarchy should
            # be mutually exclusive.
            else:
                children.extendleft(child.children)

        # If there wasn't a table explicitly added somewhere, add one
        # implicitly at the beginning of the Directory. If there was, we'll
        # assume we need a big pointer to hold its base.
        self.implicit_table = (table is None)
        if self.implicit_table:
            table = DirectoryTable()
            self.children = (table,) + self.children
        else:
            self.big_pointer()

        table._set_regions(regions)
        table._set_is_root(self._is_root)
        table._set_directory(self)
        self._table = table

    def post_place_hook(self):
        """Record the offset (if any) of the directory table within the
           directory region.
        """
        self._set_base(self._table.placed_offset - self.placed_offset)

class Directory(DirectoryBase):
    """Actual class to use when describing a normal directory."""
    def __init__(self, region_name, *contents):
        super(Directory, self).__init__(False, region_name, *contents)

class RootDirectory(DirectoryBase):
    """The class to use as the root directory in a hierarchy."""
    def __init__(self, *contents):
        super(RootDirectory, self).__init__(True, None, *contents)


####################################################################
# Classes which represent the actual structures stored in the image.
####################################################################


class DcDirRegionPointerBase(CStruct):
    struct_members = (
        ("B", "raw_type"),
        ("B", "raw_size")
    )

    def __init__(self, buf=None):
        super(DcDirRegionPointerBase, self).__init__(buf)
        # If we're not initializing this structure from an existing pointer,
        # make sure raw_type exists so that functions which modify parts of it
        # have something to work with.
        if buf is None:
            self.raw_type = 0

    @property
    def type(self):
        return self.raw_type >> 1

    @type.setter
    def type(self, value):
        self.raw_type = (self.raw_type & 0x1) | (value << 1)

    @property
    def directory(self):
        return (self.raw_type & 0x1) == 0x1

    @directory.setter
    def directory(self, value):
        value = 1 if value else 0
        self.raw_type = (self.raw_type & ~0x1) | value

    @property
    def size(self):
        return (self.raw_size + 1) * 8

    @size.setter
    def size(self, value):
        assert value % 8 == 0
        self.raw_size = (value / 8) - 1

class DcDirRegionPointerOffset24Length24(DcDirRegionPointerBase):
    struct_members = DcDirRegionPointerBase.struct_members + (
        ("B", "offset_0"),
        ("B", "offset_1"),
        ("B", "offset_2"),
        ("B", "length_0"),
        ("B", "length_1"),
        ("B", "length_2")
    )

    def __init__(self, buf=None):
        super(DcDirRegionPointerOffset24Length24, self).__init__(buf)
        # If not initializing from memory, set up the type and size fields to
        # what they should be for this type of pointer.
        if buf is None:
            self.type = 1
            self.size = self.struct_len

    @property
    def offset(self):
        return ((self.offset_0 << 0) |
                (self.offset_1 << 8) |
                (self.offset_2 << 16))

    @offset.setter
    def offset(self, value):
        # Make sure the offset isn't too big for this pointer type.
        assert value >> 24 == 0
        self.offset_0 = (value >> 0) & 0xff
        self.offset_1 = (value >> 8) & 0xff
        self.offset_2 = (value >> 16) & 0xff

    @property
    def length(self):
        return ((self.length_0 << 0) |
                (self.length_1 << 8) |
                (self.length_2 << 16)) + 1

    @length.setter
    def length(self, value):
        value = value - 1
        # Make sure the length isn't too big for this pointer type.
        assert value >> 24 == 0
        self.length_0 = (value >> 0) & 0xff
        self.length_1 = (value >> 8) & 0xff
        self.length_2 = (value >> 16) & 0xff

class DcDirRegionPointerBase32Offset32Length32(DcDirRegionPointerBase):
    struct_members = DcDirRegionPointerBase.struct_members + (
        ("2x", ""),
        ("L", "base"),
        ("L", "offset"),
        ("L", "raw_length")
    )

    def __init__(self, buf=None):
        super(DcDirRegionPointerBase32Offset32Length32, self).__init__(buf)
        # If not initializing from memory, set up the type and size fields to
        # what they should be for this type of pointer.
        if buf is None:
            self.type = 2
            self.size = self.struct_len

    @property
    def length(self):
        return self.raw_length + 1

    @length.setter
    def length(self, value):
        self.raw_length = value - 1


class DcDirDirectory(CStruct):
    SIGNATURE = "DCDR"

    struct_members = (
        ("4s", "signature"),
        ("B", "reserved"),
        ("B", "size_0"),
        ("B", "size_1"),
        ("B", "size_2"),
    )

    def __init__(self, buf=None):
        super(DcDirDirectory, self).__init__(buf)
        # If not initializing from memory, set the signature and reserved
        # fields to default values.
        if buf is None:
            self.signature = self.SIGNATURE
            self.reserved = 0

    @property
    def size(self):
        combined = ((self.size_0 << 0) |
                    (self.size_1 << 1) |
                    (self.size_2 << 2))
        return (combined + 1) * 8

    @size.setter
    def size(self, value):
        # Make sure the directory's size is a multiple of 8 bytes.
        assert value % 8 == 0
        combined = (value / 8) - 1
        # Also make sure it fits within the three byte field.
        assert combined >> 24 == 0
        self.size_0 = (combined >> 0) & 0xff
        self.size_1 = (combined >> 8) & 0xff
        self.size_2 = (combined >> 16) & 0xff


class DcDirAnchor(CStruct):
    SIGNATURE = "DC DIR"
    MAJOR_VERSION = 1
    MINOR_VERSION = 0

    struct_members = (
        ("6s", "signature"),
        ("B", "major_version"),
        ("B", "minor_version"),
        ("L", "anchor_offset"),
        ("L", "root_base")
    )

    def __init__(self, buf=None):
        super(DcDirAnchor, self).__init__(buf)
        # If not initializing from memory, set the signature and version fields
        # to default/standard values.
        if buf is None:
            self.signature = self.SIGNATURE
            self.major_version = self.MAJOR_VERSION
            self.minor_version = self.MINOR_VERSION
