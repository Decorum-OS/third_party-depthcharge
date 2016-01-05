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
readonly-y += startrw.c

trampoline-y += load_elf.c

LDSCRIPT = $(src)/src/image/depthcharge.ldscript

# These options are passed from .config to the linker script. When making
# changes, update Kconfig as well.
link_config_options := \
	KERNEL_START \
	KERNEL_SIZE \
	BASE_ADDRESS \
	HEAP_SIZE \
	STACK_SIZE \
	GBB_COPY_SIZE



bootplan-handler= \
	$(eval bp-$2: $$$$(foreach mod,$$$$(bp-$2-modules),$$(obj)/module/$$$$(mod))) \
	$(eval bp-$2: $$$$(bp-$2-images)) \
	$(eval .PHONY: bp-$2) \
	$(eval bootplans: bp-$2)

.PHONY: bootplans
all: bootplans

$(call add-special-class,bootplan)




bootplan-$(CONFIG_BOOTPLAN_CB_SPLIT_PAYLOAD) += cb_split_payload_dev
bp-cb_split_payload_dev-modules += cb_dev_ro.bin
bp-cb_split_payload_dev-modules += cb_dev_rw.bin

bootplan-$(CONFIG_BOOTPLAN_CB_SPLIT_PAYLOAD) += cb_split_payload
bp-cb_split_payload-modules += cb_readonly.bin
bp-cb_split_payload-modules += cb_readwrite.bin
bp-cb_split_payload-modules += netboot.bin

bootplan-$(CONFIG_BOOTPLAN_CB_PAYLOAD) += cb_payload_dev
bp-cb_payload_dev-modules += cb_dev.payload

bootplan-$(CONFIG_BOOTPLAN_CB_PAYLOAD) += cb_payload
bp-cb_payload-modules += cb_payload.payload
bp-cb_payload-modules += netboot.payload
