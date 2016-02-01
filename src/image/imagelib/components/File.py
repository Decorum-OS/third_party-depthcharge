from Area import Area

import os

roots = ["/build/samus/firmware/", "/usr/share/vboot/devkeys/"]

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