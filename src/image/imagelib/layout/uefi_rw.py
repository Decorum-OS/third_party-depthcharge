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

from imagelib.components.Dcdir import Region, Directory, RootDirectory
from imagelib.components.File import File
from imagelib.components.Fwid import Fwid
from imagelib.components.Vblock import Vblock

class Image(RootDirectory):
    def __init__(self, paths, model):
	vblock = Vblock()

        super(Image, self).__init__(
            Region("VBLOCK", vblock).shrink(),
            vblock.signed(
                Directory("VERIFIED",
                    Region("MAIN", File(paths["dc_bin"])).shrink()
                ).shrink()
            ).shrink(),
            Region("FWID", Fwid(model)).shrink()
        )
        self.shrink()


def add_arguments(parser):
    group = parser.add_mutually_exclusive_group()
    group.add_argument('--dev', dest='dev', action='store_true', default=False,
                       help='Enable developer friendly gbb flags')

    parser.add_argument('--model', dest='model', required=True,
                        help='Model name to use in firmware IDs')

def prepare(options):
    gbb_flags = None
    paths = {
        "dc_bin": "uefi_rw.mod",
    }

    if options.dev:
        paths.update({
            "dc_bin": "uefi_dev_rw.mod",
        })

    return Image(paths=paths, model=options.model)
