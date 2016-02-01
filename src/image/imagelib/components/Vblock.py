from Area import Area
from File import File
from imagelib.tools.VbutilFirmware import VbutilFirmware

import os
import tempfile

class Vblock(Area):
    def __init__(self, to_sign, keyblock=None, signprivate=None, version=1,
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

        self._to_sign = to_sign
        self._version = version
        self._flags = flags
        self._data = None

        super(Vblock, self).__init__(keyblock, signprivate, kernelkey, to_sign)

        self.shrink()

    def _generate_vblock(self):
        for child in self.children:
            child.place(0, child.computed_min_size)

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

        self._data = vblock.read()
        vblock.close()

        for path in vblockp, to_signp, keyblockp, signprivatep, kernelkeyp:
            os.remove(path)

    def compute_min_size_content(self):
        if not self._data:
            self._generate_vblock()
        return len(self._data)

    def place_children(self):
        pass

    def write(self):
        return self._data
