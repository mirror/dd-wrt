VPATH = .

#CROSS = ~/openwrt/staging_dir_mipsel

#CC = $(CROSS)/bin/mipsel-linux-uclibc-gcc
#LD = $(CC)
CFLAGS = -I. $(COPTS) $(MIPS16_OPT) $(LTO) -Wall -Wno-unused -s -fpack-struct -Wpadded -ffunction-sections -fdata-sections -Wl,--gc-sections
LDFLAGS =  $(LDLTO) $(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections

OS_LAYER = osl-linux.c

all: lld2d

include common.mk

depend:
	$(CC) $(CFLAGS) -M $(DCFILES) >.depend

-include .depend
