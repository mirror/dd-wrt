# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the out-of-tree Linux APFS module.
#
IDIR := $(INSTALLDIR)/lib/modules/$(KERNEL_VERSION)/net/sched

obj-m = apfs.o
apfs-y = btree.o dir.o extents.o file.o inode.o key.o message.o \
	 namei.o node.o object.o super.o symlink.o unicode.o xattr.o xfield.o transaction.o spaceman.o

default:
	make -C $(LINUXDIR) M=$(shell pwd)
clean:
	make -C $(LINUXDIR) M=$(shell pwd) clean

install:
	install -v -m 644 -D apfs.ko $(IDIR)/apfs.ko
