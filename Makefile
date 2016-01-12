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

src  = $(shell pwd)
srck = $(src)/util/kconfig
obj ?= $(src)/build
objk = $(obj)/util/kconfig
# Since we won't don't know what board to use and how to derive it isn't always
# the same, these should be called like functions.
objb = $(obj)/$(1)
tempconfig = $(call objb,$(1))/tempconfig

# Don't let the system's CFLAGS get mixed into the build.
unexport CFLAGS

# Make is silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q:=@
.SILENT:
endif

DEFCONFIGS = $(wildcard $(src)/board/*/defconfig)
BOARDS = $(sort						\
	   $(patsubst $(src)/board/%,%,			\
	     $(patsubst %/defconfig,%,$(DEFCONFIGS))	\
	    )						\
	  )

# If nothing is specified, print a usage message and stop.
-usage-:
	@printf "\n"
	@printf "To build depthcharge, run $(MAKE) with one of the following targets:\n" | fold -s
	@printf "\n"
	@printf "all - Build all boards depthcharge knows about. This will probably require having multiple toolchains available.\n" | fold -s
	@printf "clean - Delete the entire build directory.\n" | fold -s
	@printf "clean-[board name] - Delete the build directory for board \"board name\".\n" | fold -s
	@printf "[board name] - Build the board \"board name\".\n" | fold -s
	@printf "\n"
	@printf "You can specify multiple targets, but only one is advised when deleting build directories. You can override the build directory by setting the \"obj\" variable on the command line.\n" | fold -s
	@printf "\n"
	@printf "To see what commands are being run, set V=1 on the command line.\n"
	@printf "\n"
	@printf "Available boards: $(BOARDS)\n" | fold -s
	@printf "\n"

# Build all boards.
all: $(BOARDS)

# Delete the entire build directory.
clean:
	$(Q)rm -rf $(obj)

# Delete the build directory for a particular board.
$(addprefix clean-,$(BOARDS)):
	$(Q)rm -rf $(call objb,$(patsubst clean-%,%,$@))

$(BOARDS):
	# Set up a staging dir for the config files
	$(Q)rm -rf $(call tempconfig,$@) && mkdir -p $(call tempconfig,$@)
	
	# Build a config.
	@printf "Building config files...\n"
	$(Q)$(MAKE) KCONFIG_SRC=$(srck) \
		KCONFIG_OBJ=$(objk) \
		DC_SRC=$(src) \
		DC_CONFIG=$(src)/board/$@/defconfig \
		TARGET_DIR=$(call tempconfig,$@) \
		Kconfig=$(src)/Kconfig \
		-f $(src)/buildconfig.mk defconfig
	
	# Install the config files into the build dir if they've changed.
	@printf "Syncing config...\n"
	$(Q)rsync -ac $(call tempconfig,$@)/ $(call objb,$@)
	
	# Build depthcharge using the new config.
	@printf "Building depthcharge...\n"
	$(Q)$(MAKE) DC_SRC=$(src) \
		DC_OBJ=$(call objb,$@) \
		-f $(src)/engine.mk

# kconfig can't handle doing two things at once, and mixing the output of
# multiple boards makes it really hard to tell what's happening.
.NOTPARALLEL: $(BOARDS)

.PHONY: -usage- all clean $(addprefix clean-,$(BOARDS)) $(BOARDS)
