# Makefile for uClibc
#
# Copyright (C) 2000-2003 Erik Andersen <andersen@uclibc.org>
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

# Pull in the user's uClibc configuration, but do not
# pull in Rules.mak.....
TOPDIR=../
include $(TOPDIR)Rules.mak

TARGETS = ldd ldconfig readelf

ifeq ($(strip $(UCLIBC_HAS_LOCALE)),y)
TARGET_ICONV = iconv
else
TARGET_ICONV =
endif

XXFLAGS=
ifeq ($(strip $(LDSO_LDD_SUPPORT)),y)
XXFLAGS= -D__LDSO_LDD_SUPPORT
else
XXFLAGS=
endif

ifeq ($(strip $(HAVE_SHARED)),y)
all:	$(TARGETS) $(TARGET_ICONV)
else
all:	$(TARGET_ICONV)
endif

headers:
	$(LN) -fs $(TOPDIR)include/elf.h

readelf: readelf.c
	$(CC) $(CFLAGS) -Wl,-s $^ -o $@ $(LDADD_LIBFLOAT) -L../lib
	$(STRIPTOOL) -x -R .note -R .comment $@

ldconfig:	ldconfig.c readsoname.c
	$(CC) $(CFLAGS) $(XXFLAGS) -Wl,-s -static \
		-DUCLIBC_RUNTIME_PREFIX=$(R_PREFIX) \
		-DUCLIBC_LDSO=$(UCLIBC_LDSO) -I. -I../ldso/include \
		$^ -o $@ $(LDADD_LIBFLOAT) -L../lib
	$(STRIPTOOL) -x -R .note -R .comment $@

ldd:	ldd.c
	$(CC) $(CFLAGS) $(XXFLAGS) -Wl,-s \
		-DUCLIBC_RUNTIME_PREFIX=$(R_PREFIX) \
		-DUCLIBC_LDSO=$(UCLIBC_LDSO) \
		$^ -o $@ $(LDADD_LIBFLOAT) -L../lib
	$(STRIPTOOL) -x -R .note -R .comment $@

iconv: ../libc/misc/wchar/wchar.c
	$(CC) $(CFLAGS) -DL_iconv_main ../libc/misc/wchar/wchar.c -o $@ \
		$(LDADD_LIBFLOAT) -L../lib
	$(STRIPTOOL) -x -R .note -R .comment $@

clean:
	$(RM) $(TARGETS) *.o *~ core *.target elf.h iconv


readelf.c readsoname.c ldconfig.c ldd.c:	headers

install: all
ifeq ($(strip $(HAVE_SHARED)),y)
	$(INSTALL) -d $(PREFIX)$(RUNTIME_PREFIX)sbin
	$(INSTALL) -d $(PREFIX)$(RUNTIME_PREFIX)usr/bin
	$(INSTALL) -m 755 ldd $(PREFIX)$(RUNTIME_PREFIX)usr/bin/ldd
	$(INSTALL) -m 755 ldconfig $(PREFIX)$(RUNTIME_PREFIX)sbin/ldconfig;
	# For now, don't bother with readelf since surely the host
	# system has binutils, or we couldn't have gotten this far...
	#$(INSTALL) -m 755 readelf $(PREFIX)$(RUNTIME_PREFIX)usr/bin/readelf
endif
ifeq ($(strip $(UCLIBC_HAS_LOCALE)),y)
	$(INSTALL) -m 755 iconv $(PREFIX)$(RUNTIME_PREFIX)usr/bin/iconv
endif
