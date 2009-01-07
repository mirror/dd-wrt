#
# Copyright 2004, Broadcom Corporation
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.7 2004/04/12 05:42:39 honor Exp $
#

ifndef DEBUG
#export DEBUG=1
endif
SRCBASE=../../../..

CC = $(TARGET_PREFIX)gcc
AS = $(TARGET_PREFIX)as
AR = $(TARGET_PREFIX)ar
LD = $(TARGET_PREFIX)ld
CCPP = $(TARGET_PREFIX)g++
COMPILER = $(TARGET_PREFIX)g++
STRIP = $(TARGET_PREFIX)strip
SIZE = $(TARGET_PREFIX)size
RANLIB = $(TARGET_PREFIX)ranlib

include $(SRCBASE)/router/.config

ifeq ($(CONFIG_MSSID),y)
VPATH.h=.:..:../../include:$(SRCBASE)/include.v24:$(SRCBASE)/router/shared
else
VPATH.h=.:..:../../include:$(SRCBASE)/include.v23:$(SRCBASE)/router/shared
endif

vpath %.c .:..:$(SRCBASE)/router/shared:$(SRCBASE)/shared/netconf
vpath %.h $(VPATH.h)

ifeq ($(DEBUG),1)
CFLAGS += -g -DDEBUG -UNDEBUG -DBCMDBG
else
CFLAGS += -O2
endif
CFLAGS += -I$(subst :, -I,$(VPATH.h))
CFLAGS += -Wall -Wunused 
#CFLAGS += --trace-includes
OBJDIR=obj

SOURCES = upnp.c ssdp.c http.c gena.c soap.c \
	  service.c device.c  \
	  uio.c

SOURCES.OBJ := $(SOURCES)
SOURCES.OBJ := $(patsubst %.c,$(OBJDIR)/%.o,$(SOURCES.OBJ))

vpath %.c .:..:
vpath %.h .:..:../../include

TARGET = libupnp.a
LIBS = 

all : $(OBJDIR) $(TARGET)

$(OBJDIR):
	mkdir -p $(OBJDIR)

ifeq ($(CONFIG_MSSID),y)
$(SOURCES.OBJ): upnp.h  $(SRCBASE)/include.v24/cyutils.h $(SRCBASE)/include.v24/code_pattern.h
else
$(SOURCES.OBJ): upnp.h  $(SRCBASE)/include.v23/cyutils.h $(SRCBASE)/include.v23/code_pattern.h
endif

$(OBJDIR)/%.o:	%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<


$(TARGET) : $(SOURCES.OBJ)
	$(AR) -rcs $(TARGET) $(SOURCES.OBJ) 
	$(RANLIB) $(TARGET)
	$(SIZE)  $(TARGET)

clean: 
	rm -rf obj 



timertest: $(OBJDIR)/linux_timer.o $(OBJDIR)/timertest.o
	$(CC) $(OBJDIR)/linux_timer.o $(OBJDIR)/timertest.o -o timertest
