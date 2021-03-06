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

from Area import Area, DerivedArea
from imagelib.tools.Binutils import GccLd, Objcopy
from imagelib.tools.Tool import FileHarness

_linker_script_template = """
SECTIONS {{
	. = {base:#x};
	.load : {{ *(.load) }}
	.noload : {{ *(.noload) }}

	/DISCARD/ : {{ *(*) }}
}}
"""

class Xip(DerivedArea):
    """An area which does a final link of a partially linked object so that
       it can be executed in place from a memory mapped image.
    """

    def __init__(self, data):
        """data is the partially linked object (probably a file) which will
           be linked to execute in place based on its position in the image.
        """
        super(Xip, self).__init__(data)
        self._data = data
        self._image_base = 0
        self._extra_symbols = {}
        self.shrink()

    def symbols_append(self, name, value):
        """When linking, set symbol "name" to the value "value"."""
        self._extra_symbols[name] = value

    def symbols_extend(self, symbols):
        """When linking, set symbols using the dict "symbols". The key in the
           dict is the name of the symbol, and the value is the value mapped
           to by "name".
        """
        for name, value in symbols.iteritems():
            self.symbols_append(name, value)

    def symbols_add(self, **kwargs):
        """When linking, set symbols whos names are the name of keyword
           arguments to this function, and whos values are what those
           arguments are set to.
        """
        self.symbols_extend(kwargs)

    def image_base(self, new_image_base):
        """Set the address the base of the image is mapped to."""
        self._image_base = new_image_base
        return self

    def compute_min_size_content(self):
        self.handle_children()
        objcopy = Objcopy()
        # Assume the final linking of the object won't change its size.
        with FileHarness(None, self._data.write()) as [binary, partial]:
            objcopy.copy(partial, binary, "-O", "binary")
            with open(binary, "rb") as data:
                return len(data.read())

    def write(self):
        linker_script = _linker_script_template.format(
                base=(self.placed_offset + self._image_base))
        objcopy = Objcopy()
        gcc = GccLd()
        with FileHarness(None, None, linker_script,
                         self._data.write()) as files:
            binary, elf, script, partial = files
            # Do the final link to prepare the image for its new home.
            args = [partial, "-T", script, "-Wl,--no-gc-sections"]
            # Add in the extra symbols we've been supplied.
            defsym_template = "-Wl,--defsym={name}={value:#x}"
            for name, value in self._extra_symbols.iteritems():
                args.append(defsym_template.format(name=name, value=value))
            gcc.link(elf, *args)

            # Convert it to a flat binary.
            objcopy.copy(elf, binary, "-O", "binary")
            with open(binary, "rb") as data:
                return data.read()

    def log_get_additional_properties(self):
        if self._image_base != 0:
            return ["image_base={}".format(self._image_base)]
        return []

    def log_area_content(self, indent):
        child_props = {
            "Original object": self._data
        }

        return self.log_child_props(indent, child_props)
