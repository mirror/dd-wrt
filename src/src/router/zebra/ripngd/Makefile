# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.4 from Makefile.am

# Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.


SHELL = /bin/sh

srcdir = .
top_srcdir = ..
prefix = /usr/local/zebra
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/zebra
pkglibdir = $(libdir)/zebra
pkgincludedir = $(includedir)/zebra

top_builddir = ..

ACLOCAL = aclocal
AUTOCONF = autoconf
AUTOMAKE = automake
AUTOHEADER = autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
host_alias = i686-pc-linux-gnu
host_triplet = i686-pc-linux-gnu
AR = ar
BGPD = 
CC = gcc
CPP = gcc -E
CURSES = 
IF_METHOD = if_netlink.o
IF_PROC = if_proc.o
IPFORWARD = ipforward_proc.o
KERNEL_METHOD = 
LIBPAM = 
LIB_IPV6 = 
LIB_REGEX = 
MAKEINFO = makeinfo
MULTIPATH_NUM = 1
OSPF6D = 
OSPFD = 
OTHER_METHOD = 
PACKAGE = zebra
RANLIB = ranlib
RIPD = ripd
RIPNGD = 
RTREAD_METHOD = rtread_netlink.o
RT_METHOD = rt_netlink.o
VERSION = 0.92a
VTYSH = 
ZEBRA = zebra

INCLUDES =  -I.. -I$(top_srcdir) -I$(top_srcdir)/lib
DEFS = -DHAVE_CONFIG_H -DSYSCONFDIR=\"$(sysconfdir)/\"
INSTALL_SDATA = /usr/bin/install -c -m 600

noinst_LIBRARIES = libripng.a
sbin_PROGRAMS = ripngd

libripng_a_SOURCES =  	ripng_interface.c ripngd.c ripng_zebra.c ripng_route.c ripng_debug.c 	ripng_routemap.c


noinst_HEADERS =  	ripng_debug.h ripng_route.h ripngd.h


ripngd_SOURCES =  	ripng_main.c $(libripng_a_SOURCES)


ripngd_LDADD = ../lib/libzebra.a

sysconf_DATA = ripngd.conf.sample

EXTRA_DIST = $(sysconf_DATA)
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../config.h
CONFIG_CLEAN_FILES = 
LIBRARIES =  $(noinst_LIBRARIES)

CPPFLAGS = 
LDFLAGS = 
LIBS = -lm -lcrypt 
libripng_a_LIBADD = 
libripng_a_OBJECTS =  ripng_interface.o ripngd.o ripng_zebra.o \
ripng_route.o ripng_debug.o ripng_routemap.o
PROGRAMS =  $(sbin_PROGRAMS)

ripngd_OBJECTS =  ripng_main.o ripng_interface.o ripngd.o ripng_zebra.o \
ripng_route.o ripng_debug.o ripng_routemap.o
ripngd_DEPENDENCIES =  ../lib/libzebra.a
ripngd_LDFLAGS = 
CFLAGS = -g -Os -Wall
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@
DATA =  $(sysconf_DATA)

HEADERS =  $(noinst_HEADERS)

DIST_COMMON =  ChangeLog Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = tar
GZIP_ENV = --best
SOURCES = $(libripng_a_SOURCES) $(ripngd_SOURCES)
OBJECTS = $(libripng_a_OBJECTS) $(ripngd_OBJECTS)

all: all-redirect
.SUFFIXES:
.SUFFIXES: .S .c .o .s
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --foreign --include-deps ripngd/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status


mostlyclean-noinstLIBRARIES:

clean-noinstLIBRARIES:
	-test -z "$(noinst_LIBRARIES)" || rm -f $(noinst_LIBRARIES)

distclean-noinstLIBRARIES:

maintainer-clean-noinstLIBRARIES:

.c.o:
	$(COMPILE) -c $<

.s.o:
	$(COMPILE) -c $<

.S.o:
	$(COMPILE) -c $<

mostlyclean-compile:
	-rm -f *.o core *.core

clean-compile:

distclean-compile:
	-rm -f *.tab.c

maintainer-clean-compile:

libripng.a: $(libripng_a_OBJECTS) $(libripng_a_DEPENDENCIES)
	-rm -f libripng.a
	$(AR) cru libripng.a $(libripng_a_OBJECTS) $(libripng_a_LIBADD)
	$(RANLIB) libripng.a

mostlyclean-sbinPROGRAMS:

clean-sbinPROGRAMS:
	-test -z "$(sbin_PROGRAMS)" || rm -f $(sbin_PROGRAMS)

distclean-sbinPROGRAMS:

maintainer-clean-sbinPROGRAMS:

install-sbinPROGRAMS: $(sbin_PROGRAMS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(sbindir)
	@list='$(sbin_PROGRAMS)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo "  $(INSTALL_PROGRAM) $$p $(DESTDIR)$(sbindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`"; \
	     $(INSTALL_PROGRAM) $$p $(DESTDIR)$(sbindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	  else :; fi; \
	done

uninstall-sbinPROGRAMS:
	@$(NORMAL_UNINSTALL)
	list='$(sbin_PROGRAMS)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(sbindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	done

ripngd: $(ripngd_OBJECTS) $(ripngd_DEPENDENCIES)
	@rm -f ripngd
	$(LINK) $(ripngd_LDFLAGS) $(ripngd_OBJECTS) $(ripngd_LDADD) $(LIBS)

uninstall-sysconfDATA:
	@$(NORMAL_UNINSTALL)
	list='$(sysconf_DATA)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(sysconfdir)/$$p; \
	done

tags: TAGS

ID: $(HEADERS) $(SOURCES) $(LISP)
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	here=`pwd` && cd $(srcdir) \
	  && mkid -f$$here/ID $$unique $(LISP)

TAGS:  $(HEADERS) $(SOURCES)  $(TAGS_DEPENDENCIES) $(LISP)
	tags=; \
	here=`pwd`; \
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	test -z "$(ETAGS_ARGS)$$unique$(LISP)$$tags" \
	  || (cd $(srcdir) && etags $(ETAGS_ARGS) $$tags  $$unique $(LISP) -o $$here/TAGS)

mostlyclean-tags:

clean-tags:

distclean-tags:
	-rm -f TAGS ID

maintainer-clean-tags:

distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = ripngd

distdir: $(DISTFILES)
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  if test -d $$d/$$file; then \
	    cp -pr $$/$$file $(distdir)/$$file; \
	  else \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file || :; \
	  fi; \
	done
info-am:
info: info-am
dvi-am:
dvi: dvi-am
check-am: all-am
check: check-am
installcheck-am:
installcheck: installcheck-am
install-exec-am: install-sbinPROGRAMS install-sysconfDATA
install-exec: install-exec-am

install-data-am:
install-data: install-data-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-am
uninstall-am: uninstall-sbinPROGRAMS uninstall-sysconfDATA
uninstall: uninstall-am
all-am: Makefile $(LIBRARIES) $(PROGRAMS) $(DATA) $(HEADERS)
all-redirect: all-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs:
	$(mkinstalldirs)  $(DESTDIR)$(sbindir) $(DESTDIR)$(sysconfdir)


mostlyclean-generic:

clean-generic:

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-noinstLIBRARIES mostlyclean-compile \
		mostlyclean-sbinPROGRAMS mostlyclean-tags \
		mostlyclean-generic

mostlyclean: mostlyclean-am

clean-am:  clean-noinstLIBRARIES clean-compile clean-sbinPROGRAMS \
		clean-tags clean-generic mostlyclean-am

clean: clean-am

distclean-am:  distclean-noinstLIBRARIES distclean-compile \
		distclean-sbinPROGRAMS distclean-tags distclean-generic \
		clean-am

distclean: distclean-am

maintainer-clean-am:  maintainer-clean-noinstLIBRARIES \
		maintainer-clean-compile maintainer-clean-sbinPROGRAMS \
		maintainer-clean-tags maintainer-clean-generic \
		distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-am

.PHONY: mostlyclean-noinstLIBRARIES distclean-noinstLIBRARIES \
clean-noinstLIBRARIES maintainer-clean-noinstLIBRARIES \
mostlyclean-compile distclean-compile clean-compile \
maintainer-clean-compile mostlyclean-sbinPROGRAMS \
distclean-sbinPROGRAMS clean-sbinPROGRAMS maintainer-clean-sbinPROGRAMS \
uninstall-sbinPROGRAMS install-sbinPROGRAMS uninstall-sysconfDATA \
install-sysconfDATA tags mostlyclean-tags distclean-tags clean-tags \
maintainer-clean-tags distdir info-am info dvi-am dvi check check-am \
installcheck-am installcheck install-exec-am install-exec \
install-data-am install-data install-am install uninstall-am uninstall \
all-redirect all-am all installdirs mostlyclean-generic \
distclean-generic clean-generic maintainer-clean-generic clean \
mostlyclean distclean maintainer-clean


install-sysconfDATA: $(sysconf_DATA)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(sysconfdir)
	@list='$(sysconf_DATA)'; for p in $$list; do \
	  if test -f $(srcdir)/$$p; then \
	    echo " $(INSTALL_SDATA) $(srcdir)/$$p $(DESTDIR)$(sysconfdir)/$$p"; \
	    $(INSTALL_SDATA) $(srcdir)/$$p $(DESTDIR)$(sysconfdir)/$$p; \
	  else if test -f $$p; then \
	    echo " $(INSTALL_SDATA) $$p $(DESTDIR)$(sysconfdir)/$$p"; \
	    $(INSTALL_SDATA) $$p $(DESTDIR)$(sysconfdir)/$$p; \
	  fi; fi; \
	done

depend:
	@$(CPP) -MM $(INCLUDES) $(LDFLAGS) *.c

ripng_debug.o: ripng_debug.c ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../ripngd/ripng_debug.h
ripng_interface.o: ripng_interface.c ../lib/linklist.h ../lib/if.h \
 ../lib/prefix.h ../lib/memory.h ../lib/network.h ../lib/filter.h \
 ../lib/log.h ../lib/stream.h ../lib/zclient.h ../lib/command.h \
 ../lib/vector.h ../lib/vty.h ../lib/table.h ../lib/thread.h \
 ../ripngd/ripngd.h ../ripngd/ripng_debug.h
ripng_main.o: ripng_main.c ../lib/version.h ../lib/vector.h \
 ../lib/vty.h ../lib/command.h ../lib/thread.h ../lib/log.h \
 ../lib/prefix.h ../lib/if.h ../lib/linklist.h ../ripngd/ripngd.h
ripng_route.o: ripng_route.c ../lib/prefix.h ../lib/table.h \
 ../lib/memory.h ../lib/if.h ../lib/linklist.h ../ripngd/ripngd.h \
 ../ripngd/ripng_route.h
ripng_routemap.o: ripng_routemap.c ../lib/if.h ../lib/linklist.h \
 ../lib/memory.h ../lib/prefix.h ../lib/routemap.h ../lib/command.h \
 ../lib/vector.h ../lib/vty.h ../ripngd/ripngd.h
ripng_zebra.o: ripng_zebra.c ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/prefix.h ../lib/stream.h ../lib/zclient.h \
 ../lib/if.h ../lib/linklist.h ../lib/log.h ../ripngd/ripngd.h
ripngd.o: ripngd.c ../lib/prefix.h ../lib/filter.h ../lib/if.h \
 ../lib/linklist.h ../lib/log.h ../lib/thread.h ../lib/memory.h \
 ../lib/stream.h ../lib/table.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/sockopt.h ../lib/distribute.h ../lib/plist.h \
 ../lib/routemap.h ../lib/if_rmap.h ../ripngd/ripngd.h \
 ../ripngd/ripng_route.h ../ripngd/ripng_debug.h

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
