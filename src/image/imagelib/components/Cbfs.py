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

from Area import DerivedArea
from imagelib.tools.Cbfstool import Cbfstool

import os
import tempfile

class CbfsFile(DerivedArea):
    def __init__(self, name, data, t):
        super(CbfsFile, self).__init__(data)
        self._name = name
        self._data = data
        self._t = t
        self._base = None

        self.shrink()

    def compute_min_size_content(self):
        if not self.handle_children():
            self._data_buf = self._data.write()
        return len(self._data_buf)

    def base(self, base):
        self._base = base
        return self

    def install(self, cbfstool):
        h, path = tempfile.mkstemp()
        with os.fdopen(h, "w+b") as f:
            f.write(self._data_buf)
        cbfstool.add(path, self._name, self._t, self._base)
        os.remove(path)

class CbfsPayload(DerivedArea):
    def __init__(self, name, data):
        super(CbfsPayload, self).__init__(data)
        self._name = name
        self._data = data
        self._base = None
        self._compression = None

        self.shrink()

    def compute_min_size_content(self):
        if not self.handle_children():
            self._data_buf = self._data.write()
        return len(self._data_buf)

    def base(self, base):
        self._base = base
        return self

    def compression(self, compression):
        self._compression = compression
        return self

    def install(self, cbfstool):
        h, path = tempfile.mkstemp()
        with os.fdopen(h, "w+b") as f:
            f.write(self._data_buf)
        cbfstool.add_payload(path, self._name, self._compression, self._base)
        os.remove(path)

class Cbfs(DerivedArea):
    def __init__(self, *args):
        super(Cbfs, self).__init__(*args)
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
        h, path = tempfile.mkstemp()
        with os.fdopen(h, "w+b") as f:
            if self._base:
                f.write(self._base.write())
                cbfstool = Cbfstool(path)
            else:
                cbfstool = Cbfstool()
                if not self._bootblock:
                    raise ValueError("No bootblock specified")
                cbfstool.create(path, self.placed_size, self._bootblock,
                                self._arch, self._align, self._offset)
            for child in self.children:
                if child is not self._base:
                    child.install(cbfstool)
            f.seek(0)
            buf = f.read()
        os.remove(path)
        return buf
