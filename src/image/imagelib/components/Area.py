from imagelib.util import Buffer

class Area(object):
    def __init__(self, *args):
        super(Area, self).__init__()
        self.children = args

        self._fill = None
        self._size = None
        self._expand_weight = None
        self._shrink = None

        self.computed_fill_byte = None
        self.computed_min_size = None
        self.placed_size = None
        self.placed_offset = None


    # Methods for querying an Area's configuration.

    def is_expanding(self):
        """Returns whether an expand weight is set."""
        return self._expand_weight is not None

    def get_expand_weight(self):
        """Returns the expand weight."""
        return self._expand_weight

    def is_shrinking(self):
        """Returns whether "shrink" has been set"""
        return self._shrink

    def is_fixed_size(self):
        """Returns whether this Area is fixed size, or in other words that
           it's neither expanding nor shrinking.
        """
        return not (self.is_expanding() or self.is_shrinking())

    def get_fill_byte(self):
        """Returns the configured fill byte, if any."""
        return self.computed_fill_byte

    def get_fixed_size(self):
        """Returns the configured fixed size, if any."""
        return self._size


    # Methods for configuring an Area.

    def add_child(self, child):
        """Append a child to the list of children."""
        self.children += (child,)

    def expand(self, weight=1):
        """Expand to fill the available space within the containing Area. If
           more than one sibling Area has expand set, then the available
           space is divided up based on their weight.
        """
        if self._size is not None:
            raise ValueError("Expand set on Area with fixed size %d." %
                             self._size)
        self._expand_weight = weight
        return self

    def shrink(self):
        """Shrink to the minimum size necessary to contain this Area's contents.
           If a child of this area has expand set, that conflicts with this
           setting and is considered an error.
        """
        if self._size is not None:
            raise ValueError("Shrink set on Area with fixed size %d." %
                             self._size)
        self._shrink = True
        return self

    def fill(self, fill):
        """Set the fill byte for this Area. This setting is propagated down
           the heirarchy unless explicitly overridden, at which point the
           new fill byte will be propagated.
        """
        self._fill = fill
        return self

    def size(self, size):
        """Force the area to a particular, fixed size. This is incompatible
           with the expand and shrink settings.
        """
        if not self.is_fixed_size():
            raise ValueError("Area is not fixed size, but size set to %d" %
                             size)
        if self._size and self._size != size:
            raise ValueError(("Attempted to set fixed size to %d, but it's " +
                              "already set to %d") % (size, self._size))
        self._size = size
        return self


    # Interface for processing with a hierarchy of objects.

    def _ensure_params_are_set(self, *params):
        """Make sure all the parameters which should be set by a certain
           point have been.
        """
        for param in params:
            if getattr(self, param) is None:
                raise ValueError(("Value \"%s\" should have been computed " +
                                  "for %s but wasn't.") % (param, self))
        for child in self.children:
            child._ensure_params_are_set(*params)

    def build(self):
        """Size, place, and write out a buffer with the contents of this
           Area.
        """
        self.propagate_fill_byte(0xff)
        self._ensure_params_are_set("computed_fill_byte")
        self.compute_min_size()
        self._ensure_params_are_set("computed_min_size")
        self.place(0, self._size)
        self._ensure_params_are_set("placed_offset", "placed_size")
        return self.write()

    def propagate_fill_byte(self, default):
        self.computed_fill_byte = default if self._fill is None else self._fill
        for child in self.children:
            child.propagate_fill_byte(self.computed_fill_byte)

    def compute_min_size(self):
        """Determine the minimum size this Area needs, based on how it was
           configured and what its contents are.
        """
        if self.computed_min_size is not None:
            raise ValueError("Area's minimum size was already calculated.")

        for child in self.children:
            child.compute_min_size()
        min_size_content = self.compute_min_size_content()

        if self._size is not None:
            if self._size < min_size_content:
                raise ValueError("Area's contents take up %d bytes, which " +
                                 "is too big to fit in %d bytes" %
                                 (min_size_content, self._size))
            self.computed_min_size = self._size
        elif self._shrink or self._expand_weight is not None:
            self.computed_min_size = min_size_content
        else:
            raise ValueError("No sizing policy for Area %s." % self)

        return self.computed_min_size

    def place(self, offset, size):
        """Define the location and size of this Area within the image, and
           then propagate that to its children.
        """
        if self.placed_offset is not None or self.placed_size is not None:
            raise ValueError("Area %s was already placed." % self)

        self.placed_offset = offset
        self.placed_size = size

        self.place_children()

    # Methods a subclass of Area can override to change how it composes itself
    # from its component parts.

    def compute_min_size_content(self):
        """Returns how much space this Area needs based on what's in it."""
        return sum(child.computed_min_size for child in self.children)

    def place_children(self):
        """Once this Area's placed_offset and placed_size have been set,
           determine the offset and size of this Area's children, and place
           them there.
        """
        required_size = sum(child.computed_min_size for child in self.children)

        extra_size = self.placed_size - required_size

        expanding_weight = sum(child._expand_weight for child in
                               self.children if child.is_expanding())

        pos = self.placed_offset
        for child in self.children:
            size = child.computed_min_size
            # Distribute the extra size among the expanding children,
            # proportionate to their weight.
            if child.is_expanding():
                size += (extra_size * child._expand_weight) / expanding_weight
            child.place(pos, size)
            pos += size

    def write(self):
        buf = Buffer(self)
        buf.inject_areas(*self.children)
        return buf.data()
