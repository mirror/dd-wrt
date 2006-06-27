#
#  Copyright 2000 MontaVista Software Inc.
#  Author: MontaVista Software, Inc.
#     	ppopov@mvista.com or source@mvista.com
#
# Makefile for the Alchemy Semiconductor PB1000 board.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#

USE_STANDARD_AS_RULE := true

O_TARGET := db1x00.o

obj-y := init.o board_setup.o irqmap.o
obj-$(CONFIG_WM97XX_COMODULE) += mirage_ts.o

ifdef CONFIG_MIPS_DB1100
ifdef CONFIG_MMC
obj-y += mmc_support.o
export-objs += mmc_support.o
endif
endif

include $(TOPDIR)/Rules.make
