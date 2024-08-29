#
# man/Makefile	man/ Makefile for the net-tools Package
#
# NET-TOOLS	A collection of programs that form the base set of the
#		NET-3 Networking Distribution for the LINUX operating
#		system.
#
# Version:	Makefile 0.03 (2000-10-08)
#
# Authors:	Bernd Eckenfels <net-tools@lina.inka.de>
#               Arnaldo Carvalhoa de Melo <acme@conectiva.com.br>
#		Copyright 1995-1996 Bernd Eckebnfels, Germany
#                              1998 Arnaldo Carvalho de Melo, Brazil
#
#960322	{0.01}	Bernd Eckenfels:	creation to make main Makefile cleaner
#980701	{0.02}	Arnaldo C. Melo:	making the main Makefile *more* cleaner :)
#001008 {0.03}  Bernd Eckenfels:	RH patch for FHS (man page now in mandir)
#
#
#	This program is free software; you can redistribute it
#	and/or  modify it under  the terms of  the GNU General
#	Public  License as  published  by  the  Free  Software
#	Foundation;  either  version 2 of the License, or  (at
#	your option) any later version.
#

INSTALL=install
mandir=/usr/share/man

-include ../config.make
ifeq ($(I18N),1)
ALL_LANGS = $(filter-out Makefile,$(wildcard *))
ifeq ($(LINGUAS),)
LANGS = $(ALL_LANGS)
else
LANGS = $(sort $(filter $(LINGUAS),$(ALL_LANGS)) en_US)
endif
else
LANGS = en_US
endif

IGNORE_CMDS = ""
ifneq ($(HAVE_ARP_TOOLS),1)
IGNORE_CMDS += |arp.?|rarp.?
endif
ifneq ($(HAVE_HOSTNAME_TOOLS),1)
IGNORE_CMDS += |dnsdomainname.?|hostname.?
endif
ifneq ($(HAVE_HOSTNAME_SYMLINKS),1)
IGNORE_CMDS += |domainname.?|nisdomainname.?|ypdomainname.?
endif
ifneq ($(HAVE_AFDECnet),1)
IGNORE_CMDS += |nodename.?
endif
ifneq ($(HAVE_IP_TOOLS),1)
IGNORE_CMDS += |ipmaddr.?|iptunnel.?
endif
ifneq ($(HAVE_MII),1)
IGNORE_CMDS += |mii-tool.?
endif
ifneq ($(HAVE_PLIP_TOOLS),1)
IGNORE_CMDS += |plipconfig.?
endif
ifneq ($(HAVE_SERIAL_TOOLS),1)
IGNORE_CMDS += |slattach.?
endif

all:

clean:
		rm -f -- DEADJOE *~ *.orig
		rm -f -- */DEADJOE */*~ */*.orig

clobber:	clean

install:
		LANGS='$(LANGS)' ; \
		for LANG in $$LANGS; do \
			cd $$LANG; \
			for SECTION in 1 5 8; do \
				if [ "$$LANG" = "en_US" ] ; then \
					MAN_DIR=${BASEDIR}${mandir}/man$$SECTION; \
				else \
					MAN_DIR=${BASEDIR}${mandir}/$$LANG/man$$SECTION; \
				fi ; \
				$(INSTALL) -d -m 755 $$MAN_DIR; \
				for CMD in *.$$SECTION; do \
					[ -e $$CMD ] || continue; \
					case $$CMD in $(IGNORE_CMDS)) continue;; esac; \
					$(INSTALL) -m 644 $$CMD $$MAN_DIR || exit 1; \
				done; \
			done; \
			cd ..; \
		done

# End of man/Makefile.
