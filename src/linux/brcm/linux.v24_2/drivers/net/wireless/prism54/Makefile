
O_TARGET := prism54.o

EXTRA_CFLAGS += -DPRISM54_COMPAT24

obj-y		:= isl_38xx.o islpci_dev.o islpci_eth.o \
		   islpci_mgt.o islpci_hotplug.o isl_ioctl.o \
		   oid_mgt.o

obj-m		+= prism54.o

include $(TOPDIR)/Rules.make
