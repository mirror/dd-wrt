#
# Makefile for the PMC-Sierra Titan 
#

USE_STANDARD_AS_RULE := true

O_TARGET:= titan.o

obj-y    += irq-handler.o irq.o i2c-yosemite.o prom.o setup.o

obj-$(CONFIG_KGDB)		+= dbg_io.o
obj-$(CONFIG_PCI)		+= pci-irq.o pci.o
obj-$(CONFIG_SMP)          	+= smp.o
obj-$(CONFIG_HYPERTRANSPORT)	+= ht-irq.o ht.o

include $(TOPDIR)/Rules.make
