import struct

KB = 2 ** 10
MB = 2 ** 20


# This class and metaclass make it easier to define and access structures
# which live in physical memory. To use them, inherit from CStruct and define
# a class member called struct_members which is a tuple of pairs. The first
# item in the pair is the type format specifier that should be used with
# struct.unpack to read that member from memory. The second item is the name
# that member should have in the resulting object.

class MetaCStruct(type):
    def __init__(cls, name, bases, dct):
        struct_members = dct["struct_members"]
        cls.struct_fmt = "="
        for char, name in struct_members:
            cls.struct_fmt += char
        cls.struct_len = struct.calcsize(cls.struct_fmt)
        super(MetaCStruct, cls).__init__(name, bases, dct)

class CStruct(object):
    __metaclass__ = MetaCStruct
    struct_members = ()

    def __init__(self, buf=None):
        if buf is not None:
            self.unpack(buf)

    def unpack(self, buf):
        values = struct.unpack(self.struct_fmt, buf)
        # Skip "x" members which are padding bytes.
        names = (name for char, name in self.struct_members if 'x' not in char)
        for name, value in zip(names, values):
            setattr(self, name, value)

    def pack(self):
        # Skip "x" members which are padding bytes.
        names = (name for char, name in self.struct_members if 'x' not in char)
        values = (getattr(self, name) for name in names)
        return struct.pack(self.struct_fmt, *values)


def pad_buf(buf, size, fill):
    if len(buf) < size:
        buf += struct.pack("B", fill) * (size - len(buf))
    return buf

__all__ = ["KB", "MB", "CStruct", "pad_buf"]
