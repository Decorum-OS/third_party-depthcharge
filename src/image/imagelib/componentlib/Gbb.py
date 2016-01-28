from Area import Area
from File import File
from imagelib.tools import GbbUtility

import os
import tempfile

class Gbb(Area):
    def __init__(self, hwid, flags=None, bmpfv=None,
                 rootkey=None, recoverykey=None):
        super(Gbb, self).__init__()
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

    def place(self, offset, size):
        self.placed_offset = offset
        self.placed_size = size

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
