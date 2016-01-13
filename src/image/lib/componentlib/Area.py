from util import pad_buf

import struct

class Area(object):
  def __init__(self, *args):
    super(Area, self).__init__()
    self._expand = False
    self._fill = None
    self._items = args
    self._size = 0

  def expand(self):
    self._expand = True
    return self

  def fill(self, fill):
    self._fill = fill
    return self

  def size(self, size):
    if self._size is not None and size < self._size:
      raise ValueError("Attempted to decrease size from %d to %d" %
		       (self._size, size))
    self._size = size
    return self

  def build(self):
    self.place(0, self._size)
    return self.write()

  def place(self, offset, size):
    self.placed_offset = offset
    self.placed_size = size

    start = []
    end = []
    expanding = None
    current = start
    used = 0
    for item in self._items:
      if item._expand:
        expanding = item
        current = end
      else:
        current.append(item)
        used += item._size

    sizes = []
    for item in start:
      sizes.append(item._size)
    if expanding:
      sizes.append(size - used)
    for item in end:
      sizes.append(item._size)

    pos = offset
    for item, size in zip(self._items, sizes):
      item.place(pos, size)
      pos += size

  def write(self):
    fill = 0xff
    if self._fill is not None:
      fill = self._fill
    buf = ""
    for item in self._items:
      rel_offset = item.placed_offset - self.placed_offset
      new_buf = item.write()
      buf += pad_buf(new_buf, item.placed_size, fill)
    buf = pad_buf(buf, self.placed_size, fill)
    return buf
