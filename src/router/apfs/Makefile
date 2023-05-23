# SPDX-License-Identifier: GPL-2.0-only
#
# Makefile for the out-of-tree Linux APFS module.
#
KERNEL_VERSION := $(shell cat $(LINUXDIR)/include/config/kernel.release 2> /dev/null)
IDIR := $(INSTALLDIR)/lib/modules/$(KERNEL_VERSION)/net/sched
#CFLAGS_EXTRA += 

obj-m = apfs.o

apfs-y := btree.o compress.o dir.o extents.o file.o inode.o key.o libzbitmap.o \
	  lzfse/lzfse_decode.o lzfse/lzfse_decode_base.o lzfse/lzfse_fse.o \
	  lzfse/lzvn_decode_base.o message.o namei.o node.o object.o snapshot.o \
	  spaceman.o super.o symlink.o transaction.o unicode.o xattr.o xfield.o

default:
	make -C $(LINUXDIR) M=$(shell pwd) EXTRA_CFLAGS="$(CFLAGS_EXTRA)"
clean:
	make -C $(LINUXDIR) M=$(shell pwd) clean

install:
	install -v -m 644 -D apfs.ko $(IDIR)/apfs.ko
