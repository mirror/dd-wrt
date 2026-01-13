# Pound - the reverse-proxy load-balancer
# Copyright (C) 2002-2010 Apsis GmbH
# Copyright (C) 2018-2025 Sergey Poznyakoff
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

AM_CPPFLAGS = @SSL_CPPFLAGS@ @LUA_CFLAGS@ @PCRE_CFLAGS@ \
 -DSYSCONFDIR=\"$(sysconfdir)\"\
 -DLOCALSTATEDIR=\"$(localstatedir)\"\
 -DPKGDATADIR=\"$(pkgdatadir)\"
AM_LDFLAGS  = @SSL_LDFLAGS@
LDADD = libpound.a @LUA_LIBS@ @PCRE_LIBS@ @PTHREAD_LIBS@

sbin_PROGRAMS=pound
pound_SOURCES=\
 bauth.c\
 config.c\
 genpat.c\
 http.c\
 log.c\
 metrics.c\
 pound.c\
 svc.c\
 tbf.c\
 watcher.c

if COND_PCRE
  pound_SOURCES += regex_pcre.c
endif
if COND_PCRE2
  pound_SOURCES += regex_pcre2.c
endif
if COND_DYNAMIC_BACKENDS
  pound_SOURCES += resolver.c dynbe.c
endif
if COND_LUA
  pound_SOURCES += pndlua.c
endif

if COND_INOTIFY
  pound_SOURCES += fsevmon.c watcher/inotify.c
  AM_CPPFLAGS += -DWITH_INOTIFY=1
else
if  COND_KQUEUE
  pound_SOURCES += fsevmon.c watcher/kqueue.c
  AM_CPPFLAGS += -DWITH_KQUEUE=1
else
  pound_SOURCES += watcher/periodic.c
endif
endif

noinst_LIBRARIES = libpound.a
libpound_a_SOURCES = \
 cctype.c\
 cctype.h\
 cfgparser.c\
 cfgparser.h\
 json.c\
 json.h\
 mem.c\
 mem.h\
 progname.c

pkgdata_DATA = poundctl.tmpl mvh.inc

EXTRA_DIST = $(pkgdata_DATA) pound-mode.el

noinst_HEADERS=pound.h extern.h list.h ht.h resolver.h watcher.h

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
poundctl_SOURCES=poundctl.c tmpl.c
poundctl_LDFLAGS=-lm
