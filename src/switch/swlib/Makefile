
#*****************************************************************************
#
#  Copyright 2000
#  Broadcom Corporation
#  16215 Alton Parkway
#  PO Box 57013
#  Irvine CA 92619-7013
#
#****************************************************************************/
###############################################################################
# 
# swlib SDK Makefile
#
###############################################################################


ifeq ($(LINUXDIR),)
ifeq ($(LINUXVER),)
# Neither one is specified, use uname for version
LINUXVER = $(shell uname -r)
endif
else
ifeq ($(LINUXVER),)
# divine LINUXVER from LINUXDIR
LINUXVER = $(shell sed -ne 's/.*UTS_RELEASE \([^ ]\)/\1/p' -e q < $(LINUXDIR)/include/linux/version.h)
endif
endif

# Now try a couple of places for LINUXDIR if not specified
ifeq ($(LINUXDIR),)
ifneq ($(wildcard /lib/modules/$(LINUXVER)/build/include/linux/version.h),)
LINUXDIR = /lib/modules/$(LINUXVER)/build
else
ifneq ($(wildcard /tools/linux/src/linux-$(LINUXVER)/include/linux/version.h),)
LINUXDIR = /tools/linux/src/linux-$(LINUXVER)
else
LINUXDIR = /usr/src/linux
endif
endif
endif

CROSS_COMPILATION       =1

# Note: Never set the below in makefiles -- it should already be set
#CROSS_COMPILE=mipsel-linux-

CC              =$(CROSS_COMPILE)gcc
LD              =$(CROSS_COMPILE)ld
RANLIB          =$(CROSS_COMPILE)ranlib
SWAPIDIR        =../sys
LINUX          =$(LINUXDIR)

CFLAGS          = -I$(LINUX)/include -Dlinux -D__linux__
LDFLAGS         = -r
SWLIBFLAGS      = -I$(SWAPIDIR) -I../swmod -I../../include -D__klsi__

CMD_ECHO= @	# Suppresses shell command echos; comment out for verbose
MAKE_ECHO= -s	# Suppresses make command echos; comment out for verbose
SHELL		= /bin/sh


BINDIR		= ../../router/install-mipsel/lib
DBGFLAGS += -g #-DDEBUG
WARNINGS= -Wall
CFLAGS  += -O2 -ansi $(INCLUDES) $(DBGFLAGS) $(WARNINGS)
CFLAGS += $(SWLIBFLAGS)
LDFLAGS += -r 
CFLAGS += $(BCM_OEM_DEFINES) -DUSER_APPLICATION
ifeq ($(BOARD),bcm94702cpci)
CFLAGS += -DBCM5380
endif
ifeq ($(BOARD),bcm5380rr)
CFLAGS += -DBCM5380
endif

ifndef BINNAME
BINNAME=libsw.a
endif

LIBNAME=

RM = /bin/rm
RMFLAGS = -f

# to build shared library
SHRCFLAGS = -O2 -ansi -pedantic -fPIC -c $(INCLUDES) -w $(DBGFLAGS)
SHRLDFLAGS = -shared $(LINUXIA64) 
SHRCFLAGS += $(SWLIBFLAGS)
SHRCFLAGS += $(BCM_OEM_DEFINES) -DUSER_APPLICATION
ifeq ($(BOARD),bcm94702cpci)
SHRCFLAGS += -DBCM5380
endif
ifeq ($(BOARD),bcm5380rr)
SHRCFLAGS += -DBCM5380
endif

ifndef SHRBINNAME
SHRBINNAME=libsw.so
endif


.SUFFIXES:
.SUFFIXES: .c .o .so

# files for a (source) distribution

LIBSWOBJ =	swapi.o osl_linux.o
LIBSWSOBJ =	swapi.so

all: $(BINNAME)  $(SHRBINNAME)

shared: $(SHRBINNAME)

$(BINNAME): $(LIBSWOBJ) $(LIBNAME)
#	@echo Linking...
#	$(CMD_ECHO)
	$(LD) -o $(BINNAME)  $(LDFLAGS) $(LIBSWOBJ) $(LIBNAME)

$(SHRBINNAME): $(LIBSWSOBJ) $(LIBSWOBJ) $(LIBNAME)
#	@echo Linking...
#	$(CMD_ECHO)
	$(LD) -o $(SHRBINNAME) $(SHRLDFLAGS) $(LIBSWOBJ) $(LIBNAME)

clean:
#	@echo Cleaning up files...
#	$(CMD_ECHO)
	$(RM) $(RMFLAGS) $(BINNAME) $(LIBSWOBJ) $(LIBSWSOBJ) $(SHRBINNAME) *~

rebuild:
	$(CMD_ECHO)make $(MAKE_ECHO) clean
	$(CMD_ECHO)make $(MAKE_ECHO)

install:  $(SHRBINNAME)
#	@echo Installing shared swlib...
#	$(CMD_ECHO)
	install $(SHRBINNAME) $(BINDIR)

%.o: $(SWAPIDIR)/%.c
#	@echo Compiling $<...
#	$(CMD_ECHO)
	$(CC) $(CFLAGS) -c $<

%.so: $(SWAPIDIR)/%.c
#	@echo Compiling $< \(shared\) $(LINUXIA64)...
#	$(CMD_ECHO)
	$(CC) $(SHRCFLAGS)  -c $< -o $@

# Dependecies

ifneq ($(wildcard ../switch/swmod),)
DEPENDH = $(SWAPIDIR)/swapi.h $(SWAPIDIR)/etc53xx.h
endif

swapi.o: $(DEPENDH)
osl_linux.o: $(DEPENDH)
swapi.so: $(DEPENDH)


