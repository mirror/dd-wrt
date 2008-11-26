#
# Makefile for the Marvell USB device controller
#
ifeq ($(CONFIG_ARCH_FEROCEON_ORION),y)
	include $(TOPDIR)/arch/arm/mach-feroceon-orion/config/mvRules.mk
endif
 
ifeq ($(CONFIG_ARCH_FEROCEON_KW),y)
	include $(TOPDIR)/arch/arm/mach-feroceon-kw/config/mvRules.mk
endif

ifeq ($(CONFIG_ARCH_FEROCEON_MV78XX0),y)
	include $(TOPDIR)/arch/arm/mach-feroceon-mv78xx0/config/mvRules.mk
endif

USB_DIR := ../mv_hal/usb/
USB_PATH := $(TOPDIR)/arch/arm/plat-feroceon/mv_hal/usb

USB_DEV_DIR := $(USB_DIR)/device

CFLAGS  += $(EXTRA_CFLAGS) -I$(USB_PATH)/api -I$(USB_PATH)/common -I$(USB_PATH)/device

OBJS        := mv_udc_main.o ../$(USB_DEV_DIR)/mvUsbDevCh9.o ../$(USB_DEV_DIR)/mvUsbDevMain.o 	\
               ../$(USB_DEV_DIR)/mvUsbDevRecv.o ../$(USB_DEV_DIR)/mvUsbDevSend.o ../$(USB_DEV_DIR)/mvUsbDevUtl.o 	\
	       ../$(USB_DEV_DIR)/mvUsbHsDevUtl.o  ../$(USB_DEV_DIR)/mvUsbHsDevMain.o ../$(USB_DEV_DIR)/mvUsbHsDevCncl.o

mv_udc-objs := $(OBJS)
obj-m := mv_udc.o

