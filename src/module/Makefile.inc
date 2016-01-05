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

module_obj=$(obj)/module

# The default linking step for a module. You can add linking options by
# defining a variable with the prefix "mod-", the name of your module, and
# then "-ldopts", for example "mod-foo-ldopts" for the module foo.
#
# The object files this rule consumes will be attached as prerequisites when
# the module is set up.
$(module_obj)/%.elf.tmp:
	@printf "    LD         $(subst $(obj)/,,$@)\n"
	$(Q)$(CC) $(LINK_FLAGS) -o $@ $+ $(mod-$*-ldopts) $(LIBGCC)


# The default post processing of an ELF module strips out the .cparams,
# .gbb_copy, and .shared_data sections, and fix up linking with the trampoline
# blob. Those sections have symbols but are not allocated in the binary so that
# different instances of depthcharge can share them without overwriting each
# other.
$(module_obj)/%.elf: $(module_obj)/%.elf.tmp
	@printf "    OBJCOPY    $(subst $(obj)/,,$@)\n"
	$(Q)$(OBJCOPY) --remove-section=".shared_data" \
		--remove-section=".cparams" --remove-section=".gbb_copy" \
		--redefine-sym $(TRAMP_PREFIX)_\$$a=\$$a \
		--redefine-sym $(TRAMP_PREFIX)_\$$t=\$$t \
		--redefine-sym $(TRAMP_PREFIX)_\$$d=\$$d \
		$< $@

# By default, binary modules are stripped of symbols and then compressed.
$(module_obj)/%.bin: $(module_obj)/%.elf
	@printf "    STRIP      $(subst $(obj)/,,$<)\n"
	$(Q)$(STRIP) -o $@.tmp $<
	@printf "    LZMA       $(subst $(obj)/,,$@)\n"
	$(Q)$(LZMA) --stdout $@.tmp > $@

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

.PRECIOUS: $(module_obj)/%.elf $(module_obj)/%.map
.PRECIOUS: $(module_obj)/%.bin $(module_obj)/%.payload

module-handler= \
	$(call add-class,mod-$2) \
	$(eval $(module_obj)/$2.elf.tmp: $$$$(mod-$2-objs)) \
	$(eval $(module_obj)/$2.elf.tmp: $$$$(mod-$2-libs))

$(call add-special-class,module)


DONT_GC_NETBOOT = -Wl,-u,netboot_entry

libpayload-objects += $(libpci-objs) $(libc-objs) $(libcbfs-objs)
libpayload-objects += $(liblzma-objs) $(liblz4-objs) $(head.o-objs)

common-objects = $(depthcharge-objs) $(VB_LIB) $(libpayload-objects)
tramp-objects = $(TRAMPOLINE) $(tramp-libc-objs)

net-objects = $(netboot-objs) $(net-objs) $(uip-objs)
dev-objects = $(net-objects) $(dev-objs) $(gdb-objs)


module-y += cb_payload
mod-cb_payload-y += cb_payload.c
mod-cb_payload-libs += $(common-objects) $(unified-objs)

module-y += cb_dev
mod-cb_dev-y += $(mod-cb_payload-y)
mod-cb_dev-libs += $(mod-cb_payload-libs) $(dev-objects)
mod-cb_dev-ldopts = $(DONT_GC_NETBOOT)

module-y += cb_readonly
mod-cb_readonly-y += cb_readonly.c
mod-cb_readonly-libs += $(common-objects) $(readonly-objs) $(tramp-objects)

module-y += cb_dev_ro
mod-cb_dev_ro-y += $(mod-cb_readonly-y)
mod-cb_dev_ro-libs += $(mod-cb_readonly-libs) $(dev-objects)
mod-cb_dev_ro-ldopts = $(DONT_GC_NETBOOT)

module-y += cb_readwrite
mod-cb_readwrite-y += cb_readwrite.c
mod-cb_readwrite-libs += $(common-objects) $(readwrite-objs)

module-y += cb_dev_rw
mod-cb_dev_rw-y += $(mod-cb_readwrite-y)
mod-cb_dev_rw-libs += $(mod-cb_readwrite-libs) $(dev-objects)
mod-cb_dev_rw-ldopts = $(DONT_GC_NETBOOT)

module-y += netboot
mod-netboot-y += netboot.c
mod-netboot-libs += $(common-objects) $(net-objects)
