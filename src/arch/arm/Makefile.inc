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

ifeq ($(CONFIG_ARCH_ARM_V7),y)
depthcharge-y += boot.c
depthcharge-y += boot_asm.S
libc-y += eabi_compat.c
depthcharge-y += enter_trampoline.c
libc-y += memcpy.S
libc-y += memmove.S
libc-y += memset.S
depthcharge-y += physmem.c
gdb-y += gdb.c
endif

ifeq ($(CONFIG_ARCH_ARM_V8),y)
depthcharge-y += boot_asm64.S
depthcharge-y += boot64.c
libc-y += memcpy64.S
libc-y += memmove64.S
libc-y += memset64.S
depthcharge-y += physmem_arm64.c
gdb-y += gdb64.c
endif

depthcharge-y += coreboot.c
depthcharge-$(CONFIG_KERNEL_FIT) += fit.c
depthcharge-y += gcc.c
depthcharge-y += util.S
