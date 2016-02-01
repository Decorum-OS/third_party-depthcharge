from Area import Area

import hashlib

class Sha256(Area):
    def __init__(self, data):
        super(Sha256, self).__init__(data)
        self._data = data
        self.size(256 / 8)

    def compute_min_size_content(self):
        return 0

    def place_children(self):
        for child in self.children:
            child.place(0, child.computed_min_size)

    def write(self):
        hasher = hashlib.sha256()
        hasher.update(self._data.write())
        return hasher.digest()
