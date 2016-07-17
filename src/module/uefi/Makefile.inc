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


module-y += uefi
mod-uefi-title = "UEFI chain loader"
mod-uefi-y += uefi.c
mod-uefi-libs += $(COMMON_OBJECTS) $(uefi_handoff-objs)
mod-uefi-ldopts += -Wl,-znocombreloc -Wl,-Bsymbolic -pie
mod-uefi-ldopts += -Wl,-Bsymbolic-functions -Wl,--undefined=_entry
mod-uefi-ldscript := $(cursrcdir)/uefi.ldscript
