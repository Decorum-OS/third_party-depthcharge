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

subdirs-y += cb fsp qemu

depthcharge-y += module.c

DEFAULT_LDSCRIPT = $(src)/src/module/depthcharge.ldscript

# These options are passed from .config to the linker script. When making
# changes, update Kconfig as well.
link_config_options := \
	KERNEL_START \
	KERNEL_SIZE \
	BASE_ADDRESS \
	GBB_COPY_SIZE

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
$(module_obj)/%.elf.tmp:
	@printf "    LD         $(subst $(obj)/,,$@)\n"
	$(Q)$(CC) $(LINK_FLAGS) -Wl,-T,$(mod-$*-ldscript) -o $@ $+ \
		$(mod-$*-ldopts) $(LIBGCC)


# The default post processing of an ELF module strips out the .cparams,
# .gbb_copy, and .shared_data sections, and fix up linking with the trampoline
# blob. Those sections have symbols but are not allocated in the binary so that
# different instances of depthcharge can share them without overwriting each
# other.
$(module_obj)/%.elf: $(module_obj)/%.elf.tmp
	@printf "    OBJCOPY    $(subst $(obj)/,,$@)\n"
	$(Q)$(OBJCOPY) --remove-section=".shared_data" \
		--remove-section=".cparams" --remove-section=".gbb_copy" \
		$< $@

# By default, binary modules are stripped of symbols and then compressed.
$(module_obj)/%.bin: $(module_obj)/%.elf
	@printf "    STRIP      $(subst $(obj)/,,$<)\n"
	$(Q)$(STRIP) -o $@.tmp $<
	@printf "    LZMA       $(subst $(obj)/,,$@)\n"
	$(Q)$(LZMA) --stdout $@.tmp > $@

.PRECIOUS: $(module_obj)/%.elf $(module_obj)/%.map $(module_obj)/%.bin

# By default, just copy any .elf.tmp file to its .mod counterpart. This can
# be overridden if more processing is necessary.
$(module_obj)/%.mod: $(module_obj)/%.elf.tmp
	@printf "    OBJCOPY    $(subst $(obj)/,,$@)\n"
	$(Q)$(OBJCOPY) $< $@

module-handler= \
	$(call add-class,mod-$2) \
	$(eval $(module_obj)/$2.elf.tmp: $$$$(mod-$2-objs)) \
	$(eval $(module_obj)/$2.elf.tmp: $$$$(mod-$2-libs)) \
	$(eval mod-$2-ldscript ?= $$(DEFAULT_LDSCRIPT))

$(call add-special-class,module)



DONT_GC_NETBOOT = -Wl,-u,netboot_entry

LIBPAYLOAD_OBJECTS += $(libc-objs) $(libcbfs-objs) $(liblzma-objs)
LIBPAYLOAD_OBJECTS += $(liblz4-objs)

COMMON_OBJECTS = $(depthcharge-objs) $(VB_LIB) $(LIBPAYLOAD_OBJECTS)

NET_OBJECTS = $(netboot-objs) $(net-objs) $(uip-objs)
DEV_OBJECTS = $(NET_OBJECTS) $(dev-objs) $(gdb-objs)



TRAMPOLINE = $(module_obj)/trampoline.mod

module-y += trampoline
mod-trampoline-y += trampoline.c
mod-trampoline-libs += $(libc-objs)
mod-trampoline-ldscript += $(src)/src/module/trampoline.ldscript
mod-trampoline-ldopts += -Wl,--relocatable -Wl,-d -Wl,--entry=trampoline
mod-trampoline-ldopts += -Wl,--undefined=trampoline_stack -Wl,--warn-common

# Make all the symbols in the trampoline local, except the trampoline entry
# point and stack.
$(module_obj)/trampoline.mod: $(module_obj)/trampoline.elf.tmp
	@printf "    OBJCOPY    $(subst $(obj)/,,$@)\n"
	$(Q)$(OBJCOPY) $< $@ \
		--keep-global-symbol=trampoline \
		--keep-global-symbol=trampoline_stack
