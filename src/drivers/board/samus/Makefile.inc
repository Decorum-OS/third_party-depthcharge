##
## Copyright 2013 Google Inc.
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

depthcharge-y += board.c
depthcharge-y += fsp.c
depthcharge-y += preram_board_init.S

board_obj := $(curobjdir)

$(call add_spd_file,6,$(board_obj)/spd/samsung_4.spd)
$(call add_spd_file,8,$(board_obj)/spd/hynix_4.spd)
$(call add_spd_file,a,$(board_obj)/spd/samsung_8.spd)
$(call add_spd_file,c,$(board_obj)/spd/hynix_8.spd)
$(call add_spd_file,d,$(board_obj)/spd/hynix_16.spd)
$(call add_spd_file,f,$(board_obj)/spd/elpida_16.spd)
