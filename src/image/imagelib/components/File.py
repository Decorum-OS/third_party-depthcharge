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

def read_file(filename):
    # Figure out where the file is.
    if os.path.isabs(filename):
        paths = [filename]
    else:
        paths = [os.path.join(root, filename) for root in roots]
    hits = [path for path in paths if os.path.isfile(path)]
    if len(hits) == 0:
        raise RuntimeError("File %s not found" % filename)
    elif len(hits) > 1:
        print "Warning: More than one %s found" % filename
        print hits

    # Read its data.
    with open(hits[0], "rb") as f:
        return f.read()

class FileBase(Area):
    def __init__(self, filename, data):
        super(FileBase, self).__init__()
        self.filename = filename
        self.data = data
        self.size(len(data))

    def write(self):
        return self.data

    def log_area_name(self):
        name = super(FileBase, self).log_area_name()
        return name + "[%s]" % self.filename

class File(FileBase):
    def __init__(self, filename):
        data = read_file(filename)
        super(File, self).__init__(filename, data)

class PartialFile(FileBase):
    def __init__(self, filename, start, size):
        self.extra_props = ["start={:#x}".format(start),
                            "size={:#x}".format(size)]

        data = read_file(filename)
        length = len(data)
        if start < 0 or start >= length or start + size > length or size < 0:
            raise ValueError("Start and/or size of partial file are out " +
                             "of bounds")
        super(PartialFile, self).__init__(filename, data[start:start + size])

    def log_get_additional_properties(self):
        return self.extra_props
