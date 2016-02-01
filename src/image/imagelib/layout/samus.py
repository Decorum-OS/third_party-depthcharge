import argparse
import os

from imagelib.components import Area
from imagelib.components import Cbfs
from imagelib.components import CbfsPayload
from imagelib.components import File
from imagelib.components import Fmap
from imagelib.components import Fwid
from imagelib.components import Gbb
from imagelib.components import Ifd
from imagelib.components import Index
from imagelib.components import Sha256
from imagelib.components import Vblock
from imagelib.util import MB
from imagelib.util import KB

class Image(Area):
    model = "Google_Samus"

    def build_rw(self, fmap, suffix):
        verified_rw = Index(
            File(self.dc_bin), Sha256(File(self.ec_bin)),
            Sha256(File(self.pd_bin)), File(self.ramstage),
            File(self.refcode)
        )

        fmap = self.fmap

        return fmap.section("RW_SECTION" + suffix,
            fmap.section("VBLOCK" + suffix,
                Vblock(verified_rw).expand()
            ).size(64 * KB),
            fmap.section("FW_MAIN" + suffix, verified_rw).expand(),
            Area(
                fmap.section("PD_MAIN" + suffix,
                    Index(File(self.pd_bin))
                ).size(64 * KB),
                fmap.section("EC_MAIN" + suffix,
                    Index(File(self.ec_bin))
                ).expand(),
                fmap.section("RW_FWID" + suffix,
                    Fwid(self.model)
                ).size(64).fill(0x00)
            ).size(192 * KB)
        ).size(960 * KB)

    def __init__(self, serial, size, gbb_flags=None):
        self.dc_bin = os.path.join("depthcharge", "depthcharge.payload")
        self.ec_bin = "ec.RW.bin"
        self.pd_bin = os.path.join("samus_pd", "ec.RW.bin")
        self.refcode = "refcode.stage"
        priv_dir = os.path.join("coreboot-private", "3rdparty", "blobs",
                                "mainboard", "google", "samus")
        self.ifd = os.path.join(priv_dir, "descriptor.bin")
        self.me = os.path.join(priv_dir, "me.bin")

        if serial:
            self.coreboot = "coreboot.rom.serial"
            self.ramstage = "ramstage.stage.serial"
            self.legacy = "seabios.cbfs.serial"
        else:
            self.coreboot = "coreboot.rom"
            self.ramstage = "ramstage.stage"
            self.legacy = "seabios.cbfs"

        self.fmap = Fmap(size)
        fmap = self.fmap

        self.si_bios = fmap.section("SI_BIOS",
            Area(
                self.build_rw(fmap, "_A"),
                self.build_rw(fmap, "_B"),
                fmap.section("RW_MRC_CACHE").size(64 * KB).fill(0xff),
                fmap.section("RW_ELOG").size(16 * KB).fill(0xff),
                fmap.section("RW_SHARED",
                    fmap.section("SHARED_DATA").size(8 * KB).fill(0x00),
                    fmap.section("VBLOCK_DEV").size(8 * KB).fill(0xff)
                ).size(16 * KB),
                fmap.section("RW_VPD").size(8 * KB).fill(0xff),
                fmap.section("RW_UNUSED").expand().fill(0xff),
            ).size(2 * MB),

            fmap.section("RW_LEGACY",
                File(self.legacy)
            ).size(2 * MB),

            fmap.section("WP_RO",
                Area(
                    fmap.section("RO_VPD").size(16 * KB).fill(0xff),
                    fmap.section("RO_UNUSED").expand().fill(0xff)
                ).size(64 * KB),
                fmap.section("RO_SECTION",
                    Area(
                        fmap.section("FMAP", fmap).size(2 * KB),
                        fmap.section("RO_FRID",
                            Fwid(self.model)
                        ).size(64).fill(0),
                        fmap.section("RO_FRID_PAD").expand().fill(0xff)
                    ).size(4 * KB),
                    fmap.section("GBB",
                        Gbb(hwid="SAMUS TEST 8028", flags=gbb_flags).expand()
                    ).expand(),
                    fmap.section("BOOT_STUB",
                        #self.coreboot
                    ).size(1 * MB)
                ).expand()
            ).size(2 * MB)
        ).size(6 * MB)

        self.si_desc = fmap.section("SI_DESC", File(self.ifd)).size(4 * KB)
        self.si_me = fmap.section("SI_ME", File(self.me)).expand()

        # This part of the FMAP is odd because it covers two parts of the IFD
        # but not the whole thing. It doesn't sit inside the IFD because that
        # would imply it sits within one of it's regions, and it doesn't sit
        # outside of the IFD because it doesn't cover all of it. It will have
        # to be handled specially in the Image's "place" function.
        self.si_all = fmap.section("SI_ALL")

        ifd = Ifd(self.si_desc).expand()
        ifd.region("me", self.si_me)
        ifd.region("bios", self.si_bios)

        super(Image, self).__init__(ifd)
        self.size(size)

    def place(self, offset, size):
        super(Image, self).place(offset, size)

        # Manually place the "SI_ALL" fmap region over the IFD descriptor and
        # ME region.
        offset = self.si_desc.placed_offset
        size = self.si_desc.placed_size + self.si_me.placed_size
        self.si_all.place(0, size)


def prepare(layout_args):
    parser = argparse.ArgumentParser()

    parser.add_argument('--serial', dest='serial', action='store_true',
                        default=False, help='Enable serial output')

    parser.add_argument('--size', dest='size', required=True, type=int,
                        help='Size of the image in MB')

    parser.add_argument('--dev-gbb', dest='dev_gbb', action='store_true',
                        default=False,
                        help='Enable developer friendly gbb flags')

    options = parser.parse_args(layout_args)

    gbb_flags = None
    if options.dev_gbb:
        gbb_flags = (
            Gbb.DevScreenShortDelay |
            Gbb.ForceDevSwitchOn |
            Gbb.ForceDevBootUsb |
            Gbb.DisableFwRollbackCheck
        )

    return Image(serial=options.serial,
                 size=options.size * MB,
                 gbb_flags=gbb_flags)
