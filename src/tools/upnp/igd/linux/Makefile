#
# Copyright 2005, Broadcom Corporation
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.8 2005/03/07 08:35:32 kanki Exp $
#

ifndef DEBUG
#export DEBUG=1
endif

SRCBASE=../../../..
ifeq ($(PLATFORM), mipsel)
    export PATH:=/projects/hnd/tools/linux/hndtools-mipsel-linux/bin:${PATH}
    TARGET_PREFIX = mipsel-linux-
    CFLAGS += -DMIPS
else
    TARGET_PREFIX =
endif

CC = $(TARGET_PREFIX)gcc
AS = $(TARGET_PREFIX)as
AR = $(TARGET_PREFIX)ar
LD = $(TARGET_PREFIX)ld
CCPP = $(TARGET_PREFIX)g++
COMPILER = $(TARGET_PREFIX)g++
STRIP = $(TARGET_PREFIX)strip
SIZE = $(TARGET_PREFIX)size
include $(SRCBASE)/router/.config


ifeq ($(CONFIG_MSSID),y)
VPATH.h = .:..:../../include:$(SRCBASE)/include.v24:$(SRCBASE)/router/shared
else
VPATH.h = .:..:../../include:$(SRCBASE)/include.v23:$(SRCBASE)/router/shared
endif
vpath %.c .:..:$(SRCBASE)/shared/netconf
vpath %.h $(VPATH.h)


ifeq ($(DEBUG),1)
CFLAGS += -g -DDEBUG -UNDEBUG -DBCMDBG
else
CFLAGS += -O2 -DNDEBUG
endif
CFLAGS += -I$(subst :, -I,$(VPATH.h))
CFLAGS += -Wall -Wunused -ffunction-sections -fdata-sections
#CFLAGS += --trace-includes
OBJDIR=obj

# compile in the generic UPNP IGD files.
#
SOURCES = igd.c igd_desc.c wancommon.c wanipc.c ipt.c \
	  layer3.c  \
	  x_wanpppconnection.c wanppp.c \
	  mapmgr.c

# Add the OS-specific source files.
SOURCES += linux_main.c linux_net.c linux_igd.c linux_timer.c


SOURCES.TEST = test.c shutils.c 

SOURCES.OBJ := $(SOURCES)
SOURCES.OBJ := $(patsubst %.c,$(OBJDIR)/%.o,$(SOURCES.OBJ))

TARGET = upnp
LIBS = -L../../upnp/linux -L$(SRCBASE)/router/netconf \
	-L$(SRCBASE)/router/nvram -L$(SRCBASE)/router/shared -L$(SRCBASE)/router/libutils -lupnp -lnetconf -lnvram -lutils -lshared -L../zlib -Wl,--gc-sections

all : $(OBJDIR) $(TARGET)

$(OBJDIR):
	mkdir -p $(OBJDIR)

wanipc.c : wanipc.h

ipt.c : wanipc.h

ifeq ($(CONFIG_MSSID),y)
$(SOURCES.OBJ): upnp.h  $(SRCBASE)/include.v24/cyutils.h $(SRCBASE)/include.v24/code_pattern.h
else
$(SOURCES.OBJ): upnp.h  $(SRCBASE)/include.v23/cyutils.h $(SRCBASE)/include.v23/code_pattern.h
endif
$(OBJDIR)/%.o:	%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

# uncomment BUILD_LIBS if you want this makefile to build the netconf and itables libraries.
#BUILD_LIBS=1

ifneq ($(BUILD_LIBS),)

$(TARGET) :: libnetconf.so

$(TARGET) :: $(SRCBASE)/router/iptables/libiptables.a 

$(TARGET) :: $(SRCBASE)/router/nvram/libnvram.a 

$(SRCBASE)/router/iptables/libiptables.a : FORCE
	$(MAKE) -C $(SRCBASE)/router/iptables PLATFORM=x86 CC=$(CC) LD=$(LD) SRCBASE=../.. TOP=..
$(SRCBASE)/router/nvram/libnvram.a : FORCE
	$(MAKE) -C $(SRCBASE)/router/nvram PLATFORM=x86 CC=$(CC) LD=$(LD) SRCBASE=../.. TOP=..

$(SRCBASE)/router/netconf/libnetconf.so : $(SRCBASE)/router/iptables/libiptables.a FORCE
	$(MAKE) -C $(SRCBASE)/router/netconf DEBUG=1 PLATFORM=x86 CC=$(CC) LD=$(LD) SRCBASE=../.. TOP=..

libnetconf.so : $(SRCBASE)/router/netconf/libnetconf.so
	#	cp $(SRCBASE)/router/netconf/libnetconf.so /router/usr/lib/

endif


../../upnp/linux/libupnp.a : FORCE
	$(MAKE) -C ../../upnp/linux DEBUG=$(DEBUG) PLATFORM=x86 CC=$(CC) LD=$(LD) VENDOR=$(VENDOR)

$(TARGET) :: ../../upnp/linux/libupnp.a  $(SOURCES.OBJ)
	$(CC) -o $(TARGET) $(SOURCES.OBJ) $(LIBS)
ifneq ($(DEBUG),1)
	$(STRIP)  $(TARGET)
endif
	$(SIZE)  $(TARGET)

clean: 
	rm -rf obj 
	rm -f upnp
	$(MAKE) -C ../../upnp/linux clean
ifneq ($(BUILD_LIBS),)
	$(MAKE) -C $(SRCBASE)/router/netconf clean
	$(MAKE) -C $(SRCBASE)/router/iptables clean
	$(MAKE) -C $(SRCBASE)/router/nvram clean
endif

fptest: fptest.o
	$(CC) -o fptest fptest.o $(LIBS)

btest: btest.o
	$(CC) -o btest btest.o

FORCE:

.PHONY: FORCE clean libs
