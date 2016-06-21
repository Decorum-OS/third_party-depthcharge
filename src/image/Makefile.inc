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



image_obj := $(curobjdir)
image_src := $(cursrcdir)



build_image = $(image_obj)/build_image
build_image_src = $(image_src)/build_image


space = $(empty) $(empty)
comma = ,
BUILD_IMAGE_SPD_OPT = $(BUILD_IMAGE_SPD_OPT_NAME)$(subst $(space),$(comma),$(BUILD_IMAGE_SPD_OPT_SPACE))
add_spd_file = \
	$(eval BUILD_IMAGE_SPD_FILES += $(2)) \
	$(eval BUILD_IMAGE_SPD_OPT_NAME = --spds ) \
	$(eval BUILD_IMAGE_SPD_OPT_SPACE += $(1):$(2)) \

modules: $$(BUILD_IMAGE_SPD_FILES)



BUILD_IMAGE_OPTS = --verbose --log-file $@.log -o $@
BUILD_IMAGE = BUILD_IMAGE_PATH="${BUILD_IMAGE_PATH}:$(obj):$(module_obj)" \
        CC="$(CC)" \
        LINK_FLAGS="$(LINK_FLAGS)" \
        OBJCOPY="$(OBJCOPY)" \
	$(build_image) $(BUILD_IMAGE_OPTS) $(BUILD_IMAGE_LAYOUT) \
        $(BUILD_IMAGE_LAYOUT_OPTS)
imagelib_files = $(shell find $(image_src) -name '*.py')
$(build_image): $(build_image_src) $(imagelib_files)
	@printf "    CP         $(subst $(obj)/,,$@)\n"
	$(Q)cp $< $@
	$(Q)cp -a $(image_src)/imagelib $(image_obj)/imagelib



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

$(image_obj)/netboot.%: $$(module_obj)/cb_netboot.$$*
	@printf "    CP         $(subst $(obj)/,,$@)\n"
	$(Q)cp $< $@

endif # CONFIG_BOOTPLAN_CB_PAYLOAD



ifeq ($(CONFIG_BOOTPLAN_CB_IMAGE),y)

BUILD_IMAGE_LAYOUT = samus_cb

BUILD_IMAGE_LAYOUT_OPTS = \
	--size=$(CONFIG_IMAGE_SIZE_KB) \
	--hwid=$(CONFIG_IMAGE_HWID) \
	--model=$(CONFIG_BOOTPLAN_CB_IMAGE_MODEL)

modules: $$(module_obj)/cb_payload.elf
modules: $$(module_obj)/cb_payload.payload
modules: $$(module_obj)/cb_dev.elf
modules: $$(module_obj)/cb_dev.payload
modules: $$(module_obj)/cb_netboot.elf
modules: $$(module_obj)/cb_netboot.payload

STANDARD_IMAGES=y

endif # CONFIG_BOOTPLAN_CB_IMAGE


ifeq ($(CONFIG_BOOTPLAN_FSP_V1_1),y)

BUILD_IMAGE_LAYOUT = fsp_v1_1

BUILD_IMAGE_LAYOUT_OPTS = \
	--size=$(CONFIG_IMAGE_SIZE_KB) \
	--hwid=$(CONFIG_IMAGE_HWID) \
	--model=$(CONFIG_BOOTPLAN_FSP_MODEL) \
	--ecs=$(CONFIG_BOOTPLAN_FSP_ECS) \
	$(BUILD_IMAGE_SPD_OPT)

modules: $$(module_obj)/cb_payload.payload
modules: $$(module_obj)/cb_dev.payload
modules: $$(module_obj)/cb_netboot.payload

modules: $$(module_obj)/fsp_v1_1_entry.mod
modules: $$(module_obj)/fsp_v1_1_entry.trampoline
modules: $$(module_obj)/fsp_v1_1_fw_sel.bin

STANDARD_IMAGES=y

endif # CONFIG_BOOTPLAN_FSP_V1_1


ifeq ($(CONFIG_BOOTPLAN_QEMU_X86),y)

BUILD_IMAGE_LAYOUT = qemu-x86

BUILD_IMAGE_LAYOUT_OPTS = \
	--size=$(CONFIG_IMAGE_SIZE_KB) \
	--hwid=$(CONFIG_IMAGE_HWID)

modules: $$(module_obj)/qemu_x86_entry.mod
modules: $$(module_obj)/qemu_x86_entry.trampoline
modules: $$(module_obj)/qemu_firmware_select.elf

STANDARD_IMAGES=y

endif # CONFIG_BOOTPLAN_QEMU_X86


ifeq ($(CONFIG_BOOTPLAN_UEFI),y)

image-y += uefi.efi

$(image_obj)/uefi.efi: $$(module_obj)/uefi.efi
	@printf "    CP         $(subst $(obj)/,,$@)\n"
	$(Q)cp $< $@

endif # CONFIG_BOOTPLAN_UEFI


ifeq ($(STANDARD_IMAGES),y)

image-y += image.bin image.serial.bin image.dev.bin image.net.bin

.PHONY: modules
$(addprefix $(image_obj)/,$(image-y)): modules

$(addprefix $(image_obj)/,$(image-y)): $(build_image)

$(image_obj)/image.bin:
	@printf "    BUILD_IMAGE $(subst $(obj)/,,$@)\n"
	$(Q)$(BUILD_IMAGE)

$(image_obj)/image.serial.bin:
	@printf "    BUILD_IMAGE $(subst $(obj)/,,$@)\n"
	$(Q)$(BUILD_IMAGE) --serial

$(image_obj)/image.dev.bin:
	@printf "    BUILD_IMAGE $(subst $(obj)/,,$@)\n"
	$(Q)$(BUILD_IMAGE) --serial --dev

$(image_obj)/image.net.bin:
	@printf "    BUILD_IMAGE $(subst $(obj)/,,$@)\n"
	$(Q)$(BUILD_IMAGE) --serial --netboot

endif # STANDARD_IMAGES
