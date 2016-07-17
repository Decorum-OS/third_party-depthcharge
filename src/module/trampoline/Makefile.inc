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

$(module_obj)/trampoline-%.bin.o: $(module_obj)/trampoline-%.bin
	@printf "    CC         $(subst $(obj)/,,$@)\n"
	$(Q)TEMPDIR=`mktemp -d -t trampoline.XXXXXXXX` && \
		cp $+ "$${TEMPDIR}"/trampoline && \
		pushd "$${TEMPDIR}" > /dev/null && \
		$(CC) -Wl,--relocatable -Wl,-b,binary -fuse-ld=bfd -nostdlib \
			-o $(abspath $@) -Xlinker trampoline; \
		popd > /dev/null; \
		rm -rf "$${TEMPDIR}"

trampoline_bin_at = $(module_obj)/trampoline-$(1).bin.o
trampoline_elf_at = $(module_obj)/trampoline-$(1).elf

TRAMP_KS_ELF = $(call trampoline_elf_at,$(CONFIG_KERNEL_START))
TRAMP_KS_BIN = $(call trampoline_bin_at,$(CONFIG_KERNEL_START))

module-y += trampoline
mod-trampoline-y += trampoline.c
mod-trampoline-libs += $(libc-objs)
mod-trampoline-ldscript += $(TRAMPOLINE_LDSCRIPT)
mod-trampoline-ldopts += -Wl,--relocatable -Wl,-d -Wl,--warn-common
mod-trampoline-ldopts += $(TRAMPOLINE_LDOPTS)
