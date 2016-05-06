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
from imagelib.tools.FspBct import FspBct
from imagelib.tools.Tool import FileHarness

class Fsp(DerivedArea):
    def __init__(self, raw):
        self._raw = raw
        super(Fsp, self).__init__(raw)
        self.shrink()

    def base_address(self, _address):
        self._fsp_base_address = _address
        return self

    def compute_min_size_content(self):
        self.handle_children()
        return self._raw.computed_min_size

    def write(self):
        fsp_bct = FspBct()
        with FileHarness(None, self._raw.write()) as (relocated, raw):
            fsp_bct.relocate(raw, relocated, self._fsp_base_address)

            with open(relocated, "rb") as data:
                return data.read()

    def log_get_additional_properties(self):
        props = []
        if hasattr(self, "_fsp_base_address"):
            props.append("base=%#x" % self._fsp_base_address)
	return props

    def log_area_content(self, indent):
        child_areas = {
            "Raw FSP": self._raw,
        }
        return self.log_child_props(indent, child_areas)
