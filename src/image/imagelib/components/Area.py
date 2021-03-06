# Copyright 2016 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import textwrap

from imagelib.util import Buffer

class Area(object):
    LOG_WRAP_WIDTH = 78
    LOG_INDENT_STEP = 4

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

    def _call_on_tree(self, func_name):
        for child in self.children:
            child._call_on_tree(func_name)
        func = getattr(self, func_name)
        func()

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

    def post_config_hook(self):
        """Called after the layout is configured, but before any processing."""
        pass

    def post_fill_byte_hook(self):
        """Called after the fill byte values have been propogated through the
           tree
        """
        pass

    def post_min_size_hook(self):
        """Called after minimum sizes have been computed."""
        pass

    def post_place_hook(self):
        """Called after Areas final position and size have been computed."""
        pass

    def build(self, log_file=None):
        """Size, place, and write out a buffer with the contents of this
           Area.
        """
        self.log(log_file, "Layout populated")
        self._call_on_tree("post_config_hook")

        # Propogate fill byte values through the tree.
        self.propagate_fill_byte(0xff)
        self._ensure_params_are_set("computed_fill_byte")
        self._call_on_tree("post_fill_byte_hook")

        # Compute the minimum size required for Areas.
        self.compute_min_size()
        self._ensure_params_are_set("computed_min_size")
        self.log(log_file, "Minimum size computed")
        self._call_on_tree("post_min_size_hook")

        # Place Areas in their final positions.
        self.place(0, self.computed_min_size)
        self._ensure_params_are_set("placed_offset", "placed_size")
        self.log(log_file, "Areas placed")
        self._call_on_tree("post_place_hook")
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
                raise ValueError(("Area's contents take up %d bytes, which " +
                                  "is too big to fit in %d bytes") %
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


    # Logging support. These can be overridden as well.

    def log(self, log_file, title):
        """Generate log output for the hierarchy rooted at this object, and
           write the log to the provided file. A header with a descriptive
           title will be output before the actual output.
        """
        log_text = (
"""{bar}
# {title:{width}} #
{bar}

{area}

""")

        log_text = log_text.format(title=title, width=self.LOG_WRAP_WIDTH - 4,
                                   bar="#" * self.LOG_WRAP_WIDTH,
                                   area=self.log_area())
        if log_file:
            log_file.write(log_text)

    def log_wrap(self, indent, *args, **kwargs):
        """A utility wrapper for textwrap.wrap. Probably not useful to
           override.
        """
        kwargs["width"] = self.LOG_WRAP_WIDTH
        kwargs.setdefault("initial_indent", indent)
        kwargs.setdefault("subsequent_indent", indent)
        return "\n".join(textwrap.wrap(*args, **kwargs))

    def log_step_indent(self, indent):
        """A utility wrapper to increase an indent prefix by one level."""
        return indent + " " * self.LOG_INDENT_STEP

    def log_area(self, indent=""):
        """Return log output corresponding to this particular Area, including
           its properties and children.
        """
        name = self.log_area_name()
        properties = self.log_area_properties()
        content = self.log_area_content(self.log_step_indent(indent))

        if properties:
            name = name + "("
            properties = properties + ")"
        log_text = self.log_wrap(indent, name + properties,
                                 subsequent_indent=indent + " " * (len(name)))
        if content:
            log_text += "\n{indent}{{\n{content}\n{indent}}}".format(
                indent=indent, content=content)
        return log_text

    def log_area_name(self):
        """Returns the name of this Area. It defaults to the current class,
           and can be overridden if something else would be more appropriate.
        """
        return type(self).__name__

    def log_get_additional_properties(self):
        """Return a list of any additional property strings that should be
           included for this area. This can be overridden to augment the
           default set without entirely replacing log_area_properties.
        """
        return []

    def log_area_properties(self):
        """Return a string representing the properties of this particular
           Area. If you want to print additional properties beyond the
           defaults, you can override log_get_additional_properties.
        """
        properties = []

        if None not in (self.placed_offset, self.placed_size):
            properties.append(
                "[{:#x} -> {:#x})".format(
                    self.placed_offset, self.placed_offset + self.placed_size))

        if self._fill is not None:
            properties.append("fill={:#02x}".format(self._fill))

        if self._size is not None:
            properties.append("fixed_size={:#x}".format(self._size))
        elif self.computed_min_size is not None:
            properties.append("min_size={:#x}".format(self.computed_min_size))

        if self._expand_weight is not None:
            properties.append("expand_weight={}".format(self._expand_weight))

        if self._shrink is not None:
            properties.append("shrinking")

        properties.extend(self.log_get_additional_properties())
        return ", ".join(properties)

    def log_area_content(self, indent):
        """Return a string representing the content of this Area. The default
           is to just output each of the children in succession with
           appropriate indentation, but this method can be overriden to
           encode the children differently, or even not at all.
        """
        return "\n".join(child.log_area(indent) for child in self.children)


class DerivedArea(Area):
    """A base class for Areas which have children but don't put those children
       directly in their portion of the image. When asked to compute the
       minimum size of the area's contents, it will find the minimal size of
       the children, and place them as if they were at the start of their own
       image with minimal size.
    """

    def __init__(self, *args):
        super(DerivedArea, self).__init__(*args)
        self._children_handled = False

    def handle_children(self):
        """Sizes and places this Area's children. Returns whether that had
           been done already.
        """
        if self._children_handled:
            return True

        for child in self.children:
            child.place(0, child.computed_min_size)

        self._children_handled = True
        return False

    def compute_min_size_content(self):
        self.handle_children()
        return 0

    def place_children(self):
        pass

    def log_area_content(self, indent):
        return ""

    def log_child_props(self, indent, child_props):
        """Log children with labels in front of them."""
        return "\n\n".join("{}{}:\n{}".format(indent, label,
                                              child.log_area(indent))
                           for label, child in child_props.iteritems())
