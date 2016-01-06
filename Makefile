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

src ?= $(shell pwd)
srck = $(src)/util/kconfig
obj ?= $(src)/build
objb = $(src)/build/$(BOARD)
objk = $(obj)/util/kconfig
tempconfig = $(objb)/tempconfig

# Make is silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q:=@
.SILENT:
endif

# Add this check for all rules where "BOARD" needs to be set.
BOARD_MESSAGE="BOARD not set. Set BOARD to select a board to build for."
BOARD_CHECK=if [[ -z \"$(BOARD)\" ]]; then @printf \"$(BOARD_MESSAGE)\"; exit 1; fi

build:
	# Make sure BOARD is set.
	$(BOARD_CHECK)
	
	# Set up a staging dir for the config files
	$(Q)rm -rf $(tempconfig) && mkdir -p $(tempconfig)
	
	# Build a config.
	@printf "Building config files...\n"
	$(Q)$(MAKE) KCONFIG_SRC=$(srck) \
		KCONFIG_OBJ=$(objk) \
		DC_SRC=$(src) \
		DC_CONFIG=$(src)/board/$(BOARD)/defconfig \
		TARGET_DIR=$(tempconfig) \
		-f $(src)/buildconfig.mk defconfig
	
	# Install the config files into the build dir if they've changed.
	@printf "Syncing config...\n"
	$(Q)rsync -ac $(tempconfig)/ $(objb)
	
	# Build depthcharge using the new config.
	@printf "Building depthcharge...\n"
	$(Q)$(MAKE) DC_SRC=$(src) \
		DC_OBJ=$(objb) \
		-f $(src)/engine.mk

clean:
	# Make sure BOARD is set so we know what build dir to clear.
	$(BOARD_CHECK)
	$(Q)rm -rf $(objb)

cleanall:
	$(Q)rm -rf $(obj)

.PHONY: all build clean
