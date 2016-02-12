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

class Cbfstool(Tool):
    cmd = "cbfstool"

    def __init__(self, base=None, verbose=False):
        super(Cbfstool, self).__init__(verbose)
        self.base = base

    def create(self, name, size, bootblock, arch, align=None, offset=None):
        args = [self.cmd, name, "create", "-s", "%d" % size, "-B", bootblock,
                "-m", arch]
        if align is not None:
            args.extend(["-a", "%d" % align])
        if offset is not None:
            args.extend(["-o", "%d" % offset])
        ret, stdout = self.run(args)
        if ret != 0:
            raise RuntimeError("cbfstool create failed:\n%s" % stdout)
        self.base = name

    def add(self, name, cbfs_name, cbfs_type, base=None):
        args = [self.cmd, self.base, "add", "-f", name, "-n", cbfs_name,
                "-t", str(cbfs_type)]
        if base is not None:
            args.extend(["-b", "%d"])
        ret, stdout = self.run(args)
        if ret != 0:
            raise RuntimeError("cbfstool add failed:\n%s" % stdout)

    def add_payload(self, name, cbfs_name, compression=None, base=None):
        args = [self.cmd, self.base, "add-payload", "-f", name, "-n", cbfs_name]
        if compression is not None:
            args.extend(["-c", compression])
        if base is not None:
            args.extend(["-b", "%d"])
        ret, stdout = self.run(args)
        if ret != 0:
            raise RuntimeError("cbfstool add-payload failed:\n%s" % stdout)
