##
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

subdirs-y += cb fsp qemu trampoline

depthcharge-y += module.c

DEFAULT_LDSCRIPT = $(src)/src/module/depthcharge.ldscript

# These options are passed from .config to the linker script. When making
# changes, update Kconfig as well.
link_config_options := \
	KERNEL_START \
	KERNEL_SIZE \
	BASE_ADDRESS

# Kconfig options intended for the linker.
$(foreach option,$(link_config_options), \
	$(eval LINK_FLAGS += -Wl,--defsym=$(option)=$$(CONFIG_$(option))))



module_obj=$(obj)/module

# The default linking step for a module. You can add linking options by
# defining a variable with the prefix "mod-", the name of your module, and
# then "-ldopts", for example "mod-foo-ldopts" for the module foo.
#
# The object files this rule consumes will be attached as prerequisites when
# the module is set up.
$(module_obj)/%.elf:
	@printf "    LD         $(subst $(obj)/,,$@)\n"
	$(Q)$(CC) $(LINK_FLAGS) -Wl,-T,$(mod-$*-ldscript) -o $@ $+ \
		$(mod-$*-ldopts) $(LIBGCC)

# By default, binary modules are stripped of symbols and then compressed.
$(module_obj)/%.bin: $(module_obj)/%.elf
	@printf "    STRIP      $(subst $(obj)/,,$<)\n"
	$(Q)$(STRIP) -o $@.tmp $<
	@printf "    LZMA       $(subst $(obj)/,,$@)\n"
	$(Q)$(LZMA) --stdout $@.tmp > $@

.PRECIOUS: $(module_obj)/%.elf $(module_obj)/%.map $(module_obj)/%.bin

# By default, just copy any .elf file to its .mod counterpart. This can
# be overridden if more processing is necessary.
$(module_obj)/%.mod: $(module_obj)/%.elf
	@printf "    OBJCOPY    $(subst $(obj)/,,$@)\n"
	$(Q)$(OBJCOPY) $< $@

module-handler= \
	$(call add-class,mod-$2) \
	$(eval $(module_obj)/$2.elf: $$$$(mod-$2-objs)) \
	$(eval $(module_obj)/$2.elf: $$$$(mod-$2-libs)) \
	$(eval mod-$2-ldscript ?= $$(DEFAULT_LDSCRIPT)) \
	$(eval mod-$2-y += $(src)/src/module/title.c) \
	$(eval mod-$2-title ?= "$2") \
	$(eval mod-$2-c-ccopts += -D __MODULE_TITLE__=$$$$(mod-$2-title)) \
	$(eval mod-$2-S-ccopts += -D __MODULE_TITLE__=$$$$(mod-$2-title))

$(call add-special-class,module)



DONT_GC_NETBOOT = -Wl,-u,netboot_entry

LIBPAYLOAD_OBJECTS += $(libc-objs) $(libcbfs-objs) $(liblzma-objs)
LIBPAYLOAD_OBJECTS += $(liblz4-objs)

COMMON_OBJECTS = $(depthcharge-objs) $(VB_LIB) $(LIBPAYLOAD_OBJECTS)

NET_OBJECTS = $(netboot-objs) $(net-objs) $(uip-objs)
DEV_OBJECTS = $(NET_OBJECTS) $(dev-objs) $(gdb-objs)
