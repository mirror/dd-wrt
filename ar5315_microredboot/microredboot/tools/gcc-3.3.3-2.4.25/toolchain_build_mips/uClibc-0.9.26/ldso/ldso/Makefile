# Makefile for uClibc
#
# Copyright (C) 2000 by Lineo, inc.
# Copyright (C) 2000-2002 Erik Andersen <andersen@uclibc.org>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Library General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
# details.
#
# You should have received a copy of the GNU Library General Public License
# along with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
# Derived in part from the Linux-8086 C library, the GNU C Library, and several
# other sundry sources.  Files within this library are copyright by their
# respective copyright holders.


TOPDIR=../../
include $(TOPDIR)Rules.mak
LDSO_FULLNAME=ld-uClibc-$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL).so


XXFLAGS=$(XWARNINGS) $(OPTIMIZATION) $(XARCH_CFLAGS) $(CPU_CFLAGS) $(PICFLAG) \
	-DUCLIBC_RUNTIME_PREFIX=\"$(RUNTIME_PREFIX)\" \
	-fno-builtin -nostdinc -I$(TOPDIR)ldso/include -I. -I$(TOPDIR)include

ifeq ($(SUPPORT_LD_DEBUG),y)
XXFLAGS=$(XWARNINGS) $(XARCH_CFLAGS) $(CPU_CFLAGS) $(PICFLAG) \
	-DUCLIBC_RUNTIME_PREFIX=\"$(RUNTIME_PREFIX)\" \
	-fno-builtin -nostdinc -I$(TOPDIR)ldso/include -I. -I$(TOPDIR)include
ifeq ($(TARGET_ARCH),powerpc)
XXFLAGS+=-Os -g3
else
XXFLAGS+=-O0 -g3
endif
endif

XXFLAGS+=$(shell $(CC) -print-search-dirs | sed -ne "s/install: *\(.*\)/-I\1include/gp")
LDFLAGS=$(CPU_LDFLAGS-y) -shared --warn-common --export-dynamic --sort-common \
	-z combreloc --discard-locals --discard-all

CSRC= ldso.c #hash.c readelflib1.c $(TARGET_ARCH)/elfinterp.c
COBJS=$(patsubst %.c,%.o, $(CSRC))
ASRC=$(shell ls $(TARGET_ARCH)/*.S)
AOBJS=$(patsubst %.S,%.o, $(ASRC))
OBJS=$(AOBJS) $(COBJS)

ifneq ($(strip $(SUPPORT_LD_DEBUG)),y)
LDFLAGS+=-s
endif

ifeq ($(strip $(SUPPORT_LD_DEBUG)),y)
XXFLAGS+=-D__SUPPORT_LD_DEBUG__
endif

ifeq ($(strip $(SUPPORT_LD_DEBUG_EARLY)),y)
XXFLAGS+=-D__SUPPORT_LD_DEBUG_EARLY__
endif

ifeq ($(strip $(FORCE_SHAREABLE_TEXT_SEGMENTS)),y)
XXFLAGS+=-DFORCE_SHAREABLE_TEXT_SEGMENTS
endif

all: lib

lib:: _dl_progname.h $(OBJS) $(DLINK_OBJS)
	$(LD) $(LDFLAGS) -e _dl_boot -soname=$(UCLIBC_LDSO) \
		-o $(LDSO_FULLNAME) $(OBJS) $(LIBGCC);
	$(INSTALL) -d $(TOPDIR)lib
	$(INSTALL) -m 755 $(LDSO_FULLNAME) $(TOPDIR)lib
	$(LN) -sf $(LDSO_FULLNAME) $(TOPDIR)lib/$(UCLIBC_LDSO)

_dl_progname.h: Makefile
	echo "const char *_dl_progname=\""$(UCLIBC_LDSO)"\";" > _dl_progname.h
	echo "#include \"$(TARGET_ARCH)/elfinterp.c\"" >> _dl_progname.h


$(COBJS): %.o : %.c
	$(CC) $(XXFLAGS) -I../libdl -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(AOBJS): %.o : %.S
	$(CC) $(XXFLAGS) -I../libdl -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

ldso.o: ldso.c hash.c readelflib1.c $(TARGET_ARCH)/elfinterp.c _dl_progname.h

clean:
	$(RM) $(UCLIBC_LDSO)* $(OBJS) $(LDSO_FULLNAME)* core *.o *.a *.s *.i _dl_progname.h ldso.h *~
