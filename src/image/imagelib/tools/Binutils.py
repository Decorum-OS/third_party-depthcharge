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

import os

from Tool import Tool

class GccLd(Tool):
    default_cc = "gcc"

    def __init__(self, verbose=False):
        super(GccLd, self).__init__(verbose)
        self.cc = os.getenv("CC", self.default_cc)
        self.link_flags = os.getenv("LINK_FLAGS", "").split()

    def link(self, output, *args):
        command = [self.cc, "-o", output]
        command.extend(self.link_flags)
        command.extend(args)
        ret, stdout = self.run(command)
        if ret != 0:
            raise RuntimeError("Linking with gcc failed:\n%s" % stdout)

class Objcopy(Tool):
    default_objcopy = "objcopy"

    def __init__(self, verbose=False):
        super(Objcopy, self).__init__(verbose)
        self.objcopy = os.getenv("OBJCOPY", self.default_objcopy)

    def copy(self, source, dest, *args):
        command = [self.objcopy, source, dest]
        command.extend(args)
        ret, stdout = self.run(command)
        if ret != 0:
            raise RuntimeError("Objcopy failed:\n%s" % stdout)
