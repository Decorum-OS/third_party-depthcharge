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

subdirs-y += cbfs libc lz4 lzma

depthcharge-y += banner.c
depthcharge-y += cleanup_funcs.c
depthcharge-y += console.c
depthcharge-y += coreboot.c
depthcharge-y += dc_stack.c
depthcharge-y += dcdir.c
depthcharge-y += device_tree.c
depthcharge-y += die.c
depthcharge-y += fwdb.c
depthcharge-y += graphics.c
depthcharge-y += hexdump.c
depthcharge-y += init_funcs.c
depthcharge-y += ipchecksum.c
depthcharge-y += list.c
depthcharge-y += physmem.c
depthcharge-y += power.c
depthcharge-y += queue.c
depthcharge-y += ranges.c
depthcharge-y += state_machine.c
depthcharge-y += time.c
depthcharge-y += timestamp.c
