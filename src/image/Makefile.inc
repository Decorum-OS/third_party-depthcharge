##
## Copyright 2012 Google Inc.
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

depthcharge-y += fmap.c
depthcharge-y += index.c



image_obj=$(obj)/image



.PHONY: images
all: images

image-handler=$(eval images: $(image_obj)/$2)

$(call add-special-class,image)



ifeq ($(CONFIG_BOOTPLAN_CB_SPLIT_PAYLOAD),y)

image-y += depthcharge.ro.elf depthcharge.ro.bin \
	depthcharge.rw.elf depthcharge.rw.bin \
	dev.ro.bin dev.ro.elf dev.rw.bin dev.rw.elf

$(image_obj)/depthcharge.ro.%: $$(module_obj)/cb_readonly.$$*
	@printf "    CP         $(subst $(obj)/,,$@)\n"
	$(Q)cp $< $@

$(image_obj)/depthcharge.rw.%: $$(module_obj)/cb_readwrite.$$*
	@printf "    CP         $(subst $(obj)/,,$@)\n"
	$(Q)cp $< $@

$(image_obj)/dev.ro.%: $$(module_obj)/cb_dev_ro.$$*
	@printf "    CP         $(subst $(obj)/,,$@)\n"
	$(Q)cp $< $@

$(image_obj)/dev.rw.%: $$(module_obj)/cb_dev_rw.$$*
	@printf "    CP         $(subst $(obj)/,,$@)\n"
	$(Q)cp $< $@

endif # CONFIG_BOOTPLAN_CB_SPLIT_PAYLOAD



ifeq ($(CONFIG_BOOTPLAN_CB_PAYLOAD),y)

image-y += depthcharge.elf depthcharge.payload \
	dev.elf dev.payload netboot.elf netboot.payload

$(image_obj)/depthcharge.%: $$(module_obj)/cb_payload.$$*
	@printf "    CP         $(subst $(obj)/,,$@)\n"
	$(Q)cp $< $@

$(image_obj)/dev.%: $$(module_obj)/cb_dev.$$*
	@printf "    CP         $(subst $(obj)/,,$@)\n"
	$(Q)cp $< $@

$(image_obj)/netboot.%: $$(module_obj)/netboot.$$*
	@printf "    CP         $(subst $(obj)/,,$@)\n"
	$(Q)cp $< $@

endif # CONFIG_BOOTPLAN_CB_PAYLOAD
