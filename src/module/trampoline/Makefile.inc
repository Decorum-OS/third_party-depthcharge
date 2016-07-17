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
# It does this in a couple ways. First, it performs an incremental link with
# gcc and passes in the binary as an input. ld will create a section for it,
# and will create symbols relative to that section which say where it starts
# and what its size is. When the .bin.o is linked into another code, those
# symbols let that code find the trampoline image so it can be loaded into
# place.
#
# Also, this rule extracts symbols from the trampoline's elf file (before
# stripping, compression, etc.) and injects them into the .bin.o using
# --defsym options passed to ld. That means that the outer program will be
# able to have references to symbols from the trampoline which will
# become valid/populated when the trampoline is loaded.
$(module_obj)/trampoline-%.bin.o: $(module_obj)/trampoline-%.bin \
				  $(module_obj)/trampoline-%.elf \
				  $$(NM_TO_DEFSYM_SED)
	@printf "    CC         $(subst $(obj)/,,$@)\n"
	$(Q)TEMPDIR=`mktemp -d -t trampoline.XXXXXXXX` && \
		DEFSYM_OPTS=`nm --extern-only --radix=x --defined-only \
			$(filter %.elf,$+) | \
			sed -r -f $(NM_TO_DEFSYM_SED) | \
			tr '\n' ' ' \
		` && \
		cp $(filter %.bin,$+) "$${TEMPDIR}"/trampoline && \
		pushd "$${TEMPDIR}" > /dev/null && \
		$(CC) -Wl,--relocatable -Wl,-b,binary -fuse-ld=bfd -nostdlib \
			-o $(abspath $@) -Xlinker trampoline $${DEFSYM_OPTS}; \
		popd > /dev/null; \
		rm -rf "$${TEMPDIR}"

trampoline_bin_at = $(module_obj)/trampoline-$(1).bin.o
trampoline_elf_at = $(module_obj)/trampoline-$(1).elf

TRAMP_KS = $(call trampoline_bin_at,$(CONFIG_KERNEL_START))

module-y += trampoline
mod-trampoline-y += trampoline.c
mod-trampoline-libs += $(libc-objs)
mod-trampoline-ldscript += $(TRAMPOLINE_LDSCRIPT)
mod-trampoline-ldopts += -Wl,--relocatable -Wl,-d -Wl,--warn-common
mod-trampoline-ldopts += $(TRAMPOLINE_LDOPTS)
