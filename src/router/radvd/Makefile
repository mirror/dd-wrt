#   Authors:
#    Lars Fenneberg		<lf@elemental.net>	 
#
#   This software is Copyright 1996,1997 by the above mentioned author(s), 
#   All Rights Reserved.
#
#   The license which is distributed with this software in the file COPYRIGHT
#   applies to this software. If your distribution is missing this file, you
#   may request it from <lutchann@litech.org>.
#


SHELL = /bin/sh

srcdir = .
top_srcdir = .
prefix = ../mipsel-uclibc/install/radvd
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/tmp
sysconfdir = ${prefix}/tmp
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = /opt/openwrt401/lib/
infodir = ${prefix}/info
includedir = /opt/openwrt401/include

DESTDIR =

pkgdatadir = $(datadir)
pkglibdir = $(libdir)
pkgincludedir = $(includedir)

top_builddir = .

ACLOCAL = aclocal
AUTOCONF = autoconf
AUTOMAKE = automake
AUTOHEADER = autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
build_alias = i686-pc-linux-gnu
build_triplet = i686-pc-linux-gnu
host_alias = i686-pc-linux-gnu
host_triplet = i686-pc-linux-gnu
target_alias = $(ARCH)-linux
target_triplet = $(ARCH)-unknown-linux-gnu
#CC = mipsel-linux-uclibc-gcc
#STRIP = mipsel-linux-uclibc-strip
GZIP = /usr/bin/gzip
LEX = flex
LN = /bin/ln
LOG_FACILITY = LOG_DAEMON
MAKEINFO = makeinfo
PACKAGE = radvd
PATH_RADVD_CONF = /tmp/radvd.conf
PATH_RADVD_LOG = /var/log/radvd.log
PATH_RADVD_PID = /var/run/radvd.pid
RM = /bin/rm
SED = /usr/bin/sed
TAR = /bin/tar
VERSION = 0.7.2
YACC = bison -y
LDADD = libfl.a ../iproute2/lib/inet_ntop.o ../iproute2/lib/inet_pton.o

DEFS =  -DPATH_RADVD_CONF=\"$(PATH_RADVD_CONF)\" \
	-DPATH_RADVD_LOG=\"$(PATH_RADVD_LOG)\" -DLOG_FACILITY=$(LOG_FACILITY) \
	-DPATH_RADVD_PID=\"$(PATH_RADVD_PID)\" \
	-DVERSION=\"$(VERSION)\" -DINET6=1

INCLUDES =  -I$(srcdir) -I.

########################################################################

COMMON_SRC = log.c socket.c recv.c util.c radvd.h defaults.h pathnames.h \
	includes.h


sbin_PROGRAMS = radvd radvdump

radvd_SOURCES = $(COMMON_SRC) radvd.c timer.c send.c process.c interface.c \
	device.c device-common.c gram.c gram.h scanner.c

radvd_LDADD = $(LDADD)

EXTRA_radvd_SOURCES = device-linux.c device-bsd44.c
YFLAGS = -d

radvdump_SOURCES = $(COMMON_SRC) radvdump.c

BUILT_SOURCES = device.c

CLEANFILES = radvd.8 radvdump.8 radvd.conf.5
DISTCLEANFILES = device.c

ACLOCAL_M4 = $(top_srcdir)/aclocal.m4
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = config.h
CONFIG_CLEAN_FILES = 
PROGRAMS =  $(sbin_PROGRAMS)

CPPFLAGS = 
LDFLAGS = 
LIBS = -lc -lutil
radvd_OBJECTS =  log.o socket.o recv.o util.o radvd.o timer.o send.o \
process.o interface.o device.o device-common.o gram.o scanner.o
radvd_DEPENDENCIES = 
radvd_LDFLAGS = 
radvdump_OBJECTS =  log.o socket.o recv.o util.o radvdump.o
radvdump_LDADD = $(LDADD)
radvdump_DEPENDENCIES = 
radvdump_LDFLAGS = 
LEX_OUTPUT_ROOT = lex.yy
LEXLIB = 
CFLAGS = $(COPTS)
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(AM_LDFLAGS) $(LDFLAGS) -o $@

NROFF = nroff
DIST_COMMON =  README TODO \
acconfig.h gram.c install-sh missing mkinstalldirs scanner.c


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

GZIP_ENV = --best
SOURCES = $(radvd_SOURCES) $(EXTRA_radvd_SOURCES) $(radvdump_SOURCES)
OBJECTS = $(radvd_OBJECTS) $(radvdump_OBJECTS)

all: all-redirect
.SUFFIXES:
.SUFFIXES: .S .c .l .o .s .y

mostlyclean-hdr:

clean-hdr:

distclean-hdr:
	-rm -f config.h

maintainer-clean-hdr:

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

radvd: $(radvd_OBJECTS) $(radvd_DEPENDENCIES)
	@rm -f radvd
	$(LINK) $(radvd_LDFLAGS) $(radvd_OBJECTS) $(radvd_LDADD) $(LIBS)
	$(STRIP) radvd

radvdump: $(radvdump_OBJECTS) $(radvdump_DEPENDENCIES)
	@rm -f radvdump
	$(LINK) $(radvdump_LDFLAGS) $(radvdump_OBJECTS) $(radvdump_LDADD) $(LIBS)
	$(STRIP) radvdump
gram.h: gram.c


tags: TAGS

ID: $(HEADERS) $(SOURCES) $(LISP)
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	here=`pwd` && cd $(srcdir) \
	  && mkid -f$$here/ID $$unique $(LISP)

TAGS:  $(HEADERS) $(SOURCES) config.h.in $(TAGS_DEPENDENCIES) $(LISP)
	tags=; \
	here=`pwd`; \
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	test -z "$(ETAGS_ARGS)config.h.in$$unique$(LISP)$$tags" \
	  || (cd $(srcdir) && etags $(ETAGS_ARGS) $$tags config.h.in $$unique $(LISP) -o $$here/TAGS)

mostlyclean-tags:

clean-tags:

distclean-tags:
	-rm -f TAGS ID

maintainer-clean-tags:

distdir = $(PACKAGE)-$(VERSION)
top_distdir = $(distdir)

# This target untars the dist file and tries a VPATH configuration.  Then
# it guarantees that the distribution is self-contained by making another
# tarfile.
distcheck: dist
	-rm -rf $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) zxf $(distdir).tar.gz
	mkdir $(distdir)/=build
	mkdir $(distdir)/=inst
	dc_install_base=`cd $(distdir)/=inst && pwd`; \
	cd $(distdir)/=build \
	  && ../configure --srcdir=.. --prefix=$$dc_install_base \
	  && $(MAKE) $(AM_MAKEFLAGS) \
	  && $(MAKE) $(AM_MAKEFLAGS) dvi \
	  && $(MAKE) $(AM_MAKEFLAGS) check \
	  && $(MAKE) $(AM_MAKEFLAGS) install \
	  && $(MAKE) $(AM_MAKEFLAGS) installcheck \
	  && $(MAKE) $(AM_MAKEFLAGS) dist
	-rm -rf $(distdir)
	@banner="$(distdir).tar.gz is ready for distribution"; \
	dashes=`echo "$$banner" | sed s/./=/g`; \
	echo "$$dashes"; \
	echo "$$banner"; \
	echo "$$dashes"
dist: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) chozf $(distdir).tar.gz $(distdir)
	-rm -rf $(distdir)
dist-all: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) chozf $(distdir).tar.gz $(distdir)
	-rm -rf $(distdir)
distdir: $(DISTFILES)
	-rm -rf $(distdir)
	mkdir $(distdir)
	-chmod 777 $(distdir)
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  if test -d $$d/$$file; then \
	    cp -pr $$d/$$file $(distdir)/$$file; \
	  else \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file || :; \
	  fi; \
	done
	$(MAKE) $(AM_MAKEFLAGS) top_distdir="$(top_distdir)" distdir="$(distdir)" dist-hook

info-am:
info: info-am
dvi-am:
dvi: dvi-am
check-am: all-am
check: check-am
installcheck-am:
installcheck: installcheck-am
all-recursive-am: config.h
	$(MAKE) $(AM_MAKEFLAGS) all-recursive

install-exec-am: install-sbinPROGRAMS
install-exec: install-exec-am

install-data: install-data-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-am
uninstall-am: uninstall-sbinPROGRAMS
uninstall: uninstall-am
all-am: Makefile $(PROGRAMS) config.h
all-redirect: all-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs:
	$(mkinstalldirs)  $(DESTDIR)$(sbindir)


mostlyclean-generic:

clean-generic:
	-test -z "$(CLEANFILES)" || rm -f $(CLEANFILES)

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*
	-test -z "$(DISTCLEANFILES)" || rm -f $(DISTCLEANFILES)

maintainer-clean-generic:
	-test -z "scannerlgramhgramc$(BUILT_SOURCES)" || rm -f scannerl gramh gramc $(BUILT_SOURCES)
mostlyclean-am:  mostlyclean-hdr mostlyclean-sbinPROGRAMS \
		mostlyclean-compile mostlyclean-tags \
		mostlyclean-generic

mostlyclean: mostlyclean-am

clean-am:  clean-hdr clean-sbinPROGRAMS clean-compile clean-tags \
		clean-generic mostlyclean-am

clean: clean-am

distclean-am:  distclean-hdr distclean-sbinPROGRAMS distclean-compile \
		distclean-tags distclean-generic clean-am

distclean: distclean-am
	-rm -f config.status

maintainer-clean-am:  maintainer-clean-hdr maintainer-clean-sbinPROGRAMS \
		maintainer-clean-compile maintainer-clean-tags \
		maintainer-clean-generic distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-am
	-rm -f config.status

.PHONY: mostlyclean-hdr distclean-hdr clean-hdr maintainer-clean-hdr \
mostlyclean-sbinPROGRAMS distclean-sbinPROGRAMS clean-sbinPROGRAMS \
maintainer-clean-sbinPROGRAMS uninstall-sbinPROGRAMS \
install-sbinPROGRAMS mostlyclean-compile distclean-compile \
clean-compile maintainer-clean-compile tags \
mostlyclean-tags distclean-tags clean-tags maintainer-clean-tags \
distdir info-am info dvi-am dvi check check-am installcheck-am \
installcheck all-recursive-am install-exec-am install-exec \
install-data-am install-data install-am install uninstall-am uninstall \
all-redirect all-am all installdirs mostlyclean-generic \
distclean-generic clean-generic maintainer-clean-generic clean \
mostlyclean distclean maintainer-clean


dist-hook:
	rm -f $(distdir)/device.c	
	mkdir $(distdir)/redhat
	cp -p $(srcdir)/redhat/radvd.init $(srcdir)/redhat/radvd.spec \
		$(srcdir)/redhat/radvd.sysconfig $(distdir)/redhat

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
