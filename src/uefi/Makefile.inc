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

UEFI_ARCH_INCLUDE-$(CONFIG_ARCH_X86_AMD64) = X64
UEFI_ARCH_INCLUDE-$(CONFIG_ARCH_X86_IA32) = Ia32

src-includes-y += $(UEFI_ARCH_INCLUDE-y)