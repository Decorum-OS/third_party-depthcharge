##
## Copyright 2012 Google Inc.
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; version 2 of the License.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
##

src-includes-handler = $(eval INCLUDE_DIRS += -I"$(abspath $(1)/$(2))")
abs-includes-handler = $(eval INCLUDE_DIRS += -I"$(2)")

# Class for adding include directories relative to "src".
$(call add-special-class,src-includes)
# Class for adding include directories with an absolute path.
$(call add-special-class,abs-includes)


classes-y := depthcharge uip net netboot dev

subdirs-y := src

src-includes-y += src
abs-includes-y += $(VB_SOURCE)/firmware/include
