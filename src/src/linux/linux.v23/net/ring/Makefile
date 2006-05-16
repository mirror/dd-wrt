#
# Makefile for the ring driver.
#

O_TARGET := ring.o

export-objs := ring_packet.o

obj-y := ring_packet.o

ifeq ($(CONFIG_RING),m)
  obj-m += $(O_TARGET)
endif

include $(TOPDIR)/Rules.make

