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

from Tool import Tool

import re

class ChromeosVersion(Tool):
    def __init__(self, verbose=False):
        super(ChromeosVersion, self).__init__(verbose)

    def get(self):
        ret, stdout = self.run(["bash", "-c", "chromeos_version.sh"])
        if ret != 0:
            raise RuntimeError("chromeos_version.sh failed:\n%s" % stdout)

        pairs = re.findall(r"([A-Za-z_]*)=(.*)", stdout)
        version_vars = {}
        for pair in pairs:
            version_vars[pair[0]] = pair[1]
        return version_vars
