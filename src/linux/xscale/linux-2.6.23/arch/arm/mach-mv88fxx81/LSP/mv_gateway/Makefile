#
# Makefile for the Marvell gateway ethernet driver
#

include $(TOPDIR)/arch/arm/mach-mv88fxx81/mv88f5181/mvRules.mk

obj-$(CONFIG_ARCH_MV88f5181) += mv_gateway.o

MV_GTW_OBJS = mv_gtw_main.o

ifeq ($(CONFIG_MV_GTW_PROC),y)
MV_GTW_OBJS += mv_gtw_proc.o
endif
 
mv_gateway-objs := $(MV_GTW_OBJS)
