#
# Makefile for the BCM947xx specific kernel interface routines
# under Linux.
#

EXTRA_CFLAGS+=-I$(TOPDIR)/arch/mips/bcm947xx/include -DBCMDRIVER

O_TARGET        := bcm947xx.o

export-objs     := nvram_linux.o setup.o
obj-y		:= prom.o setup.o time.o sbmips.o gpio.o perfcntr.o
obj-y		+= nvram.o nvram_linux.o
obj-$(CONFIG_PCI) += sbpci.o pcibios.o
obj-$(CONFIG_SFLASH) += sflash.o

include $(TOPDIR)/Rules.make
