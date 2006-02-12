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

noinst_LIBRARIES = libospf.a
sbin_PROGRAMS = ospfd

libospf_a_SOURCES =  	ospfd.c ospf_zebra.c ospf_interface.c ospf_ism.c ospf_neighbor.c 	ospf_nsm.c ospf_dump.c ospf_network.c ospf_packet.c ospf_lsa.c 	ospf_spf.c ospf_route.c ospf_ase.c ospf_abr.c ospf_ia.c ospf_flood.c 	ospf_lsdb.c ospf_asbr.c ospf_routemap.c ospf_snmp.c


noinst_HEADERS =  	ospf_dump.h ospf_interface.h ospf_ism.h ospf_neighbor.h 	ospf_network.h ospf_nsm.h ospf_packet.h ospf_zebra.h ospfd.h 	ospf_lsa.h ospf_spf.h ospf_route.h ospf_ase.h ospf_abr.h ospf_ia.h 	ospf_flood.h ospf_lsdb.h ospf_asbr.h ospf_snmp.h


ospfd_SOURCES =  	ospf_main.c $(libospf_a_SOURCES)


ospfd_LDADD = ../lib/libzebra.a

sysconf_DATA = ospfd.conf.sample

EXTRA_DIST = $(sysconf_DATA) OSPF-MIB.txt OSPF-TRAP-MIB.txt
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../config.h
CONFIG_CLEAN_FILES = 
LIBRARIES =  $(noinst_LIBRARIES)

CPPFLAGS = 
LDFLAGS = 
LIBS = -lm -lcrypt 
libospf_a_LIBADD = 
libospf_a_OBJECTS =  ospfd.o ospf_zebra.o ospf_interface.o ospf_ism.o \
ospf_neighbor.o ospf_nsm.o ospf_dump.o ospf_network.o ospf_packet.o \
ospf_lsa.o ospf_spf.o ospf_route.o ospf_ase.o ospf_abr.o ospf_ia.o \
ospf_flood.o ospf_lsdb.o ospf_asbr.o ospf_routemap.o ospf_snmp.o
PROGRAMS =  $(sbin_PROGRAMS)

ospfd_OBJECTS =  ospf_main.o ospfd.o ospf_zebra.o ospf_interface.o \
ospf_ism.o ospf_neighbor.o ospf_nsm.o ospf_dump.o ospf_network.o \
ospf_packet.o ospf_lsa.o ospf_spf.o ospf_route.o ospf_ase.o ospf_abr.o \
ospf_ia.o ospf_flood.o ospf_lsdb.o ospf_asbr.o ospf_routemap.o \
ospf_snmp.o
ospfd_DEPENDENCIES =  ../lib/libzebra.a
ospfd_LDFLAGS = 
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
SOURCES = $(libospf_a_SOURCES) $(ospfd_SOURCES)
OBJECTS = $(libospf_a_OBJECTS) $(ospfd_OBJECTS)

all: all-redirect
.SUFFIXES:
.SUFFIXES: .S .c .o .s
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --foreign --include-deps ospfd/Makefile

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

libospf.a: $(libospf_a_OBJECTS) $(libospf_a_DEPENDENCIES)
	-rm -f libospf.a
	$(AR) cru libospf.a $(libospf_a_OBJECTS) $(libospf_a_LIBADD)
	$(RANLIB) libospf.a

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

ospfd: $(ospfd_OBJECTS) $(ospfd_DEPENDENCIES)
	@rm -f ospfd
	$(LINK) $(ospfd_LDFLAGS) $(ospfd_OBJECTS) $(ospfd_LDADD) $(LIBS)

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

subdir = ospfd

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

ospf_abr.o: ospf_abr.c ../lib/thread.h ../lib/memory.h \
 ../lib/linklist.h ../lib/prefix.h ../lib/if.h ../lib/table.h \
 ../lib/vty.h ../lib/filter.h ../lib/log.h ../ospfd/ospfd.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h \
 ../ospfd/ospf_nsm.h ../ospfd/ospf_spf.h ../ospfd/ospf_route.h \
 ../ospfd/ospf_ia.h ../ospfd/ospf_flood.h ../ospfd/ospf_abr.h \
 ../ospfd/ospf_ase.h ../ospfd/ospf_zebra.h ../ospfd/ospf_dump.h
ospf_asbr.o: ospf_asbr.c ../lib/thread.h ../lib/memory.h \
 ../lib/linklist.h ../lib/prefix.h ../lib/if.h ../lib/table.h \
 ../lib/vty.h ../lib/filter.h ../lib/log.h ../ospfd/ospfd.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_asbr.h ../ospfd/ospf_lsa.h \
 ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h ../ospfd/ospf_spf.h \
 ../ospfd/ospf_flood.h ../ospfd/ospf_route.h ../ospfd/ospf_zebra.h \
 ../ospfd/ospf_dump.h
ospf_ase.o: ospf_ase.c ../lib/thread.h ../lib/memory.h ../lib/hash.h \
 ../lib/linklist.h ../lib/prefix.h ../lib/if.h ../lib/table.h \
 ../lib/vty.h ../lib/log.h ../ospfd/ospfd.h ../lib/filter.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h \
 ../ospfd/ospf_nsm.h ../ospfd/ospf_spf.h ../ospfd/ospf_route.h \
 ../ospfd/ospf_ase.h ../ospfd/ospf_zebra.h ../ospfd/ospf_dump.h
ospf_dump.o: ospf_dump.c ../lib/linklist.h ../lib/thread.h \
 ../lib/prefix.h ../lib/command.h ../lib/vector.h ../lib/vty.h \
 ../lib/stream.h ../lib/log.h ../ospfd/ospfd.h ../lib/filter.h \
 ../lib/if.h ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h \
 ../ospfd/ospf_asbr.h ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h \
 ../ospfd/ospf_neighbor.h ../ospfd/ospf_nsm.h ../ospfd/ospf_dump.h \
 ../ospfd/ospf_packet.h ../ospfd/ospf_network.h
ospf_flood.o: ospf_flood.c ../lib/linklist.h ../lib/prefix.h \
 ../lib/if.h ../lib/command.h ../lib/vector.h ../lib/vty.h \
 ../lib/table.h ../lib/thread.h ../lib/memory.h ../lib/log.h \
 ../lib/zclient.h ../ospfd/ospfd.h ../lib/filter.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h \
 ../ospfd/ospf_nsm.h ../ospfd/ospf_spf.h ../ospfd/ospf_flood.h \
 ../ospfd/ospf_packet.h ../ospfd/ospf_abr.h ../ospfd/ospf_route.h \
 ../ospfd/ospf_zebra.h ../ospfd/ospf_dump.h
ospf_ia.o: ospf_ia.c ../lib/thread.h ../lib/memory.h ../lib/hash.h \
 ../lib/linklist.h ../lib/prefix.h ../lib/table.h ../lib/log.h \
 ../ospfd/ospfd.h ../lib/filter.h ../lib/if.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h \
 ../ospfd/ospf_nsm.h ../ospfd/ospf_spf.h ../ospfd/ospf_route.h \
 ../ospfd/ospf_ase.h ../ospfd/ospf_abr.h ../ospfd/ospf_ia.h \
 ../ospfd/ospf_dump.h
ospf_interface.o: ospf_interface.c ../lib/thread.h ../lib/linklist.h \
 ../lib/prefix.h ../lib/if.h ../lib/table.h ../lib/memory.h \
 ../lib/command.h ../lib/vector.h ../lib/vty.h ../lib/stream.h \
 ../lib/log.h ../ospfd/ospf_spf.h ../ospfd/ospf_network.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h \
 ../ospfd/ospf_nsm.h ../ospfd/ospf_packet.h ../ospfd/ospf_abr.h \
 ../ospfd/ospfd.h ../lib/filter.h ../ospfd/ospf_dump.h
ospf_ism.o: ospf_ism.c ../lib/thread.h ../lib/linklist.h \
 ../lib/prefix.h ../lib/if.h ../lib/table.h ../lib/log.h \
 ../ospfd/ospfd.h ../lib/filter.h ../ospfd/ospf_interface.h \
 ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h ../ospfd/ospf_lsa.h \
 ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h ../ospfd/ospf_nsm.h \
 ../ospfd/ospf_network.h ../ospfd/ospf_dump.h ../ospfd/ospf_packet.h \
 ../ospfd/ospf_flood.h ../ospfd/ospf_abr.h
ospf_lsa.o: ospf_lsa.c ../lib/linklist.h ../lib/prefix.h ../lib/if.h \
 ../lib/table.h ../lib/memory.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/stream.h ../lib/log.h ../lib/thread.h \
 ../lib/hash.h ../ospfd/ospfd.h ../lib/filter.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h \
 ../ospfd/ospf_nsm.h ../ospfd/ospf_flood.h ../ospfd/ospf_packet.h \
 ../ospfd/ospf_spf.h ../ospfd/ospf_dump.h ../ospfd/ospf_route.h \
 ../ospfd/ospf_ase.h ../ospfd/ospf_zebra.h
ospf_lsdb.o: ospf_lsdb.c ../lib/thread.h ../lib/memory.h ../lib/hash.h \
 ../lib/linklist.h ../lib/prefix.h ../lib/if.h ../lib/table.h \
 ../lib/command.h ../lib/vector.h ../lib/vty.h ../lib/stream.h \
 ../lib/log.h ../ospfd/ospfd.h ../lib/filter.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h \
 ../ospfd/ospf_nsm.h ../ospfd/ospf_flood.h ../ospfd/ospf_packet.h \
 ../ospfd/ospf_spf.h ../ospfd/ospf_dump.h ../ospfd/ospf_route.h
ospf_main.o: ospf_main.c ../lib/version.h ../lib/thread.h \
 ../lib/prefix.h ../lib/linklist.h ../lib/if.h ../lib/vector.h \
 ../lib/vty.h ../lib/command.h ../lib/filter.h ../lib/stream.h \
 ../lib/log.h ../lib/memory.h ../ospfd/ospfd.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_asbr.h ../ospfd/ospf_lsa.h \
 ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h ../ospfd/ospf_dump.h \
 ../ospfd/ospf_zebra.h
ospf_neighbor.o: ospf_neighbor.c ../lib/linklist.h ../lib/prefix.h \
 ../lib/memory.h ../lib/command.h ../lib/vector.h ../lib/vty.h \
 ../lib/thread.h ../lib/stream.h ../lib/table.h ../lib/log.h \
 ../ospfd/ospfd.h ../lib/filter.h ../lib/if.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_asbr.h ../ospfd/ospf_lsa.h \
 ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h ../ospfd/ospf_nsm.h \
 ../ospfd/ospf_packet.h ../ospfd/ospf_network.h ../ospfd/ospf_flood.h
ospf_network.o: ospf_network.c ../lib/thread.h ../lib/linklist.h \
 ../lib/prefix.h ../lib/if.h ../lib/sockunion.h ../lib/log.h \
 ../ospfd/ospfd.h ../lib/filter.h ../ospfd/ospf_network.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_asbr.h ../ospfd/ospf_lsa.h \
 ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h ../ospfd/ospf_packet.h
ospf_nsm.o: ospf_nsm.c ../lib/thread.h ../lib/memory.h ../lib/hash.h \
 ../lib/linklist.h ../lib/prefix.h ../lib/if.h ../lib/table.h \
 ../lib/stream.h ../lib/log.h ../ospfd/ospfd.h ../lib/filter.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h \
 ../ospfd/ospf_nsm.h ../ospfd/ospf_network.h ../ospfd/ospf_packet.h \
 ../ospfd/ospf_dump.h ../ospfd/ospf_flood.h ../ospfd/ospf_abr.h
ospf_packet.o: ospf_packet.c ../lib/thread.h ../lib/memory.h \
 ../lib/linklist.h ../lib/prefix.h ../lib/if.h ../lib/table.h \
 ../lib/sockunion.h ../lib/stream.h ../lib/log.h ../lib/md5-gnu.h \
 ../ospfd/ospfd.h ../lib/filter.h ../ospfd/ospf_network.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h \
 ../ospfd/ospf_nsm.h ../ospfd/ospf_packet.h ../ospfd/ospf_spf.h \
 ../ospfd/ospf_flood.h ../ospfd/ospf_dump.h
ospf_route.o: ospf_route.c ../lib/prefix.h ../lib/table.h \
 ../lib/memory.h ../lib/linklist.h ../lib/log.h ../lib/if.h \
 ../lib/command.h ../lib/vector.h ../lib/vty.h ../lib/sockunion.h \
 ../ospfd/ospfd.h ../lib/filter.h ../ospfd/ospf_interface.h \
 ../ospfd/ospf_asbr.h ../ospfd/ospf_lsa.h ../ospfd/ospf_route.h \
 ../ospfd/ospf_spf.h ../ospfd/ospf_zebra.h ../ospfd/ospf_dump.h
ospf_routemap.o: ospf_routemap.c ../lib/memory.h ../lib/prefix.h \
 ../lib/routemap.h ../lib/command.h ../lib/vector.h ../lib/vty.h \
 ../lib/log.h ../ospfd/ospfd.h ../lib/filter.h ../lib/if.h \
 ../lib/linklist.h ../ospfd/ospf_asbr.h ../ospfd/ospf_interface.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_route.h ../ospfd/ospf_zebra.h
ospf_snmp.o: ospf_snmp.c
ospf_spf.o: ospf_spf.c ../lib/thread.h ../lib/memory.h ../lib/hash.h \
 ../lib/linklist.h ../lib/prefix.h ../lib/if.h ../lib/table.h \
 ../lib/log.h ../lib/sockunion.h ../ospfd/ospfd.h ../lib/filter.h \
 ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h ../ospfd/ospf_neighbor.h \
 ../ospfd/ospf_nsm.h ../ospfd/ospf_spf.h ../ospfd/ospf_route.h \
 ../ospfd/ospf_ia.h ../ospfd/ospf_ase.h ../ospfd/ospf_abr.h \
 ../ospfd/ospf_dump.h
ospf_zebra.o: ospf_zebra.c ../lib/thread.h ../lib/command.h \
 ../lib/vector.h ../lib/vty.h ../lib/network.h ../lib/prefix.h \
 ../lib/routemap.h ../lib/table.h ../lib/stream.h ../lib/memory.h \
 ../lib/zclient.h ../lib/if.h ../lib/linklist.h ../lib/filter.h \
 ../lib/log.h ../ospfd/ospfd.h ../ospfd/ospf_interface.h \
 ../ospfd/ospf_ism.h ../ospfd/ospf_asbr.h ../ospfd/ospf_abr.h \
 ../ospfd/ospf_lsa.h ../ospfd/ospf_dump.h ../ospfd/ospf_route.h \
 ../ospfd/ospf_zebra.h
ospfd.o: ospfd.c ../lib/thread.h ../lib/vty.h ../lib/command.h \
 ../lib/vector.h ../lib/linklist.h ../lib/prefix.h ../lib/table.h \
 ../lib/if.h ../lib/memory.h ../lib/stream.h ../lib/log.h \
 ../lib/sockunion.h ../lib/zclient.h ../ospfd/ospfd.h ../lib/filter.h \
 ../ospfd/ospf_network.h ../ospfd/ospf_interface.h ../ospfd/ospf_ism.h \
 ../ospfd/ospf_asbr.h ../ospfd/ospf_lsa.h ../ospfd/ospf_lsdb.h \
 ../ospfd/ospf_neighbor.h ../ospfd/ospf_nsm.h ../ospfd/ospf_spf.h \
 ../ospfd/ospf_packet.h ../ospfd/ospf_dump.h ../ospfd/ospf_zebra.h \
 ../ospfd/ospf_abr.h ../ospfd/ospf_flood.h ../ospfd/ospf_route.h \
 ../ospfd/ospf_ase.h

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
