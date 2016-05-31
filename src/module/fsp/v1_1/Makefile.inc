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

FSP_V1_1_OBJECTS = $(COMMON_OBJECTS) $(fsp_common-objs)

module-y += fsp_v1_1_entry
mod-fsp_v1_1_entry-title = "fsp v1.1 entry point"
mod-fsp_v1_1_entry-y += entry.S ram_stack.c temp_stack.c
mod-fsp_v1_1_entry-libs += $(FSP_V1_1_OBJECTS) $(TRAMP_KS_BIN)
mod-fsp_v1_1_entry-ldscript += $(src)/src/module/fsp/v1_1/entry.ldscript
mod-fsp_v1_1_entry-ldopts += -Wl,--relocatable -Wl,-d -Wl,--warn-common
mod-fsp_v1_1_entry-ldopts += -Wl,--entry=_entry

# Make the trampoline ELF easy to find for XIP linking.
$(module_obj)/fsp_v1_1_entry.trampoline: $(TRAMP_KS_ELF)
	@printf "    OBJCOPY    $(subst $(obj)/,,$@)\n"
	$(Q)$(OBJCOPY) $< $@

module-y += fsp_v1_1_fw_sel
mod-fsp_v1_1_fw_sel-title = "fsp v1.1 firmware selection"
mod-fsp_v1_1_fw_sel-y += fw_sel.c
mod-fsp_v1_1_fw_sel-libs += $(FSP_V1_1_OBJECTS) $(module_handoff-objs)
