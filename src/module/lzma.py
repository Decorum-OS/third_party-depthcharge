#!/usr/bin/python
#
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

import os
import struct
import subprocess
import sys

def main():
    lsmz = os.environ.get('LZMA', 'lzma')

    old_name = sys.argv[1]
    new_name = sys.argv[2]

    # Run the system's lzma utility.
    p = subprocess.Popen(['lzma', '--stdout', old_name], stdout=subprocess.PIPE)
    new, _ = p.communicate()

    # When lzma is a symlink to xz, it always sets the original size to
    # "unknown", even if the source file was a regular file and it actually
    # knew what the size was. To cover that case, we'll manually overwrite the
    # appropriate field in the LZMA header with the correct uncompressed size.
    new = new[:5] + struct.pack('<Q', os.stat(old_name).st_size) + new[13:]

    with open(new_name, 'w') as f:
        f.write(new)

if __name__ == '__main__':
    main()
