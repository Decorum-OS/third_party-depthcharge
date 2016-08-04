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

$(module_obj)/%.efi: $(module_obj)/%.elf
	@printf "    UEFIAPP    $(subst $(obj)/,,$@)\n"
	$(Q)$(OBJCOPY) --target=efi-app-x86_64 --strip-debug $< $@

.PRECIOUS: $(module_obj)/%.efi


$(module_obj)/uefi.ro.bin.o: $$(image_obj)/uefi.ro.bin $$(BIN_TO_O)
	@printf "    BIN_TO_O   $(subst $(obj)/,,$@)\n"
	$(Q)$(BIN_TO_O) $@ ro_image $(filter %.bin,$+) \

$(module_obj)/uefi_dev.ro.bin.o: $$(image_obj)/uefi_dev.ro.bin $$(BIN_TO_O)
	@printf "    BIN_TO_O   $(subst $(obj)/,,$@)\n"
	$(Q)$(BIN_TO_O) $@ ro_image $(filter %.bin,$+) \

module-y += uefi
mod-uefi-title = "UEFI chain loader"
mod-uefi-y += uefi.c fwdb.c module.c
mod-uefi-libs += $(COMMON_OBJECTS) $(uefi_handoff-objs)
mod-uefi-libs += $(module_obj)/uefi.ro.bin.o
mod-uefi-ldopts += -Wl,-znocombreloc -Wl,-Bsymbolic -pie
mod-uefi-ldopts += -Wl,-Bsymbolic-functions -Wl,--undefined=_entry
mod-uefi-ldscript := $(cursrcdir)/uefi.ldscript

module-y += uefi_dev
mod-uefi_dev-title = $(mod-uefi-title)
mod-uefi_dev-y = $(mod-uefi-y)
mod-uefi_dev-libs += $(COMMON_OBJECTS) $(uefi_handoff-objs)
mod-uefi_dev-libs += $(module_obj)/uefi_dev.ro.bin.o
mod-uefi_dev-ldopts = $(mod-uefi-ldopts)
mod-uefi_dev-ldscript = $(mod-uefi-ldscript)

module-y += uefi_rw
mod-uefi_rw-title = "UEFI chain loader RW"
mod-uefi_rw-y += uefi_rw.c
mod-uefi_rw-libs += $(COMMON_OBJECTS) $(uefi_module_handoff-objs)
mod-uefi_rw-ldopts = $(mod-uefi-ldopts)
mod-uefi_rw-ldscript = $(mod-uefi-ldscript)

module-y += uefi_dev_rw
mod-uefi_dev_rw-title = $(mod-uefi_rw-title)
mod-uefi_dev_rw-y = $(mod-uefi_rw-y)
mod-uefi_dev_rw-libs += $(mod-uefi_rw-libs)
mod-uefi_dev_rw-ldopts = $(mod-uefi_rw-ldopts)
mod-uefi_dev_rw-ldscript = $(mod-uefi_rw-ldscript)
