from Area import Area
from imagelib.util import Buffer
from imagelib.util import CStruct

class IndexHeader(CStruct):
    struct_members = (
        ("L", "count"),
    )

class IndexEntry(CStruct):
    struct_members = (
        ("L", "offset"),
        ("L", "size")
    )

def _align4(offset):
    """Align a value to be a multiple of 4."""
    return (offset + 3) & ~3

class Index(Area):
    def __init__(self, *args):
        super(Index, self).__init__(*args)
        self.shrink()

    def _index_size(self):
        return (IndexHeader.struct_len +
                len(self.children) * IndexEntry.struct_len)

    def compute_min_size_content(self):
        return self._index_size() + sum(_align4(child.computed_min_size) for
                                        child in self.children)

    def place_children(self):
        if any((child.is_expanding() for child in self.children)):
            raise ValueError("An Index can't handle expanding children.")

        pos = self.placed_offset + self._index_size()
        for child in self.children:
            child.place(pos, child.computed_min_size)
            pos += _align4(child.placed_size)

    def write(self):
        buf = Buffer(self)

        header = IndexHeader()
        header.count = len(self.children)
        index = header.pack()

        for child in self.children:
            entry = IndexEntry()
            entry.offset = child.placed_offset - self.placed_offset
            entry.size = child.placed_size
            index += entry.pack()

        buf.inject(index, self.placed_offset, len(index))
        buf.inject_areas(*self.children)

        return buf.data()
