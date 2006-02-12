
#******************************************************************************
# 
#   Copyright 2000
#   Broadcom Corporation
#   16215 Alton Parkway
#   PO Box 57013
#   Irvine CA 92619-7013
# 
#*****************************************************************************/

#******************************************************************************
#
# Device makefile for switch module
# 
#******************************************************************************



SHELL = /bin/sh
#CMD_ECHO= @	# Suppresses shell command echos; comment out for verbose
MAKE_ECHO= -s	# Suppresses make command echos; comment out for verbose
CSH=/bin/csh


# Try to determine the platform's CPU
# Just in case this fails, we default to an x86 CPU

ifndef LDFLAGS
  LDFLAGS =  -r   # PC platform linker flags
endif

# compiler warning switch
WARNING=-Wall -Wno-unused -Wstrict-prototypes -Wno-trigraphs

# directories for header files and library file
OPTFLAGS=-O2

BCM_ROBOUTILS_PATH=../sys
CFLAGS += -I../../include -I$(BCM_ROBOUTILS_PATH)
CFLAGS += $(OPTFLAGS) -I$(LINUXDIR)/include $(INCLFLAGS) $(WARNING) $(DEBUG_SWITCH)  $(MODULE_FLAGS) $(KERNEL_FLAGS)
CFLAGS += -D__KERNEL__ -fno-strict-aliasing -fno-common -fomit-frame-pointer
CFLAGS += -I $(LINUXDIR)/include/asm/gcc -G 0 -mno-abicalls -fno-pic -pipe -mcpu=r4600 -mips2
CFLAGS += -Wa,--trap -m4710a0kern -nostdinc -iwithprefix include -DPORT_SCAN_TIME=0
CFLAGS += -DBCM5850 -I../../et/sys


CC = mipsel-linux-gcc
LD = mipsel-linux-ld -r
RM = /bin/rm
RMFLAGS = -f
BINNAME=robo.o
BINDIR=../../router/install-mipsel/usr/lib
SWLIBDIR = ../swlib
LIBNAME = $(SWLIBDIR)/libswk.a

.SUFFIXES:
.SUFFIXES: .c .o

# files for a (source) distribution

ROBOSRC =		\
	swmod.c

OBJSROBO =		\
	swmod.o if_robo.o

all: $(BINNAME)

$(BINNAME): $(OBJSROBO)
	@echo Linking...
	$(CMD_ECHO)$(LD) -o $(BINNAME)  $(LDFLAGS) $(OBJSROBO) $(LIBNAME)
clean:
	@echo Cleaning up object files...
	$(CMD_ECHO)$(RM) $(RMFLAGS) $(BINNAME) $(OBJSROBO) *~

rebuild:
	$(CMD_ECHO)make $(MAKE_ECHO) clean
	$(CMD_ECHO)make $(MAKE_ECHO) 

install:  $(BINNAME)
	@echo Installing Robo switch module...
	$(CMD_ECHO)install $(BINNAME) $(BINDIR)

swmod.o: swmod.c
	@echo Compiling $(SMP_STRING)$(CHIP_STRING) $(LINUXIA64) $<...
	$(CMD_ECHO)$(CC) $(CFLAGS) -c $<

if_robo.o: $(BCM_ROBOUTILS_PATH)/if_robo.c
	@echo Compiling $(SMP_STRING)$(CHIP_STRING) $(LINUXIA64) $<...
	$(CMD_ECHO)$(CC) $(CFLAGS) -c $<

DEPENDH = swmod.h $(BCM_ROBOUTILS_PATH)/if_robo.h \
	$(BCM_ROBOUTILS_PATH)/swapi.h $(BCM_ROBOUTILS_PATH)/etc53xx.h

swmod.o: $(DEPENDH)

if_robo.o: $(DEPENDH)

