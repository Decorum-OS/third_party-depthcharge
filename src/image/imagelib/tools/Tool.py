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

import subprocess


class Tool(object):
    def __init__(self, verbose):
        self.verbose = verbose

    def run(self, args, verbose=None):
        if verbose is None:
            verbose = self.verbose
        p = subprocess.Popen(args, stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)
        if verbose:
            print ' '.join(args)
        stdout, _ = p.communicate()
        if verbose:
            print stdout
        return p.returncode, stdout
