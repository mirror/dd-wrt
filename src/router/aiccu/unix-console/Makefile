# /**********************************************************
#  SixXS - Automatic IPv6 Connectivity Configuration Utility
# ***********************************************************
#  Copyright 2003-2005 SixXS - http://www.sixxs.net
# ***********************************************************
#  unix-client/Makefile - Makefile for the UNIX client
# ***********************************************************
#  $Author: jeroen $
#  $Id: Makefile,v 1.32 2007-01-15 11:04:27 jeroen Exp $
#  $Date: 2007-01-15 11:04:27 $
# **********************************************************/

SRCS	= main.c ../common/tun.c ../common/aiccu.c ../common/hash_md5.c ../common/hash_sha1.c ../common/common.c ../common/heartbeat.c ../common/tic.c ../common/ayiya.c ../common/aiccu_test.c ../common/resolver.c ../common/dn_skipname.c
INCS	= ../common/tun.h ../common/aiccu.h ../common/hash_md5.h ../common/hash_sha1.h ../common/common.h ../common/heartbeat.h ../common/tic.h ../common/ayiya.h ../common/resolver.h
OBJS	= main.o ../common/tun.o ../common/aiccu.o ../common/hash_md5.o ../common/hash_sha1.o ../common/common.o ../common/heartbeat.o ../common/tic.o ../common/ayiya.o ../common/aiccu_test.o ../common/resolver.o ../common/dn_skipname.o

# New features not fully implemented and thus disabled for now
#CFLAGS	+= -D NEWSTUFF_TSP -D NEWSTUFF_TEEPEE
#SRCS += ../common/tsp.c ../common/teepee.c
#OBJS += ../common/tsp.o ../common/teepee.o
#INCS += ../common/tsp.h ../common/teepee.h

# Our very *bliep* set of options to make sure that these things can't cause any issues
CWARNS += -W -Wall -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Winline -Wbad-function-cast -fshort-enums -fstrict-aliasing -fno-common -Wpadded -pedantic -D_REENTRANT
# CWARNS += -Wpacked

#CFLAGS	+= $(CWARNS) -D_GNU_SOURCE -D_DEBUG -g3 -O0
CFLAGS	+= $(CWARNS) $(EXTRA_CFLAGS) -D_GNU_SOURCE
LDFLAGS += $(EXTRA_LDFLAGS)
#CC      = @gcc
RM      = rm

# Add -O3 when nothing is specified yet
ifeq ($(shell echo $(CFLAGS) | grep -c "\-O"),0)
CFLAGS	+= -O3
endif

# This is a console client
CFLAGS	+= -D AICCU_CONSOLE

# GnuTLS Support ?
# Used by TIC to secure that communication
# Currently defaultly builds only on Linux, but other platforms might easily also support it
ifneq ($(HAVE_GNUTLS),)
CFLAGS	+= -D AICCU_GNUTLS
LDFLAGS += -lgnutls
endif

# Linux
ifeq ($(OS_NAME),Linux)
CFLAGS  += -D_LINUX -DHAS_IFHEAD -D AICCU_TYPE="\"linux\""
SRCS	+= ../common/aiccu_linux.c
OBJS	+= ../common/aiccu_linux.o
LDFLAGS	+= -pthread -lresolv
endif

# FreeBSD
ifeq ($(OS_NAME),FreeBSD)
CFLAGS	+= -D_FREEBSD

# FreeBSD 4.x
ifeq ($(shell echo $(OS_VERSION) | cut -c 1),4)
CFLAGS	+= -D AICCU_TYPE="\"freebsd4\""
SRCS	+= ../common/aiccu_freebsd4.c
OBJS	+= ../common/aiccu_freebsd4.o
else
# FreeBSD 5.x/6.x/7.x
CFLAGS	+= -D NEED_IFHEAD -D AICCU_TYPE="\"kame\""
SRCS	+= ../common/aiccu_kame.c
OBJS	+= ../common/aiccu_kame.o
endif
endif

# DragonFlyBSD
ifeq ($(OS_NAME),DragonFly)
CFLAGS	+= -D_DFBSD -D NEED_IFHEAD -D AICCU_TYPE="\"dragonfly\""
SRCS	+= ../common/aiccu_freebsd4.c
OBJS	+= ../common/aiccu_freebsd4.o
CFLAGS	+= -pthread
endif

# NetBSD
ifeq ($(OS_NAME),NetBSD)
CFLAGS	+= -D_NETBSD -D AICCU_TYPE="\"kame\""

# Check if net/if_tun.h has TUNSIFHEAD and enable support for it
ifneq ($(shell grep -c TUNSIFHEAD /usr/include/net/if_tun.h 2>/dev/null),1)
# Supports TUNSIFHEAD -> AYIYA/l2tp available
CFLAGS	+= -D NEED_IFHEAD
else
# Doesn't support TUNSIFHEAD -> No AYIYA/l2tp available
CFLAGS	+= -D NO_IFHEAD
endif

SRCS	+= ../common/aiccu_kame.c
OBJS	+= ../common/aiccu_kame.o
CFLAGS	+= -pthread -D_NETBSD_SOURCE
endif

# OpenBSD
ifeq ($(OS_NAME),OpenBSD)
CFLAGS	+= -D_OPENBSD -D HAS_IFHEAD
# 2.7-2.9
ifeq ($(shell echo $(OS_VERSION) | cut -c 1),2)
CFLAGS	+= -D AICCU_TYPE="\"openbsd2\""
SRCS	+= ../common/aiccu_openbsd2.c
OBJS	+= ../common/aiccu_openbsd2.o
else
# 3.x-4.x etc
CFLAGS	+= -D AICCU_TYPE="\"openbsd\""
SRCS	+= ../common/aiccu_openbsd.c
OBJS	+= ../common/aiccu_openbsd.o
endif
CFLAGS	+= -pthread
endif

# Darwin
ifeq ($(OS_NAME),Darwin)
CFLAGS	+= -D_DARWIN -D NEED_IFHEAD -D AICCU_TYPE="\"darwin\""
SRCS	+= ../common/aiccu_darwin.c
OBJS	+= ../common/aiccu_darwin.o
LDFLAGS	+= -lresolv
endif

# SunOS / Solaris
ifeq ($(OS_NAME),SunOS)
CFLAGS	+= -D_SUNOS -D AICCU_TYPE="\"sunos\""
SRCS	+= ../common/aiccu_sunos.c
OBJS	+= ../common/aiccu_sunos.o
LDFLAGS	+= -lsocket -lnsl -lresolv
endif

# AIX
ifeq ($(OS_NAME),AIX)
CC	= @/usr/vac/bin/xlc_r
CFLAGS	= -qthreaded -q64 -qlanglvl=stdc99 -bmaxdata:0xD0000000 -D_64BIT -g -qdbxextra -qfullpath -qheapdebug -qformat=all -qcheck=all
CFLAGS	+= -D AICCU_CONSOLE
CFLAGS	+= -D_AIX -D AICCU_TYPE="\"aix\""
SRCS	+= ../common/aiccu_aix.c
OBJS	+= ../common/aiccu_aix.o
LDFLAGS	+= -lpthread
endif


all: aiccu

aiccu:	$(OBJS) ${SRCS} ${INCS}
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)
ifeq ($(shell echo $(CFLAGS) | grep -c "DEBUG"),0)
ifeq ($(shell echo "$(RPM_OPT_FLAGS)" | wc -c),1)
	$(CROSS_COMPILE)strip $@
endif
endif

clean:
	$(RM) -f $(OBJS) aiccu

install: aiccu
	cp aiccu ${DESTDIR}${dirsbin}aiccu

