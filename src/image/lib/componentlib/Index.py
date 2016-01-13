from Area import Area
from util import CStruct

class IndexHeader(CStruct):
  struct_members = (
    ("L", "count"),
  )

class IndexEntry(CStruct):
  struct_members = (
    ("L", "offset"),
    ("L", "size")
  )

class Index(Area):
  def __init__(self, *args):
    super(Index, self).__init__(*args)
    size = IndexHeader.struct_len + len(args) * IndexEntry.struct_len
    self.index_size = size
    for item in self._items:
      size += item._size
      size = ((size + 3) & ~3)
    self.size(size)

  def place(self, offset, size):
    return super(Index, self).place(offset + self.index_size,
				    size - self.index_size)

  def write(self):
    header = IndexHeader()
    header.count = len(self._items)
    index = header.pack()

    data = ""
    offset = self.index_size
    for item in self._items:
      entry = IndexEntry()
      entry.offset = offset
      entry.size = item.placed_size
      pad_len = ((entry.size + 3) & ~3) - entry.size
      offset += entry.size + pad_len
      index += entry.pack()
      data += item.write()
      data += chr(0xff) * pad_len

    return index + data
