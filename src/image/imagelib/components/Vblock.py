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
from File import File
from imagelib.tools.Tool import FileHarness
from imagelib.tools.VbutilFirmware import VbutilFirmware

class Vblock(DerivedArea):
    def __init__(self, keyblock=None, signprivate=None, version=1,
                 kernelkey=None, flags=None):

        if keyblock is None:
            keyblock = File("firmware.keyblock")
        self._keyblock = keyblock

        if signprivate is None:
            signprivate = File("firmware_data_key.vbprivk")
        self._signprivate = signprivate

        if kernelkey is None:
            kernelkey = File("kernel_subkey.vbpubk")
        self._kernelkey = kernelkey

        self._version = version
        self._flags = flags
        self._data = None

        super(Vblock, self).__init__(keyblock, signprivate, kernelkey)

        self._to_sign = None

        self.shrink()

    def signed(self, *args, **kwargs):
        class SignedArea(Area):
            """Area which caches its write data."""
            def write(self):
                if not hasattr(self, "_vblock_signed_data"):
                    self._vblock_signed_data = super(SignedArea, self).write()
                return self._vblock_signed_data

        assert self._to_sign is None
        self._to_sign = SignedArea(*args, **kwargs)
        return self._to_sign

    def _generate_vblock(self, to_sign):
        vbutil = VbutilFirmware()
        with FileHarness(None, to_sign, self._keyblock.write(),
                         self._signprivate.write(),
                         self._kernelkey.write()) as files:
            vblock, to_sign, keyblock, signprivate, kernelkey = files
            vbutil.vblock(vblock, keyblock, signprivate, self._version,
                          to_sign, kernelkey, self._flags)

            with open(vblock, "rb") as data:
                return data.read()

    def compute_min_size_content(self):
        if not self.handle_children():
            # Generate a dummy vblock which signs nothing to see how big the
            # file is.
            self._dummy_vblock = self._generate_vblock("test")
        return len(self._dummy_vblock)

    def write(self):
        if self._data is None:
            self._data = self._generate_vblock(self._to_sign.write())
        return self._data

    def log_get_additional_properties(self):
        props = ["version={}".format(self._version)]
        if self._flags is not None:
            props.append("flags={}".format(self._flags))
        return props

    def log_area_content(self, indent):
        child_props = {
            "Key block": self._keyblock,
            "Private key": self._signprivate,
            "Kernel key": self._kernelkey,
        }
        if self._to_sign:
            child_props["Signed data"] = self._to_sign

        return self.log_child_props(indent, child_props)
