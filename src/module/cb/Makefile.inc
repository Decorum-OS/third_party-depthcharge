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

CBFS_ARCH-$(CONFIG_ARCH_X86) = x86
CBFS_ARCH-$(CONFIG_ARCH_ARM_V7) = arm
CBFS_ARCH-$(CONFIG_ARCH_ARM_V8) = arm64

$(module_obj)/%.payload: $(module_obj)/%.elf
	@printf "    PAYLOAD    $(subst $(obj)/,,$@)\n"
	$(Q)-rm -f $*.rom $*.bb
	$(Q)dd if=/dev/zero of=$*.bb bs=512 count=1
	$(Q)cbfstool $*.rom create -m $(CBFS_ARCH-y) -s 1024K -B $*.bb
	$(Q)cbfstool $*.rom add-payload -f $< -n dc.elf -c lzma
	$(Q)cbfstool $*.rom extract -n dc.elf -f $@
	$(Q)rm -f $*.rom $*.bb

.PRECIOUS: $(module_obj)/%.payload


module-y += cb_payload
mod-cb_payload-y += payload.c
mod-cb_payload-libs += $(COMMON_OBJECTS) $(unified-objs)
mod-cb_payload-libs += $(coreboot_handoff-objs)

module-y += cb_dev
mod-cb_dev-y += $(mod-cb_payload-y)
mod-cb_dev-libs += $(mod-cb_payload-libs) $(DEV_OBJECTS)
mod-cb_dev-ldopts = $(DONT_GC_NETBOOT)

module-y += cb_readonly
mod-cb_readonly-y += readonly.c
mod-cb_readonly-libs += $(COMMON_OBJECTS) $(readonly-objs) $(TRAMPOLINE)
mod-cb-readonly-libs += $(coreboot_handoff-objs)

module-y += cb_dev_ro
mod-cb_dev_ro-y += $(mod-cb_readonly-y)
mod-cb_dev_ro-libs += $(mod-cb_readonly-libs) $(DEV_OBJECTS)
mod-cb_dev_ro-ldopts = $(DONT_GC_NETBOOT)

module-y += cb_readwrite
mod-cb_readwrite-y += readwrite.c
mod-cb_readwrite-libs += $(COMMON_OBJECTS) $(readwrite-objs)
mod-cb-readwrite-libs += $(module_handoff-objs)

module-y += cb_dev_rw
mod-cb_dev_rw-y += $(mod-cb_readwrite-y)
mod-cb_dev_rw-libs += $(mod-cb_readwrite-libs) $(DEV_OBJECTS)
mod-cb_dev_rw-ldopts = $(DONT_GC_NETBOOT)

module-y += cb_netboot
mod-cb_netboot-y += ../netboot.c
mod-cb_netboot-libs += $(COMMON_OBJECTS) $(NET_OBJECTS)
mod-cb_netboot-libs += $(coreboot_handoff-objs)
