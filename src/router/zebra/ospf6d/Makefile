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

noinst_LIBRARIES = libospf6.a
sbin_PROGRAMS = ospf6d

libospf6_a_SOURCES =  	ospf6_dump.c ospf6d.c ospf6_interface.c ospf6_network.c 	ospf6_neighbor.c ospf6_message.c ospf6_lsa.c ospf6_spf.c 	ospf6_route.c ospf6_zebra.c ospf6_ism.c ospf6_dbex.c 	ospf6_lsdb.c ospf6_prefix.c ospf6_top.c ospf6_area.c ospf6_nsm.c 	ospf6_redistribute.c ospf6_routemap.c ospf6_list.c ospf6_proto.c


noinst_HEADERS =  	ospf6_area.h ospf6_dump.h ospf6_interface.h ospf6_lsa.h 	ospf6_message.h ospf6_neighbor.h ospf6_network.h ospf6_proto.h 	ospf6_spf.h ospf6_route.h ospf6_types.h ospf6_zebra.h ospf6d.h 	ospf6_ism.h ospf6_dbex.h ospf6_lsdb.h ospf6_prefix.h 	ospf6_top.h ospf6_nsm.h ospf6_redistribute.h ospf6_routemap.h 	ospf6_list.h


ospf6d_SOURCES =  	ospf6_main.c $(libospf6_a_SOURCES)


ospf6d_LDADD = ../lib/libzebra.a

sysconf_DATA = ospf6d.conf.sample

EXTRA_DIST = $(sysconf_DATA)
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../config.h
CONFIG_CLEAN_FILES = 
LIBRARIES =  $(noinst_LIBRARIES)

CPPFLAGS = 
LDFLAGS = 
LIBS = -lm -lcrypt 
libospf6_a_LIBADD = 
libospf6_a_OBJECTS =  ospf6_dump.o ospf6d.o ospf6_interface.o \
ospf6_network.o ospf6_neighbor.o ospf6_message.o ospf6_lsa.o \
ospf6_spf.o ospf6_route.o ospf6_zebra.o ospf6_ism.o ospf6_dbex.o \
ospf6_lsdb.o ospf6_prefix.o ospf6_top.o ospf6_area.o ospf6_nsm.o \
ospf6_redistribute.o ospf6_routemap.o ospf6_list.o ospf6_proto.o
PROGRAMS =  $(sbin_PROGRAMS)

ospf6d_OBJECTS =  ospf6_main.o ospf6_dump.o ospf6d.o ospf6_interface.o \
ospf6_network.o ospf6_neighbor.o ospf6_message.o ospf6_lsa.o \
ospf6_spf.o ospf6_route.o ospf6_zebra.o ospf6_ism.o ospf6_dbex.o \
ospf6_lsdb.o ospf6_prefix.o ospf6_top.o ospf6_area.o ospf6_nsm.o \
ospf6_redistribute.o ospf6_routemap.o ospf6_list.o ospf6_proto.o
ospf6d_DEPENDENCIES =  ../lib/libzebra.a
ospf6d_LDFLAGS = 
CFLAGS = -g -Os -Wall
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@
DATA =  $(sysconf_DATA)

HEADERS =  $(noinst_HEADERS)

DIST_COMMON =  README ChangeLog Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = tar
GZIP_ENV = --best
SOURCES = $(libospf6_a_SOURCES) $(ospf6d_SOURCES)
OBJECTS = $(libospf6_a_OBJECTS) $(ospf6d_OBJECTS)

all: all-redirect
.SUFFIXES:
.SUFFIXES: .S .c .o .s
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --foreign --include-deps ospf6d/Makefile

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

libospf6.a: $(libospf6_a_OBJECTS) $(libospf6_a_DEPENDENCIES)
	-rm -f libospf6.a
	$(AR) cru libospf6.a $(libospf6_a_OBJECTS) $(libospf6_a_LIBADD)
	$(RANLIB) libospf6.a

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

ospf6d: $(ospf6d_OBJECTS) $(ospf6d_DEPENDENCIES)
	@rm -f ospf6d
	$(LINK) $(ospf6d_LDFLAGS) $(ospf6d_OBJECTS) $(ospf6d_LDADD) $(LIBS)

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

subdir = ospf6d

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

ospf6_area.o: ospf6_area.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6_dbex.o: ospf6_dbex.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6_dump.o: ospf6_dump.c ospf6_dump.h ../lib/command.h ../lib/log.h
ospf6_interface.o: ospf6_interface.c ospf6d.h ../lib/version.h \
 ../lib/log.h ../lib/linklist.h ../lib/thread.h ../lib/command.h \
 ../lib/vector.h ../lib/vty.h ../lib/memory.h ../lib/sockunion.h \
 ../lib/if.h ../lib/prefix.h ../lib/stream.h ../lib/filter.h \
 ../lib/zclient.h ../lib/table.h ../lib/plist.h ospf6_types.h \
 ospf6_prefix.h ospf6_message.h ospf6_proto.h ospf6_spf.h \
 ospf6_redistribute.h ospf6_top.h ../lib/routemap.h ospf6_area.h \
 ospf6_interface.h ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h \
 ospf6_lsa.h ospf6_lsdb.h ospf6_route.h ospf6_dbex.h ospf6_network.h \
 ospf6_zebra.h ospf6_dump.h ospf6_routemap.h
ospf6_ism.o: ospf6_ism.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6_list.o: ospf6_list.c ../lib/memory.h ../lib/log.h ospf6_list.h
ospf6_lsa.o: ospf6_lsa.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6_lsdb.o: ospf6_lsdb.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6_main.o: ospf6_main.c ../lib/thread.h ../lib/log.h \
 ../lib/version.h ../lib/command.h ../lib/vector.h ../lib/vty.h \
 ../lib/memory.h ospf6d.h ../lib/linklist.h ../lib/sockunion.h \
 ../lib/if.h ../lib/prefix.h ../lib/stream.h ../lib/filter.h \
 ../lib/zclient.h ../lib/table.h ../lib/plist.h ospf6_types.h \
 ospf6_prefix.h ospf6_message.h ospf6_proto.h ospf6_spf.h \
 ospf6_redistribute.h ospf6_top.h ../lib/routemap.h ospf6_area.h \
 ospf6_interface.h ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h \
 ospf6_lsa.h ospf6_lsdb.h ospf6_route.h ospf6_dbex.h ospf6_network.h \
 ospf6_zebra.h ospf6_dump.h ospf6_routemap.h
ospf6_message.o: ospf6_message.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6_neighbor.o: ospf6_neighbor.c ospf6d.h ../lib/version.h \
 ../lib/log.h ../lib/linklist.h ../lib/thread.h ../lib/command.h \
 ../lib/vector.h ../lib/vty.h ../lib/memory.h ../lib/sockunion.h \
 ../lib/if.h ../lib/prefix.h ../lib/stream.h ../lib/filter.h \
 ../lib/zclient.h ../lib/table.h ../lib/plist.h ospf6_types.h \
 ospf6_prefix.h ospf6_message.h ospf6_proto.h ospf6_spf.h \
 ospf6_redistribute.h ospf6_top.h ../lib/routemap.h ospf6_area.h \
 ospf6_interface.h ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h \
 ospf6_lsa.h ospf6_lsdb.h ospf6_route.h ospf6_dbex.h ospf6_network.h \
 ospf6_zebra.h ospf6_dump.h ospf6_routemap.h ospf6_list.h
ospf6_network.o: ospf6_network.c ../lib/memory.h ../lib/log.h \
 ../lib/sockunion.h ospf6d.h ../lib/version.h ../lib/linklist.h \
 ../lib/thread.h ../lib/command.h ../lib/vector.h ../lib/vty.h \
 ../lib/if.h ../lib/prefix.h ../lib/stream.h ../lib/filter.h \
 ../lib/zclient.h ../lib/table.h ../lib/plist.h ospf6_types.h \
 ospf6_prefix.h ospf6_message.h ospf6_proto.h ospf6_spf.h \
 ospf6_redistribute.h ospf6_top.h ../lib/routemap.h ospf6_area.h \
 ospf6_interface.h ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h \
 ospf6_lsa.h ospf6_lsdb.h ospf6_route.h ospf6_dbex.h ospf6_network.h \
 ospf6_zebra.h ospf6_dump.h ospf6_routemap.h
ospf6_nsm.o: ospf6_nsm.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6_prefix.o: ospf6_prefix.c ../lib/log.h ../lib/prefix.h \
 ../lib/memory.h ../lib/linklist.h 
ospf6_proto.o: ospf6_proto.c ospf6_proto.h
ospf6_redistribute.o: ospf6_redistribute.c ospf6d.h ../lib/version.h \
 ../lib/log.h ../lib/linklist.h ../lib/thread.h ../lib/command.h \
 ../lib/vector.h ../lib/vty.h ../lib/memory.h ../lib/sockunion.h \
 ../lib/if.h ../lib/prefix.h ../lib/stream.h ../lib/filter.h \
 ../lib/zclient.h ../lib/table.h ../lib/plist.h ospf6_types.h \
 ospf6_prefix.h ospf6_message.h ospf6_proto.h ospf6_spf.h \
 ospf6_redistribute.h ospf6_top.h ../lib/routemap.h ospf6_area.h \
 ospf6_interface.h ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h \
 ospf6_lsa.h ospf6_lsdb.h ospf6_route.h ospf6_dbex.h ospf6_network.h \
 ospf6_zebra.h ospf6_dump.h ospf6_routemap.h
ospf6_route.o: ospf6_route.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6_routemap.o: ospf6_routemap.c ../lib/linklist.h ../lib/prefix.h \
 ../lib/command.h ../lib/vector.h ../lib/vty.h ../lib/routemap.h \
 ../lib/plist.h ../lib/memory.h ../lib/log.h ospf6_redistribute.h \
 ospf6_top.h
ospf6_spf.o: ospf6_spf.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6_top.o: ospf6_top.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6_zebra.o: ospf6_zebra.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h
ospf6d.o: ospf6d.c ospf6d.h ../lib/version.h ../lib/log.h \
 ../lib/linklist.h ../lib/thread.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h ../lib/sockunion.h ../lib/if.h \
 ../lib/prefix.h ../lib/stream.h ../lib/filter.h ../lib/zclient.h \
 ../lib/table.h ../lib/plist.h ospf6_types.h ospf6_prefix.h \
 ospf6_message.h ospf6_proto.h ospf6_spf.h ospf6_redistribute.h \
 ospf6_top.h ../lib/routemap.h ospf6_area.h ospf6_interface.h \
 ospf6_neighbor.h ospf6_ism.h ospf6_nsm.h ospf6_lsa.h ospf6_lsdb.h \
 ospf6_route.h ospf6_dbex.h ospf6_network.h ospf6_zebra.h ospf6_dump.h \
 ospf6_routemap.h

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
