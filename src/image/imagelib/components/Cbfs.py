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

from Area import Area, DerivedArea
from imagelib.tools.Cbfstool import Cbfstool
from imagelib.tools.Tool import FileHarness

class CbfsFileBase(Area):
    def __init__(self, name, data):
        super(CbfsFileBase, self).__init__(data)
        self._name = name
        self._data = data
        self._type = None
        self._base = None
        self._compression = None

        self.shrink()

    def base(self, base):
        self._base = base
        return self

    def log_area_name(self):
        name = super(CbfsFileBase, self).log_area_name()
        return name + "[{}]".format(self._name)

    def log_get_additional_properties(self):
        props = []
        if self._type is not None:
            props.append("type={}".format(self._type))
        if self._base is not None:
            props.append("base={:#x}".format(self._base))
        if self._compression is not None:
            props.append("compression={}".format(self._compression))
        return props

class CbfsFile(CbfsFileBase):
    def __init__(self, name, data, cbfs_type):
        super(CbfsFile, self).__init__(name, data)
        self._type = cbfs_type

    def install(self, cbfstool):
        with FileHarness(self._data.write()) as [data]:
            cbfstool.add(data, self._name, self._type, self._base)

class CbfsPayload(CbfsFileBase):
    def compression(self, compression):
        self._compression = compression
        return self

    def install(self, cbfstool):
        with FileHarness(self._data.write()) as [data]:
            cbfstool.add_payload(data, self._name, self._compression,
                                 self._base)

class Cbfs(DerivedArea):
    def __init__(self, *args):
        super(Cbfs, self).__init__(*args)
        self._files = args
        self._base = None
        self._bootblock = None
        self._arch = None
        self._align = None
        self._offset = None

    def base(self, base):
        self._base = base
        self.add_child(base)
        self.shrink()
        return self

    def bootblock(self, bootblock):
        self._bootblock = bootblock
        self.add_child(bootblock)
        return self

    def arch(self, arch):
        self._arch = arch
        return self

    def align(self, align):
        self._align = align
        return self

    def compute_min_size_content(self):
        self.handle_children()
        return self._base.computed_min_size if self._base else 0

    def write(self):
        blobs = []
        if self._base:
            blobs.append(self._base.write())
        else:
            blobs.append(None)
            if not self._bootblock:
                raise ValueError("No bootblock specified")
            blobs.append(self._bootblock.write())

        with FileHarness(*blobs) as files:
            if self._base:
                [base] = files
                cbfstool = Cbfstool(base)
            else:
                base, bootblock = files
                cbfstool = Cbfstool()
                cbfstool.create(base, self.placed_size, bootblock,
                                self._arch, self._align, self._offset)
            for f in self._files:
                f.install(cbfstool)
            with open(base, "rb") as data:
                return data.read()

    def log_get_additional_properties(self):
        props = []
        if self._arch is not None:
            props.append("arch={}".format(self._arch))
        if self._align is not None:
            props.append("align={}".format(self._align))
        if self._offset is not None:
            props.append("offset={}".format(self._offset))
        return props

    def log_area_content(self, indent):
        child_props = {}
        if self._base is not None:
            child_props["Base"] = self._base
        if self._bootblock is not None:
            child_props["Boot block"] = self._bootblock
        content = self.log_child_props(indent, child_props)

        if len(self._files):
            content += "\n\n{}Files:\n{}".format(
                indent, "\n".join(f.log_area(indent) for f in self._files))

        return content
