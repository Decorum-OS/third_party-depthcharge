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
	$(Q)$(OBJCOPY) --target=efi-app-x86_64 --strip-debug \
	               --remove-section=.hash $< $@

.PRECIOUS: $(module_obj)/%.efi


UEFI_INCLUDE_ARCH-$(CONFIG_ARCH_X86_AMD64) = X64
UEFI_INCLUDE_ARCH-$(CONFIG_ARCH_X86_IA32) = Ia32


module-y += uefi
mod-uefi-title = "UEFI chain loader"
mod-uefi-y += uefi.c
mod-uefi-libs += $(depthcharge-objs) $(uefi_handoff-objs)
mod-uefi-c-ccopts := -I$(cursrcdir)/efi-include
mod-uefi-c-ccopts += -I$(cursrcdir)/efi-include/$(UEFI_INCLUDE_ARCH-y)
mod-uefi-ldscript := $(cursrcdir)/uefi.ldscript
