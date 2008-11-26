

#
# Makefile for the Marvell Phone Device Driver
#

include $(TOPDIR)/arch/arm/mach-mv88fxx81/mv88f5181/mvRules.mk

obj-$(CONFIG_MV_PHONE) += mv_phone.o

MP_OBJS = mp.o ../../Board/slic/proslic.o

ifeq ($(CONFIG_VOIP_RD2_5181L),y)
MP_OBJS += ../../Soc/tdm/mvTdm.o
else
MP_OBJS += ../../Board/tdm-fpga/mvTdmFpga.o
endif

mv_phone-objs := $(MP_OBJS)

