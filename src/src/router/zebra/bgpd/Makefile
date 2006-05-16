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

noinst_LIBRARIES = libbgp.a
sbin_PROGRAMS = bgpd
#noinst_PROGRAMS = bgp_btoa

libbgp_a_SOURCES =  	bgpd.c bgp_fsm.c bgp_aspath.c bgp_community.c bgp_attr.c 	bgp_debug.c bgp_route.c bgp_zebra.c bgp_open.c bgp_routemap.c 	bgp_packet.c bgp_network.c bgp_filter.c bgp_regex.c bgp_clist.c 	bgp_dump.c bgp_snmp.c bgp_ecommunity.c bgp_mplsvpn.c bgp_nexthop.c 	bgp_damp.c


noinst_HEADERS =  	bgp_aspath.h bgp_attr.h bgp_community.h bgp_debug.h bgp_fsm.h 	bgp_network.h bgp_open.h bgp_packet.h bgp_regex.h bgp_route.h 	bgpd.h bgp_filter.h bgp_clist.h bgp_dump.h bgp_zebra.h 	bgp_ecommunity.h bgp_mplsvpn.h bgp_nexthop.h bgp_damp.h


bgpd_SOURCES =  	bgp_main.c $(libbgp_a_SOURCES)


bgpd_LDADD = ../lib/libzebra.a

#bgp_btoa_SOURCES = \
#	bgp_btoa.c

#bgp_btoa_LDADD = libbgp.a ../lib/libzebra.a

sysconf_DATA = bgpd.conf.sample bgpd.conf.sample2

EXTRA_DIST = $(sysconf_DATA) BGP4-MIB.txt
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../config.h
CONFIG_CLEAN_FILES = 
LIBRARIES =  $(noinst_LIBRARIES)

CPPFLAGS = 
LDFLAGS = 
LIBS = -lm -lcrypt 
libbgp_a_LIBADD = 
libbgp_a_OBJECTS =  bgpd.o bgp_fsm.o bgp_aspath.o bgp_community.o \
bgp_attr.o bgp_debug.o bgp_route.o bgp_zebra.o bgp_open.o \
bgp_routemap.o bgp_packet.o bgp_network.o bgp_filter.o bgp_regex.o \
bgp_clist.o bgp_dump.o bgp_snmp.o bgp_ecommunity.o bgp_mplsvpn.o \
bgp_nexthop.o bgp_damp.o
PROGRAMS =  $(sbin_PROGRAMS)

bgpd_OBJECTS =  bgp_main.o bgpd.o bgp_fsm.o bgp_aspath.o bgp_community.o \
bgp_attr.o bgp_debug.o bgp_route.o bgp_zebra.o bgp_open.o \
bgp_routemap.o bgp_packet.o bgp_network.o bgp_filter.o bgp_regex.o \
bgp_clist.o bgp_dump.o bgp_snmp.o bgp_ecommunity.o bgp_mplsvpn.o \
bgp_nexthop.o bgp_damp.o
bgpd_DEPENDENCIES =  ../lib/libzebra.a
bgpd_LDFLAGS = 
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
SOURCES = $(libbgp_a_SOURCES) $(bgpd_SOURCES)
OBJECTS = $(libbgp_a_OBJECTS) $(bgpd_OBJECTS)

all: all-redirect
.SUFFIXES:
.SUFFIXES: .S .c .o .s
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --foreign --include-deps bgpd/Makefile

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

libbgp.a: $(libbgp_a_OBJECTS) $(libbgp_a_DEPENDENCIES)
	-rm -f libbgp.a
	$(AR) cru libbgp.a $(libbgp_a_OBJECTS) $(libbgp_a_LIBADD)
	$(RANLIB) libbgp.a

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

bgpd: $(bgpd_OBJECTS) $(bgpd_DEPENDENCIES)
	@rm -f bgpd
	$(LINK) $(bgpd_LDFLAGS) $(bgpd_OBJECTS) $(bgpd_LDADD) $(LIBS)

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

subdir = bgpd

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

bgp_aspath.o: bgp_aspath.c ../lib/hash.h ../lib/memory.h \
 ../lib/vector.h ../lib/vty.h ../lib/str.h ../lib/log.h ../bgpd/bgpd.h \
 ../lib/sockunion.h ../bgpd/bgp_aspath.h
bgp_attr.o: bgp_attr.c ../lib/linklist.h ../lib/prefix.h \
 ../lib/memory.h ../lib/vector.h ../lib/vty.h ../lib/stream.h \
 ../lib/log.h ../lib/hash.h ../bgpd/bgpd.h ../lib/sockunion.h \
 ../bgpd/bgp_attr.h ../bgpd/bgp_route.h ../bgpd/bgp_aspath.h \
 ../bgpd/bgp_community.h ../bgpd/bgp_debug.h ../bgpd/bgp_packet.h \
 ../bgpd/bgp_ecommunity.h
bgp_btoa.o: bgp_btoa.c ../lib/stream.h ../lib/log.h ../lib/prefix.h \
 ../lib/command.h ../lib/vector.h ../lib/vty.h ../bgpd/bgpd.h \
 ../lib/sockunion.h ../bgpd/bgp_dump.h ../bgpd/bgp_attr.h \
 ../bgpd/bgp_aspath.h
bgp_clist.o: bgp_clist.c ../lib/command.h ../lib/vector.h ../lib/vty.h \
 ../lib/memory.h ../lib/log.h ../lib/buffer.h ../bgpd/bgpd.h \
 ../lib/sockunion.h ../bgpd/bgp_community.h ../bgpd/bgp_clist.h \
 ../bgpd/bgp_aspath.h ../bgpd/bgp_regex.h ../lib/regex-gnu.h
bgp_community.o: bgp_community.c ../lib/hash.h ../lib/memory.h \
 ../lib/vector.h ../lib/vty.h ../lib/str.h ../bgpd/bgp_community.h
bgp_damp.o: bgp_damp.c ../lib/prefix.h ../lib/table.h \
 ../lib/linklist.h ../lib/memory.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/stream.h ../lib/filter.h ../lib/if.h ../lib/str.h \
 ../lib/log.h ../lib/routemap.h ../lib/buffer.h ../lib/sockunion.h \
 ../lib/plist.h ../lib/thread.h ../bgpd/bgp_damp.h ../bgpd/bgpd.h \
 ../bgpd/bgp_route.h
bgp_debug.o: bgp_debug.c ../lib/version.h ../lib/prefix.h \
 ../lib/linklist.h ../lib/stream.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/str.h ../lib/log.h ../lib/sockunion.h \
 ../bgpd/bgpd.h ../bgpd/bgp_aspath.h ../bgpd/bgp_route.h \
 ../bgpd/bgp_attr.h ../bgpd/bgp_debug.h ../bgpd/bgp_community.h
bgp_dump.o: bgp_dump.c ../lib/log.h ../lib/stream.h ../lib/sockunion.h \
 ../lib/command.h ../lib/vector.h ../lib/vty.h ../lib/prefix.h \
 ../lib/thread.h ../lib/table.h ../bgpd/bgpd.h ../bgpd/bgp_route.h \
 ../bgpd/bgp_attr.h ../bgpd/bgp_dump.h
bgp_ecommunity.o: bgp_ecommunity.c ../lib/hash.h ../lib/memory.h \
 ../lib/prefix.h ../lib/command.h ../lib/vector.h ../lib/vty.h \
 ../bgpd/bgpd.h ../lib/sockunion.h ../bgpd/bgp_ecommunity.h
bgp_filter.o: bgp_filter.c ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/log.h ../lib/memory.h ../lib/buffer.h \
 ../bgpd/bgpd.h ../lib/sockunion.h ../bgpd/bgp_aspath.h \
 ../bgpd/bgp_regex.h ../lib/regex-gnu.h ../bgpd/bgp_filter.h
bgp_fsm.o: bgp_fsm.c ../lib/linklist.h ../lib/prefix.h ../lib/vty.h \
 ../lib/sockunion.h ../lib/thread.h ../lib/log.h ../lib/stream.h \
 ../lib/memory.h ../bgpd/bgpd.h ../bgpd/bgp_attr.h ../bgpd/bgp_debug.h \
 ../bgpd/bgp_fsm.h ../bgpd/bgp_packet.h ../bgpd/bgp_network.h \
 ../bgpd/bgp_route.h ../bgpd/bgp_dump.h
bgp_main.o: bgp_main.c ../lib/vector.h ../lib/vty.h ../lib/command.h \
 ../lib/thread.h ../lib/version.h ../lib/memory.h ../lib/prefix.h \
 ../lib/log.h ../bgpd/bgpd.h ../lib/sockunion.h ../bgpd/bgp_network.h \
 ../bgpd/bgp_attr.h ../bgpd/bgp_mplsvpn.h
bgp_mplsvpn.o: bgp_mplsvpn.c ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/prefix.h ../lib/table.h ../lib/log.h \
 ../lib/memory.h ../lib/stream.h ../bgpd/bgpd.h ../lib/sockunion.h \
 ../bgpd/bgp_route.h ../bgpd/bgp_attr.h ../bgpd/bgp_mplsvpn.h
bgp_network.o: bgp_network.c ../lib/thread.h ../lib/sockunion.h \
 ../lib/memory.h ../lib/log.h ../lib/if.h ../lib/linklist.h \
 ../lib/prefix.h ../bgpd/bgpd.h ../bgpd/bgp_fsm.h ../bgpd/bgp_attr.h \
 ../bgpd/bgp_debug.h
bgp_nexthop.o: bgp_nexthop.c ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/thread.h ../lib/prefix.h ../lib/table.h \
 ../lib/zclient.h ../lib/if.h ../lib/linklist.h ../lib/stream.h \
 ../lib/network.h ../lib/log.h ../lib/memory.h ../bgpd/bgpd.h \
 ../lib/sockunion.h ../bgpd/bgp_route.h ../bgpd/bgp_attr.h \
 ../bgpd/bgp_nexthop.h ../zebra/rib.h ../zebra/zserv.h
bgp_open.o: bgp_open.c ../lib/linklist.h ../lib/prefix.h \
 ../lib/stream.h ../lib/thread.h ../lib/log.h ../lib/command.h \
 ../lib/vector.h ../lib/vty.h ../bgpd/bgpd.h ../lib/sockunion.h \
 ../bgpd/bgp_attr.h ../bgpd/bgp_debug.h ../bgpd/bgp_fsm.h \
 ../bgpd/bgp_packet.h ../bgpd/bgp_open.h
bgp_packet.o: bgp_packet.c ../lib/thread.h ../lib/stream.h \
 ../lib/network.h ../lib/prefix.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/log.h ../lib/memory.h ../lib/sockunion.h \
 ../lib/linklist.h ../bgpd/bgpd.h ../bgpd/bgp_dump.h \
 ../bgpd/bgp_attr.h ../bgpd/bgp_debug.h ../bgpd/bgp_fsm.h \
 ../bgpd/bgp_route.h ../bgpd/bgp_packet.h ../bgpd/bgp_open.h \
 ../bgpd/bgp_aspath.h ../bgpd/bgp_community.h ../bgpd/bgp_network.h \
 ../bgpd/bgp_mplsvpn.h
bgp_regex.o: bgp_regex.c ../lib/log.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/memory.h bgpd.h ../lib/sockunion.h bgp_aspath.h \
 bgp_regex.h ../lib/regex-gnu.h
bgp_route.o: bgp_route.c ../lib/prefix.h ../lib/table.h \
 ../lib/linklist.h ../lib/memory.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/stream.h ../lib/filter.h ../lib/if.h ../lib/str.h \
 ../lib/log.h ../lib/routemap.h ../lib/buffer.h ../lib/sockunion.h \
 ../lib/plist.h ../lib/thread.h ../bgpd/bgpd.h ../bgpd/bgp_route.h \
 ../bgpd/bgp_attr.h ../bgpd/bgp_debug.h ../bgpd/bgp_aspath.h \
 ../bgpd/bgp_clist.h ../bgpd/bgp_community.h ../bgpd/bgp_ecommunity.h \
 ../bgpd/bgp_packet.h ../bgpd/bgp_regex.h ../lib/regex-gnu.h \
 ../bgpd/bgp_filter.h ../bgpd/bgp_fsm.h ../bgpd/bgp_mplsvpn.h \
 ../bgpd/bgp_nexthop.h ../bgpd/bgp_damp.h
bgp_routemap.o: bgp_routemap.c ../lib/memory.h ../lib/prefix.h \
 ../lib/filter.h ../lib/if.h ../lib/linklist.h ../lib/routemap.h \
 ../lib/command.h ../lib/vector.h ../lib/vty.h ../lib/log.h \
 ../lib/plist.h ../lib/regex-gnu.h ../lib/buffer.h ../lib/table.h \
 ../lib/sockunion.h ../bgpd/bgpd.h ../bgpd/bgp_attr.h \
 ../bgpd/bgp_aspath.h ../bgpd/bgp_route.h ../bgpd/bgp_regex.h \
 ../bgpd/bgp_community.h ../bgpd/bgp_clist.h ../bgpd/bgp_filter.h \
 ../bgpd/bgp_mplsvpn.h ../bgpd/bgp_ecommunity.h
bgp_snmp.o: bgp_snmp.c
bgp_zebra.o: bgp_zebra.c ../lib/command.h ../lib/vector.h ../lib/vty.h \
 ../lib/stream.h ../lib/network.h ../lib/prefix.h ../lib/log.h \
 ../lib/sockunion.h ../lib/zclient.h ../lib/if.h ../lib/linklist.h \
 ../lib/routemap.h ../bgpd/bgpd.h ../bgpd/bgp_route.h \
 ../bgpd/bgp_attr.h ../bgpd/bgp_nexthop.h
bgpd.o: bgpd.c ../lib/prefix.h ../lib/thread.h ../lib/buffer.h \
 ../lib/stream.h ../lib/table.h ../lib/command.h ../lib/vector.h \
 ../lib/vty.h ../lib/sockunion.h ../lib/network.h ../lib/memory.h \
 ../lib/filter.h ../lib/if.h ../lib/linklist.h ../lib/routemap.h \
 ../lib/str.h ../lib/log.h ../lib/plist.h ../bgpd/bgpd.h \
 ../bgpd/bgp_aspath.h ../bgpd/bgp_route.h ../bgpd/bgp_dump.h \
 ../bgpd/bgp_debug.h ../bgpd/bgp_attr.h ../bgpd/bgp_community.h \
 ../bgpd/bgp_clist.h ../bgpd/bgp_fsm.h ../bgpd/bgp_packet.h \
 ../bgpd/bgp_zebra.h ../bgpd/bgp_open.h ../bgpd/bgp_filter.h \
 ../bgpd/bgp_nexthop.h ../bgpd/bgp_damp.h

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
