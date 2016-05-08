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
from imagelib.components.Fsp import Fsp
from imagelib.components.Gbb import Gbb
from imagelib.components.Ifd import Ifd
from imagelib.components.Sha256 import Sha256
from imagelib.components.Vblock import Vblock
from imagelib.components.X86 import Reljump
from imagelib.components.Xip import Xip
from imagelib.util import KB, MB, GB

def _dict_to_dir(d, leaf=lambda x: File(x)):
    """Expand a nested dictionary into a dcdir directory."""
    ret = []
    for name, entry in sorted(d.iteritems()):
        if isinstance(entry, dict):
            sub_entries = _dict_to_dir(entry, leaf)
            ret.append(Directory(name, *sub_entries).shrink())
        else:
            ret.append(Region(name, leaf(entry)).shrink())
    return ret

class RwArea(Directory):
    """An RW section of the image"""
    def __init__(self, rw_name, model, signed_files, verified_files):
        vblock = Vblock()

        signed = []
        unsigned = []

        # Signed files are entirely in the area signed by the vblock. They
        # are definitely required for boot, and so loading/hashing them
        # seperately doesn't save any work and makes things more complicated.
        signed += _dict_to_dir(signed_files)

        # Files which are just verified (a slight abuse of terminology) have
        # their hash signed, but the data itself is not loaded or hashed
        # until and if it's needed. This saves a lot of access to the flash
        # and hashing of data if a component might not actually be used on
        # a particular boot.
        signed += _dict_to_dir(verified_files,
                               leaf=lambda x: Sha256(File(x)))
        unsigned += _dict_to_dir(verified_files)

        # Because python won't let us put the FWID after expanding unsigned,
        # we have to lump it into the unsigned list.
        unsigned.append(Region("FWID", Fwid(model)).shrink())

        # Data in an RW area is structured as follows:
        # VBLOCK - a region which contains the vblock
        # VERIFIED - a directory which contains all signed data/hashes. A hash
        #            of a file is stored with the same name as the file itself.
        # [various] - the verified files
        # FWID - the ID of this RW firmware version
        super(RwArea, self).__init__(rw_name,
            Region("VBLOCK", vblock).shrink(),
            vblock.signed(
                Directory("VERIFIED", *signed).shrink()
            ).shrink(),
            *unsigned
        )
        self.expand()

class Image(RootDirectory):
    def __init__(self, paths, model, size, hwid, gbb_flags=None, ecs=[]):
        # The main firmware blob which starts RW execution and the Intel
        # reference code are necessarily used during an RW boot.
        signed = {
            "MAIN": paths["dc_bin"],
        }
        # The EC images (main EC and PD RW firmwares, for instance) are used
        # if those components need to be updated or their RW image has been
        # damaged somehow.
        verified = {
            "EC": {
                str(idx): os.path.join(name, "ec.RW.bin")
                    for idx, name in enumerate(ecs)
            }
        }

        backjump = Reljump()
        fsp = Fsp(File(paths["fsp"]))
        image_base = 4 * GB - size
        xip_entry = Xip(File(paths["entry"])).image_base(image_base)
        dcdir_table = DirectoryTable()

        si_bios = Area(
            Directory("RW",
                Region("MRCCACHE").size(64 * KB),
                Directory("SCRATCH").size(16 * KB),
                Region("VPD").size(8 * KB),
                RwArea("A", model, signed, verified).expand(),
                RwArea("B", model, signed, verified).expand()
            ).size(size / 2),
            dcdir_table,
            Directory("RO",
                Region("GBB",
                    Gbb(hwid=Gbb.hwid(hwid), flags=gbb_flags).expand()
                ).expand(),
                Region("VPD").size(16 * KB),
                Region("FWID", Fwid(model)).shrink(),
                Directory("FIRMWARE",
                    Region("FSP", fsp).shrink(),
                    backjump.target_marker(),
                    Region("ENTRY", xip_entry).shrink()
                ).shrink(),
                Area(backjump).size(0x10).fill(0x00)
            ).expand(),
        ).expand()

        si_me = Region("ME", File(paths["me"])).expand()
        si_desc = Region("IFD_DESC", File(paths["ifd"])).expand()

        ifd = Ifd(si_desc).expand()
        ifd.region("me", si_me)
        ifd.region("bios", si_bios)

        self._dcdir_table = dcdir_table
        self._fsp = fsp
        self._image_base = image_base
        self._xip_entry = xip_entry

        super(Image, self).__init__(ifd)
        self.big_pointer()
        self.size(size)

    def post_place_hook(self):
        # Once we know where the FSP is going to be, prepare to set it up to
        # work at the new location. The actual relocation will happen when
        # the fsp is written into the image.
        fsp_base = self._image_base + self._fsp.placed_offset
        self._fsp.base_address(fsp_base)

        # Once we know where the base dcdir table will be, set a symbol to
        # its address in the real mode entry point.
        anchor_addr = self._image_base + self._dcdir_table.placed_offset
        self._xip_entry.symbols_add(dcdir_anchor_addr=anchor_addr,
                                    rom_image_base=self._image_base)


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

    parser.add_argument('--ecs', dest='ecs', default=None,
                        help=('Images for devices which support ' +
                              'EC software sync'))

    parser.add_argument('--hwid', dest='hwid', required=True,
                        help='Hardware ID to put in the GBB')

def prepare(options):
    gbb_flags = None
    paths = {
        "dc_bin": "cb_payload.payload",
        "entry": "fsp_entry.mod",
        "fsp": "FSP.fd",
        "ifd": "descriptor.bin",
        "me": "me.bin",
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
            "dc_bin": "cb_netboot.payload",
        })

    if options.serial:
        pass

    ecs = options.ecs.split(':') if options.ecs else []
    hwid = options.hwid

    return Image(paths=paths, model=options.model, size=options.size * KB,
                 gbb_flags=gbb_flags, ecs=ecs, hwid=hwid)
