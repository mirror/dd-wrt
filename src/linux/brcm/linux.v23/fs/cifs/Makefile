#
# Makefile for Linux CIFS VFS client 
#
O_TARGET := cifs.o

obj-y := cifsfs.o cifssmb.o cifs_debug.o connect.o dir.o file.o inode.o link.o misc.o netmisc.o smbdes.o smbencrypt.o transport.o asn1.o md4.o md5.o cifs_unicode.o nterr.o cifsencrypt.o

obj-m := $(O_TARGET)

include $(TOPDIR)/Rules.make
