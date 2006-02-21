#
# Makefile for the Linux 802.x protocol layers.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definition is now in the main makefile...

O_TARGET := 802.o

export-objs = llc_macinit.o p8022.o psnap.o fc.o

obj-y			:= p8023.o

obj-$(CONFIG_SYSCTL)	+= sysctl_net_802.o

subdir-$(CONFIG_LLC)	+= transit
obj-$(CONFIG_LLC)	+= llc_sendpdu.o llc_utility.o cl2llc.o llc_macinit.o
obj-$(CONFIG_LLC)	+= p8022.o psnap.o

obj-$(CONFIG_TR)	+= p8022.o psnap.o tr.o

obj-$(CONFIG_NET_FC)	+=                 fc.o

obj-$(CONFIG_FDDI)	+=                 fddi.o

obj-$(CONFIG_HIPPI)	+=                 hippi.o

obj-$(CONFIG_IPX)	+= p8022.o psnap.o

obj-$(CONFIG_ATALK)	+= p8022.o psnap.o


include $(TOPDIR)/Rules.make

cl2llc.c: cl2llc.pre
	@rm -f $@
	sed -f ./pseudo/opcd2num.sed cl2llc.pre >cl2llc.c
