from Area import Area
from util import CStruct

class FmapHeader(CStruct):
  struct_members = (
    ("8s", "signature"),
    ("B", "major"),
    ("B", "minor"),
    ("Q", "base"),
    ("L", "size"),
    ("32s", "name"),
    ("H", "nareas")
  )

class FmapArea(CStruct):
  struct_members = (
    ("L", "offset"),
    ("L", "size"),
    ("32s", "name"),
    ("H", "flags")
  )

class Section(Area):
  def __init__(self, label, *args):
    super(Section, self).__init__(*args)
    self._label = label
    self._flags = 0

  def flags(self, flags):
    self._flags = flags
    return self

class Fmap(Area):
  def __init__(self, image_size):
    super(Fmap, self).__init__()
    self.sections = []

    header = FmapHeader()
    header.signature = "__FMAP__"
    header.major = 1
    header.minor = 0
    header.base = 0
    header.size = image_size
    header.name = "FMAP"
    header.nareas = 0
    self.header = header

    self.size(header.struct_len)

  def section(self, name, *args):
    section = Section(name, *args)
    self.sections.append(section)
    self.header.nareas += 1
    self.size(self._size + FmapArea.struct_len)
    return section

  def write(self):
    def compare(a, b):
      res = a.placed_offset - b.placed_offset
      if res:
        return res
      return b.placed_size - a.placed_size

    ordered = sorted(self.sections, cmp=compare)
    fmap = self.header.pack()

    for section in ordered:
      area = FmapArea()
      area.offset = section.placed_offset
      area.size = section.placed_size
      area.name = section._label
      area.flags = section._flags

      fmap += area.pack()

    return fmap
