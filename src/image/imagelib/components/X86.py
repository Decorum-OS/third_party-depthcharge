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

class ReljumpInstStruct(CStruct):
    struct_members = (
        ("B", "opcode"),
        ("h", "offset")
    )
    def __init__(self, buf=None):
        super(ReljumpInstStruct, self).__init__(buf)
        if buf is None:
            self.opcode = 0xe9

class Reljump(Area):

    def __init__(self):
        super(Reljump, self).__init__()
        # The target for this jump. The target needs to implement a
        # compute_rel_offset function described below.
        self._target = None
        self.shrink()

    def _rel_offset(self):
        """Compute the offset to use in the jump based on the target."""
        # If we don't know where we are or what the target is, we can't
        # compute the offset.
        if self.placed_offset is None or self._target is None:
            return None
        # The jump instruction offset is relative to the next instruction.
        next_inst = self.placed_offset + ReljumpInstStruct.struct_len
        # The "compute_rel_offset" function computes a relative offset from
        # the address passed in as its one parameter.
        return self._target.compute_rel_offset(next_inst)

    def target_marker(self):
        """Returns an Area who's placement will be the target of the jump. It
           will have no size itself, so placing it immediately before another
           Area will effectively make the jump go to the start of that area.
        """
        class ReljumpTarget(Area):
            def __init__(self):
                super(ReljumpTarget, self).__init__()
                self.shrink()
            def compute_rel_offset(self, source):
                """The relative offset is just our position minus the other."""
                return self.placed_offset - source

        assert self._target is None
        self._target = ReljumpTarget()
        return self._target

    def compute_min_size_content(self):
        return ReljumpInstStruct.struct_len

    def write(self):
        assert self._target is not None
        inst = ReljumpInstStruct()
        inst.offset = self._rel_offset()
        return inst.pack()

    def log_get_additional_properties(self):
        rel_offset = self._rel_offset()
        # If we can compute an offset, return that as a property.
        if rel_offset is not None:
            return ["offset={:#x}".format(rel_offset)]
        return []
