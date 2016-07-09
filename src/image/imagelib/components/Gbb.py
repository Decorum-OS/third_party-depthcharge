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
from File import File
from imagelib.tools.GbbUtility import GbbUtility
from imagelib.tools.Tool import FileHarness

import binascii

class Gbb(DerivedArea):
    # The flags in this dictionary will be turned into independent members of
    # the class programatically after the class definition is complete.
    AllFlags = {
        "DevScreenShortDelay": 0x00000001,
        "LoadOptionRoms": 0x00000002,
        "EnableAlternateOs": 0x00000004,
        "ForceDevSwitchOn": 0x00000008,
        "ForceDevBootUsb": 0x00000010,
        "DisableFwRollbackCheck": 0x00000020,
        "EnterTriggersTonorm": 0x00000040,
        "ForceDevBootLegacy": 0x00000080,
        "FaftKeyOverride": 0x00000100,
        "DisableEcSoftwareSync": 0x00000200,
        "DefaultDevBootLegacy": 0x00000400,
        "DisablePdSoftwareSync": 0x00000800,
        "ForceDevBootFastbootFullCap": 0x00002000,
    }


    @staticmethod
    def hwid(hwid):
        checksum = (binascii.crc32(hwid) & 0xffffffff) % 10000
        return "%s %d" % (hwid, checksum)

    def __init__(self, hwid, flags=None, bmpfv=None,
                 rootkey=None, recoverykey=None):
        self._hwid = hwid
        if flags is None:
            flags = 0
        self._flags = flags
        if bmpfv is None:
            bmpfv = File("bmpblk.bin")
        self._bmpfv = bmpfv
        if rootkey is None:
            rootkey = File("root_key.vbpubk")
        self._rootkey = rootkey
        if recoverykey is None:
            recoverykey = File("recovery_key.vbpubk")
        self._recoverykey = recoverykey

        self._data = None

        super(Gbb, self).__init__(bmpfv, rootkey, recoverykey)

    def write(self):
        gbb_utility = GbbUtility()
        with FileHarness(None, self._bmpfv.write(), self._rootkey.write(),
                         self._recoverykey.write()) as files:
            gbb, bmpfv, rootkey, recoverykey = files
            sizes = [0x100, 0x1000, self.placed_size - 0x2180, 0x1000]
            gbb_utility.create(sizes, gbb)
            gbb_utility.set(self._hwid, self._flags, bmpfv,
                            rootkey, recoverykey)

            with open(gbb, "rb") as data:
                return data.read()

    def log_get_additional_properties(self):
        props = ["hwid=\"%s\"" % self._hwid]
        props.extend([name for name, val in Gbb.AllFlags.iteritems()
                      if val & self._flags])
        return props

    def log_area_content(self, indent):
        child_areas = {
            "Bitmap block": self._bmpfv,
            "Root key": self._rootkey,
            "Recovery key": self._recoverykey,
        }
        return self.log_child_props(indent, child_areas)

for name, val in Gbb.AllFlags.iteritems():
    setattr(Gbb, name, val)
