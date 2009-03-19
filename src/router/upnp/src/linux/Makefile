#
# Copyright (C) 2008, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.10 2008/10/27 08:55:56 Exp $
#

include $(SRCBASE)/router/.config

ifndef DEBUG
#export DEBUG=1
endif
#SRCBASE=../../../..
#UPNPBASE=../upnp
UPNP= $(shell (cd ../.. && pwd -P))
UPNPBASE=$(UPNP)/src
UPNPDEV=$(UPNP)/device
CC = $(TARGET_PREFIX)gcc
AS = $(TARGET_PREFIX)as
AR = $(TARGET_PREFIX)ar
LD = $(TARGET_PREFIX)ld
CCPP = $(TARGET_PREFIX)g++
COMPILER = $(TARGET_PREFIX)g++
SIZE = $(TARGET_PREFIX)size
RANLIB = $(TARGET_PREFIX)ranlib

WFAPATH=$(SRCBASE)/wps/common/include
VPATH.h=$(shell pwd):$(UPNPDEV):$(UPNP)/include:$(UPNPDEV)/InternetGatewayDevice:$(UPNPDEV)/WFADevice:$(SRCBASE)/include.v24:$(SRCBASE)/router/shared:$(WFAPATH)

vpath %.c .:$(UPNPDEV):$(UPNPDEV)/InternetGatewayDevice:$(UPNPDEV)/WFADevice:$(SRCBASE)/router/shared:$(SRCBASE)/shared/netconf
vpath %.h $(VPATH.h)

ifeq ($(DEBUG),1)
CFLAGS += -g -DDEBUG -UNDEBUG -DBCMDBG
else
CFLAGS += -Os -DNDEBUG
endif
CFLAGS += -I$(subst :, -I,$(VPATH.h))
CFLAGS += -Wall -Wunused 
#CFLAGS += --trace-includes
OBJDIR=obj

SOURCES = device_init.c

#ifeq ($(CONFIG_NAT),y)
ifeq ($(strip $(CFG_BCMIGD)),1)
SOURCES += InternetGatewayDevice_table.c \
		  InternetGatewayDevice.c \
		  soap_x_lanhostconfigmanagement.c \
		  soap_x_layer3forwarding.c \
		  soap_x_wancommoninterfaceconfig.c \
		  soap_x_wanipconnection.c \
		  xml_InternetGatewayDevice.c \
		  xml_x_lanhostconfigmanagement.c \
		  xml_x_layer3forwarding.c \
		  xml_x_wancommoninterfaceconfig.c \
		  xml_x_wanipconnection.c
endif
#endif


SOURCES += linux_main.c linux_osl.c

SOURCES += UdpLib.c

export LDFLAGS = -shared -o			
SOURCES.OBJ := $(SOURCES)
SOURCES.OBJ := $(patsubst %.c,$(OBJDIR)/%.o,$(SOURCES.OBJ))

TARGET = upnp
LIBS = -L$(UPNPBASE) -L$(SRCBASE)/router/netconf -L$(TOP)/libutils \
	-L$(SRCBASE)/router/nvram -L$(SRCBASE)/router/shared -lupnp -lnetconf -lutils -lnvram -lshared

all : $(OBJDIR) $(TARGET)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(SOURCES.OBJ): upnp.h

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


$(UPNPBASE)/libupnp.a : FORCE
	$(MAKE) -C $(UPNPBASE) DEBUG=$(DEBUG) PLATFORM=x86 CC=$(CC) LD=$(LD) VENDOR=$(VENDOR)

$(TARGET) :: $(UPNPBASE)/libupnp.a  $(SOURCES.OBJ)
	$(CC) -o $(TARGET) $(SOURCES.OBJ) $(LIBS)
	$(SIZE)  $(TARGET)

clean: 
	rm -rf obj 
	rm -f upnp
	$(MAKE) -C $(UPNPBASE) clean
ifneq ($(BUILD_LIBS),)
	$(MAKE) -C $(SRCBASE)/router/netconf clean
	$(MAKE) -C $(SRCBASE)/router/iptables clean
	$(MAKE) -C $(SRCBASE)/router/nvram clean
endif

FORCE:

.PHONY: FORCE clean libs
