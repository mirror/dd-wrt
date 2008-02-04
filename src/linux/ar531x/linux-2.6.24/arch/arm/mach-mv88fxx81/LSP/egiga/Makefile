#
# Makefile for the Marvell egiga ethernet driver
#

include $(TOPDIR)/arch/arm/mach-mv88fxx81/mv88f5181/mvRules.mk

obj-$(CONFIG_ARCH_MV88f5181) += egiga.o

EGIGA_OBJS = mv_e_main.o

ifeq ($(CONFIG_EGIGA_PROC),y)
EGIGA_OBJS += mv_e_proc.o
endif
 
egiga-objs := $(EGIGA_OBJS)
