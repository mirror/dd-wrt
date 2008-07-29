#
# Makefile for USB Host Controller Drivers
#

ifeq ($(CONFIG_USB_DEBUG),y)
	EXTRA_CFLAGS		+= -DDEBUG
endif

obj-$(CONFIG_PCI)		+= pci-quirks.o

obj-$(CONFIG_USB_EHCI_HCD)	+= ehci-hcd.o
obj-$(CONFIG_USB_ISP116X_HCD)	+= isp116x-hcd.o
obj-$(CONFIG_USB_OHCI_HCD)	+= ohci-hcd.o
obj-$(CONFIG_USB_UHCI_HCD)	+= uhci-hcd.o
obj-$(CONFIG_USB_SL811_HCD)	+= sl811-hcd.o
obj-$(CONFIG_USB_SL811_CS)	+= sl811_cs.o
obj-$(CONFIG_USB_U132_HCD)	+= u132-hcd.o
obj-$(CONFIG_USB_R8A66597_HCD)	+= r8a66597-hcd.o
obj-$(CONFIG_USB_DWC_HCD) += dwc_otg_hcd.o dwc_otg_hcd_intr.o dwc_otg_hcd_queue.o \
			dwc_otg_attr.o dwc_otg_driver.o dwc_otg_cil.o \
			dwc_otg_cil_intr.o dwc_otg_proc.o dwc_otg_ifx.o
