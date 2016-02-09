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
from imagelib.util import CStruct

class FmapHeader(CStruct):
    struct_members = (
        ("8s", "signature"),
        ("B", "major"),
        ("B", "minor"),
        ("Q", "base"),
        ("L", "size"),
        ("32s", "name"),
        ("H", "nareas")
    )

class FmapArea(CStruct):
    struct_members = (
        ("L", "offset"),
        ("L", "size"),
        ("32s", "name"),
        ("H", "flags")
    )

class Section(Area):
    def __init__(self, label, *args):
        super(Section, self).__init__(*args)
        self._label = label
        self._flags = Fmap.Static

    def flags(self, flags):
        self._flags = flags
        return self

    def log_area_name(self):
        return "Fmap[%s]" % self._label

    def log_get_additional_properties(self):
        return ["flags={:#04x}".format(self._flags)]

class Fmap(Area):
    Static = 1 << 0
    Compressed = 1 << 1
    ReadOnly = 1 << 2

    def __init__(self, image_size):
        super(Fmap, self).__init__()
        self.sections = []

        header = FmapHeader()
        header.signature = "__FMAP__"
        header.major = 1
        header.minor = 0
        header.base = 0
        header.size = image_size
        header.name = "FMAP"
        header.nareas = 0
        self.header = header

        self.shrink()

    def compute_min_size_content(self):
        return (self.header.struct_len +
                len(self.sections) * FmapArea.struct_len)

    def section(self, name, *args):
        section = Section(name, *args)
        self.sections.append(section)
        self.header.nareas += 1
        return section

    def write(self):
        def compare(a, b):
            res = a.placed_offset - b.placed_offset
            return res if res else b.placed_size - a.placed_size

        ordered = sorted(self.sections, cmp=compare)
        fmap = self.header.pack()

        for section in ordered:
            area = FmapArea()
            area.offset = section.placed_offset
            area.size = section.placed_size
            area.name = section._label
            area.flags = section._flags

            fmap += area.pack()

        return fmap

    def log_get_additional_properties(self):
        return ["image_size={:#x}".format(self.header.size)]
