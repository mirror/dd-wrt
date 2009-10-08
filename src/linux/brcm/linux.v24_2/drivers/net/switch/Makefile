#
# drivers/net/wireless/Makefile
#
# Makefile for the Linux Wireless network device drivers.
#
EXTRA_CFLAGS := -I$(TOPDIR)/arch/mips/bcm947xx/include -DCONFIG_NOPRINTK


O_TARGET := broadcom_switch.o

obj-y		:=
obj-m		:=
obj-n		:=
obj-		:=

# Things that need to export symbols
export-objs	:= switch-core.o switch-adm.o switch-robo.o

obj-$(CONFIG_BRCMSWITCH)		+= switch-core.o switch-adm.o switch-robo.o 

include $(TOPDIR)/Rules.make
