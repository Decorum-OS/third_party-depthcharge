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

import os

path_seperator = ":"

default_roots = (
    os.path.join("usr", "share", "vboot", "devkeys"),
    os.getcwd()
)
default_path = path_seperator.join(default_roots)

roots = os.getenv("BUILD_IMAGE_PATH", default_path).split(path_seperator)

class File(Area):
    def __init__(self, filename):
        super(File, self).__init__()

        self.filename = filename

        # Figure out where the file is.
        paths = [os.path.join(root, filename) for root in roots]
        hits = [path for path in paths if os.path.isfile(path)]
        if len(hits) == 0:
            raise RuntimeError("File %s not found" % filename)
        elif len(hits) > 1:
            print "Warning: More than one %s found" % filename
            print hits

        # Read its data.
        with open(hits[0], "rb") as f:
            self._data = f.read()

        # Our size was determined by what was in the file.
        self.size(len(self._data))

    def write(self):
        return self._data
