from Area import Area
from imagelib.util import Buffer
from imagelib.util import CStruct

import copy

class Ifd(Area):
    class Region(object):
        def __init__(self, tag, description, fixed=False):
            self.tag = tag
            self.description = description
            self.item = None
            self.fixed = fixed

    regions_template = (
        Region("fd", "Flash descriptor", fixed=True),
        Region("bios", "BIOS"),
        Region("me", "Management engine"),
        Region("gbe", "Gigabit ethernet firmware"),
        Region("pd", "Platform data"),
        Region("res1", "Reserved"),
        Region("res2", "Reserved"),
        Region("res3", "Reserved"),
        Region("ec", "Embedded controller")
    )

    region_map = { region.tag: idx for idx, region in
                   enumerate(regions_template) }

    MaxRegions = len(regions_template)

    def __init__(self, descriptor):
        self.descriptor = descriptor
        self.regions = copy.deepcopy(Ifd.regions_template)

        super(Ifd, self).__init__(descriptor)

    def region(self, tag, item):
        # Find the region of interest.
        if tag not in self.region_map:
            raise KeyError("Unknown firmware descriptor region %s." % tag)
        region = self.regions[self.region_map[tag]]

        # Make sure writing to it is sensible.
        if region.fixed:
            raise ValueError("Firmware descriptor region %s is fixed." % tag)
        # TODO check whether the region has any space in it.

        if region.item:
            raise ValueError("Region '%s' already has an item assigned." % tag)

        # Record that this region is supposed to be filled with "item".
        region.item = item

        self.add_child(item)
        return self

    def place_children(self):
        if self.placed_offset != 0:
            raise ValueError("The intel firmware descriptor must be at the" +
                             "start of the image.")

        # Place the descriptor blob.
        self.descriptor.place(0, self.descriptor.computed_min_size)
        buf = self.descriptor.write()

        # Find the actual descriptor data structure.
        fd_offset = next((offset for offset in xrange(0, len(buf), 4) if
                          buf[offset:offset + 4] == "\x5a\xa5\xf0\x0f"), None)
        if fd_offset is None:
            raise ValueError("No firmware descriptor found in the " +
                             "descriptor blob")

        # Unpack it.
        fd_buf = buf[fd_offset:fd_offset + FlashDescriptor.struct_len]
        fd = FlashDescriptor(fd_buf)

        # Locate the regions data structure and unpack it.
        region_offset = ((fd.flmap0 >> 16) & 0xff) << 4
        region_buf = buf[region_offset:region_offset + Regions.struct_len]
        regions = Regions(region_buf)

        # For regions which have data assigned to them, figure out where they
        # go in the image and "place" it there.
        for idx, region in enumerate(self.regions):
            if not region.item:
                continue
            region_val = getattr(regions, "flreg%d" % idx)
            base = (region_val & 0x7fff) << 12
            limit = ((region_val & 0x7fff0000) >> 4) | 0xfff
            size = limit - base + 1
            if size <= 0:
                raise ValueError("Can't put data into unused section %s (%s)" %
                                 (region.tag, region.description))
            region.item.place(base, size)


class FlashDescriptor(CStruct):
    struct_members = (
        ("L", "flvalsig"),
        ("L", "flmap0"),
        ("L", "flmap1"),
        ("L", "flmap2"),
        ("%dx" % (0xefc - 0x20), ""), # Reserved
        ("L", "flumap1")
    )

class Regions(CStruct):
    struct_members = (
        list(("L", "flreg%d" % num) for num in range(Ifd.MaxRegions))
    )

class Components(CStruct):
    struct_members = (
        ("L", "flcomp"),
        ("L", "flill"),
        ("L", "flpb")
    )

_num_straps = 18
class PchStraps(CStruct):
    num_straps = _num_straps
    struct_members = (
        list(("L", "pchstrp%d" % num) for num in range(_num_straps))
    )

_max_flmstr = 5
class Master(CStruct):
    max_flmstr = _max_flmstr
    struct_members = (
        list(("L", "flmstr%d" % num) for num in range(1, _max_flmstr))
    )

class ProcessorStrap(CStruct):
    struct_members = (
        list(("L", "data%d" % num) for num in range(8))
    )

class MeVscc(CStruct):
    struct_members = (
        ("L", "jid"),
        ("L", "vscc")
    )
