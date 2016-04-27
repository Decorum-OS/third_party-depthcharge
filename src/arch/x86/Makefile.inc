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

subdirs-y += handoff

depthcharge-y += boot.c
readonly-y += commonparams.c
readwrite-y += commonparams.c
depthcharge-y += enter_trampoline.c
libc-y += exception.c
libc-y += exception_asm.S
depthcharge-y += physmem.c
depthcharge-y += sign_of_life.c
depthcharge-y += util.S
depthcharge-$(CONFIG_KERNEL_ZIMAGE) += zimage.c

gdb-y += gdb.c

libc-y += string.c

ARCH_ABI_FLAGS += -mpreferred-stack-boundary=2 -m32

TC_ARCH = x86_32
