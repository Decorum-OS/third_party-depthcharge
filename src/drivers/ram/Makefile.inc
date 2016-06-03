##
## Copyright 2016 Google Inc.
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

# This long, complicated prerequisite is necessary because make "helps" us by
# mucking around with paths and doesn't allow a straight pattern substitution
# when setting up pattern rules.
%.spd: $$(dir $$(subst $$(abspath $$(obj)),$$(src)/src,$$(abspath $$@)))$$*.hex
	@printf "    SPD        $(subst $(obj)/,,$@)\n"
	mkdir -p $(dir $(abspath $@))
	for c in $$(cat $< | grep -v ^#); \
		do echo -e -n "\\x$$c"; \
	done > $@
