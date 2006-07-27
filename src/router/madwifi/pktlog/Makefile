#
# Makefile for Atheros packet log.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...

ifeq ($(obj),)
obj= .
endif

obj-m		+= ath_pktlog.o
ath_pktlog_objs	:= pktlog.o

DEPTH=..

# release tag versioning
-include $(KERNELPATH)/ath_version.mk
-include $(obj)/${DEPTH}/Makefile.inc

ifeq ($(findstring 2.6,$(KERNELRELEASE)),)
export-objs	:= pktlog.o
list-multi	:= ath_pktlog.o
endif

ifneq ($(MAKING_MODULES),1)
obj-$(CONFIG_PKTLOG) := $(ath_pktlog_objs)
ifeq ($(findstring 2.6,$(KERNELRELEASE)),)
O_TARGET := ath_pktlog.o
endif
endif

INCS+=  -I. -I${HAL} -I${HAL}/${OS} -I${obj}/${ATH} -I${obj}/${WLAN} \
        -I${obj}/${DEPTH}

EXTRA_CFLAGS+=${INCS} ${COPTS2}

-include $(TOPDIR)/Rules.make

all:
	$(MAKE)	-C $(KERNELPATH) SUBDIRS=$(shell pwd) modules

install: all
	$(STRIP) -S pktlog.o
	cp pktlog.o ${KERNELPATH}/arch/mips/ar531x/ROOTDISK/rootdir/lib/modules/$(strip $(shell head -n 1 $(KERNELPATH)/Makefile | cut -f 2 -d'=')).$(strip $(shell head -n 2 $(KERNELPATH)/Makefile | tail -1 | cut -f 2 -d'=')).$(strip $(shell head -n 3 $(KERNELPATH)/Makefile | tail -1 | cut -f 2 -d'='))${EXTRAVERSION}/net/.

clean:
	-rm -f *~ *.o *.ko *.mod.c
	-rm -f .version .*.o.flags .*.o.d .*.o.cmd .*.ko.cmd

ath_pktlog.o: ${ath_pktlog_objs}
	$(LD) $(LDOPTS) -o ath_pktlog.$(KMODSUF) -r $(ath_pktlog_objs)
