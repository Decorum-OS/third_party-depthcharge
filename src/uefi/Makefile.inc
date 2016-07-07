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

subdirs-y += edk

UEFI_ARCH-$(CONFIG_ARCH_ARM_V7) = Arm
UEFI_ARCH-$(CONFIG_ARCH_ARM_V8) = AArch64
UEFI_ARCH-$(CONFIG_ARCH_X86_AMD64) = X64
UEFI_ARCH-$(CONFIG_ARCH_X86_IA32) = Ia32
UEFI_ARCH = $(UEFI_ARCH-y)

UEFI_ARCH_SUPPORTED = $(findstring y,$(patsubst %,y,$(UEFI_ARCH)))

depthcharge-$(UEFI_ARCH_SUPPORTED) += uefi.c
