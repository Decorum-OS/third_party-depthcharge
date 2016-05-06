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

from Tool import Tool

class FspBct(Tool):
    cmd = "bct"

    def __init__(self, verbose=False):
        super(FspBct, self).__init__(verbose)

    def relocate(self, original, relocated, address):
        args = [self.cmd, "--bin", original, "--reloc", "%#x" % address,
                "--bout", relocated]
        ret, stdout = self.run(args)
        if ret != 0:
            raise RuntimeError("FSP relocation failed:\n%s" % stdout)

    def get_base(self, image):
        args = [self.cmd, "--bin", image, "--reloc"]
        ret, stdout = self.run(args)
        if ret != 0:
            raise RuntimeError("Getting the FSP base failed:\n%s" % stdout)
        return int(stdout, base=16)
