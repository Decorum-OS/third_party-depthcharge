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

import os
import tempfile

class Gbb(DerivedArea):
    DevScreenShortDelay = 0x00000001
    LoadOptionRoms = 0x00000002
    EnableAlternateOs = 0x00000004
    ForceDevSwitchOn = 0x00000008
    ForceDevBootUsb = 0x00000010
    DisableFwRollbackCheck = 0x00000020
    EnterTriggersTonorm = 0x00000040
    ForceDevBootLegacy = 0x00000080
    FaftKeyOverride = 0x00000100
    DisableEcSoftwareSync = 0x00000200
    DefaultDevBootLegacy = 0x00000400
    DisablePdSoftwareSync = 0x00000800
    ForceDevBootFastbootFullCap = 0x00002000


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
        gbb, gbbp = tempfile.mkstemp()
        bmpfv, bmpfvp = tempfile.mkstemp()
        rootkey, rootkeyp = tempfile.mkstemp()
        recoverykey, recoverykeyp = tempfile.mkstemp()
        gbb, bmpfv, rootkey, recoverykey = \
            (os.fdopen(f, "w+b") for f in (gbb, bmpfv, rootkey, recoverykey))

        gbb_utility = GbbUtility()
        gbb_utility.create([0x100, 0x1000, self.placed_size - 0x2180, 0x1000],
                           gbbp)

        bmpfv.write(self._bmpfv.write())
        rootkey.write(self._rootkey.write())
        recoverykey.write(self._recoverykey.write())
        for f in bmpfv, rootkey, recoverykey:
            f.close()
        gbb_utility.set(self._hwid, self._flags, bmpfvp,
                        rootkeyp, recoverykeyp)

        buf = gbb.read()
        gbb.close()

        for path in gbbp, bmpfvp, rootkeyp, recoverykeyp:
            os.remove(path)

        return buf
