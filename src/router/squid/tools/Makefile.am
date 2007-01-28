#
#  Makefile for the Squid Object Cache server
#
#  $Id: Makefile.am,v 1.3 2006/09/22 09:08:46 hno Exp $
#
#  Uncomment and customize the following to suit your needs:
#


AUTOMAKE_OPTIONS = subdir-objects

INCLUDES        = -I. -I$(srcdir) -I$(top_builddir)/include -I$(top_srcdir)/include -I$(top_srcdir)/lib/libTrie/include

TESTS=$(check_PROGRAMS)
check_PROGRAMS=

SUBDIRS		= 

if NEED_COSSDUMP
COSSDUMP = cossdump
else
COSSDUMP =
endif

bin_PROGRAMS = \
	squidclient \
	$(COSSDUMP)

libexec_PROGRAMS = \
	cachemgr$(CGIEXT)

squidclient_SOURCES = squidclient.c
cossdump_SOURCES = cossdump.c
cachemgr__CGIEXT__SOURCES = cachemgr.c
cachemgr__CGIEXT__CFLAGS = -DDEFAULT_CACHEMGR_CONFIG=\"$(DEFAULT_CACHEMGR_CONFIG)\" $(AM_CFLAGS)

LDADD = -L../lib -lmiscutil $(XTRA_LIBS)

EXTRA_DIST = \
	cachemgr.conf

DEFAULT_CACHEMGR_CONFIG = $(sysconfdir)/cachemgr.conf

$(OBJS): $(top_srcdir)/include/version.h ../include/autoconf.h

install-data-local:
	@if test -f $(DESTDIR)$(DEFAULT_CACHEMGR_CONFIG) ; then \
	        echo "$@ will not overwrite existing $(DESTDIR)$(DEFAULT_CACHEMGR_CONFIG)" ; \
	else \
	        echo "$(INSTALL_DATA) $(srcdir)/cachemgr.conf $(DESTDIR)$(DEFAULT_CACHEMGR_CONFIG)"; \
	        $(INSTALL_DATA) $(srcdir)/cachemgr.conf $(DESTDIR)$(DEFAULT_CACHEMGR_CONFIG); \
	fi

uninstall-local:

# Don't automatically uninstall config files
#	@if test -f $(DESTDIR)$(DEFAULT_CONFIG_FILE) ; then \
#		echo "rm -f $(DESTDIR)$(DEFAULT_CONFIG_FILE)"; \
#		$(RM) -f $(DESTDIR)$(DEFAULT_CONFIG_FILE); \
#	fi

DISTCLEANFILES = 

