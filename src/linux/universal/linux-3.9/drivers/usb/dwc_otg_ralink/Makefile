#
# Makefile for DWC_otg Highspeed USB controller driver
#

ifeq ($(CONFIG_DWC_OTG_DEBUG_RALINK),y)
EXTRA_CFLAGS   += -DDEBUG
endif

# Use one of the following flags to compile the software in host-only or
# device-only mode.
ifeq ($(CONFIG_DWC_OTG_HOST_ONLY_RALINK),y)
EXTRA_CFLAGS   += -DDWC_HOST_ONLY
EXTRA_CFLAGS   += -DDWC_EN_ISOC 
endif

ifeq ($(CONFIG_DWC_OTG_DEVICE_ONLY_RALINK),y)
EXTRA_CFLAGS   += -DDWC_DEVICE_ONLY
endif

EXTRA_CFLAGS	+= -DDWC_HS_ELECT_TST

obj-$(CONFIG_DWC_OTG_RALINK)	:= dwc_otg.o

dwc_otg-objs	:= dwc_otg_driver.o dwc_otg_attr.o 
dwc_otg-objs	+= dwc_otg_cil.o dwc_otg_cil_intr.o 
dwc_otg-objs	+= dwc_otg_pcd.o dwc_otg_pcd_intr.o 
dwc_otg-objs	+= dwc_otg_hcd.o dwc_otg_hcd_intr.o dwc_otg_hcd_queue.o

