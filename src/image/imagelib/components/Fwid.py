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

from Area import Area
from imagelib.tools.ChromeosVersion import ChromeosVersion

class Fwid(Area):
    _versions = None
    def __init__(self, model):
        super(Fwid, self).__init__()
        if not self._versions:
            cv = ChromeosVersion()
            self._versions = cv.get()
        self._version = model + "." + self._versions["CHROMEOS_VERSION_STRING"]
        self.size(len(self._version))

    def write(self):
        return self._version

    def log_area_content(self, indent):
        return self.log_wrap(indent, "\"" + self._version + "\"")
