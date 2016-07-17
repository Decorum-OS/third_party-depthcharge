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


TRAMPOLINE_LDSCRIPT := $(cursrcdir)/trampoline.ldscript
TRAMPOLINE_LDOPTS = -Wl,--entry=trampoline -Wl,--undefined=trampoline_stack
TRAMPOLINE_GLOBALS = _tramp_start _tramp_end trampoline trampoline_stack

$(module_obj)/trampoline-%.elf: $(module_obj)/trampoline.elf
	@printf "    LD         $(subst $(obj)/,,$@)\n"
	$(Q)$(CC) $(LINK_FLAGS) -Wl,-T,$(TRAMPOLINE_LDSCRIPT).final \
		-Wl,--defsym=_final_link_base=$* \
		-o $@.tmp $+ $(TRAMPOLINE_LDOPTS) $(LIBGCC)
	@printf "    OBJCOPY    $(subst $(obj)/,,$@)\n"
	$(Q)$(OBJCOPY) $@.tmp $@ \
		$(addprefix --keep-global-symbol=,$(TRAMPOLINE_GLOBALS))

# This rule creates a .bin.o for a version of the trampoline linked to run at
# a particular address. This creates a .o file which can be linked into other
# code, making the trampoline's binary ELF image available at run time.
#
# It does this in a couple ways. First, it uses the bin_to_o utility script
# to wrap the binary version of the trampoline in a .o file with symbols that
# show where the binary image starts, stops, and its size.
#
# Second, it extracts symbols from the trampoline's elf file (before
# stripping, compression, etc.) and passes them to bin_to_o to be included in
# the resulting .bin.o file. That means that the outer program will be
# able to have references to symbols from the trampoline which will become
# valid/populated when the trampoline is loaded, without having to have them
# map to anything which is actually present before that.
$(module_obj)/trampoline-%.bin.o: $(module_obj)/trampoline-%.bin \
				  $(module_obj)/trampoline-%.elf \
				  $$(NM_TO_SYM_ARGS_SED) \
				  $$(BIN_TO_O)
	@printf "    BIN_TO_O   $(subst $(obj)/,,$@)\n"
	$(Q)$(BIN_TO_O) $@ trampoline $(filter %.bin,$+) \
		$$(nm --extern-only --radix=x --defined-only \
			$(filter %.elf,$+) | \
			sed -r -f $(filter %.sed,$+) | \
			tr '\n' ' ' \
		)

trampoline_bin_at = $(module_obj)/trampoline-$(1).bin.o
trampoline_elf_at = $(module_obj)/trampoline-$(1).elf

TRAMP_KS = $(call trampoline_bin_at,$(CONFIG_KERNEL_START))

module-y += trampoline
mod-trampoline-y += trampoline.c
mod-trampoline-libs += $(libc-objs)
mod-trampoline-ldscript += $(TRAMPOLINE_LDSCRIPT)
mod-trampoline-ldopts += -Wl,--relocatable -Wl,-d -Wl,--warn-common
mod-trampoline-ldopts += $(TRAMPOLINE_LDOPTS)
