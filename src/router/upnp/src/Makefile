#
# Broadcom UPnP module makefile for linux platform
#
# Copyright (C) 2008, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.6 2007/11/23 09:58:38 Exp $
#

ifndef DEBUG
#export DEBUG=1
endif
SRCBASE=../../..

CC = $(TARGET_PREFIX)gcc
AS = $(TARGET_PREFIX)as
AR = $(TARGET_PREFIX)ar
LD = $(TARGET_PREFIX)ld
CCPP = $(TARGET_PREFIX)g++
COMPILER = $(TARGET_PREFIX)g++
SIZE = $(TARGET_PREFIX)size
RANLIB = $(TARGET_PREFIX)ranlib

VPATH.h=.:../include:$(SRCBASE)/include:$(SRCBASE)/router/shared:$(SRCBASE)/include/bcmcrypto

vpath %.c .:$(SRCBASE)/router/shared:$(SRCBASE)/shared/netconf
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

SOURCES = upnp.c ssdp.c upnp_http.c gena.c soap.c \
	  upnp_description.c upnp_device.c upnp_req.c upnp_util.c

SOURCES.OBJ := $(SOURCES)
SOURCES.OBJ := $(patsubst %.c,$(OBJDIR)/%.o,$(SOURCES.OBJ))

vpath %.c .:
vpath %.h .:../include

TARGET = libupnp.a
LIBS = 

all : $(OBJDIR) $(TARGET)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(SOURCES.OBJ): upnp.h

$(OBJDIR)/%.o:	%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<


$(TARGET) : $(SOURCES.OBJ)
	$(AR) -rcs $(TARGET) $(SOURCES.OBJ) 
	$(RANLIB) $(TARGET)
	$(SIZE)  $(TARGET)

clean: 
	rm -rf obj 
