##
## Copyright (C) 2008 Advanced Micro Devices, Inc.
## Copyright 2015 Google Inc.
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

export srck ?= $(KCONFIG_SRC)
export src ?= $(DC_SRC)
export tgt ?= $(TARGET_DIR)
export objk ?= $(KCONFIG_OBJ)
export KBUILD_DEFCONFIG := $(DC_CONFIG)

export KERNELVERSION      := 0.1.0

export KCONFIG_AUTOHEADER := $(tgt)/config.h
export KCONFIG_AUTOCONFIG := $(tgt)/auto.conf
export KCONFIG_CONFIG := $(tgt)/.config
export KCONFIG_DEPENDENCIES := $(tgt)/auto.conf.cmd
export KCONFIG_SPLITCONFIG := $(tgt)/config
export KCONFIG_TRISTATE := $(tgt)/tristate.conf
export KCONFIG_NEGATIVES := 1
export KCONFIG_STRICT := 1

export CONFIG_SHELL := sh
export UNAME_RELEASE := $(shell uname -r)
export MAKEFLAGS += -rR --no-print-directory

export HOSTCC = gcc
export HOSTCXX = g++
export HOSTCFLAGS := -I$(srck) -I$(objk)
export HOSTCXXFLAGS := -I$(srck) -I$(objk)

all:

ifndef NOMKDIR
$(shell mkdir -p $(KCONFIG_SPLITCONFIG) $(objk)/lxdialog)
endif

include $(KCONFIG_SRC)/Makefile

.PHONY: buildconfig $(PHONY)
