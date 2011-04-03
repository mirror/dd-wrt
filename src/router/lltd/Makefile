VPATH = .

#CROSS = ~/openwrt/staging_dir_mipsel

#CC = $(CROSS)/bin/mipsel-linux-uclibc-gcc
#LD = $(CC)
CFLAGS = -I. $(COPTS) -Wall -Wno-unused -s -fpack-struct -Wpadded
#LDFLAGS = -L~/openwrt/staging_dir_mipsel/usr/lib

OS_LAYER = osl-linux.c

all: lld2d

include common.mk

depend:
	$(CC) $(CFLAGS) -M $(DCFILES) >.depend

-include .depend
