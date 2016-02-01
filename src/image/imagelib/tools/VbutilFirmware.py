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

class VbutilFirmware(Tool):
    cmd = "vbutil_firmware"

    def __init__(self, verbose=False):
        super(VbutilFirmware, self).__init__(verbose)

    def vblock(self, name, keyblock, signprivate, version, fv, kernelkey,
               flags=None):
        args = [self.cmd, "--vblock", name, "--keyblock", keyblock,
                "--signprivate", signprivate, "--version", "%d" % version,
                "--fv", fv, "--kernelkey", kernelkey]
        if flags is not None:
            args.extend(["--flags", "%d" % flags])
        ret, stdout = self.run(args)
        if ret != 0:
            raise RuntimeError("vbutil_firmware failed:\n%s" % stdout)
