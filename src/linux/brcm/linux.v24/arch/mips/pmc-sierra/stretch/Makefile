#
# Makefile for PMC-Sierra Stretch Board
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#

USE_STANDARD_AS_RULE := true

O_TARGET:= stretch.o

obj-y	 += prom.o reset.o setup.o irq-handler.o irq.o

obj-$(CONFIG_PCI)	   += pci-irq.o pci.o
obj-$(CONFIG_REMOTE_DEBUG) += dbg_io.o

include $(TOPDIR)/Rules.make
