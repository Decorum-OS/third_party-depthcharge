##
## This file is part of the libpayload project.
##
## Copyright (C) 2008 Advanced Micro Devices, Inc.
## Copyright (C) 2008 Uwe Hermann <uwe@hermann-uwe.de>
## Copyright (C) 2011 secunet Security Networks AG
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
## 1. Redistributions of source code must retain the above copyright
##    notice, this list of conditions and the following disclaimer.
## 2. Redistributions in binary form must reproduce the above copyright
##    notice, this list of conditions and the following disclaimer in the
##    documentation and/or other materials provided with the distribution.
## 3. The name of the author may not be used to endorse or promote products
##    derived from this software without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
## ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
## FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
## DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
## OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
## HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
## LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
## OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
## SUCH DAMAGE.
##

export KERNELVERSION      := 0.2.0

ARCHDIR-$(CONFIG_ARCH_ARM_V7)  := arm
ARCHDIR-$(CONFIG_ARCH_ARM_V8)  := arm64
ARCHDIR-$(CONFIG_ARCH_X86)     := x86

classes-$(CONFIG_PCI) += libpci
classes-$(CONFIG_LIBC) += libc
classes-$(CONFIG_CBFS) += libcbfs
classes-$(CONFIG_LZMA) += liblzma
classes-$(CONFIG_LZ4) += liblz4
classes-y += head.o
libraries := $(classes-y)
classes-$(CONFIG_REMOTEGDB) += libgdb

lib: $(addsuffix .a,$(addprefix $(obj)/,$(libraries))) $(obj)/libpayload.a $(obj)/libgdb.a

subdirs-y := arch/$(ARCHDIR-y)
subdirs-y += crypto libc drivers libpci gdb
subdirs-$(CONFIG_CBFS) += libcbfs
subdirs-$(CONFIG_LZMA) += liblzma
subdirs-$(CONFIG_LZ4) += liblz4

INCLUDES := -Iinclude -Iinclude/$(ARCHDIR-y) -I$(obj) -include include/kconfig.h

CFLAGS +=  $(EXTRA_CFLAGS) $(INCLUDES) -Os -pipe -nostdinc -ggdb3
CFLAGS += -nostdlib -fno-builtin -ffreestanding -fomit-frame-pointer
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wall -Wundef -Wstrict-prototypes -Wmissing-prototypes
CFLAGS += -Wwrite-strings -Wredundant-decls -Wno-trigraphs
CFLAGS += -Wstrict-aliasing -Wshadow -Werror

$(obj)/libpayload-config.h: $(KCONFIG_AUTOHEADER)
	cmp $@ $< 2>/dev/null || cp $< $@

$(obj)/libpayload.a: $(foreach class,$(libraries),$$($(class)-objs))
	printf "    AR         $(subst $(shell pwd)/,,$(@))\n"
	$(AR) rc $@ $^

$(obj)/%.a: $$(%-objs)
	printf "    AR         $(subst $(shell pwd)/,,$(@))\n"
	$(AR) rc $@ $^
