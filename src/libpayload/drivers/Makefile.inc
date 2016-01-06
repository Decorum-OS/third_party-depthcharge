##
## This file is part of the libpayload project.
##
## Copyright (C) 2007 Uwe Hermann <uwe@hermann-uwe.de>
## Copyright (C) 2008 Advanced Micro Devices, Inc.
## Copyright (C) 2008 coresystems GmbH
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

libc-$(CONFIG_PCI) += pci.c

libc-$(CONFIG_SPEAKER) += speaker.c

libc-$(CONFIG_PC_KEYBOARD) += keyboard.c

libc-$(CONFIG_CBMEM_CONSOLE) += cbmem_console.c

libc-$(CONFIG_NVRAM) += nvram.c
libc-$(CONFIG_NVRAM) += options.c

# Timer drivers
libc-$(CONFIG_TIMER_MTK) += timer/mtk_timer.c
libc-$(CONFIG_TIMER_MCT) += timer/mct.c
libc-$(CONFIG_TIMER_RDTSC) += timer/rdtsc.c
libc-$(CONFIG_TIMER_TEGRA_1US) += timer/tegra_1us.c
libc-$(CONFIG_TIMER_IPQ806X) += timer/ipq806x.c
libc-$(CONFIG_TIMER_RK) += timer/rktimer.c
libc-$(CONFIG_TIMER_IMG_PISTACHIO) += timer/img_pistachio.c
libc-$(CONFIG_TIMER_CYGNUS) += timer/cygnus.c

# Video console drivers
libc-$(CONFIG_VIDEO_CONSOLE) += video/video.c
libc-$(CONFIG_VGA_VIDEO_CONSOLE) += video/vga.c

# Geode LX console drivers
libc-$(CONFIG_GEODELX_VIDEO_CONSOLE) += video/geodelx.c
libc-$(CONFIG_GEODELX_VIDEO_CONSOLE) += video/font8x16.c

# coreboot generic framebuffer driver
libc-$(CONFIG_COREBOOT_VIDEO_CONSOLE) += video/corebootfb.c
libc-$(CONFIG_COREBOOT_VIDEO_CONSOLE) += video/font8x16.c

# cbgfx: coreboot graphics library
libc-y += video/graphics.c

# USB stack
libc-$(CONFIG_USB) += usb/usbinit.c
libc-$(CONFIG_USB) += usb/usb.c
libc-$(CONFIG_USB) += usb/usb_dev.c
libc-$(CONFIG_USB) += usb/quirks.c
libc-$(CONFIG_USB_GEN_HUB) += usb/generic_hub.c
libc-$(CONFIG_USB_HUB) += usb/usbhub.c
libc-$(CONFIG_USB_UHCI) += usb/uhci.c
libc-$(CONFIG_USB_UHCI) += usb/uhci_rh.c
libc-$(CONFIG_USB_OHCI) += usb/ohci.c
libc-$(CONFIG_USB_OHCI) += usb/ohci_rh.c
libc-$(CONFIG_USB_EHCI) += usb/ehci.c
libc-$(CONFIG_USB_EHCI) += usb/ehci_rh.c
libc-$(CONFIG_USB_XHCI) += usb/xhci.c
libc-$(CONFIG_USB_XHCI) += usb/xhci_debug.c
libc-$(CONFIG_USB_XHCI) += usb/xhci_devconf.c
libc-$(CONFIG_USB_XHCI) += usb/xhci_events.c
libc-$(CONFIG_USB_XHCI) += usb/xhci_commands.c
libc-$(CONFIG_USB_XHCI) += usb/xhci_rh.c
libc-$(CONFIG_USB_HID) += usb/usbhid.c
libc-$(CONFIG_USB_MSC) += usb/usbmsc.c
libc-$(CONFIG_USB_DWC2) += usb/dwc2.c
libc-$(CONFIG_USB_DWC2) += usb/dwc2_rh.c

# USB device stack
libc-$(CONFIG_UDC) += udc/udc.c
libc-$(CONFIG_UDC_CI) += udc/chipidea.c
libc-$(CONFIG_UDC_DWC2) += udc/dwc2.c

# used by both USB HID and keyboard
libc-y += hid.c
