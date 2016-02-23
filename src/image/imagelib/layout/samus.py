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
from imagelib.components.Dcdir import \
    Region, Directory, RootDirectory, DirectoryTable
from imagelib.components.File import File
from imagelib.components.Fwid import Fwid
from imagelib.components.Gbb import Gbb
from imagelib.components.Ifd import Ifd
from imagelib.components.Sha256 import Sha256
from imagelib.components.Vblock import Vblock
from imagelib.util import MB
from imagelib.util import KB

class VerifiedRw(Directory):
    """The stuff in the RW section that gets verified"""
    def __init__(self, paths, name=None):
        super(VerifiedRw, self).__init__(name,
            Region("MAIN", File(paths["dc_bin"])).shrink(),
            Region("EC_HASH", Sha256(File(paths["ec"]))).shrink(),
            Region("PD_HASH", Sha256(File(paths["pd"]))).shrink(),
            Region("RAMSTAGE", File(paths["ramstage"])).shrink(),
            Region("REFCODE", File(paths["refcode"])).shrink()
        )
        self.shrink()

class RwArea(Directory):
    """An RW section of the image"""
    def __init__(self, name, paths, model):
        super(RwArea, self).__init__(name,
            Region("VBLOCK", Vblock(VerifiedRw(paths))).size(64 * KB),
            VerifiedRw(paths, "VERIFIED").expand(),
            Region("PD", File(paths["pd"])).shrink(),
            Region("EC", File(paths["ec"])).shrink(),
            Region("FWID", Fwid(model)).size(64).fill(0x00)
        )
        self.expand()

class Image(RootDirectory):
    model = "Google_Samus"

    def __init__(self, paths, model, size, gbb_flags=None):
        si_bios = Area(
            Directory("RW",
                Directory("LEGACY").size(2 * MB),
                Region("MRCCACHE").size(64 * KB),
                Region("ELOG").size(16 * KB),
                Directory("SCRATCH").size(16 * KB),
                Region("VPD").size(8 * KB),
                RwArea("A", paths, model).expand(),
                RwArea("B", paths, model).expand()
            ).size(size / 2),
            DirectoryTable(),
            Directory("RO",
                Region("GBB",
                    Gbb(hwid="SAMUS TEST 8028", flags=gbb_flags).expand()
                ).expand(),
                Region("VPD").size(16 * KB),
                Region("FWID", Fwid(self.model)).size(64).fill(0x00),
                Directory("BOOTSTUB").size(1 * MB)
            ).expand(),
        ).expand()

        si_me = Region("ME", File(paths["me"])).expand()
        si_desc = Region("IFD_DESC", File(paths["ifd"])).expand()

        ifd = Ifd(si_desc).expand()
        ifd.region("me", si_me)
        ifd.region("bios", si_bios)

        super(Image, self).__init__(ifd)
        self.big_pointer()
        self.size(size)


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
        "ec": "ec.RW.bin",
        "pd": os.path.join("samus_pd", "ec.RW.bin"),
        "refcode": "refcode.stage",
        "ifd": "descriptor.bin",
        "me": "me.bin",
        "ramstage": "ramstage.stage",
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
        })

    if options.netboot:
        paths.update({
            "dc_bin": "netboot.payload",
        })

    if options.serial:
        paths.update({
            "ramstage": "ramstage.stage.serial",
        })

    return Image(paths=paths, model=options.model, size=options.size * KB,
                 gbb_flags=gbb_flags)
