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

import argparse
import os

from imagelib.components.Area import Area
from imagelib.components.Cbfs import Cbfs, CbfsPayload, CbfsFile
from imagelib.components.File import File, PartialFile
from imagelib.components.Fmap import Fmap
from imagelib.components.Fwid import Fwid
from imagelib.components.Gbb import Gbb
from imagelib.components.Ifd import Ifd
from imagelib.components.Index import Index
from imagelib.components.Sha256 import Sha256
from imagelib.components.Vblock import Vblock
from imagelib.util import MB
from imagelib.util import KB

class VerifiedRw(Index):
    """The stuff in the RW section that gets verified"""
    def __init__(self, paths):
        super(VerifiedRw, self).__init__(
            File(paths["dc_bin"]), Sha256(File(paths["ec"])),
            Sha256(File(paths["pd"])), File(paths["ramstage"]),
            File(paths["refcode"])
        )
        self.fill(0xff)

class RwArea(Area):
    """An RW section of the image"""
    def __init__(self, paths, model, fmap, suffix):
        super(RwArea, self).__init__(
            fmap.section("VBLOCK" + suffix,
                Vblock(VerifiedRw(paths))
            ).size(64 * KB),
            fmap.section("FW_MAIN" + suffix, VerifiedRw(paths)).expand(),
            Area(
                fmap.section("PD_MAIN" + suffix,
                    Index(File(paths["pd"]))
                ).size(64 * KB),
                fmap.section("EC_MAIN" + suffix,
                    Index(File(paths["ec"]))
                ).expand(),
                fmap.section("RW_FWID" + suffix,
                    Fwid(model)
                ).size(64).fill(0x00)
            ).size(192 * KB)
        )
        self.expand()

class Image(Area):
    model = "Google_Samus"

    def __init__(self, paths, model, size, gbb_flags=None):
        self.fmap = Fmap(size)
        fmap = self.fmap

        self.si_bios = fmap.section("SI_BIOS",
            Area(
                fmap.section("RW_SECTION_A",
                    RwArea(paths, model, fmap, "_A")
                ).size(960 * KB),
                fmap.section("RW_SECTION_B",
                    RwArea(paths, model, fmap, "_B")
                ).size(960 * KB),
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
                File(paths["legacy"])
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
                        ).size(64).fill(0x00),
                        fmap.section("RO_FRID_PAD").expand().fill(0xff)
                    ).size(4 * KB),
                    fmap.section("GBB",
                        Gbb(hwid="SAMUS TEST 8028", flags=gbb_flags).expand()
                    ).expand(),
                    fmap.section("BOOT_STUB",
                        Cbfs(
                            CbfsPayload(
                                "fallback/payload", File(paths["dc_elf"])
                            ).compression("lzma")
                        ).base(PartialFile(paths["coreboot"], 7 * MB, 1 * MB))
                    ).size(1 * MB)
                ).expand()
            ).size(2 * MB)
        ).size(6 * MB)

        self.si_desc = fmap.section("SI_DESC", File(paths["ifd"])).size(4 * KB)
        self.si_me = fmap.section("SI_ME", File(paths["me"])).expand()

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
        self.fill(0x00)

    def place(self, offset, size):
        super(Image, self).place(offset, size)

        # Manually place the "SI_ALL" fmap region over the IFD descriptor and
        # ME region.
        offset = self.si_desc.placed_offset
        size = self.si_desc.placed_size + self.si_me.placed_size
        self.si_all.place(0, size)


def add_arguments(parser):
    parser.add_argument('--serial', dest='serial', action='store_true',
                        default=False, help='Enable serial output')

    group = parser.add_mutually_exclusive_group()
    group.add_argument('--dev', dest='dev', action='store_true', default=False,
                       help='Enable developer friendly gbb flags')

    group.add_argument('--netboot', dest='netboot', action='store_true',
                       default=False, help='Build a netbooting image')

    parser.add_argument('--size', dest='size', required=True, type=int,
                        help='Size of the image in KB')

    parser.add_argument('--model', dest='model', required=True,
                        help='Model name to use in firmware IDs')

def prepare(options):
    gbb_flags = None
    paths = {
        "dc_bin": "cb_payload.payload",
        "dc_elf": "cb_payload.elf",
        "ec": "ec.RW.bin",
        "pd": os.path.join("samus_pd", "ec.RW.bin"),
        "refcode": "refcode.stage",
        "ifd": "descriptor.bin",
        "me": "me.bin",
        "coreboot": "coreboot.rom",
        "ramstage": "ramstage.stage",
        "legacy": "seabios.cbfs",
    }

    if options.dev or options.netboot:
        gbb_flags = (
            Gbb.DevScreenShortDelay |
            Gbb.ForceDevSwitchOn |
            Gbb.ForceDevBootUsb |
            Gbb.DisableFwRollbackCheck
        )

    if options.dev:
        paths.update({
            "dc_bin": "cb_dev.payload",
            "dc_elf": "cb_dev.elf",
        })

    if options.netboot:
        paths.update({
            "dc_bin": "netboot.payload",
            "dc_elf": "netboot.elf",
        })

    if options.serial:
        paths.update({
            "coreboot": "coreboot.rom.serial",
            "ramstage": "ramstage.stage.serial",
            "legacy": "seabios.cbfs.serial",
        })

    return Image(paths=paths, model=options.model, size=options.size * KB,
                 gbb_flags=gbb_flags)
