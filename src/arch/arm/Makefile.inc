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

subdirs-$(CONFIG_ARCH_ARM_V7) += v7
subdirs-$(CONFIG_ARCH_ARM_V8) += v8
subdirs-y += handoff

depthcharge-$(CONFIG_KERNEL_FIT) += fit.c
depthcharge-y += gcc.c
depthcharge-y += util.S
