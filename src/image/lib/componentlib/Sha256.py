from Area import Area

import hashlib

class Sha256(Area):
  def __init__(self, data):
    super(Sha256, self).__init__()
    self._data = data
    self.size(256 / 8)

  def write(self):
    hasher = hashlib.sha256()
    hasher.update(self._data.write())
    return hasher.digest()
