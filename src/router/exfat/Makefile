# SPDX-License-Identifier: GPL-2.0-or-later
#
# Makefile for the linux exFAT filesystem support.
#
obj-$(CONFIG_EXFAT_FS) += exfat.o

exfat-y	:= inode.o namei.o dir.o super.o fatent.o cache.o nls.o misc.o \
	   file.o balloc.o
KERNELRELEASE := $(shell cat $(LINUXDIR)/include/config/kernel.release 2> /dev/null)
MDIR := $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)
KDIR := $(LINUXDIR)
PWD	:= $(shell pwd)

export CONFIG_EXFAT_FS := m

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

help:
	$(MAKE) -C $(KDIR) M=$(PWD) help

install: exfat.ko
	rm -f ${MDIR}/kernel/fs/exfat/exfat.ko
	install -m644 -b -D exfat.ko ${MDIR}/kernel/fs/exfat/exfat.ko
	depmod -aq

uninstall:
	rm -rf ${MDIR}/kernel/fs/exfat
	depmod -aq


.PHONY : all clean install uninstal
