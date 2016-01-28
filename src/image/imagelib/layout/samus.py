import argparse

from imagelib.components import Area
from imagelib.components import Cbfs
from imagelib.components import CbfsPayload
from imagelib.components import File
from imagelib.components import Fmap
from imagelib.components import Fwid
from imagelib.components import Gbb
from imagelib.components import Index
from imagelib.components import Sha256
from imagelib.components import Vblock
from imagelib.util import MB
from imagelib.util import KB

class Image(Area):
    model = "Google_Samus"

    def build_rw(self, fmap, suffix):
        verified_rw = Index(
            self.dc_bin, Sha256(self.ec_bin), Sha256(self.pd_bin),
            self.ramstage, self.refcode
        )

        fmap = self.fmap

        return fmap.section("RW_SECTION" + suffix,
            fmap.section("VBLOCK" + suffix,
                Vblock(verified_rw).expand()
            ).size(64 * KB),
            fmap.section("FW_MAIN" + suffix, verified_rw).expand(),
            Area(
                fmap.section("PD_MAIN" + suffix,
                    Index(self.pd_bin)
                ).size(64 * KB),
                fmap.section("EC_MAIN" + suffix,
                    Index(self.ec_bin)
                ).expand(),
                fmap.section("RW_FWID" + suffix,
                    Fwid(self.model)
                ).size(64).fill(0x00)
            ).size(192 * KB)
        ).size(960 * KB)

    def __init__(self, serial, size):
        self.dc_bin = File("depthcharge/depthcharge.payload")
        self.ec_bin = File("ec.RW.bin")
        self.pd_bin = File("samus_pd/ec.RW.bin")
        self.refcode = File("refcode.stage")

        if serial:
            self.coreboot = File("coreboot.rom.serial")
            self.ramstage = File("ramstage.stage.serial")
            self.legacy = File("seabios.cbfs.serial")
        else:
            self.coreboot = File("coreboot.rom")
            self.ramstage = File("ramstage.stage")
            self.legacy = File("seabios.cbfs")

        self.fmap = Fmap(size)
        fmap = self.fmap

        si_all = fmap.section("SI_ALL",
            fmap.section("SI_DESC").size(4 * KB),
            fmap.section("SI_ME").expand()
        ).size(2 * MB)

        si_bios = fmap.section("SI_BIOS",
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
                self.legacy.expand()
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
                        Gbb(hwid="SAMUS TEST 8028").expand()
                    ).expand(),
                    fmap.section("BOOT_STUB",
                        self.coreboot
                    ).size(1 * MB)
                ).expand()
            ).size(2 * MB)
        ).size(6 * MB)

        super(Image, self).__init__(si_all, si_bios)
        self.size(size)


def prepare(layout_args):
    parser = argparse.ArgumentParser()

    parser.add_argument('--serial', dest='serial', action='store_true',
                        default=False, help='Enable serial output')

    parser.add_argument('--size', dest='size', required=True, type=int,
                        help='Size of the image in MB')

    options = parser.parse_args(layout_args)

    return Image(serial=options.serial, size=options.size * MB)
