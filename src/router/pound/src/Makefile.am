# Pound - the reverse-proxy load-balancer                -*- automake -*-
# Copyright (C) 2002-2010 Apsis GmbH
# Copyright (C) 2018-2024 Sergey Poznyakoff
#
# Pound is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# Pound is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with pound.  If not, see <http://www.gnu.org/licenses/>.

AM_CFLAGS = @PTHREAD_CFLAGS@

AM_CPPFLAGS = @SSL_CPPFLAGS@ @PCREPOSIX_CFLAGS@ \
 -DSYSCONFDIR=\"$(sysconfdir)\"\
 -DLOCALSTATEDIR=\"$(localstatedir)\"\
 -DPKGDATADIR=\"$(pkgdatadir)\"
AM_LDFLAGS  = @SSL_LDFLAGS@
LDADD = libpound.a @PCREPOSIX_LIBS@ @PTHREAD_LIBS@

sbin_PROGRAMS=pound
pound_SOURCES=\
 bauth.c\
 config.c\
 http.c\
 log.c\
 metrics.c\
 pound.c\
 svc.c

noinst_LIBRARIES = libpound.a
libpound_a_SOURCES = json.c json.h mem.c progname.c tmpl.c

pkgdata_DATA = poundctl.tmpl mvh.inc
EXTRA_DIST = poundctl.tmpl mvh.inc

install-exec-hook:
	-@if test "$${EUID:-65535}" -eq 0; then \
		for program in $(sbin_PROGRAMS); do \
			chown @I_OWNER@:@I_GRP@ $(DESTDIR)$(sbindir)/$$program$(EXEEXT); \
		done; \
		for program in $(bin_PROGRAMS); do \
			chown @I_OWNER@:@I_GRP@ $(DESTDIR)$(bindir)/$$program$(EXEEXT); \
		done; \
	fi

noinst_HEADERS=pound.h extern.h list.h ht.h

if SET_DH_AUTO
DHSRC =
nodist_pound_SOURCES =
else
nodist_pound_SOURCES = dh.h
DHSRC = dh512.h dh@DH_LEN@.h

dh512.h:
	openssl dhparam -5 -C -noout 512 > dh512.h

dh@DH_LEN@.h:
	openssl dhparam -5 -C -noout @DH_LEN@ > dh@DH_LEN@.h

dh.h: $(DHSRC)
	cat $(DHSRC) > dh.h

BUILT_SOURCES=dh.h $(DHSRC)
DISTCLEANFILES=dh.h dh512.h dh1024.h dh2048.h
endif

bin_PROGRAMS=poundctl
poundctl_SOURCES=poundctl.c
