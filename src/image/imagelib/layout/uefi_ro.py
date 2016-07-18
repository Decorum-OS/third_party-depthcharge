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

from imagelib.components.Dcdir import Region, RootDirectory
from imagelib.components.Fwid import Fwid
from imagelib.components.Gbb import Gbb
from imagelib.util import KB, MB, GB

class Image(RootDirectory):
    def __init__(self, model, hwid, gbb_flags=None):
        super(Image, self).__init__(
            Region("GBB",
                Gbb(hwid=Gbb.hwid(hwid), flags=gbb_flags).expand()
            ).size(1 * MB),
            Region("FWID", Fwid(model)).shrink(),
        )

        self.shrink()

def add_arguments(parser):
    group = parser.add_mutually_exclusive_group()
    group.add_argument('--dev', dest='dev', action='store_true', default=False,
                       help='Enable developer friendly gbb flags')

    parser.add_argument('--model', dest='model', required=True,
                        help='Model name to use in firmware IDs')

    parser.add_argument('--hwid', dest='hwid', required=True,
                        help='Hardware ID to put in the GBB')

def prepare(options):
    gbb_flags = None
    if options.dev:
        gbb_flags = (
            Gbb.DevScreenShortDelay |
            Gbb.ForceDevSwitchOn |
            Gbb.ForceDevBootUsb |
            Gbb.DisableFwRollbackCheck
        )

    return Image(model=options.model, gbb_flags=gbb_flags, hwid=options.hwid)
