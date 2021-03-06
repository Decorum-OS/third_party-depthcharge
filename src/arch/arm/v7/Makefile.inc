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

subdirs-y += handoff

depthcharge-y += boot.c
depthcharge-y += boot_asm.S

libc-y += cache.c
libc-y += cpu.S
libc-y += eabi_compat.c
depthcharge-y += enter_trampoline.c
libc-y += exception.c
libc-y += exception_asm.S
libc-y += memcpy.S
libc-y += memmove.S
libc-y += memset.S
depthcharge-y += physmem.c
libc-y += virtual.c

gdb-y += gdb.c

src-includes-y += includes

ARCH_ABI_FLAGS += -mthumb -march=armv7-a
ARCH_ABI_FLAGS += -Wa,-mthumb -Wa,-mimplicit-it=always -Wa,-mno-warn-deprecated

TC_ARCH = arm
