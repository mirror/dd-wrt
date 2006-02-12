# Generated automatically from Makefile.depend by configure.
#
# Minimum environment and virtual path setup
#
SHELL		= /bin/sh
srcdir		= .
top_srcdir	= ..
VERSION		= 5.0.9


#
# Paths
#
prefix		= /tmp/snmp
exec_prefix	= /tmp/snmp
bindir		= ${exec_prefix}/bin
sbindir		= ${exec_prefix}/sbin
libdir		= ${exec_prefix}/lib
datadir		= ${prefix}/share
includedir	= ${prefix}/include/net-snmp
ucdincludedir	= ${prefix}/include/ucd-snmp
mandir		= ${prefix}/man
man1dir		= $(mandir)/man1
man3dir		= $(mandir)/man3
man5dir		= $(mandir)/man5
man8dir		= $(mandir)/man8
snmplibdir	= $(datadir)/snmp
mibdir		= $(snmplibdir)/mibs
persistentdir	= /tmp/snmp-persist


#
# Programs
#
INSTALL		= $(LIBTOOL) --mode=install /usr/bin/install -c
INSTALL_DATA    = ${INSTALL} -m 644
SED		= /home/backup/mikrotik/toolchain/bin//sed
LN_S		= ln -s
AUTOCONF	= :
AUTOHEADER	= :

#
# Compiler arguments
#
CFLAGS		= -Os -mtune=mips32 -mips32 -Delf 
EXTRACPPFLAGS	= -x c
LDFLAGS		= -s 
LIBTOOL		= $(SHELL) $(top_builddir)/libtool 
EXEEXT		= 

# Misc Compiling Stuff
CC	        = mipsel-linux-uclibc-gcc
# version number is prefixed by a 0 for a better shared library version number
#
# ok, use libtool proper versioning:
LIBCURRENT  = 5
LIBREVISION = 9
LIBAGE      = 0
# generally, LIBCURRENT should stay the same unless major changes occur.
# Increment REVISION every release.  
# Increment AGE if new stuff is available.
LIB_LD_CMD      = $(LIBTOOL) --mode=link $(CC) $(CFLAGS) -rpath $(libdir) -version-info $(LIBCURRENT):$(LIBREVISION):$(LIBAGE) -o
LIB_EXTENSION   = la
LIB_VERSION     =
LIB_LDCONFIG_CMD = $(LIBTOOL) --mode=finish $(libdir)
LINK		= $(LIBTOOL) --mode=link $(CC)
# RANLIB 	= ranlib
RANLIB		= :

# libtool definitions
.SUFFIXES: .c .o .lo
.c.lo:
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
# Portions of this file are subject to the following copyright.  See
# the Net-SNMP's COPYING file for more details and other copyrights
# that may apply:
#
# Copyright © 2003 Sun Microsystems, Inc. All rights reserved.
# Use is subject to license terms specified in the COPYING file
# distributed with the Net-SNMP package.
#

#
# Makefile for snmplib
#

top_builddir=..

#
# Things to install
#

# headers
INSTALLHEADERS=\
	net-snmp-includes.h \
	config_api.h  \
	mib_api.h     \
	output_api.h  \
	pdu_api.h     \
	session_api.h \
	snmpv3_api.h  \
	varbind_api.h \
	types.h       \
	utilities.h   \
	version.h     \
	definitions.h

INCLUDESUBDIR=library
INCLUDESUBDIRHEADERS=README \
	asn1.h \
	callback.h \
	container.h \
	container_binary_array.h \
	factory.h \
	data_list.h \
	default_store.h \
	int64.h \
	keytools.h \
	mib.h \
	md5.h \
	parse.h \
	read_config.h \
	scapi.h \
	snmp.h \
	snmp_alarm.h \
	snmp_api.h \
	snmp_assert.h \
	snmp_client.h \
	snmp_debug.h \
	snmp_impl.h \
	snmp_logging.h \
	snmp_parse_args.h \
	snmp-tc.h \
	snmpv3.h \
	system.h \
	tools.h \
	transform_oids.h \
	winservice.h \
        cmu_compat.h \
        getopt.h \
        lcd_time.h \
        mt_support.h \
        oid_stash.h \
        snmp_enum.h \
        snmp_locking.h \
        snmp_secmod.h \
        ucd_compat.h \
        vacm.h \
	check_varbind.h \
	snmp_transport.h \
	snmpUDPDomain.h snmpCallbackDomain.h  \
	 snmpusm.h

INSTALLBUILTSUBDIRHEADERS=../include/net-snmp/library/snmpv3-security-includes.h
INSTALLBUILTSUBDIR=library


INSTALLUCDHEADERS= asn1.h \
	callback.h	\
	default_store.h	\
	int64.h		\
	keytools.h	\
	mib.h		\
	parse.h		\
	read_config.h	\
	scapi.h		\
	snmp_alarm.h	\
	snmp_api.h	\
	snmp_client.h	\
	snmp_debug.h	\
	snmp.h		\
	snmp_impl.h	\
	snmp_logging.h	\
	snmp_parse_args.h \
	snmp-tc.h	\
	snmpusm.h	\
	snmpv3.h	\
	snmp_vars.h	\
	struct.h	\
	system.h	\
	tools.h		\
	transform_oids.h

# libraries
INSTALLLIBS=libnetsnmp.$(LIB_EXTENSION)$(LIB_VERSION)
INSTALLUCDLIBS=libsnmp.$(LIB_EXTENSION)$(LIB_VERSION)

#
# Things to build
#
CSRCS=	snmp_client.c mib.c parse.c snmp_api.c snmp.c 		\
	snmp_auth.c asn1.c md5.c snmp_parse_args.c		\
	system.c vacm.c int64.c read_config.c 			\
	snmp_debug.c tools.c  snmp_logging.c			\
	snmpv3.c lcd_time.c keytools.c		                \
	scapi.c callback.c default_store.c snmp_alarm.c		\
	data_list.c oid_stash.c 				\
	mt_support.c snmp_enum.c snmp-tc.c 			\
	snprintf.c strlcpy.c strtol.c strtoul.c			\
	snmp_transport.c                                        \
	snmpUDPDomain.c snmpCallbackDomain.c  \
	snmp_secmod.c snmp_version.c                            \
	 snmpusm.c \
	check_varbind.c container.c container_binary_array.c	\
	cmu_compat.c ucd_compat.c

OBJS=	$(CSRCS:.c=.o)

LOBJS=	$(CSRCS:.c=.lo)

# just in case someone wants to remove libtool, change this to OBJS.
TOBJS=$(LOBJS)

CPPFLAGS = -I$(top_srcdir)/include -I../include \
	-I. -I.. -I$(srcdir) -I$(srcdir)/.. 

all: standardall

# how to build the libraries.
libnetsnmp.$(LIB_EXTENSION)$(LIB_VERSION):    ${TOBJS}
	$(LIB_LD_CMD) libnetsnmp.$(LIB_EXTENSION)$(LIB_VERSION) ${TOBJS}
	$(RANLIB) libnetsnmp.$(LIB_EXTENSION)$(LIB_VERSION)

libsnmp.$(LIB_EXTENSION)$(LIB_VERSION):    ${TOBJS}
	$(LIB_LD_CMD) libsnmp.$(LIB_EXTENSION)$(LIB_VERSION) ${TOBJS}
	$(RANLIB) libsnmp.$(LIB_EXTENSION)$(LIB_VERSION)

#
# internal test objects
#
parse:  mib.o parse.c
	$(CC) $(CFLAGS) -DTEST parse.c -o parse \
		`$(top_srcdir)/net-snmp-config --libs`

test_binary_array: test_binary_array.c
	$(CC) $(CFLAGS) test_binary_array.c  -o $@ \
		`$(top_srcdir)/net-snmp-config --libs`

#
# standard target definitions.  Set appropriate variables to make use of them.
#
# note: the strange use of the "it" variable is for shell parsing when
# there is no targets to install for that rule.
#

# the standard items to build: libraries, bins, and sbins
STANDARDTARGETS     =$(INSTALLLIBS) $(INSTALLBINPROGS) $(INSTALLSBINPROGS)
STANDARDCLEANTARGETS=$(INSTALLLIBS) $(INSTALLBINPROGS) $(INSTALLSBINPROGS) $(INSTALLUCDLIBS)

standardall: subdirs $(STANDARDTARGETS)

subdirs:
	@if test "$(SUBDIRS)" != ""; then \
		it="$(SUBDIRS)" ; \
		for i in $$it ; do       \
			echo "making all in `pwd`/$$i"; \
	                ( cd $$i ; $(MAKE) ) ; \
                        if test $$? != 0 ; then \
				exit 1 ; \
			fi  \
	        done \
	fi

install: installlocalheaders  \
         installlocallibs        \
         installlocalbin      installlocalsbin   \
         installsubdirs      $(OTHERINSTALL)

installprogs: installbin installsbin

#
# headers
#
# set INSTALLHEADERS to a list of things to install in each makefile.
# set INSTALLBUILTHEADERS to a list of things to install from builddir
# set INSTALLSUBDIRHEADERS and INSTALLSUBDIR to subdirectory headers
# set INSTALLSUBDIRHEADERS2 and INSTALLSUBDIR2 to more subdirectory headers
# set INSTALLBUILTSUBDIRHEADERS and INSTALLBUILTSUBDIR to a list from builddir
#
installheaders: installlocalheaders  installsubdirheaders

installlocalheaders:
	@if test "$(INSTALLHEADERS)" != "" ; then \
		echo creating directory $(includedir) ; \
		it="$(INSTALLHEADERS)" ; \
		$(SHELL) $(top_srcdir)/mkinstalldirs $(includedir) ; \
	        for i in $$it ; do \
			$(INSTALL) $(top_srcdir)/include/net-snmp/$$i $(includedir) ; \
	                echo "installing $$i in $(includedir)" ; \
	        done \
	fi
	@if test "$(INSTALLBUILTHEADERS)" != "" ; then \
		echo creating directory $(includedir) ; \
		it="$(INSTALLBUILTHEADERS)" ; \
		$(SHELL) $(top_srcdir)/mkinstalldirs $(includedir) ; \
	        for i in $$it ; do \
			$(INSTALL) $$i $(includedir) ; \
	                echo "installing $$i in $(includedir)" ; \
	        done \
	fi
	@if test "$(INCLUDESUBDIRHEADERS)" != "" ; then \
		echo creating directory $(includedir)/$(INCLUDESUBDIR) ; \
		it="$(INCLUDESUBDIRHEADERS)" ; \
		$(SHELL) $(top_srcdir)/mkinstalldirs $(includedir)/$(INCLUDESUBDIR) ; \
	        for i in $$it ; do \
			$(INSTALL) $(top_srcdir)/include/net-snmp/$(INCLUDESUBDIR)/$$i $(includedir)/$(INCLUDESUBDIR) ; \
	                echo "installing $$i in $(includedir)/$(INCLUDESUBDIR)" ; \
	        done \
	fi
	@if test "$(INCLUDESUBDIRHEADERS2)" != "" ; then \
		echo creating directory $(includedir)/$(INCLUDESUBDIR2) ; \
		it="$(INCLUDESUBDIRHEADERS2)" ; \
		$(SHELL) $(top_srcdir)/mkinstalldirs $(includedir)/$(INCLUDESUBDIR2) ; \
	        for i in $$it ; do \
			$(INSTALL) $(top_srcdir)/include/net-snmp/$(INCLUDESUBDIR2)/$$i $(includedir)/$(INCLUDESUBDIR2) ; \
	                echo "installing $$i in $(includedir)/$(INCLUDESUBDIR2)" ; \
	        done \
	fi
	@if test "$(INSTALLBUILTSUBDIRHEADERS)" != "" ; then \
		echo creating directory $(includedir)/$(INSTALLBUILTSUBDIR) ; \
		it="$(INSTALLBUILTSUBDIRHEADERS)" ; \
		$(SHELL) $(top_srcdir)/mkinstalldirs $(includedir)/$(INSTALLBUILTSUBDIR) ; \
	        for i in $$it ; do \
			$(INSTALL) $$i $(includedir)/$(INSTALLBUILTSUBDIR) ; \
	                echo "installing $$i in $(includedir)/$(INSTALLBUILTSUBDIR)" ; \
	        done \
	fi

installsubdirheaders:
	@if test "$(SUBDIRS)" != ""; then \
		it="$(SUBDIRS)" ; \
		for i in $$it ; do       \
			echo "making installheaders in `pwd`/$$i"; \
	                ( cd $$i ; $(MAKE) installheaders) ;   \
                        if test $$? != 0 ; then \
				exit 1 ; \
			fi  \
	        done \
	fi

installucdheaders:
	@if test "$(INSTALLUCDHEADERS)" != "" ; then \
		echo creating directory $(ucdincludedir) ; \
		it="$(INSTALLUCDHEADERS)" ; \
		$(SHELL) $(top_srcdir)/mkinstalldirs $(ucdincludedir) ; \
	        for i in $$it ; do \
			$(INSTALL) $(top_srcdir)/include/ucd-snmp/$$i $(ucdincludedir) ; \
	                echo "installing $$i in $(ucdincludedir)" ; \
	        done \
	fi

#
# libraries
#
# set INSTALLLIBS to a list of things to install in each makefile.
#
installlibs: installlocallibs  installsubdirlibs

installlocallibs: $(INSTALLLIBS)
	@if test "$(INSTALLLIBS)" != ""; then \
		it="$(INSTALLLIBS)" ; \
		$(SHELL) $(top_srcdir)/mkinstalldirs $(libdir) ; \
                $(INSTALL) $(INSTALLLIBS) $(libdir) ;              \
	        for i in $$it ; do    \
	                echo "installing $$i in $(libdir)"; \
	                $(RANLIB) $(libdir)/$$i ; \
	        done ; \
		$(LIB_LDCONFIG_CMD) ; \
	fi

installucdlibs: $(INSTALLUCDLIBS)
	@if test "$(INSTALLUCDLIBS)" != ""; then \
		it="$(INSTALLUCDLIBS)" ; \
		$(SHELL) $(top_srcdir)/mkinstalldirs $(libdir) ; \
                $(INSTALL) $(INSTALLUCDLIBS) $(libdir) ;              \
	        for i in $$it ; do    \
	                echo "installing $$i in $(libdir)"; \
	                $(RANLIB) $(libdir)/$$i ; \
	        done ; \
		$(LIB_LDCONFIG_CMD) ; \
	fi

installsubdirlibs:
	@if test "$(SUBDIRS)" != ""; then \
		it="$(SUBDIRS)" ; \
		for i in $$it ; do       \
			echo "making installlibs in `pwd`/$$i"; \
	                ( cd $$i ; $(MAKE) installlibs) ;   \
                        if test $$? != 0 ; then \
				exit 1 ; \
			fi  \
	        done \
	fi

#
# normal bin binaries
#
# set INSTALLBINPROGS to a list of things to install in each makefile.
#
installbin: installlocalbin installsubdirbin

installlocalbin: $(INSTALLBINPROGS)
	@if test "$(INSTALLBINPROGS) $(INSTALLBINSCRIPTS)" != " "; then \
		$(SHELL) $(top_srcdir)/mkinstalldirs $(bindir) ; \
		it="$(INSTALLBINPROGS) $(INSTALLBINSCRIPTS)" ; \
	        $(INSTALL) $(INSTALLBINPROGS) $(INSTALLBINSCRIPTS) $(bindir) ;              \
	        for i in $$it ; do   \
	                echo "installing $$i in $(bindir)"; \
	        done \
	fi

installsubdirbin:
	@if test "$(SUBDIRS)" != ""; then \
		it="$(SUBDIRS)" ; \
		for i in $$it ; do       \
			echo "making installbin in `pwd`/$$i"; \
	                ( cd $$i ; $(MAKE) installbin) ;   \
                        if test $$? != 0 ; then \
				exit 1 ; \
			fi  \
	        done \
	fi

#
# sbin binaries
#
# set INSTALLSBINPROGS to a list of things to install in each makefile.
#
installsbin: installlocalsbin installsubdirsbin

installlocalsbin: $(INSTALLSBINPROGS)
	@if test "$(INSTALLSBINPROGS)" != ""; then \
		it="$(INSTALLSBINPROGS)" ; \
		$(SHELL) $(top_srcdir)/mkinstalldirs $(sbindir) ; \
	        $(INSTALL) $(INSTALLSBINPROGS) $(sbindir) ;              \
	        for i in $$it ; do   \
	                echo "installing $$i in $(sbindir)"; \
	        done \
	fi

installsubdirsbin:
	@if test "$(SUBDIRS)" != ""; then \
		it="$(SUBDIRS)" ; \
		for i in $$it ; do       \
			echo "making installsbin in `pwd`/$$i"; \
	                ( cd $$i ; $(MAKE) installsbin) ;   \
                        if test $$? != 0 ; then \
				exit 1 ; \
			fi  \
	        done \
	fi

#
# general make install target for subdirs
#
installsubdirs:
	@if test "$(SUBDIRS)" != ""; then \
		it="$(SUBDIRS)" ; \
		for i in $$it ; do       \
			echo "making install in `pwd`/$$i"; \
	                ( cd $$i ; $(MAKE) install) ;   \
                        if test $$? != 0 ; then \
				exit 1 ; \
			fi  \
	        done \
	fi

#
# cleaning targets
#
clean: cleansubdirs $(OTHERCLEANTODOS)
	rm -f core ${OBJS} ${LOBJS} $(STANDARDCLEANTARGETS) $(OTHERCLEANTARGETS)
	rm -rf .libs

cleansubdirs:
	@if test "$(SUBDIRS)" != ""; then \
		it="$(SUBDIRS)" ; \
		for i in $$it ; do       \
			echo "making clean in `pwd`/$$i"; \
	                ( cd $$i ; $(MAKE) clean) ;   \
                        if test $$? != 0 ; then \
				exit 1 ; \
			fi  \
	        done \
	fi

lint:
	lint -nhx $(CSRCS)

#
# wacky dependency building.
#
depend: dependdirs
	@if test -f Makefile.depend ; then \
		makedepend `echo $(CPPFLAGS) | sed 's/-f[-a-z]*//g'` -o .lo $(srcdir)/*.c $(srcdir)/*/*.c ; \
	fi


nosysdepend: nosysdependdirs
	@if test -f Makefile.depend ; then \
		makedepend `echo $(CPPFLAGS) | sed 's/-f[-a-z]*//g'` -o .lo $(srcdir)/*.c $(srcdir)/*/*.c ; \
		perl -n -i.bak $(top_srcdir)/makenosysdepend.pl Makefile ; \
	fi

distdepend: nosysdepend distdependdirs
	@if test -f Makefile.depend ; then \
		perl $(top_srcdir)/makefileindepend.pl ; \
	fi

dependdirs:
	@if test "$(SUBDIRS)" != ""; then \
		it="$(SUBDIRS)" ; \
		for i in $$it ; do       \
			echo "making depend in `pwd`/$$i"; \
	                ( cd $$i ; $(MAKE) depend) ;   \
                        if test $$? != 0 ; then \
				exit 1 ; \
			fi  \
	        done \
	fi

nosysdependdirs:
	@if test "$(SUBDIRS)" != ""; then \
		it="$(SUBDIRS)" ; \
		for i in $$it ; do       \
			echo "making nosysdepend in `pwd`/$$i"; \
	                ( cd $$i ; $(MAKE) nosysdepend) ;   \
                        if test $$? != 0 ; then \
				exit 1 ; \
			fi  \
	        done \
	fi

distdependdirs:
	@if test "$(SUBDIRS)" != ""; then \
		it="$(SUBDIRS)" ; \
		for i in $$it ; do       \
			echo "making distdepend in `pwd`/$$i"; \
	                ( cd $$i ; $(MAKE) distdepend) ;   \
                        if test $$? != 0 ; then \
				exit 1 ; \
			fi  \
	        done \
	fi

# These aren't real targets, let gnu's make know that.
.PHONY: clean cleansubdirs lint \
	install installprogs installheaders installlibs \
	installbin installsbin installsubdirs \
	all subdirs standardall \
	depend nosysdepend distdepend dependdirs nosysdependdirs distdependdirs
# DO NOT DELETE THIS LINE -- make depend depends on it.

./asn1.lo: ../include/net-snmp/net-snmp-config.h
./asn1.lo: ../include/net-snmp/system/linux.h
./asn1.lo: ../include/net-snmp/system/sysv.h
./asn1.lo: ../include/net-snmp/system/generic.h
./asn1.lo: ../include/net-snmp/machine/generic.h 
./asn1.lo: ../include/net-snmp/output_api.h ../include/net-snmp/types.h
./asn1.lo:  ../include/net-snmp/definitions.h
./asn1.lo: ../include/net-snmp/library/snmp_api.h
./asn1.lo: ../include/net-snmp/library/asn1.h
./asn1.lo: ../include/net-snmp/library/snmp_impl.h
./asn1.lo: ../include/net-snmp/library/snmp.h
./asn1.lo: ../include/net-snmp/library/snmp-tc.h
./asn1.lo: ../include/net-snmp/library/snmp_client.h
./asn1.lo: ../include/net-snmp/library/snmp_debug.h
./asn1.lo: ../include/net-snmp/library/snmp_logging.h 
./asn1.lo:  ../include/net-snmp/utilities.h
./asn1.lo: ../include/net-snmp/library/getopt.h
./asn1.lo: ../include/net-snmp/library/system.h
./asn1.lo: ../include/net-snmp/library/tools.h
./asn1.lo: ../include/net-snmp/library/int64.h
./asn1.lo: ../include/net-snmp/library/mt_support.h
./asn1.lo: ../include/net-snmp/library/snmp_alarm.h
./asn1.lo: ../include/net-snmp/library/data_list.h
./asn1.lo: ../include/net-snmp/library/callback.h
./asn1.lo: ../include/net-snmp/library/oid_stash.h
./asn1.lo: ../include/net-snmp/library/check_varbind.h
./asn1.lo: ../include/net-snmp/version.h ../include/net-snmp/library/mib.h
./callback.lo: ../include/net-snmp/net-snmp-config.h
./callback.lo: ../include/net-snmp/system/linux.h
./callback.lo: ../include/net-snmp/system/sysv.h
./callback.lo: ../include/net-snmp/system/generic.h
./callback.lo: ../include/net-snmp/machine/generic.h 
./callback.lo:  ../include/net-snmp/types.h
./callback.lo:  ../include/net-snmp/definitions.h
./callback.lo: ../include/net-snmp/library/snmp_api.h
./callback.lo: ../include/net-snmp/library/asn1.h
./callback.lo: ../include/net-snmp/library/snmp_impl.h
./callback.lo: ../include/net-snmp/library/snmp.h
./callback.lo: ../include/net-snmp/library/snmp-tc.h
./callback.lo: ../include/net-snmp/output_api.h
./callback.lo: ../include/net-snmp/library/snmp_client.h
./callback.lo: ../include/net-snmp/library/snmp_debug.h
./callback.lo: ../include/net-snmp/library/snmp_logging.h
./callback.lo: ../include/net-snmp/utilities.h
./callback.lo: ../include/net-snmp/library/getopt.h
./callback.lo: ../include/net-snmp/library/system.h
./callback.lo: ../include/net-snmp/library/tools.h
./callback.lo: ../include/net-snmp/library/int64.h
./callback.lo: ../include/net-snmp/library/mt_support.h
./callback.lo: ../include/net-snmp/library/snmp_alarm.h
./callback.lo: ../include/net-snmp/library/data_list.h
./callback.lo: ../include/net-snmp/library/callback.h
./callback.lo: ../include/net-snmp/library/oid_stash.h
./callback.lo: ../include/net-snmp/library/check_varbind.h
./callback.lo: ../include/net-snmp/version.h
./check_varbind.lo: ../include/net-snmp/net-snmp-config.h
./check_varbind.lo: ../include/net-snmp/system/linux.h
./check_varbind.lo: ../include/net-snmp/system/sysv.h
./check_varbind.lo: ../include/net-snmp/system/generic.h
./check_varbind.lo: ../include/net-snmp/machine/generic.h
./check_varbind.lo: ../include/net-snmp/net-snmp-includes.h
./check_varbind.lo: ../include/net-snmp/definitions.h
./check_varbind.lo: ../include/net-snmp/types.h
./check_varbind.lo: ../include/net-snmp/library/snmp_api.h
./check_varbind.lo: ../include/net-snmp/library/asn1.h
./check_varbind.lo: ../include/net-snmp/library/snmp_impl.h
./check_varbind.lo: ../include/net-snmp/library/snmp.h
./check_varbind.lo: ../include/net-snmp/library/snmp-tc.h
./check_varbind.lo: ../include/net-snmp/utilities.h
./check_varbind.lo: ../include/net-snmp/library/snmp_client.h
./check_varbind.lo: ../include/net-snmp/library/getopt.h
./check_varbind.lo: ../include/net-snmp/library/system.h
./check_varbind.lo: ../include/net-snmp/library/tools.h
./check_varbind.lo: ../include/net-snmp/library/int64.h
./check_varbind.lo: ../include/net-snmp/library/mt_support.h
./check_varbind.lo: ../include/net-snmp/library/snmp_alarm.h
./check_varbind.lo: ../include/net-snmp/library/data_list.h
./check_varbind.lo: ../include/net-snmp/library/callback.h
./check_varbind.lo: ../include/net-snmp/library/oid_stash.h
./check_varbind.lo: ../include/net-snmp/library/check_varbind.h
./check_varbind.lo: ../include/net-snmp/version.h
./check_varbind.lo: ../include/net-snmp/session_api.h
./check_varbind.lo: ../include/net-snmp/library/snmp_transport.h
./check_varbind.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./check_varbind.lo: ../include/net-snmp/library/snmpUnixDomain.h
./check_varbind.lo: ../include/net-snmp/library/snmpUDPDomain.h
./check_varbind.lo: ../include/net-snmp/library/snmpTCPDomain.h
./check_varbind.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./check_varbind.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./check_varbind.lo: ../include/net-snmp/library/snmpIPXDomain.h
./check_varbind.lo: ../include/net-snmp/library/ucd_compat.h
./check_varbind.lo: ../include/net-snmp/pdu_api.h
./check_varbind.lo: ../include/net-snmp/mib_api.h
./check_varbind.lo: ../include/net-snmp/library/mib.h
./check_varbind.lo: ../include/net-snmp/library/parse.h
./check_varbind.lo: ../include/net-snmp/varbind_api.h
./check_varbind.lo: ../include/net-snmp/config_api.h
./check_varbind.lo: ../include/net-snmp/library/read_config.h
./check_varbind.lo: ../include/net-snmp/library/default_store.h
./check_varbind.lo: ../include/net-snmp/library/snmp_parse_args.h
./check_varbind.lo: ../include/net-snmp/library/snmp_enum.h
./check_varbind.lo: ../include/net-snmp/library/vacm.h
./check_varbind.lo: ../include/net-snmp/output_api.h
./check_varbind.lo: ../include/net-snmp/library/snmp_debug.h
./check_varbind.lo: ../include/net-snmp/library/snmp_logging.h
./check_varbind.lo: ../include/net-snmp/snmpv3_api.h
./check_varbind.lo: ../include/net-snmp/library/snmpv3.h
./check_varbind.lo: ../include/net-snmp/library/transform_oids.h
./check_varbind.lo: ../include/net-snmp/library/keytools.h
./check_varbind.lo: ../include/net-snmp/library/scapi.h
./check_varbind.lo: ../include/net-snmp/library/lcd_time.h
./check_varbind.lo: ../include/net-snmp/library/snmp_secmod.h
./check_varbind.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./check_varbind.lo: ../include/net-snmp/library/snmpusm.h
./container_binary_array.lo: ../include/net-snmp/net-snmp-config.h
./container_binary_array.lo: ../include/net-snmp/system/linux.h
./container_binary_array.lo: ../include/net-snmp/system/sysv.h
./container_binary_array.lo: ../include/net-snmp/system/generic.h
./container_binary_array.lo: ../include/net-snmp/machine/generic.h
./container_binary_array.lo: ../include/net-snmp/net-snmp-includes.h
./container_binary_array.lo: ../include/net-snmp/definitions.h
./container_binary_array.lo: ../include/net-snmp/types.h
./container_binary_array.lo: ../include/net-snmp/library/snmp_api.h
./container_binary_array.lo: ../include/net-snmp/library/asn1.h
./container_binary_array.lo: ../include/net-snmp/library/snmp_impl.h
./container_binary_array.lo: ../include/net-snmp/library/snmp.h
./container_binary_array.lo: ../include/net-snmp/library/snmp-tc.h
./container_binary_array.lo: ../include/net-snmp/utilities.h
./container_binary_array.lo: ../include/net-snmp/library/snmp_client.h
./container_binary_array.lo: ../include/net-snmp/library/getopt.h
./container_binary_array.lo: ../include/net-snmp/library/system.h
./container_binary_array.lo: ../include/net-snmp/library/tools.h
./container_binary_array.lo: ../include/net-snmp/library/int64.h
./container_binary_array.lo: ../include/net-snmp/library/mt_support.h
./container_binary_array.lo: ../include/net-snmp/library/snmp_alarm.h
./container_binary_array.lo: ../include/net-snmp/library/data_list.h
./container_binary_array.lo: ../include/net-snmp/library/callback.h
./container_binary_array.lo: ../include/net-snmp/library/oid_stash.h
./container_binary_array.lo: ../include/net-snmp/library/check_varbind.h
./container_binary_array.lo: ../include/net-snmp/version.h
./container_binary_array.lo: ../include/net-snmp/session_api.h
./container_binary_array.lo: ../include/net-snmp/library/snmp_transport.h
./container_binary_array.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./container_binary_array.lo: ../include/net-snmp/library/snmpUnixDomain.h
./container_binary_array.lo: ../include/net-snmp/library/snmpUDPDomain.h
./container_binary_array.lo: ../include/net-snmp/library/snmpTCPDomain.h
./container_binary_array.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./container_binary_array.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./container_binary_array.lo: ../include/net-snmp/library/snmpIPXDomain.h
./container_binary_array.lo: ../include/net-snmp/library/ucd_compat.h
./container_binary_array.lo: ../include/net-snmp/pdu_api.h
./container_binary_array.lo: ../include/net-snmp/mib_api.h
./container_binary_array.lo: ../include/net-snmp/library/mib.h
./container_binary_array.lo: ../include/net-snmp/library/parse.h
./container_binary_array.lo: ../include/net-snmp/varbind_api.h
./container_binary_array.lo: ../include/net-snmp/config_api.h
./container_binary_array.lo: ../include/net-snmp/library/read_config.h
./container_binary_array.lo: ../include/net-snmp/library/default_store.h
./container_binary_array.lo: ../include/net-snmp/library/snmp_parse_args.h
./container_binary_array.lo: ../include/net-snmp/library/snmp_enum.h
./container_binary_array.lo: ../include/net-snmp/library/vacm.h
./container_binary_array.lo: ../include/net-snmp/output_api.h
./container_binary_array.lo: ../include/net-snmp/library/snmp_debug.h
./container_binary_array.lo: ../include/net-snmp/library/snmp_logging.h
./container_binary_array.lo: ../include/net-snmp/snmpv3_api.h
./container_binary_array.lo: ../include/net-snmp/library/snmpv3.h
./container_binary_array.lo: ../include/net-snmp/library/transform_oids.h
./container_binary_array.lo: ../include/net-snmp/library/keytools.h
./container_binary_array.lo: ../include/net-snmp/library/scapi.h
./container_binary_array.lo: ../include/net-snmp/library/lcd_time.h
./container_binary_array.lo: ../include/net-snmp/library/snmp_secmod.h
./container_binary_array.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./container_binary_array.lo: ../include/net-snmp/library/snmpusm.h
./container_binary_array.lo: ../include/net-snmp/library/container.h
./container_binary_array.lo: ../include/net-snmp/library/factory.h
./container_binary_array.lo: ../include/net-snmp/library/container_binary_array.h
./container_binary_array.lo: ../include/net-snmp/library/snmp_assert.h
./container.lo: ../include/net-snmp/net-snmp-config.h
./container.lo: ../include/net-snmp/system/linux.h
./container.lo: ../include/net-snmp/system/sysv.h
./container.lo: ../include/net-snmp/system/generic.h
./container.lo: ../include/net-snmp/machine/generic.h
./container.lo: ../include/net-snmp/net-snmp-includes.h 
./container.lo:  ../include/net-snmp/definitions.h
./container.lo: ../include/net-snmp/types.h
./container.lo: ../include/net-snmp/library/snmp_api.h
./container.lo: ../include/net-snmp/library/asn1.h
./container.lo: ../include/net-snmp/library/snmp_impl.h
./container.lo: ../include/net-snmp/library/snmp.h
./container.lo: ../include/net-snmp/library/snmp-tc.h
./container.lo: ../include/net-snmp/utilities.h
./container.lo: ../include/net-snmp/library/snmp_client.h
./container.lo: ../include/net-snmp/library/getopt.h
./container.lo: ../include/net-snmp/library/system.h
./container.lo: ../include/net-snmp/library/tools.h
./container.lo: ../include/net-snmp/library/int64.h
./container.lo: ../include/net-snmp/library/mt_support.h
./container.lo: ../include/net-snmp/library/snmp_alarm.h
./container.lo: ../include/net-snmp/library/data_list.h
./container.lo: ../include/net-snmp/library/callback.h
./container.lo: ../include/net-snmp/library/oid_stash.h
./container.lo: ../include/net-snmp/library/check_varbind.h
./container.lo: ../include/net-snmp/version.h
./container.lo: ../include/net-snmp/session_api.h
./container.lo: ../include/net-snmp/library/snmp_transport.h
./container.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./container.lo: ../include/net-snmp/library/snmpUnixDomain.h
./container.lo: ../include/net-snmp/library/snmpUDPDomain.h
./container.lo: ../include/net-snmp/library/snmpTCPDomain.h
./container.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./container.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./container.lo: ../include/net-snmp/library/snmpIPXDomain.h
./container.lo: ../include/net-snmp/library/ucd_compat.h
./container.lo: ../include/net-snmp/pdu_api.h ../include/net-snmp/mib_api.h
./container.lo: ../include/net-snmp/library/mib.h
./container.lo: ../include/net-snmp/library/parse.h
./container.lo: ../include/net-snmp/varbind_api.h
./container.lo: ../include/net-snmp/config_api.h
./container.lo: ../include/net-snmp/library/read_config.h
./container.lo: ../include/net-snmp/library/default_store.h
./container.lo: ../include/net-snmp/library/snmp_parse_args.h
./container.lo: ../include/net-snmp/library/snmp_enum.h
./container.lo: ../include/net-snmp/library/vacm.h
./container.lo: ../include/net-snmp/output_api.h
./container.lo: ../include/net-snmp/library/snmp_debug.h
./container.lo: ../include/net-snmp/library/snmp_logging.h
./container.lo: ../include/net-snmp/snmpv3_api.h
./container.lo: ../include/net-snmp/library/snmpv3.h
./container.lo: ../include/net-snmp/library/transform_oids.h
./container.lo: ../include/net-snmp/library/keytools.h
./container.lo: ../include/net-snmp/library/scapi.h
./container.lo: ../include/net-snmp/library/lcd_time.h
./container.lo: ../include/net-snmp/library/snmp_secmod.h
./container.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./container.lo: ../include/net-snmp/library/snmpusm.h
./container.lo: ../include/net-snmp/library/container.h
./container.lo: ../include/net-snmp/library/factory.h
./container.lo: ../include/net-snmp/library/container_binary_array.h
./data_list.lo: ../include/net-snmp/net-snmp-config.h
./data_list.lo: ../include/net-snmp/system/linux.h
./data_list.lo: ../include/net-snmp/system/sysv.h
./data_list.lo: ../include/net-snmp/system/generic.h
./data_list.lo: ../include/net-snmp/machine/generic.h
./data_list.lo:  ../include/net-snmp/library/data_list.h
./data_list.lo: ../include/net-snmp/library/snmp_impl.h 
./data_list.lo: ../include/net-snmp/types.h 
./data_list.lo: ../include/net-snmp/definitions.h
./data_list.lo: ../include/net-snmp/library/snmp_api.h
./data_list.lo: ../include/net-snmp/library/asn1.h
./data_list.lo: ../include/net-snmp/library/snmp.h
./data_list.lo: ../include/net-snmp/library/snmp-tc.h
./data_list.lo: ../include/net-snmp/library/tools.h
./default_store.lo: ../include/net-snmp/net-snmp-config.h
./default_store.lo: ../include/net-snmp/system/linux.h
./default_store.lo: ../include/net-snmp/system/sysv.h
./default_store.lo: ../include/net-snmp/system/generic.h
./default_store.lo: ../include/net-snmp/machine/generic.h
./default_store.lo: ../include/net-snmp/types.h 
./default_store.lo: ../include/net-snmp/definitions.h
./default_store.lo: ../include/net-snmp/library/snmp_api.h
./default_store.lo: ../include/net-snmp/library/asn1.h
./default_store.lo: ../include/net-snmp/library/snmp_impl.h
./default_store.lo: ../include/net-snmp/library/snmp.h
./default_store.lo: ../include/net-snmp/library/snmp-tc.h
./default_store.lo: ../include/net-snmp/output_api.h
./default_store.lo: ../include/net-snmp/library/snmp_client.h
./default_store.lo: ../include/net-snmp/library/snmp_debug.h
./default_store.lo: ../include/net-snmp/library/snmp_logging.h
./default_store.lo: ../include/net-snmp/config_api.h
./default_store.lo: ../include/net-snmp/library/read_config.h
./default_store.lo: ../include/net-snmp/library/default_store.h
./default_store.lo: ../include/net-snmp/library/snmp_parse_args.h
./default_store.lo: ../include/net-snmp/library/snmp_enum.h
./default_store.lo: ../include/net-snmp/library/vacm.h
./default_store.lo: ../include/net-snmp/utilities.h
./default_store.lo: ../include/net-snmp/library/getopt.h
./default_store.lo: ../include/net-snmp/library/system.h
./default_store.lo: ../include/net-snmp/library/tools.h
./default_store.lo: ../include/net-snmp/library/int64.h
./default_store.lo: ../include/net-snmp/library/mt_support.h
./default_store.lo: ../include/net-snmp/library/snmp_alarm.h
./default_store.lo: ../include/net-snmp/library/data_list.h
./default_store.lo: ../include/net-snmp/library/callback.h
./default_store.lo: ../include/net-snmp/library/oid_stash.h
./default_store.lo: ../include/net-snmp/library/check_varbind.h
./default_store.lo: ../include/net-snmp/version.h
./int64.lo: ../include/net-snmp/net-snmp-config.h
./int64.lo: ../include/net-snmp/system/linux.h
./int64.lo: ../include/net-snmp/system/sysv.h
./int64.lo: ../include/net-snmp/system/generic.h
./int64.lo: ../include/net-snmp/machine/generic.h 
./int64.lo: ../include/net-snmp/types.h 
./int64.lo: ../include/net-snmp/definitions.h
./int64.lo: ../include/net-snmp/library/snmp_api.h
./int64.lo: ../include/net-snmp/library/asn1.h
./int64.lo: ../include/net-snmp/library/snmp_impl.h
./int64.lo: ../include/net-snmp/library/snmp.h
./int64.lo: ../include/net-snmp/library/snmp-tc.h
./int64.lo: ../include/net-snmp/library/int64.h
./keytools.lo: ../include/net-snmp/net-snmp-config.h
./keytools.lo: ../include/net-snmp/system/linux.h
./keytools.lo: ../include/net-snmp/system/sysv.h
./keytools.lo: ../include/net-snmp/system/generic.h
./keytools.lo: ../include/net-snmp/machine/generic.h 
./keytools.lo: ../include/net-snmp/types.h 
./keytools.lo: ../include/net-snmp/definitions.h
./keytools.lo: ../include/net-snmp/library/snmp_api.h
./keytools.lo: ../include/net-snmp/library/asn1.h
./keytools.lo: ../include/net-snmp/library/snmp_impl.h
./keytools.lo: ../include/net-snmp/library/snmp.h
./keytools.lo: ../include/net-snmp/library/snmp-tc.h
./keytools.lo: ../include/net-snmp/output_api.h
./keytools.lo: ../include/net-snmp/library/snmp_client.h
./keytools.lo: ../include/net-snmp/library/snmp_debug.h
./keytools.lo: ../include/net-snmp/library/snmp_logging.h
./keytools.lo: ../include/net-snmp/utilities.h
./keytools.lo: ../include/net-snmp/library/getopt.h
./keytools.lo: ../include/net-snmp/library/system.h
./keytools.lo: ../include/net-snmp/library/tools.h
./keytools.lo: ../include/net-snmp/library/int64.h
./keytools.lo: ../include/net-snmp/library/mt_support.h
./keytools.lo: ../include/net-snmp/library/snmp_alarm.h
./keytools.lo: ../include/net-snmp/library/data_list.h
./keytools.lo: ../include/net-snmp/library/callback.h
./keytools.lo: ../include/net-snmp/library/oid_stash.h
./keytools.lo: ../include/net-snmp/library/check_varbind.h
./keytools.lo: ../include/net-snmp/version.h
./keytools.lo: ../include/net-snmp/library/scapi.h
./keytools.lo: ../include/net-snmp/library/keytools.h
./keytools.lo: ../include/net-snmp/library/transform_oids.h
./lcd_time.lo: ../include/net-snmp/net-snmp-config.h
./lcd_time.lo: ../include/net-snmp/system/linux.h
./lcd_time.lo: ../include/net-snmp/system/sysv.h
./lcd_time.lo: ../include/net-snmp/system/generic.h
./lcd_time.lo: ../include/net-snmp/machine/generic.h 
./lcd_time.lo: ../include/net-snmp/types.h ../include/net-snmp/definitions.h
./lcd_time.lo: ../include/net-snmp/library/snmp_api.h
./lcd_time.lo: ../include/net-snmp/library/asn1.h
./lcd_time.lo: ../include/net-snmp/library/snmp_impl.h
./lcd_time.lo: ../include/net-snmp/library/snmp.h
./lcd_time.lo: ../include/net-snmp/library/snmp-tc.h
./lcd_time.lo: ../include/net-snmp/output_api.h
./lcd_time.lo: ../include/net-snmp/library/snmp_client.h
./lcd_time.lo: ../include/net-snmp/library/snmp_debug.h
./lcd_time.lo: ../include/net-snmp/library/snmp_logging.h
./lcd_time.lo: ../include/net-snmp/utilities.h
./lcd_time.lo: ../include/net-snmp/library/getopt.h
./lcd_time.lo: ../include/net-snmp/library/system.h
./lcd_time.lo: ../include/net-snmp/library/tools.h
./lcd_time.lo: ../include/net-snmp/library/int64.h
./lcd_time.lo: ../include/net-snmp/library/mt_support.h
./lcd_time.lo: ../include/net-snmp/library/snmp_alarm.h
./lcd_time.lo: ../include/net-snmp/library/data_list.h
./lcd_time.lo: ../include/net-snmp/library/callback.h
./lcd_time.lo: ../include/net-snmp/library/oid_stash.h
./lcd_time.lo: ../include/net-snmp/library/check_varbind.h
./lcd_time.lo: ../include/net-snmp/version.h
./lcd_time.lo: ../include/net-snmp/library/snmp_secmod.h
./lcd_time.lo: ../include/net-snmp/library/snmpusm.h
./lcd_time.lo: ../include/net-snmp/library/lcd_time.h
./lcd_time.lo: ../include/net-snmp/library/scapi.h
./lcd_time.lo: ../include/net-snmp/library/transform_oids.h
./md5.lo: ../include/net-snmp/net-snmp-config.h
./md5.lo: ../include/net-snmp/system/linux.h
./md5.lo: ../include/net-snmp/system/sysv.h
./md5.lo: ../include/net-snmp/system/generic.h
./md5.lo: ../include/net-snmp/machine/generic.h 
./md5.lo:  ../include/net-snmp/utilities.h
./md5.lo: ../include/net-snmp/types.h 
./md5.lo: ../include/net-snmp/definitions.h
./md5.lo: ../include/net-snmp/library/snmp_api.h
./md5.lo: ../include/net-snmp/library/asn1.h
./md5.lo: ../include/net-snmp/library/snmp_impl.h
./md5.lo: ../include/net-snmp/library/snmp.h
./md5.lo: ../include/net-snmp/library/snmp-tc.h
./md5.lo: ../include/net-snmp/library/snmp_client.h
./md5.lo: ../include/net-snmp/library/getopt.h
./md5.lo: ../include/net-snmp/library/system.h
./md5.lo: ../include/net-snmp/library/tools.h
./md5.lo: ../include/net-snmp/library/int64.h
./md5.lo: ../include/net-snmp/library/mt_support.h
./md5.lo: ../include/net-snmp/library/snmp_alarm.h
./md5.lo: ../include/net-snmp/library/data_list.h
./md5.lo: ../include/net-snmp/library/callback.h
./md5.lo: ../include/net-snmp/library/oid_stash.h
./md5.lo: ../include/net-snmp/library/check_varbind.h
./md5.lo: ../include/net-snmp/version.h ../include/net-snmp/library/md5.h
./mib.lo: ../include/net-snmp/net-snmp-config.h
./mib.lo: ../include/net-snmp/system/linux.h
./mib.lo: ../include/net-snmp/system/sysv.h
./mib.lo: ../include/net-snmp/system/generic.h
./mib.lo: ../include/net-snmp/machine/generic.h 
./mib.lo:  ../include/net-snmp/types.h
./mib.lo: ../include/net-snmp/definitions.h
./mib.lo: ../include/net-snmp/library/snmp_api.h
./mib.lo: ../include/net-snmp/library/asn1.h
./mib.lo: ../include/net-snmp/library/snmp_impl.h
./mib.lo: ../include/net-snmp/library/snmp.h
./mib.lo: ../include/net-snmp/library/snmp-tc.h
./mib.lo: ../include/net-snmp/output_api.h
./mib.lo: ../include/net-snmp/library/snmp_client.h
./mib.lo: ../include/net-snmp/library/snmp_debug.h
./mib.lo: ../include/net-snmp/library/snmp_logging.h 
./mib.lo:  ../include/net-snmp/config_api.h
./mib.lo: ../include/net-snmp/library/read_config.h
./mib.lo: ../include/net-snmp/library/default_store.h
./mib.lo: ../include/net-snmp/library/snmp_parse_args.h
./mib.lo: ../include/net-snmp/library/snmp_enum.h
./mib.lo: ../include/net-snmp/library/vacm.h ../include/net-snmp/utilities.h
./mib.lo: ../include/net-snmp/library/getopt.h
./mib.lo: ../include/net-snmp/library/system.h
./mib.lo: ../include/net-snmp/library/tools.h
./mib.lo: ../include/net-snmp/library/int64.h
./mib.lo: ../include/net-snmp/library/mt_support.h
./mib.lo: ../include/net-snmp/library/snmp_alarm.h
./mib.lo: ../include/net-snmp/library/data_list.h
./mib.lo: ../include/net-snmp/library/callback.h
./mib.lo: ../include/net-snmp/library/oid_stash.h
./mib.lo: ../include/net-snmp/library/check_varbind.h
./mib.lo: ../include/net-snmp/version.h ../include/net-snmp/library/mib.h
./mib.lo: ../include/net-snmp/library/parse.h
./mt_support.lo: ../include/net-snmp/net-snmp-config.h
./mt_support.lo: ../include/net-snmp/system/linux.h
./mt_support.lo: ../include/net-snmp/system/sysv.h
./mt_support.lo: ../include/net-snmp/system/generic.h
./mt_support.lo: ../include/net-snmp/machine/generic.h 
./mt_support.lo: ../include/net-snmp/library/mt_support.h
./oid_stash.lo: ../include/net-snmp/net-snmp-config.h
./oid_stash.lo: ../include/net-snmp/system/linux.h
./oid_stash.lo: ../include/net-snmp/system/sysv.h
./oid_stash.lo: ../include/net-snmp/system/generic.h
./oid_stash.lo: ../include/net-snmp/machine/generic.h 
./oid_stash.lo:  ../include/net-snmp/net-snmp-includes.h
./oid_stash.lo:  ../include/net-snmp/definitions.h
./oid_stash.lo: ../include/net-snmp/types.h
./oid_stash.lo: ../include/net-snmp/library/snmp_api.h
./oid_stash.lo: ../include/net-snmp/library/asn1.h
./oid_stash.lo: ../include/net-snmp/library/snmp_impl.h
./oid_stash.lo: ../include/net-snmp/library/snmp.h
./oid_stash.lo: ../include/net-snmp/library/snmp-tc.h
./oid_stash.lo: ../include/net-snmp/utilities.h
./oid_stash.lo: ../include/net-snmp/library/snmp_client.h
./oid_stash.lo: ../include/net-snmp/library/getopt.h
./oid_stash.lo: ../include/net-snmp/library/system.h
./oid_stash.lo: ../include/net-snmp/library/tools.h
./oid_stash.lo: ../include/net-snmp/library/int64.h
./oid_stash.lo: ../include/net-snmp/library/mt_support.h
./oid_stash.lo: ../include/net-snmp/library/snmp_alarm.h
./oid_stash.lo: ../include/net-snmp/library/data_list.h
./oid_stash.lo: ../include/net-snmp/library/callback.h
./oid_stash.lo: ../include/net-snmp/library/oid_stash.h
./oid_stash.lo: ../include/net-snmp/library/check_varbind.h
./oid_stash.lo: ../include/net-snmp/version.h
./oid_stash.lo: ../include/net-snmp/session_api.h
./oid_stash.lo: ../include/net-snmp/library/snmp_transport.h
./oid_stash.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./oid_stash.lo: ../include/net-snmp/library/snmpUnixDomain.h
./oid_stash.lo: ../include/net-snmp/library/snmpUDPDomain.h
./oid_stash.lo: ../include/net-snmp/library/snmpTCPDomain.h
./oid_stash.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./oid_stash.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./oid_stash.lo: ../include/net-snmp/library/snmpIPXDomain.h
./oid_stash.lo: ../include/net-snmp/library/ucd_compat.h
./oid_stash.lo: ../include/net-snmp/pdu_api.h ../include/net-snmp/mib_api.h
./oid_stash.lo: ../include/net-snmp/library/mib.h
./oid_stash.lo: ../include/net-snmp/library/parse.h
./oid_stash.lo: ../include/net-snmp/varbind_api.h
./oid_stash.lo: ../include/net-snmp/config_api.h
./oid_stash.lo: ../include/net-snmp/library/read_config.h
./oid_stash.lo: ../include/net-snmp/library/default_store.h
./oid_stash.lo: ../include/net-snmp/library/snmp_parse_args.h
./oid_stash.lo: ../include/net-snmp/library/snmp_enum.h
./oid_stash.lo: ../include/net-snmp/library/vacm.h
./oid_stash.lo: ../include/net-snmp/output_api.h
./oid_stash.lo: ../include/net-snmp/library/snmp_debug.h
./oid_stash.lo: ../include/net-snmp/library/snmp_logging.h
./oid_stash.lo: ../include/net-snmp/snmpv3_api.h
./oid_stash.lo: ../include/net-snmp/library/snmpv3.h
./oid_stash.lo: ../include/net-snmp/library/transform_oids.h
./oid_stash.lo: ../include/net-snmp/library/keytools.h
./oid_stash.lo: ../include/net-snmp/library/scapi.h
./oid_stash.lo: ../include/net-snmp/library/lcd_time.h
./oid_stash.lo: ../include/net-snmp/library/snmp_secmod.h
./oid_stash.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./oid_stash.lo: ../include/net-snmp/library/snmpusm.h
./parse.lo: ../include/net-snmp/net-snmp-config.h
./parse.lo: ../include/net-snmp/system/linux.h
./parse.lo: ../include/net-snmp/system/sysv.h
./parse.lo: ../include/net-snmp/system/generic.h
./parse.lo: ../include/net-snmp/machine/generic.h 
./parse.lo: ../include/net-snmp/types.h ../include/net-snmp/definitions.h
./parse.lo: ../include/net-snmp/library/snmp_api.h
./parse.lo: ../include/net-snmp/library/asn1.h
./parse.lo: ../include/net-snmp/library/snmp_impl.h
./parse.lo: ../include/net-snmp/library/snmp.h
./parse.lo: ../include/net-snmp/library/snmp-tc.h
./parse.lo: ../include/net-snmp/output_api.h
./parse.lo: ../include/net-snmp/library/snmp_client.h
./parse.lo: ../include/net-snmp/library/snmp_debug.h
./parse.lo: ../include/net-snmp/library/snmp_logging.h 
./parse.lo:  ../include/net-snmp/config_api.h
./parse.lo: ../include/net-snmp/library/read_config.h
./parse.lo: ../include/net-snmp/library/default_store.h
./parse.lo: ../include/net-snmp/library/snmp_parse_args.h
./parse.lo: ../include/net-snmp/library/snmp_enum.h
./parse.lo: ../include/net-snmp/library/vacm.h
./parse.lo: ../include/net-snmp/utilities.h
./parse.lo: ../include/net-snmp/library/getopt.h
./parse.lo: ../include/net-snmp/library/system.h
./parse.lo: ../include/net-snmp/library/tools.h
./parse.lo: ../include/net-snmp/library/int64.h
./parse.lo: ../include/net-snmp/library/mt_support.h
./parse.lo: ../include/net-snmp/library/snmp_alarm.h
./parse.lo: ../include/net-snmp/library/data_list.h
./parse.lo: ../include/net-snmp/library/callback.h
./parse.lo: ../include/net-snmp/library/oid_stash.h
./parse.lo: ../include/net-snmp/library/check_varbind.h
./parse.lo: ../include/net-snmp/version.h ../include/net-snmp/library/parse.h
./parse.lo: ../include/net-snmp/library/mib.h
./read_config.lo: ../include/net-snmp/net-snmp-config.h
./read_config.lo: ../include/net-snmp/system/linux.h
./read_config.lo: ../include/net-snmp/system/sysv.h
./read_config.lo: ../include/net-snmp/system/generic.h
./read_config.lo: ../include/net-snmp/machine/generic.h 
./read_config.lo: ../include/net-snmp/types.h
./read_config.lo: ../include/net-snmp/definitions.h
./read_config.lo: ../include/net-snmp/library/snmp_api.h
./read_config.lo: ../include/net-snmp/library/asn1.h
./read_config.lo: ../include/net-snmp/library/snmp_impl.h
./read_config.lo: ../include/net-snmp/library/snmp.h
./read_config.lo: ../include/net-snmp/library/snmp-tc.h
./read_config.lo: ../include/net-snmp/output_api.h
./read_config.lo: ../include/net-snmp/library/snmp_client.h
./read_config.lo: ../include/net-snmp/library/snmp_debug.h
./read_config.lo: ../include/net-snmp/library/snmp_logging.h
./read_config.lo: ../include/net-snmp/config_api.h
./read_config.lo: ../include/net-snmp/library/read_config.h
./read_config.lo: ../include/net-snmp/library/default_store.h
./read_config.lo: ../include/net-snmp/library/snmp_parse_args.h
./read_config.lo: ../include/net-snmp/library/snmp_enum.h
./read_config.lo: ../include/net-snmp/library/vacm.h
./read_config.lo: ../include/net-snmp/utilities.h
./read_config.lo: ../include/net-snmp/library/getopt.h
./read_config.lo: ../include/net-snmp/library/system.h
./read_config.lo: ../include/net-snmp/library/tools.h
./read_config.lo: ../include/net-snmp/library/int64.h
./read_config.lo: ../include/net-snmp/library/mt_support.h
./read_config.lo: ../include/net-snmp/library/snmp_alarm.h
./read_config.lo: ../include/net-snmp/library/data_list.h
./read_config.lo: ../include/net-snmp/library/callback.h
./read_config.lo: ../include/net-snmp/library/oid_stash.h
./read_config.lo: ../include/net-snmp/library/check_varbind.h
./read_config.lo: ../include/net-snmp/version.h
./read_config.lo: ../include/net-snmp/library/mib.h
./read_config.lo: ../include/net-snmp/library/parse.h
./scapi.lo: ../include/net-snmp/net-snmp-config.h
./scapi.lo: ../include/net-snmp/system/linux.h
./scapi.lo: ../include/net-snmp/system/sysv.h
./scapi.lo: ../include/net-snmp/system/generic.h
./scapi.lo: ../include/net-snmp/machine/generic.h 
./scapi.lo:  ../include/net-snmp/types.h
./scapi.lo:  ../include/net-snmp/definitions.h
./scapi.lo: ../include/net-snmp/library/snmp_api.h
./scapi.lo: ../include/net-snmp/library/asn1.h
./scapi.lo: ../include/net-snmp/library/snmp_impl.h
./scapi.lo: ../include/net-snmp/library/snmp.h
./scapi.lo: ../include/net-snmp/library/snmp-tc.h
./scapi.lo: ../include/net-snmp/output_api.h
./scapi.lo: ../include/net-snmp/library/snmp_client.h
./scapi.lo: ../include/net-snmp/library/snmp_debug.h
./scapi.lo: ../include/net-snmp/library/snmp_logging.h 
./scapi.lo:  ../include/net-snmp/utilities.h
./scapi.lo: ../include/net-snmp/library/getopt.h
./scapi.lo: ../include/net-snmp/library/system.h
./scapi.lo: ../include/net-snmp/library/tools.h
./scapi.lo: ../include/net-snmp/library/int64.h
./scapi.lo: ../include/net-snmp/library/mt_support.h
./scapi.lo: ../include/net-snmp/library/snmp_alarm.h
./scapi.lo: ../include/net-snmp/library/data_list.h
./scapi.lo: ../include/net-snmp/library/callback.h
./scapi.lo: ../include/net-snmp/library/oid_stash.h
./scapi.lo: ../include/net-snmp/library/check_varbind.h
./scapi.lo: ../include/net-snmp/version.h
./scapi.lo: ../include/net-snmp/library/snmp_secmod.h
./scapi.lo: ../include/net-snmp/library/snmpusm.h
./scapi.lo: ../include/net-snmp/library/keytools.h
./scapi.lo: ../include/net-snmp/library/scapi.h
./scapi.lo: ../include/net-snmp/library/mib.h
./scapi.lo: ../include/net-snmp/library/transform_oids.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/net-snmp-config.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/system/linux.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/system/sysv.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/system/generic.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/machine/generic.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/types.h 
./snmpAAL5PVCDomain.lo: ../include/net-snmp/definitions.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/library/snmp_api.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/library/asn1.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/library/snmp_impl.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/library/snmp.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/library/snmp-tc.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/output_api.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/library/snmp_client.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/library/snmp_debug.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/library/snmp_logging.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/config_api.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/library/read_config.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/library/default_store.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/library/snmp_enum.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/library/vacm.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/library/snmp_transport.h
./snmpAAL5PVCDomain.lo: ../include/net-snmp/library/snmpAAL5PVCDomain.h
./snmp_alarm.lo: ../include/net-snmp/net-snmp-config.h
./snmp_alarm.lo: ../include/net-snmp/system/linux.h
./snmp_alarm.lo: ../include/net-snmp/system/sysv.h
./snmp_alarm.lo: ../include/net-snmp/system/generic.h
./snmp_alarm.lo: ../include/net-snmp/machine/generic.h 
./snmp_alarm.lo: ../include/net-snmp/types.h 
./snmp_alarm.lo: ../include/net-snmp/definitions.h
./snmp_alarm.lo: ../include/net-snmp/library/snmp_api.h
./snmp_alarm.lo: ../include/net-snmp/library/asn1.h
./snmp_alarm.lo: ../include/net-snmp/library/snmp_impl.h
./snmp_alarm.lo: ../include/net-snmp/library/snmp.h
./snmp_alarm.lo: ../include/net-snmp/library/snmp-tc.h
./snmp_alarm.lo: ../include/net-snmp/output_api.h
./snmp_alarm.lo: ../include/net-snmp/library/snmp_client.h
./snmp_alarm.lo: ../include/net-snmp/library/snmp_debug.h
./snmp_alarm.lo: ../include/net-snmp/library/snmp_logging.h
./snmp_alarm.lo: ../include/net-snmp/config_api.h
./snmp_alarm.lo: ../include/net-snmp/library/read_config.h
./snmp_alarm.lo: ../include/net-snmp/library/default_store.h
./snmp_alarm.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmp_alarm.lo: ../include/net-snmp/library/snmp_enum.h
./snmp_alarm.lo: ../include/net-snmp/library/vacm.h
./snmp_alarm.lo: ../include/net-snmp/utilities.h
./snmp_alarm.lo: ../include/net-snmp/library/getopt.h
./snmp_alarm.lo: ../include/net-snmp/library/system.h
./snmp_alarm.lo: ../include/net-snmp/library/tools.h
./snmp_alarm.lo: ../include/net-snmp/library/int64.h
./snmp_alarm.lo: ../include/net-snmp/library/mt_support.h
./snmp_alarm.lo: ../include/net-snmp/library/snmp_alarm.h
./snmp_alarm.lo: ../include/net-snmp/library/data_list.h
./snmp_alarm.lo: ../include/net-snmp/library/callback.h
./snmp_alarm.lo: ../include/net-snmp/library/oid_stash.h
./snmp_alarm.lo: ../include/net-snmp/library/check_varbind.h
./snmp_alarm.lo: ../include/net-snmp/version.h
./snmp_api.lo: ../include/net-snmp/net-snmp-config.h
./snmp_api.lo: ../include/net-snmp/system/linux.h
./snmp_api.lo: ../include/net-snmp/system/sysv.h
./snmp_api.lo: ../include/net-snmp/system/generic.h
./snmp_api.lo: ../include/net-snmp/machine/generic.h 
./snmp_api.lo:  ../include/net-snmp/types.h
./snmp_api.lo: ../include/net-snmp/definitions.h
./snmp_api.lo: ../include/net-snmp/library/snmp_api.h
./snmp_api.lo: ../include/net-snmp/library/asn1.h
./snmp_api.lo: ../include/net-snmp/library/snmp_impl.h
./snmp_api.lo: ../include/net-snmp/library/snmp.h
./snmp_api.lo: ../include/net-snmp/library/snmp-tc.h
./snmp_api.lo: ../include/net-snmp/output_api.h
./snmp_api.lo: ../include/net-snmp/library/snmp_client.h
./snmp_api.lo: ../include/net-snmp/library/snmp_debug.h
./snmp_api.lo: ../include/net-snmp/library/snmp_logging.h
./snmp_api.lo: ../include/net-snmp/config_api.h
./snmp_api.lo: ../include/net-snmp/library/read_config.h
./snmp_api.lo: ../include/net-snmp/library/default_store.h
./snmp_api.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmp_api.lo: ../include/net-snmp/library/snmp_enum.h
./snmp_api.lo: ../include/net-snmp/library/vacm.h
./snmp_api.lo: ../include/net-snmp/utilities.h
./snmp_api.lo: ../include/net-snmp/library/getopt.h
./snmp_api.lo: ../include/net-snmp/library/system.h
./snmp_api.lo: ../include/net-snmp/library/tools.h
./snmp_api.lo: ../include/net-snmp/library/int64.h
./snmp_api.lo: ../include/net-snmp/library/mt_support.h
./snmp_api.lo: ../include/net-snmp/library/snmp_alarm.h
./snmp_api.lo: ../include/net-snmp/library/data_list.h
./snmp_api.lo: ../include/net-snmp/library/callback.h
./snmp_api.lo: ../include/net-snmp/library/oid_stash.h
./snmp_api.lo: ../include/net-snmp/library/check_varbind.h
./snmp_api.lo: ../include/net-snmp/version.h
./snmp_api.lo: ../include/net-snmp/library/parse.h
./snmp_api.lo: ../include/net-snmp/library/mib.h
./snmp_api.lo: ../include/net-snmp/library/snmpv3.h
./snmp_api.lo: ../include/net-snmp/library/container.h
./snmp_api.lo: ../include/net-snmp/library/factory.h
./snmp_api.lo: ../include/net-snmp/library/snmp_secmod.h
./snmp_api.lo: ../include/net-snmp/library/snmpusm.h
./snmp_api.lo: ../include/net-snmp/library/keytools.h
./snmp_api.lo: ../include/net-snmp/library/lcd_time.h
./snmp_api.lo: ../include/net-snmp/library/snmp_transport.h
./snmp_api.lo: ../include/net-snmp/library/transform_oids.h
./snmp_auth.lo: ../include/net-snmp/net-snmp-config.h
./snmp_auth.lo: ../include/net-snmp/system/linux.h
./snmp_auth.lo: ../include/net-snmp/system/sysv.h
./snmp_auth.lo: ../include/net-snmp/system/generic.h
./snmp_auth.lo: ../include/net-snmp/machine/generic.h 
./snmp_auth.lo:  ../include/net-snmp/types.h
./snmp_auth.lo: ../include/net-snmp/definitions.h
./snmp_auth.lo: ../include/net-snmp/library/snmp_api.h
./snmp_auth.lo: ../include/net-snmp/library/asn1.h
./snmp_auth.lo: ../include/net-snmp/library/snmp_impl.h
./snmp_auth.lo: ../include/net-snmp/library/snmp.h
./snmp_auth.lo: ../include/net-snmp/library/snmp-tc.h
./snmp_auth.lo: ../include/net-snmp/output_api.h
./snmp_auth.lo: ../include/net-snmp/library/snmp_client.h
./snmp_auth.lo: ../include/net-snmp/library/snmp_debug.h
./snmp_auth.lo: ../include/net-snmp/library/snmp_logging.h
./snmp_auth.lo: ../include/net-snmp/utilities.h
./snmp_auth.lo: ../include/net-snmp/library/getopt.h
./snmp_auth.lo: ../include/net-snmp/library/system.h
./snmp_auth.lo: ../include/net-snmp/library/tools.h
./snmp_auth.lo: ../include/net-snmp/library/int64.h
./snmp_auth.lo: ../include/net-snmp/library/mt_support.h
./snmp_auth.lo: ../include/net-snmp/library/snmp_alarm.h
./snmp_auth.lo: ../include/net-snmp/library/data_list.h
./snmp_auth.lo: ../include/net-snmp/library/callback.h
./snmp_auth.lo: ../include/net-snmp/library/oid_stash.h
./snmp_auth.lo: ../include/net-snmp/library/check_varbind.h
./snmp_auth.lo: ../include/net-snmp/version.h
./snmp_auth.lo: ../include/net-snmp/library/mib.h
./snmp_auth.lo: ../include/net-snmp/library/md5.h
./snmp_auth.lo: ../include/net-snmp/library/scapi.h
./snmp.lo: ../include/net-snmp/net-snmp-config.h
./snmp.lo: ../include/net-snmp/system/linux.h
./snmp.lo: ../include/net-snmp/system/sysv.h
./snmp.lo: ../include/net-snmp/system/generic.h
./snmp.lo: ../include/net-snmp/machine/generic.h 
./snmp.lo: ../include/net-snmp/types.h 
./snmp.lo: ../include/net-snmp/definitions.h
./snmp.lo: ../include/net-snmp/library/snmp_api.h
./snmp.lo: ../include/net-snmp/library/asn1.h
./snmp.lo: ../include/net-snmp/library/snmp_impl.h
./snmp.lo: ../include/net-snmp/library/snmp.h
./snmp.lo: ../include/net-snmp/library/snmp-tc.h
./snmp.lo: ../include/net-snmp/output_api.h
./snmp.lo: ../include/net-snmp/library/snmp_client.h
./snmp.lo: ../include/net-snmp/library/snmp_debug.h
./snmp.lo: ../include/net-snmp/library/snmp_logging.h 
./snmp.lo:  ../include/net-snmp/library/mib.h
./snmpCallbackDomain.lo: ../include/net-snmp/net-snmp-config.h
./snmpCallbackDomain.lo: ../include/net-snmp/system/linux.h
./snmpCallbackDomain.lo: ../include/net-snmp/system/sysv.h
./snmpCallbackDomain.lo: ../include/net-snmp/system/generic.h
./snmpCallbackDomain.lo: ../include/net-snmp/machine/generic.h
./snmpCallbackDomain.lo:  ../include/net-snmp/types.h
./snmpCallbackDomain.lo: ../include/net-snmp/definitions.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/snmp_api.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/asn1.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/snmp_impl.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/snmp.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/snmp-tc.h
./snmpCallbackDomain.lo: ../include/net-snmp/output_api.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/snmp_client.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/snmp_debug.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/snmp_logging.h
./snmpCallbackDomain.lo: ../include/net-snmp/config_api.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/read_config.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/default_store.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/snmp_enum.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/vacm.h
./snmpCallbackDomain.lo: ../include/net-snmp/utilities.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/getopt.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/system.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/tools.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/int64.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/mt_support.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/snmp_alarm.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/data_list.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/callback.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/oid_stash.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/check_varbind.h
./snmpCallbackDomain.lo: ../include/net-snmp/version.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/snmp_transport.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmpCallbackDomain.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmp_client.lo: ../include/net-snmp/net-snmp-config.h
./snmp_client.lo: ../include/net-snmp/system/linux.h
./snmp_client.lo: ../include/net-snmp/system/sysv.h
./snmp_client.lo: ../include/net-snmp/system/generic.h
./snmp_client.lo: ../include/net-snmp/machine/generic.h 
./snmp_client.lo: ../include/net-snmp/types.h
./snmp_client.lo: ../include/net-snmp/definitions.h
./snmp_client.lo: ../include/net-snmp/library/snmp_api.h
./snmp_client.lo: ../include/net-snmp/library/asn1.h
./snmp_client.lo: ../include/net-snmp/library/snmp_impl.h
./snmp_client.lo: ../include/net-snmp/library/snmp.h
./snmp_client.lo: ../include/net-snmp/library/snmp-tc.h
./snmp_client.lo: ../include/net-snmp/library/snmp_client.h
./snmp_client.lo: ../include/net-snmp/library/snmp_secmod.h
./snmp_client.lo: ../include/net-snmp/library/mib.h
./snmp_debug.lo: ../include/net-snmp/net-snmp-config.h
./snmp_debug.lo: ../include/net-snmp/system/linux.h
./snmp_debug.lo: ../include/net-snmp/system/sysv.h
./snmp_debug.lo: ../include/net-snmp/system/generic.h
./snmp_debug.lo: ../include/net-snmp/machine/generic.h 
./snmp_debug.lo: ../include/net-snmp/types.h 
./snmp_debug.lo: ../include/net-snmp/definitions.h
./snmp_debug.lo: ../include/net-snmp/library/snmp_api.h
./snmp_debug.lo: ../include/net-snmp/library/asn1.h
./snmp_debug.lo: ../include/net-snmp/library/snmp_impl.h
./snmp_debug.lo: ../include/net-snmp/library/snmp.h
./snmp_debug.lo: ../include/net-snmp/library/snmp-tc.h
./snmp_debug.lo: ../include/net-snmp/output_api.h
./snmp_debug.lo: ../include/net-snmp/library/snmp_client.h
./snmp_debug.lo: ../include/net-snmp/library/snmp_debug.h
./snmp_debug.lo: ../include/net-snmp/library/snmp_logging.h
./snmp_debug.lo: ../include/net-snmp/config_api.h
./snmp_debug.lo: ../include/net-snmp/library/read_config.h
./snmp_debug.lo: ../include/net-snmp/library/default_store.h
./snmp_debug.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmp_debug.lo: ../include/net-snmp/library/snmp_enum.h
./snmp_debug.lo: ../include/net-snmp/library/vacm.h
./snmp_debug.lo: ../include/net-snmp/utilities.h
./snmp_debug.lo: ../include/net-snmp/library/getopt.h
./snmp_debug.lo: ../include/net-snmp/library/system.h
./snmp_debug.lo: ../include/net-snmp/library/tools.h
./snmp_debug.lo: ../include/net-snmp/library/int64.h
./snmp_debug.lo: ../include/net-snmp/library/mt_support.h
./snmp_debug.lo: ../include/net-snmp/library/snmp_alarm.h
./snmp_debug.lo: ../include/net-snmp/library/data_list.h
./snmp_debug.lo: ../include/net-snmp/library/callback.h
./snmp_debug.lo: ../include/net-snmp/library/oid_stash.h
./snmp_debug.lo: ../include/net-snmp/library/check_varbind.h
./snmp_debug.lo: ../include/net-snmp/version.h
./snmp_debug.lo: ../include/net-snmp/library/mib.h
./snmp_enum.lo: ../include/net-snmp/net-snmp-config.h
./snmp_enum.lo: ../include/net-snmp/system/linux.h
./snmp_enum.lo: ../include/net-snmp/system/sysv.h
./snmp_enum.lo: ../include/net-snmp/system/generic.h
./snmp_enum.lo: ../include/net-snmp/machine/generic.h 
./snmp_enum.lo:  ../include/net-snmp/library/snmp_enum.h
./snmpIPXDomain.lo: ../include/net-snmp/net-snmp-config.h
./snmpIPXDomain.lo: ../include/net-snmp/system/linux.h
./snmpIPXDomain.lo: ../include/net-snmp/system/sysv.h
./snmpIPXDomain.lo: ../include/net-snmp/system/generic.h
./snmpIPXDomain.lo: ../include/net-snmp/machine/generic.h
./snmpIPXDomain.lo:  ../include/net-snmp/types.h
./snmpIPXDomain.lo:  ../include/net-snmp/definitions.h
./snmpIPXDomain.lo: ../include/net-snmp/library/snmp_api.h
./snmpIPXDomain.lo: ../include/net-snmp/library/asn1.h
./snmpIPXDomain.lo: ../include/net-snmp/library/snmp_impl.h
./snmpIPXDomain.lo: ../include/net-snmp/library/snmp.h
./snmpIPXDomain.lo: ../include/net-snmp/library/snmp-tc.h
./snmpIPXDomain.lo: ../include/net-snmp/output_api.h
./snmpIPXDomain.lo: ../include/net-snmp/library/snmp_client.h
./snmpIPXDomain.lo: ../include/net-snmp/library/snmp_debug.h
./snmpIPXDomain.lo: ../include/net-snmp/library/snmp_logging.h
./snmpIPXDomain.lo: ../include/net-snmp/config_api.h
./snmpIPXDomain.lo: ../include/net-snmp/library/read_config.h
./snmpIPXDomain.lo: ../include/net-snmp/library/default_store.h
./snmpIPXDomain.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpIPXDomain.lo: ../include/net-snmp/library/snmp_enum.h
./snmpIPXDomain.lo: ../include/net-snmp/library/vacm.h
./snmpIPXDomain.lo: ../include/net-snmp/library/snmp_transport.h
./snmpIPXDomain.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmpksm.lo: ../include/net-snmp/net-snmp-config.h
./snmpksm.lo: ../include/net-snmp/system/linux.h
./snmpksm.lo: ../include/net-snmp/system/sysv.h
./snmpksm.lo: ../include/net-snmp/system/generic.h
./snmpksm.lo: ../include/net-snmp/machine/generic.h 
./snmpksm.lo: ../include/net-snmp/output_api.h ../include/net-snmp/types.h
./snmpksm.lo: ../include/net-snmp/definitions.h
./snmpksm.lo: ../include/net-snmp/library/snmp_api.h
./snmpksm.lo: ../include/net-snmp/library/asn1.h
./snmpksm.lo: ../include/net-snmp/library/snmp_impl.h
./snmpksm.lo: ../include/net-snmp/library/snmp.h
./snmpksm.lo: ../include/net-snmp/library/snmp-tc.h
./snmpksm.lo: ../include/net-snmp/library/snmp_client.h
./snmpksm.lo: ../include/net-snmp/library/snmp_debug.h
./snmpksm.lo: ../include/net-snmp/library/snmp_logging.h
./snmpksm.lo: ../include/net-snmp/config_api.h
./snmpksm.lo: ../include/net-snmp/library/read_config.h
./snmpksm.lo: ../include/net-snmp/library/default_store.h
./snmpksm.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpksm.lo: ../include/net-snmp/library/snmp_enum.h
./snmpksm.lo: ../include/net-snmp/library/vacm.h
./snmpksm.lo: ../include/net-snmp/utilities.h
./snmpksm.lo: ../include/net-snmp/library/getopt.h
./snmpksm.lo: ../include/net-snmp/library/system.h
./snmpksm.lo: ../include/net-snmp/library/tools.h
./snmpksm.lo: ../include/net-snmp/library/int64.h
./snmpksm.lo: ../include/net-snmp/library/mt_support.h
./snmpksm.lo: ../include/net-snmp/library/snmp_alarm.h
./snmpksm.lo: ../include/net-snmp/library/data_list.h
./snmpksm.lo: ../include/net-snmp/library/callback.h
./snmpksm.lo: ../include/net-snmp/library/oid_stash.h
./snmpksm.lo: ../include/net-snmp/library/check_varbind.h
./snmpksm.lo: ../include/net-snmp/version.h
./snmpksm.lo: ../include/net-snmp/library/keytools.h
./snmpksm.lo: ../include/net-snmp/library/snmpv3.h
./snmpksm.lo: ../include/net-snmp/library/lcd_time.h
./snmpksm.lo: ../include/net-snmp/library/scapi.h
./snmpksm.lo: ../include/net-snmp/library/snmp_secmod.h
./snmpksm.lo: ../include/net-snmp/library/snmpksm.h
./snmp_logging.lo: ../include/net-snmp/net-snmp-config.h
./snmp_logging.lo: ../include/net-snmp/system/linux.h
./snmp_logging.lo: ../include/net-snmp/system/sysv.h
./snmp_logging.lo: ../include/net-snmp/system/generic.h
./snmp_logging.lo: ../include/net-snmp/machine/generic.h 
./snmp_logging.lo: ../include/net-snmp/types.h
./snmp_logging.lo: ../include/net-snmp/definitions.h
./snmp_logging.lo: ../include/net-snmp/library/snmp_api.h
./snmp_logging.lo: ../include/net-snmp/library/asn1.h
./snmp_logging.lo: ../include/net-snmp/library/snmp_impl.h
./snmp_logging.lo: ../include/net-snmp/library/snmp.h
./snmp_logging.lo: ../include/net-snmp/library/snmp-tc.h
./snmp_logging.lo: ../include/net-snmp/output_api.h
./snmp_logging.lo: ../include/net-snmp/library/snmp_client.h
./snmp_logging.lo: ../include/net-snmp/library/snmp_debug.h
./snmp_logging.lo: ../include/net-snmp/library/snmp_logging.h
./snmp_logging.lo: ../include/net-snmp/config_api.h
./snmp_logging.lo: ../include/net-snmp/library/read_config.h
./snmp_logging.lo: ../include/net-snmp/library/default_store.h
./snmp_logging.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmp_logging.lo: ../include/net-snmp/library/snmp_enum.h
./snmp_logging.lo: ../include/net-snmp/library/vacm.h
./snmp_logging.lo: ../include/net-snmp/utilities.h
./snmp_logging.lo: ../include/net-snmp/library/getopt.h
./snmp_logging.lo: ../include/net-snmp/library/system.h
./snmp_logging.lo: ../include/net-snmp/library/tools.h
./snmp_logging.lo: ../include/net-snmp/library/int64.h
./snmp_logging.lo: ../include/net-snmp/library/mt_support.h
./snmp_logging.lo: ../include/net-snmp/library/snmp_alarm.h
./snmp_logging.lo: ../include/net-snmp/library/data_list.h
./snmp_logging.lo: ../include/net-snmp/library/callback.h
./snmp_logging.lo: ../include/net-snmp/library/oid_stash.h
./snmp_logging.lo: ../include/net-snmp/library/check_varbind.h
./snmp_logging.lo: ../include/net-snmp/version.h
./snmp_parse_args.lo: ../include/net-snmp/net-snmp-config.h
./snmp_parse_args.lo: ../include/net-snmp/system/linux.h
./snmp_parse_args.lo: ../include/net-snmp/system/sysv.h
./snmp_parse_args.lo: ../include/net-snmp/system/generic.h
./snmp_parse_args.lo: ../include/net-snmp/machine/generic.h
./snmp_parse_args.lo:  ../include/net-snmp/types.h
./snmp_parse_args.lo: ../include/net-snmp/definitions.h
./snmp_parse_args.lo: ../include/net-snmp/library/snmp_api.h
./snmp_parse_args.lo: ../include/net-snmp/library/asn1.h
./snmp_parse_args.lo: ../include/net-snmp/library/snmp_impl.h
./snmp_parse_args.lo: ../include/net-snmp/library/snmp.h
./snmp_parse_args.lo: ../include/net-snmp/library/snmp-tc.h
./snmp_parse_args.lo: ../include/net-snmp/output_api.h
./snmp_parse_args.lo: ../include/net-snmp/library/snmp_client.h
./snmp_parse_args.lo: ../include/net-snmp/library/snmp_debug.h
./snmp_parse_args.lo: ../include/net-snmp/library/snmp_logging.h
./snmp_parse_args.lo: ../include/net-snmp/config_api.h
./snmp_parse_args.lo: ../include/net-snmp/library/read_config.h
./snmp_parse_args.lo: ../include/net-snmp/library/default_store.h
./snmp_parse_args.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmp_parse_args.lo: ../include/net-snmp/library/snmp_enum.h
./snmp_parse_args.lo: ../include/net-snmp/library/vacm.h
./snmp_parse_args.lo: ../include/net-snmp/utilities.h
./snmp_parse_args.lo: ../include/net-snmp/library/getopt.h
./snmp_parse_args.lo: ../include/net-snmp/library/system.h
./snmp_parse_args.lo: ../include/net-snmp/library/tools.h
./snmp_parse_args.lo: ../include/net-snmp/library/int64.h
./snmp_parse_args.lo: ../include/net-snmp/library/mt_support.h
./snmp_parse_args.lo: ../include/net-snmp/library/snmp_alarm.h
./snmp_parse_args.lo: ../include/net-snmp/library/data_list.h
./snmp_parse_args.lo: ../include/net-snmp/library/callback.h
./snmp_parse_args.lo: ../include/net-snmp/library/oid_stash.h
./snmp_parse_args.lo: ../include/net-snmp/library/check_varbind.h
./snmp_parse_args.lo: ../include/net-snmp/version.h
./snmp_parse_args.lo: ../include/net-snmp/library/mib.h
./snmp_parse_args.lo: ../include/net-snmp/library/scapi.h
./snmp_parse_args.lo: ../include/net-snmp/library/keytools.h
./snmp_parse_args.lo: ../include/net-snmp/library/parse.h
./snmp_parse_args.lo: ../include/net-snmp/library/snmpv3.h
./snmp_parse_args.lo: ../include/net-snmp/library/transform_oids.h
./snmp_secmod.lo: ../include/net-snmp/net-snmp-config.h
./snmp_secmod.lo: ../include/net-snmp/system/linux.h
./snmp_secmod.lo: ../include/net-snmp/system/sysv.h
./snmp_secmod.lo: ../include/net-snmp/system/generic.h
./snmp_secmod.lo: ../include/net-snmp/machine/generic.h 
./snmp_secmod.lo: ../include/net-snmp/types.h 
./snmp_secmod.lo: ../include/net-snmp/definitions.h
./snmp_secmod.lo: ../include/net-snmp/library/snmp_api.h
./snmp_secmod.lo: ../include/net-snmp/library/asn1.h
./snmp_secmod.lo: ../include/net-snmp/library/snmp_impl.h
./snmp_secmod.lo: ../include/net-snmp/library/snmp.h
./snmp_secmod.lo: ../include/net-snmp/library/snmp-tc.h
./snmp_secmod.lo: ../include/net-snmp/output_api.h
./snmp_secmod.lo: ../include/net-snmp/library/snmp_client.h
./snmp_secmod.lo: ../include/net-snmp/library/snmp_debug.h
./snmp_secmod.lo: ../include/net-snmp/library/snmp_logging.h
./snmp_secmod.lo: ../include/net-snmp/config_api.h
./snmp_secmod.lo: ../include/net-snmp/library/read_config.h
./snmp_secmod.lo: ../include/net-snmp/library/default_store.h
./snmp_secmod.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmp_secmod.lo: ../include/net-snmp/library/snmp_enum.h
./snmp_secmod.lo: ../include/net-snmp/library/vacm.h
./snmp_secmod.lo: ../include/net-snmp/utilities.h
./snmp_secmod.lo: ../include/net-snmp/library/getopt.h
./snmp_secmod.lo: ../include/net-snmp/library/system.h
./snmp_secmod.lo: ../include/net-snmp/library/tools.h
./snmp_secmod.lo: ../include/net-snmp/library/int64.h
./snmp_secmod.lo: ../include/net-snmp/library/mt_support.h
./snmp_secmod.lo: ../include/net-snmp/library/snmp_alarm.h
./snmp_secmod.lo: ../include/net-snmp/library/data_list.h
./snmp_secmod.lo: ../include/net-snmp/library/callback.h
./snmp_secmod.lo: ../include/net-snmp/library/oid_stash.h
./snmp_secmod.lo: ../include/net-snmp/library/check_varbind.h
./snmp_secmod.lo: ../include/net-snmp/version.h
./snmp_secmod.lo: ../include/net-snmp/library/snmp_secmod.h
./snmp_secmod.lo: ../include/net-snmp/library/snmpusm.h snmpsm_init.h
./snmp-tc.lo: ../include/net-snmp/net-snmp-config.h
./snmp-tc.lo: ../include/net-snmp/system/linux.h
./snmp-tc.lo: ../include/net-snmp/system/sysv.h
./snmp-tc.lo: ../include/net-snmp/system/generic.h
./snmp-tc.lo: ../include/net-snmp/machine/generic.h 
./snmp-tc.lo:  ../include/net-snmp/types.h
./snmp-tc.lo: ../include/net-snmp/definitions.h
./snmp-tc.lo: ../include/net-snmp/library/snmp_api.h
./snmp-tc.lo: ../include/net-snmp/library/asn1.h
./snmp-tc.lo: ../include/net-snmp/library/snmp_impl.h
./snmp-tc.lo: ../include/net-snmp/library/snmp.h
./snmp-tc.lo: ../include/net-snmp/library/snmp-tc.h
./snmpTCPDomain.lo: ../include/net-snmp/net-snmp-config.h
./snmpTCPDomain.lo: ../include/net-snmp/system/linux.h
./snmpTCPDomain.lo: ../include/net-snmp/system/sysv.h
./snmpTCPDomain.lo: ../include/net-snmp/system/generic.h
./snmpTCPDomain.lo: ../include/net-snmp/machine/generic.h
./snmpTCPDomain.lo: ../include/net-snmp/types.h 
./snmpTCPDomain.lo: ../include/net-snmp/definitions.h
./snmpTCPDomain.lo: ../include/net-snmp/library/snmp_api.h
./snmpTCPDomain.lo: ../include/net-snmp/library/asn1.h
./snmpTCPDomain.lo: ../include/net-snmp/library/snmp_impl.h
./snmpTCPDomain.lo: ../include/net-snmp/library/snmp.h
./snmpTCPDomain.lo: ../include/net-snmp/library/snmp-tc.h
./snmpTCPDomain.lo: ../include/net-snmp/output_api.h
./snmpTCPDomain.lo: ../include/net-snmp/library/snmp_client.h
./snmpTCPDomain.lo: ../include/net-snmp/library/snmp_debug.h
./snmpTCPDomain.lo: ../include/net-snmp/library/snmp_logging.h
./snmpTCPDomain.lo: ../include/net-snmp/library/snmp_transport.h
./snmpTCPDomain.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmpTCPDomain.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/net-snmp-config.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/system/linux.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/system/sysv.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/system/generic.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/machine/generic.h
./snmpTCPIPv6Domain.lo:  ../include/net-snmp/types.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/definitions.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/library/snmp_api.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/library/asn1.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/library/snmp_impl.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/library/snmp.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/library/snmp-tc.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/output_api.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/library/snmp_client.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/library/snmp_debug.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/library/snmp_logging.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/config_api.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/library/read_config.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/library/default_store.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/library/snmp_enum.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/library/vacm.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/library/snmp_transport.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmpTCPIPv6Domain.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmp_transport.lo: ../include/net-snmp/net-snmp-config.h
./snmp_transport.lo: ../include/net-snmp/system/linux.h
./snmp_transport.lo: ../include/net-snmp/system/sysv.h
./snmp_transport.lo: ../include/net-snmp/system/generic.h
./snmp_transport.lo: ../include/net-snmp/machine/generic.h
./snmp_transport.lo: ../include/net-snmp/output_api.h
./snmp_transport.lo: ../include/net-snmp/types.h 
./snmp_transport.lo: ../include/net-snmp/definitions.h
./snmp_transport.lo: ../include/net-snmp/library/snmp_api.h
./snmp_transport.lo: ../include/net-snmp/library/asn1.h
./snmp_transport.lo: ../include/net-snmp/library/snmp_impl.h
./snmp_transport.lo: ../include/net-snmp/library/snmp.h
./snmp_transport.lo: ../include/net-snmp/library/snmp-tc.h
./snmp_transport.lo: ../include/net-snmp/library/snmp_client.h
./snmp_transport.lo: ../include/net-snmp/library/snmp_debug.h
./snmp_transport.lo: ../include/net-snmp/library/snmp_logging.h
./snmp_transport.lo: ../include/net-snmp/utilities.h
./snmp_transport.lo: ../include/net-snmp/library/getopt.h
./snmp_transport.lo: ../include/net-snmp/library/system.h
./snmp_transport.lo: ../include/net-snmp/library/tools.h
./snmp_transport.lo: ../include/net-snmp/library/int64.h
./snmp_transport.lo: ../include/net-snmp/library/mt_support.h
./snmp_transport.lo: ../include/net-snmp/library/snmp_alarm.h
./snmp_transport.lo: ../include/net-snmp/library/data_list.h
./snmp_transport.lo: ../include/net-snmp/library/callback.h
./snmp_transport.lo: ../include/net-snmp/library/oid_stash.h
./snmp_transport.lo: ../include/net-snmp/library/check_varbind.h
./snmp_transport.lo: ../include/net-snmp/version.h
./snmp_transport.lo: ../include/net-snmp/library/snmp_transport.h
./snmp_transport.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmp_transport.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmp_transport.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmp_transport.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmp_transport.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmp_transport.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmpUDPDomain.lo: ../include/net-snmp/net-snmp-config.h
./snmpUDPDomain.lo: ../include/net-snmp/system/linux.h
./snmpUDPDomain.lo: ../include/net-snmp/system/sysv.h
./snmpUDPDomain.lo: ../include/net-snmp/system/generic.h
./snmpUDPDomain.lo: ../include/net-snmp/machine/generic.h
./snmpUDPDomain.lo: ../include/net-snmp/types.h 
./snmpUDPDomain.lo: ../include/net-snmp/definitions.h
./snmpUDPDomain.lo: ../include/net-snmp/library/snmp_api.h
./snmpUDPDomain.lo: ../include/net-snmp/library/asn1.h
./snmpUDPDomain.lo: ../include/net-snmp/library/snmp_impl.h
./snmpUDPDomain.lo: ../include/net-snmp/library/snmp.h
./snmpUDPDomain.lo: ../include/net-snmp/library/snmp-tc.h
./snmpUDPDomain.lo: ../include/net-snmp/output_api.h
./snmpUDPDomain.lo: ../include/net-snmp/library/snmp_client.h
./snmpUDPDomain.lo: ../include/net-snmp/library/snmp_debug.h
./snmpUDPDomain.lo: ../include/net-snmp/library/snmp_logging.h
./snmpUDPDomain.lo: ../include/net-snmp/config_api.h
./snmpUDPDomain.lo: ../include/net-snmp/library/read_config.h
./snmpUDPDomain.lo: ../include/net-snmp/library/default_store.h
./snmpUDPDomain.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpUDPDomain.lo: ../include/net-snmp/library/snmp_enum.h
./snmpUDPDomain.lo: ../include/net-snmp/library/vacm.h
./snmpUDPDomain.lo: ../include/net-snmp/library/snmp_transport.h
./snmpUDPDomain.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/net-snmp-config.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/system/linux.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/system/sysv.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/system/generic.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/machine/generic.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/types.h 
./snmpUDPIPv6Domain.lo: ../include/net-snmp/definitions.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/library/snmp_api.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/library/asn1.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/library/snmp_impl.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/library/snmp.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/library/snmp-tc.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/output_api.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/library/snmp_client.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/library/snmp_debug.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/library/snmp_logging.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/config_api.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/library/read_config.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/library/default_store.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/library/snmp_enum.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/library/vacm.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/library/snmp_transport.h
./snmpUDPIPv6Domain.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmpUnixDomain.lo: ../include/net-snmp/net-snmp-config.h
./snmpUnixDomain.lo: ../include/net-snmp/system/linux.h
./snmpUnixDomain.lo: ../include/net-snmp/system/sysv.h
./snmpUnixDomain.lo: ../include/net-snmp/system/generic.h
./snmpUnixDomain.lo: ../include/net-snmp/machine/generic.h
./snmpUnixDomain.lo: ../include/net-snmp/types.h 
./snmpUnixDomain.lo: ../include/net-snmp/definitions.h
./snmpUnixDomain.lo: ../include/net-snmp/library/snmp_api.h
./snmpUnixDomain.lo: ../include/net-snmp/library/asn1.h
./snmpUnixDomain.lo: ../include/net-snmp/library/snmp_impl.h
./snmpUnixDomain.lo: ../include/net-snmp/library/snmp.h
./snmpUnixDomain.lo: ../include/net-snmp/library/snmp-tc.h
./snmpUnixDomain.lo: ../include/net-snmp/output_api.h
./snmpUnixDomain.lo: ../include/net-snmp/library/snmp_client.h
./snmpUnixDomain.lo: ../include/net-snmp/library/snmp_debug.h
./snmpUnixDomain.lo: ../include/net-snmp/library/snmp_logging.h
./snmpUnixDomain.lo: ../include/net-snmp/config_api.h
./snmpUnixDomain.lo: ../include/net-snmp/library/read_config.h
./snmpUnixDomain.lo: ../include/net-snmp/library/default_store.h
./snmpUnixDomain.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpUnixDomain.lo: ../include/net-snmp/library/snmp_enum.h
./snmpUnixDomain.lo: ../include/net-snmp/library/vacm.h
./snmpUnixDomain.lo: ../include/net-snmp/library/snmp_transport.h
./snmpUnixDomain.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmpusm.lo: ../include/net-snmp/net-snmp-config.h
./snmpusm.lo: ../include/net-snmp/system/linux.h
./snmpusm.lo: ../include/net-snmp/system/sysv.h
./snmpusm.lo: ../include/net-snmp/system/generic.h
./snmpusm.lo: ../include/net-snmp/machine/generic.h 
./snmpusm.lo: ../include/net-snmp/types.h ../include/net-snmp/definitions.h
./snmpusm.lo: ../include/net-snmp/library/snmp_api.h
./snmpusm.lo: ../include/net-snmp/library/asn1.h
./snmpusm.lo: ../include/net-snmp/library/snmp_impl.h
./snmpusm.lo: ../include/net-snmp/library/snmp.h
./snmpusm.lo: ../include/net-snmp/library/snmp-tc.h
./snmpusm.lo: ../include/net-snmp/output_api.h
./snmpusm.lo: ../include/net-snmp/library/snmp_client.h
./snmpusm.lo: ../include/net-snmp/library/snmp_debug.h
./snmpusm.lo: ../include/net-snmp/library/snmp_logging.h
./snmpusm.lo: ../include/net-snmp/config_api.h
./snmpusm.lo: ../include/net-snmp/library/read_config.h
./snmpusm.lo: ../include/net-snmp/library/default_store.h
./snmpusm.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpusm.lo: ../include/net-snmp/library/snmp_enum.h
./snmpusm.lo: ../include/net-snmp/library/vacm.h
./snmpusm.lo: ../include/net-snmp/utilities.h
./snmpusm.lo: ../include/net-snmp/library/getopt.h
./snmpusm.lo: ../include/net-snmp/library/system.h
./snmpusm.lo: ../include/net-snmp/library/tools.h
./snmpusm.lo: ../include/net-snmp/library/int64.h
./snmpusm.lo: ../include/net-snmp/library/mt_support.h
./snmpusm.lo: ../include/net-snmp/library/snmp_alarm.h
./snmpusm.lo: ../include/net-snmp/library/data_list.h
./snmpusm.lo: ../include/net-snmp/library/callback.h
./snmpusm.lo: ../include/net-snmp/library/oid_stash.h
./snmpusm.lo: ../include/net-snmp/library/check_varbind.h
./snmpusm.lo: ../include/net-snmp/version.h
./snmpusm.lo: ../include/net-snmp/library/keytools.h
./snmpusm.lo: ../include/net-snmp/library/snmpv3.h
./snmpusm.lo: ../include/net-snmp/library/lcd_time.h
./snmpusm.lo: ../include/net-snmp/library/scapi.h
./snmpusm.lo: ../include/net-snmp/library/snmp_secmod.h
./snmpusm.lo: ../include/net-snmp/library/snmpusm.h
./snmpv3.lo: ../include/net-snmp/net-snmp-config.h
./snmpv3.lo: ../include/net-snmp/system/linux.h
./snmpv3.lo: ../include/net-snmp/system/sysv.h
./snmpv3.lo: ../include/net-snmp/system/generic.h
./snmpv3.lo: ../include/net-snmp/machine/generic.h 
./snmpv3.lo:  ../include/net-snmp/types.h
./snmpv3.lo: ../include/net-snmp/definitions.h
./snmpv3.lo: ../include/net-snmp/library/snmp_api.h
./snmpv3.lo: ../include/net-snmp/library/asn1.h
./snmpv3.lo: ../include/net-snmp/library/snmp_impl.h
./snmpv3.lo: ../include/net-snmp/library/snmp.h
./snmpv3.lo: ../include/net-snmp/library/snmp-tc.h
./snmpv3.lo: ../include/net-snmp/output_api.h
./snmpv3.lo: ../include/net-snmp/library/snmp_client.h
./snmpv3.lo: ../include/net-snmp/library/snmp_debug.h
./snmpv3.lo: ../include/net-snmp/library/snmp_logging.h 
./snmpv3.lo:  ../include/net-snmp/config_api.h
./snmpv3.lo: ../include/net-snmp/library/read_config.h
./snmpv3.lo: ../include/net-snmp/library/default_store.h
./snmpv3.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpv3.lo: ../include/net-snmp/library/snmp_enum.h
./snmpv3.lo: ../include/net-snmp/library/vacm.h
./snmpv3.lo: ../include/net-snmp/utilities.h
./snmpv3.lo: ../include/net-snmp/library/getopt.h
./snmpv3.lo: ../include/net-snmp/library/system.h
./snmpv3.lo: ../include/net-snmp/library/tools.h
./snmpv3.lo: ../include/net-snmp/library/int64.h
./snmpv3.lo: ../include/net-snmp/library/mt_support.h
./snmpv3.lo: ../include/net-snmp/library/snmp_alarm.h
./snmpv3.lo: ../include/net-snmp/library/data_list.h
./snmpv3.lo: ../include/net-snmp/library/callback.h
./snmpv3.lo: ../include/net-snmp/library/oid_stash.h
./snmpv3.lo: ../include/net-snmp/library/check_varbind.h
./snmpv3.lo: ../include/net-snmp/version.h
./snmpv3.lo: ../include/net-snmp/library/snmpv3.h
./snmpv3.lo: ../include/net-snmp/library/lcd_time.h
./snmpv3.lo: ../include/net-snmp/library/scapi.h
./snmpv3.lo: ../include/net-snmp/library/keytools.h
./snmpv3.lo: ../include/net-snmp/library/snmp_secmod.h
./snmpv3.lo: ../include/net-snmp/library/snmpusm.h
./snmpv3.lo: ../include/net-snmp/library/transform_oids.h
./snmp_version.lo: ../include/net-snmp/version.h
./snprintf.lo: ../include/net-snmp/net-snmp-config.h
./snprintf.lo: ../include/net-snmp/system/linux.h
./snprintf.lo: ../include/net-snmp/system/sysv.h
./snprintf.lo: ../include/net-snmp/system/generic.h
./snprintf.lo: ../include/net-snmp/machine/generic.h
./strlcpy.lo: ../include/net-snmp/net-snmp-config.h
./strlcpy.lo: ../include/net-snmp/system/linux.h
./strlcpy.lo: ../include/net-snmp/system/sysv.h
./strlcpy.lo: ../include/net-snmp/system/generic.h
./strlcpy.lo: ../include/net-snmp/machine/generic.h 
./strtol.lo: ../include/net-snmp/net-snmp-config.h
./strtol.lo: ../include/net-snmp/system/linux.h
./strtol.lo: ../include/net-snmp/system/sysv.h
./strtol.lo: ../include/net-snmp/system/generic.h
./strtol.lo: ../include/net-snmp/machine/generic.h
./strtoul.lo: ../include/net-snmp/net-snmp-config.h
./strtoul.lo: ../include/net-snmp/system/linux.h
./strtoul.lo: ../include/net-snmp/system/sysv.h
./strtoul.lo: ../include/net-snmp/system/generic.h
./strtoul.lo: ../include/net-snmp/machine/generic.h
./system.lo: ../include/net-snmp/net-snmp-config.h
./system.lo: ../include/net-snmp/system/linux.h
./system.lo: ../include/net-snmp/system/sysv.h
./system.lo: ../include/net-snmp/system/generic.h
./system.lo: ../include/net-snmp/machine/generic.h 
./system.lo: ../include/net-snmp/types.h ../include/net-snmp/definitions.h
./system.lo: ../include/net-snmp/library/snmp_api.h
./system.lo: ../include/net-snmp/library/asn1.h
./system.lo: ../include/net-snmp/library/snmp_impl.h
./system.lo: ../include/net-snmp/library/snmp.h
./system.lo: ../include/net-snmp/library/snmp-tc.h
./system.lo: ../include/net-snmp/output_api.h
./system.lo: ../include/net-snmp/library/snmp_client.h
./system.lo: ../include/net-snmp/library/snmp_debug.h
./system.lo: ../include/net-snmp/library/snmp_logging.h 
./system.lo:  ../include/net-snmp/utilities.h
./system.lo: ../include/net-snmp/library/getopt.h
./system.lo: ../include/net-snmp/library/system.h
./system.lo: ../include/net-snmp/library/tools.h
./system.lo: ../include/net-snmp/library/int64.h
./system.lo: ../include/net-snmp/library/mt_support.h
./system.lo: ../include/net-snmp/library/snmp_alarm.h
./system.lo: ../include/net-snmp/library/data_list.h
./system.lo: ../include/net-snmp/library/callback.h
./system.lo: ../include/net-snmp/library/oid_stash.h
./system.lo: ../include/net-snmp/library/check_varbind.h
./system.lo: ../include/net-snmp/version.h
./test_binary_array.lo: ../include/net-snmp/net-snmp-config.h
./test_binary_array.lo: ../include/net-snmp/system/linux.h
./test_binary_array.lo: ../include/net-snmp/system/sysv.h
./test_binary_array.lo: ../include/net-snmp/system/generic.h
./test_binary_array.lo: ../include/net-snmp/machine/generic.h
./test_binary_array.lo: ../include/net-snmp/net-snmp-includes.h
./test_binary_array.lo: ../include/net-snmp/definitions.h
./test_binary_array.lo: ../include/net-snmp/types.h
./test_binary_array.lo: ../include/net-snmp/library/snmp_api.h
./test_binary_array.lo: ../include/net-snmp/library/asn1.h
./test_binary_array.lo: ../include/net-snmp/library/snmp_impl.h
./test_binary_array.lo: ../include/net-snmp/library/snmp.h
./test_binary_array.lo: ../include/net-snmp/library/snmp-tc.h
./test_binary_array.lo: ../include/net-snmp/utilities.h
./test_binary_array.lo: ../include/net-snmp/library/snmp_client.h
./test_binary_array.lo: ../include/net-snmp/library/getopt.h
./test_binary_array.lo: ../include/net-snmp/library/system.h
./test_binary_array.lo: ../include/net-snmp/library/tools.h
./test_binary_array.lo: ../include/net-snmp/library/int64.h
./test_binary_array.lo: ../include/net-snmp/library/mt_support.h
./test_binary_array.lo: ../include/net-snmp/library/snmp_alarm.h
./test_binary_array.lo: ../include/net-snmp/library/data_list.h
./test_binary_array.lo: ../include/net-snmp/library/callback.h
./test_binary_array.lo: ../include/net-snmp/library/oid_stash.h
./test_binary_array.lo: ../include/net-snmp/library/check_varbind.h
./test_binary_array.lo: ../include/net-snmp/version.h
./test_binary_array.lo: ../include/net-snmp/session_api.h
./test_binary_array.lo: ../include/net-snmp/library/snmp_transport.h
./test_binary_array.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./test_binary_array.lo: ../include/net-snmp/library/snmpUnixDomain.h
./test_binary_array.lo: ../include/net-snmp/library/snmpUDPDomain.h
./test_binary_array.lo: ../include/net-snmp/library/snmpTCPDomain.h
./test_binary_array.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./test_binary_array.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./test_binary_array.lo: ../include/net-snmp/library/snmpIPXDomain.h
./test_binary_array.lo: ../include/net-snmp/library/ucd_compat.h
./test_binary_array.lo: ../include/net-snmp/pdu_api.h
./test_binary_array.lo: ../include/net-snmp/mib_api.h
./test_binary_array.lo: ../include/net-snmp/library/mib.h
./test_binary_array.lo: ../include/net-snmp/library/parse.h
./test_binary_array.lo: ../include/net-snmp/varbind_api.h
./test_binary_array.lo: ../include/net-snmp/config_api.h
./test_binary_array.lo: ../include/net-snmp/library/read_config.h
./test_binary_array.lo: ../include/net-snmp/library/default_store.h
./test_binary_array.lo: ../include/net-snmp/library/snmp_parse_args.h
./test_binary_array.lo: ../include/net-snmp/library/snmp_enum.h
./test_binary_array.lo: ../include/net-snmp/library/vacm.h
./test_binary_array.lo: ../include/net-snmp/output_api.h
./test_binary_array.lo: ../include/net-snmp/library/snmp_debug.h
./test_binary_array.lo: ../include/net-snmp/library/snmp_logging.h
./test_binary_array.lo: ../include/net-snmp/snmpv3_api.h
./test_binary_array.lo: ../include/net-snmp/library/snmpv3.h
./test_binary_array.lo: ../include/net-snmp/library/transform_oids.h
./test_binary_array.lo: ../include/net-snmp/library/keytools.h
./test_binary_array.lo: ../include/net-snmp/library/scapi.h
./test_binary_array.lo: ../include/net-snmp/library/lcd_time.h
./test_binary_array.lo: ../include/net-snmp/library/snmp_secmod.h
./test_binary_array.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./test_binary_array.lo: ../include/net-snmp/library/snmpusm.h
./test_binary_array.lo: ../include/net-snmp/library/container.h
./test_binary_array.lo: ../include/net-snmp/library/factory.h
./test_binary_array.lo: ../include/net-snmp/library/container_binary_array.h
./test_binary_array.lo: ../include/net-snmp/library/snmp_assert.h
./tools.lo: ../include/net-snmp/net-snmp-config.h
./tools.lo: ../include/net-snmp/system/linux.h
./tools.lo: ../include/net-snmp/system/sysv.h
./tools.lo: ../include/net-snmp/system/generic.h
./tools.lo: ../include/net-snmp/machine/generic.h 
./tools.lo: ../include/net-snmp/types.h ../include/net-snmp/definitions.h
./tools.lo: ../include/net-snmp/library/snmp_api.h
./tools.lo: ../include/net-snmp/library/asn1.h
./tools.lo: ../include/net-snmp/library/snmp_impl.h
./tools.lo: ../include/net-snmp/library/snmp.h
./tools.lo: ../include/net-snmp/library/snmp-tc.h
./tools.lo: ../include/net-snmp/output_api.h
./tools.lo: ../include/net-snmp/library/snmp_client.h
./tools.lo: ../include/net-snmp/library/snmp_debug.h
./tools.lo: ../include/net-snmp/library/snmp_logging.h 
./tools.lo:  ../include/net-snmp/utilities.h
./tools.lo: ../include/net-snmp/library/getopt.h
./tools.lo: ../include/net-snmp/library/system.h
./tools.lo: ../include/net-snmp/library/tools.h
./tools.lo: ../include/net-snmp/library/int64.h
./tools.lo: ../include/net-snmp/library/mt_support.h
./tools.lo: ../include/net-snmp/library/snmp_alarm.h
./tools.lo: ../include/net-snmp/library/data_list.h
./tools.lo: ../include/net-snmp/library/callback.h
./tools.lo: ../include/net-snmp/library/oid_stash.h
./tools.lo: ../include/net-snmp/library/check_varbind.h
./tools.lo: ../include/net-snmp/version.h ../include/net-snmp/library/mib.h
./tools.lo: ../include/net-snmp/library/scapi.h
./ucd_compat.lo: ../include/net-snmp/net-snmp-config.h
./ucd_compat.lo: ../include/net-snmp/system/linux.h
./ucd_compat.lo: ../include/net-snmp/system/sysv.h
./ucd_compat.lo: ../include/net-snmp/system/generic.h
./ucd_compat.lo: ../include/net-snmp/machine/generic.h
./ucd_compat.lo: ../include/net-snmp/types.h 
./ucd_compat.lo: ../include/net-snmp/definitions.h
./ucd_compat.lo: ../include/net-snmp/library/snmp_api.h
./ucd_compat.lo: ../include/net-snmp/library/asn1.h
./ucd_compat.lo: ../include/net-snmp/library/snmp_impl.h
./ucd_compat.lo: ../include/net-snmp/library/snmp.h
./ucd_compat.lo: ../include/net-snmp/library/snmp-tc.h
./ucd_compat.lo: ../include/net-snmp/session_api.h
./ucd_compat.lo: ../include/net-snmp/library/snmp_client.h
./ucd_compat.lo: ../include/net-snmp/library/callback.h
./ucd_compat.lo: ../include/net-snmp/library/snmp_transport.h
./ucd_compat.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./ucd_compat.lo: ../include/net-snmp/library/snmpUnixDomain.h
./ucd_compat.lo: ../include/net-snmp/library/snmpUDPDomain.h
./ucd_compat.lo: ../include/net-snmp/library/snmpTCPDomain.h
./ucd_compat.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./ucd_compat.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./ucd_compat.lo: ../include/net-snmp/library/snmpIPXDomain.h
./ucd_compat.lo: ../include/net-snmp/library/ucd_compat.h
./ucd_compat.lo: ../include/net-snmp/config_api.h
./ucd_compat.lo: ../include/net-snmp/library/read_config.h
./ucd_compat.lo: ../include/net-snmp/library/default_store.h
./ucd_compat.lo: ../include/net-snmp/library/snmp_parse_args.h
./ucd_compat.lo: ../include/net-snmp/library/snmp_enum.h
./ucd_compat.lo: ../include/net-snmp/library/vacm.h
./ucd_compat.lo: ../include/net-snmp/library/mib.h
./vacm.lo: ../include/net-snmp/net-snmp-config.h
./vacm.lo: ../include/net-snmp/system/linux.h
./vacm.lo: ../include/net-snmp/system/sysv.h
./vacm.lo: ../include/net-snmp/system/generic.h
./vacm.lo: ../include/net-snmp/machine/generic.h 
./vacm.lo: ../include/net-snmp/types.h ../include/net-snmp/definitions.h
./vacm.lo: ../include/net-snmp/library/snmp_api.h
./vacm.lo: ../include/net-snmp/library/asn1.h
./vacm.lo: ../include/net-snmp/library/snmp_impl.h
./vacm.lo: ../include/net-snmp/library/snmp.h
./vacm.lo: ../include/net-snmp/library/snmp-tc.h
./vacm.lo: ../include/net-snmp/output_api.h
./vacm.lo: ../include/net-snmp/library/snmp_client.h
./vacm.lo: ../include/net-snmp/library/snmp_debug.h
./vacm.lo: ../include/net-snmp/library/snmp_logging.h 
./vacm.lo:  ../include/net-snmp/config_api.h
./vacm.lo: ../include/net-snmp/library/read_config.h
./vacm.lo: ../include/net-snmp/library/default_store.h
./vacm.lo: ../include/net-snmp/library/snmp_parse_args.h
./vacm.lo: ../include/net-snmp/library/snmp_enum.h
./vacm.lo: ../include/net-snmp/library/vacm.h
./winservice.lo: ../include/net-snmp/library/winservice.h
