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

import struct

KB = 2 ** 10
MB = 2 ** 20


# This class and metaclass make it easier to define and access structures
# which live in physical memory. To use them, inherit from CStruct and define
# a class member called struct_members which is a tuple of pairs. The first
# item in the pair is the type format specifier that should be used with
# struct.unpack to read that member from memory. The second item is the name
# that member should have in the resulting object.

class MetaCStruct(type):
    def __init__(cls, name, bases, dct):
        struct_members = dct["struct_members"]
        cls.struct_fmt = "="
        for char, name in struct_members:
            cls.struct_fmt += char
        cls.struct_len = struct.calcsize(cls.struct_fmt)
        super(MetaCStruct, cls).__init__(name, bases, dct)

class CStruct(object):
    __metaclass__ = MetaCStruct
    struct_members = ()

    def __init__(self, buf=None):
        if buf is not None:
            self.unpack(buf)

    def unpack(self, buf):
        values = struct.unpack(self.struct_fmt, buf)
        # Skip "x" members which are padding bytes.
        names = (name for char, name in self.struct_members if 'x' not in char)
        for name, value in zip(names, values):
            setattr(self, name, value)

    def pack(self):
        # Skip "x" members which are padding bytes.
        names = (name for char, name in self.struct_members if 'x' not in char)
        values = (getattr(self, name) for name in names)
        return struct.pack(self.struct_fmt, *values)


class Buffer(object):
    def __init__(self, *args, **kwargs):
        if (any(arg in kwargs for arg in ("fill", "size", "base")) or
            len(args) > 1):
            self._init_with_three_args(*args, **kwargs)
        else:
            self._init_with_one_arg(*args, **kwargs)
        self._buf = struct.pack("B", self._fill) * self._size

    def _init_with_two_args(self, fill, size, base=0):
        self._fill = fill
        self._size = size
        self._base = base

    def _init_with_one_arg(self, area):
        self._fill = area.get_fill_byte()
        self._size = area.placed_size
        self._base = area.placed_offset

    def fill(self):
        return self._fill

    def size(self):
        return self._size

    def data(self):
        return self._buf

    def inject(self, data, offset, length):
        offset -= self._base
        data_size = len(data)

        if data_size != length:
            raise ValueError("Attempting to write %d into a %d byte buffer." %
                             (data_size, length))

        if offset + length > self._size:
            raise ValueError("Write beyond the end of the buffer.")

        self._buf = self._buf[:offset] + data + self._buf[offset + length:]

    def inject_areas(self, *areas):
        for area in areas:
            self.inject(area.write(), area.placed_offset, area.placed_size)

__all__ = ["KB", "MB", "CStruct", "Buffer"]
