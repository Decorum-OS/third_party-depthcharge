from Area import Area
from File import File
from imagelib.tools import VbutilFirmware

import os
import tempfile

class Vblock(Area):
    def __init__(self, to_sign, keyblock=None, signprivate=None, version=1,
                 kernelkey=None, flags=None):
        super(Vblock, self).__init__()
        self._to_sign = to_sign
        if keyblock is None:
            keyblock = File("firmware.keyblock")
        self._keyblock = keyblock
        if signprivate is None:
            signprivate = File("firmware_data_key.vbprivk")
        self._signprivate = signprivate
        self._version = version
        if kernelkey is None:
            kernelkey = File("kernel_subkey.vbpubk")
        self._kernelkey = kernelkey
        self._flags = flags

    def write(self):
        vblock, vblockp = tempfile.mkstemp()
        to_sign, to_signp = tempfile.mkstemp()
        keyblock, keyblockp = tempfile.mkstemp()
        signprivate, signprivatep = tempfile.mkstemp()
        kernelkey, kernelkeyp = tempfile.mkstemp()
        vblock, to_sign, keyblock, signprivate, kernelkey = (
            os.fdopen(f, "w+b") for f in (
                vblock, to_sign, keyblock, signprivate, kernelkey
            )
        )

        to_sign.write(self._to_sign.write())
        keyblock.write(self._keyblock.write())
        signprivate.write(self._signprivate.write())
        kernelkey.write(self._kernelkey.write())
        for f in to_sign, keyblock, signprivate, kernelkey:
            f.close()
        vbutil = VbutilFirmware()
        vbutil.vblock(vblockp, keyblockp, signprivatep, self._version, to_signp,
                      kernelkeyp, self._flags)

        buf = vblock.read()
        vblock.close()

        for path in vblockp, to_signp, keyblockp, signprivatep, kernelkeyp:
            os.remove(path)
        return buf
