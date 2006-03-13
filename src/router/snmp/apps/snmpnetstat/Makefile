# Generated automatically from Makefile.depend by configure.
#
# Minimum environment and virtual path setup
#
SHELL		= /bin/sh
srcdir		= .
top_srcdir	= ../..
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
SED		= /home/openwrt/workspace/toolchains/4.1.0-uclibc-0.9.28/bin/sed
LN_S		= ln -s
AUTOCONF	= :
AUTOHEADER	= :

#
# Compiler arguments
#
CFLAGS		= -Os -mtune=mips32 -mips32 -pipe -funit-at-a-time -DCAN_USE_SYSCTL=1 -Delf 
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
# RANLIB 	= mipsel-linux-uclibc-ranlib
RANLIB		= :

# libtool definitions
.SUFFIXES: .c .o .lo
.c.lo:
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
#
# snmpnetstat Makefile
#

top_builddir=../..
INSTALLBINPROGS=snmpnetstat$(EXEEXT)

#
# build info
#
SRCS=	inet.c inet6.c if.c main.c route.c winstub.c
OBJS=	inet.o inet6.o if.o main.o route.o winstub.o
LOBJS=	inet.lo inet6.lo if.lo main.lo route.lo winstub.lo
TARG= snmpnetstat$(EXEEXT)

CPPFLAGS=-I../../include -I../.. -I$(srcdir)/../.. -I$(srcdir)/../../include 

USELIBS= ../../snmplib/libnetsnmp.$(LIB_EXTENSION)$(LIB_VERSION)
LIBS=	 ../../snmplib/libnetsnmp.$(LIB_EXTENSION)$(LIB_VERSION) -lm 

all: standardall

snmpnetstat$(EXEEXT): ${LOBJS} ${USELIBS}
	${LINK} -o $@ ${LOBJS} ${LOCAL_LIBS} ${LDFLAGS} ${LIBS}

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

./if.lo: ../../include/net-snmp/net-snmp-config.h
./if.lo: ../../include/net-snmp/system/linux.h
./if.lo: ../../include/net-snmp/system/sysv.h
./if.lo: ../../include/net-snmp/system/generic.h
./if.lo: ../../include/net-snmp/machine/generic.h 
./if.lo:   main.h
./if.lo: ../../include/net-snmp/net-snmp-includes.h
./if.lo: ../../include/net-snmp/definitions.h ../../include/net-snmp/types.h
./if.lo: ../../include/net-snmp/library/snmp_api.h
./if.lo: ../../include/net-snmp/library/asn1.h
./if.lo: ../../include/net-snmp/library/snmp_impl.h
./if.lo: ../../include/net-snmp/library/snmp.h
./if.lo: ../../include/net-snmp/library/snmp-tc.h
./if.lo: ../../include/net-snmp/utilities.h
./if.lo: ../../include/net-snmp/library/snmp_client.h
./if.lo: ../../include/net-snmp/library/getopt.h
./if.lo: ../../include/net-snmp/library/system.h
./if.lo: ../../include/net-snmp/library/tools.h
./if.lo: ../../include/net-snmp/library/int64.h
./if.lo: ../../include/net-snmp/library/mt_support.h
./if.lo: ../../include/net-snmp/library/snmp_alarm.h
./if.lo: ../../include/net-snmp/library/data_list.h
./if.lo: ../../include/net-snmp/library/callback.h
./if.lo: ../../include/net-snmp/library/oid_stash.h
./if.lo: ../../include/net-snmp/library/check_varbind.h
./if.lo: ../../include/net-snmp/version.h
./if.lo: ../../include/net-snmp/session_api.h
./if.lo: ../../include/net-snmp/library/snmp_transport.h
./if.lo: ../../include/net-snmp/library/snmpCallbackDomain.h
./if.lo: ../../include/net-snmp/library/snmpUnixDomain.h
./if.lo:  ../../include/net-snmp/library/snmpUDPDomain.h
./if.lo: ../../include/net-snmp/library/snmpTCPDomain.h
./if.lo: ../../include/net-snmp/library/snmpUDPIPv6Domain.h
./if.lo: ../../include/net-snmp/library/snmpTCPIPv6Domain.h
./if.lo: ../../include/net-snmp/library/snmpIPXDomain.h
./if.lo: ../../include/net-snmp/library/ucd_compat.h
./if.lo: ../../include/net-snmp/pdu_api.h ../../include/net-snmp/mib_api.h
./if.lo: ../../include/net-snmp/library/mib.h
./if.lo: ../../include/net-snmp/library/parse.h
./if.lo: ../../include/net-snmp/varbind_api.h
./if.lo: ../../include/net-snmp/config_api.h
./if.lo: ../../include/net-snmp/library/read_config.h
./if.lo: ../../include/net-snmp/library/default_store.h
./if.lo: ../../include/net-snmp/library/snmp_parse_args.h
./if.lo: ../../include/net-snmp/library/snmp_enum.h
./if.lo: ../../include/net-snmp/library/vacm.h
./if.lo: ../../include/net-snmp/output_api.h
./if.lo: ../../include/net-snmp/library/snmp_debug.h
./if.lo: ../../include/net-snmp/library/snmp_logging.h 
./if.lo:  ../../include/net-snmp/snmpv3_api.h
./if.lo: ../../include/net-snmp/library/snmpv3.h
./if.lo: ../../include/net-snmp/library/transform_oids.h
./if.lo: ../../include/net-snmp/library/keytools.h
./if.lo: ../../include/net-snmp/library/scapi.h
./if.lo: ../../include/net-snmp/library/lcd_time.h
./if.lo: ../../include/net-snmp/library/snmp_secmod.h
./if.lo: ../../include/net-snmp/library/snmpv3-security-includes.h
./if.lo: ../../include/net-snmp/library/snmpusm.h netstat.h
./inet6.lo: ../../include/net-snmp/net-snmp-config.h
./inet6.lo: ../../include/net-snmp/system/linux.h
./inet6.lo: ../../include/net-snmp/system/sysv.h
./inet6.lo: ../../include/net-snmp/system/generic.h
./inet6.lo: ../../include/net-snmp/machine/generic.h
./inet.lo: ../../include/net-snmp/net-snmp-config.h
./inet.lo: ../../include/net-snmp/system/linux.h
./inet.lo: ../../include/net-snmp/system/sysv.h
./inet.lo: ../../include/net-snmp/system/generic.h
./inet.lo: ../../include/net-snmp/machine/generic.h 
./inet.lo:  main.h
./inet.lo: ../../include/net-snmp/net-snmp-includes.h 
./inet.lo: ../../include/net-snmp/definitions.h
./inet.lo: ../../include/net-snmp/types.h
./inet.lo: ../../include/net-snmp/library/snmp_api.h
./inet.lo: ../../include/net-snmp/library/asn1.h
./inet.lo: ../../include/net-snmp/library/snmp_impl.h
./inet.lo: ../../include/net-snmp/library/snmp.h
./inet.lo: ../../include/net-snmp/library/snmp-tc.h
./inet.lo: ../../include/net-snmp/utilities.h
./inet.lo: ../../include/net-snmp/library/snmp_client.h
./inet.lo: ../../include/net-snmp/library/getopt.h
./inet.lo: ../../include/net-snmp/library/system.h
./inet.lo: ../../include/net-snmp/library/tools.h
./inet.lo: ../../include/net-snmp/library/int64.h
./inet.lo: ../../include/net-snmp/library/mt_support.h
./inet.lo: ../../include/net-snmp/library/snmp_alarm.h
./inet.lo: ../../include/net-snmp/library/data_list.h
./inet.lo: ../../include/net-snmp/library/callback.h
./inet.lo: ../../include/net-snmp/library/oid_stash.h
./inet.lo: ../../include/net-snmp/library/check_varbind.h
./inet.lo: ../../include/net-snmp/version.h
./inet.lo: ../../include/net-snmp/session_api.h
./inet.lo: ../../include/net-snmp/library/snmp_transport.h
./inet.lo: ../../include/net-snmp/library/snmpCallbackDomain.h
./inet.lo: ../../include/net-snmp/library/snmpUnixDomain.h
./inet.lo: ../../include/net-snmp/library/snmpUDPDomain.h
./inet.lo: ../../include/net-snmp/library/snmpTCPDomain.h
./inet.lo: ../../include/net-snmp/library/snmpUDPIPv6Domain.h
./inet.lo: ../../include/net-snmp/library/snmpTCPIPv6Domain.h
./inet.lo: ../../include/net-snmp/library/snmpIPXDomain.h
./inet.lo: ../../include/net-snmp/library/ucd_compat.h
./inet.lo: ../../include/net-snmp/pdu_api.h ../../include/net-snmp/mib_api.h
./inet.lo: ../../include/net-snmp/library/mib.h
./inet.lo: ../../include/net-snmp/library/parse.h
./inet.lo: ../../include/net-snmp/varbind_api.h
./inet.lo: ../../include/net-snmp/config_api.h
./inet.lo: ../../include/net-snmp/library/read_config.h
./inet.lo: ../../include/net-snmp/library/default_store.h
./inet.lo: ../../include/net-snmp/library/snmp_parse_args.h
./inet.lo: ../../include/net-snmp/library/snmp_enum.h
./inet.lo: ../../include/net-snmp/library/vacm.h
./inet.lo: ../../include/net-snmp/output_api.h
./inet.lo: ../../include/net-snmp/library/snmp_debug.h
./inet.lo: ../../include/net-snmp/library/snmp_logging.h
./inet.lo: ../../include/net-snmp/snmpv3_api.h
./inet.lo: ../../include/net-snmp/library/snmpv3.h
./inet.lo: ../../include/net-snmp/library/transform_oids.h
./inet.lo: ../../include/net-snmp/library/keytools.h
./inet.lo: ../../include/net-snmp/library/scapi.h
./inet.lo: ../../include/net-snmp/library/lcd_time.h
./inet.lo: ../../include/net-snmp/library/snmp_secmod.h
./inet.lo: ../../include/net-snmp/library/snmpv3-security-includes.h
./inet.lo: ../../include/net-snmp/library/snmpusm.h netstat.h
./main.lo: ../../include/net-snmp/net-snmp-config.h
./main.lo: ../../include/net-snmp/system/linux.h
./main.lo: ../../include/net-snmp/system/sysv.h
./main.lo: ../../include/net-snmp/system/generic.h
./main.lo: ../../include/net-snmp/machine/generic.h 
./main.lo: ../../include/net-snmp/utilities.h ../../include/net-snmp/types.h
./main.lo: ../../include/net-snmp/definitions.h
./main.lo: ../../include/net-snmp/library/snmp_api.h
./main.lo: ../../include/net-snmp/library/asn1.h
./main.lo: ../../include/net-snmp/library/snmp_impl.h
./main.lo: ../../include/net-snmp/library/snmp.h
./main.lo: ../../include/net-snmp/library/snmp-tc.h
./main.lo: ../../include/net-snmp/library/snmp_client.h
./main.lo: ../../include/net-snmp/library/getopt.h
./main.lo: ../../include/net-snmp/library/system.h
./main.lo: ../../include/net-snmp/library/tools.h
./main.lo: ../../include/net-snmp/library/int64.h
./main.lo: ../../include/net-snmp/library/mt_support.h
./main.lo: ../../include/net-snmp/library/snmp_alarm.h
./main.lo: ../../include/net-snmp/library/data_list.h
./main.lo: ../../include/net-snmp/library/callback.h
./main.lo: ../../include/net-snmp/library/oid_stash.h
./main.lo: ../../include/net-snmp/library/check_varbind.h
./main.lo: ../../include/net-snmp/version.h 
./main.lo: ../../include/net-snmp/net-snmp-includes.h
./main.lo: ../../include/net-snmp/session_api.h
./main.lo: ../../include/net-snmp/library/snmp_transport.h
./main.lo: ../../include/net-snmp/library/snmpCallbackDomain.h
./main.lo: ../../include/net-snmp/library/snmpUnixDomain.h
./main.lo: ../../include/net-snmp/library/snmpUDPDomain.h
./main.lo: ../../include/net-snmp/library/snmpTCPDomain.h
./main.lo: ../../include/net-snmp/library/snmpUDPIPv6Domain.h
./main.lo: ../../include/net-snmp/library/snmpTCPIPv6Domain.h
./main.lo: ../../include/net-snmp/library/snmpIPXDomain.h
./main.lo: ../../include/net-snmp/library/ucd_compat.h
./main.lo: ../../include/net-snmp/pdu_api.h ../../include/net-snmp/mib_api.h
./main.lo: ../../include/net-snmp/library/mib.h
./main.lo: ../../include/net-snmp/library/parse.h
./main.lo: ../../include/net-snmp/varbind_api.h
./main.lo: ../../include/net-snmp/config_api.h
./main.lo: ../../include/net-snmp/library/read_config.h
./main.lo: ../../include/net-snmp/library/default_store.h
./main.lo: ../../include/net-snmp/library/snmp_parse_args.h
./main.lo: ../../include/net-snmp/library/snmp_enum.h
./main.lo: ../../include/net-snmp/library/vacm.h
./main.lo: ../../include/net-snmp/output_api.h
./main.lo: ../../include/net-snmp/library/snmp_debug.h
./main.lo: ../../include/net-snmp/library/snmp_logging.h
./main.lo: ../../include/net-snmp/snmpv3_api.h
./main.lo: ../../include/net-snmp/library/snmpv3.h
./main.lo: ../../include/net-snmp/library/transform_oids.h
./main.lo: ../../include/net-snmp/library/keytools.h
./main.lo: ../../include/net-snmp/library/scapi.h
./main.lo: ../../include/net-snmp/library/lcd_time.h
./main.lo: ../../include/net-snmp/library/snmp_secmod.h
./main.lo: ../../include/net-snmp/library/snmpv3-security-includes.h
./main.lo: ../../include/net-snmp/library/snmpusm.h netstat.h
./route.lo: ../../include/net-snmp/net-snmp-config.h
./route.lo: ../../include/net-snmp/system/linux.h
./route.lo: ../../include/net-snmp/system/sysv.h
./route.lo: ../../include/net-snmp/system/generic.h
./route.lo: ../../include/net-snmp/machine/generic.h 
./route.lo:  main.h
./route.lo: ../../include/net-snmp/net-snmp-includes.h
./route.lo:  ../../include/net-snmp/definitions.h
./route.lo: ../../include/net-snmp/types.h
./route.lo: ../../include/net-snmp/library/snmp_api.h
./route.lo: ../../include/net-snmp/library/asn1.h
./route.lo: ../../include/net-snmp/library/snmp_impl.h
./route.lo: ../../include/net-snmp/library/snmp.h
./route.lo: ../../include/net-snmp/library/snmp-tc.h
./route.lo: ../../include/net-snmp/utilities.h
./route.lo: ../../include/net-snmp/library/snmp_client.h
./route.lo: ../../include/net-snmp/library/getopt.h
./route.lo: ../../include/net-snmp/library/system.h
./route.lo: ../../include/net-snmp/library/tools.h
./route.lo: ../../include/net-snmp/library/int64.h
./route.lo: ../../include/net-snmp/library/mt_support.h
./route.lo: ../../include/net-snmp/library/snmp_alarm.h
./route.lo: ../../include/net-snmp/library/data_list.h
./route.lo: ../../include/net-snmp/library/callback.h
./route.lo: ../../include/net-snmp/library/oid_stash.h
./route.lo: ../../include/net-snmp/library/check_varbind.h
./route.lo: ../../include/net-snmp/version.h
./route.lo: ../../include/net-snmp/session_api.h
./route.lo: ../../include/net-snmp/library/snmp_transport.h
./route.lo: ../../include/net-snmp/library/snmpCallbackDomain.h
./route.lo: ../../include/net-snmp/library/snmpUnixDomain.h
./route.lo: ../../include/net-snmp/library/snmpUDPDomain.h
./route.lo: ../../include/net-snmp/library/snmpTCPDomain.h
./route.lo: ../../include/net-snmp/library/snmpUDPIPv6Domain.h
./route.lo: ../../include/net-snmp/library/snmpTCPIPv6Domain.h
./route.lo: ../../include/net-snmp/library/snmpIPXDomain.h
./route.lo: ../../include/net-snmp/library/ucd_compat.h
./route.lo: ../../include/net-snmp/pdu_api.h ../../include/net-snmp/mib_api.h
./route.lo: ../../include/net-snmp/library/mib.h
./route.lo: ../../include/net-snmp/library/parse.h
./route.lo: ../../include/net-snmp/varbind_api.h
./route.lo: ../../include/net-snmp/config_api.h
./route.lo: ../../include/net-snmp/library/read_config.h
./route.lo: ../../include/net-snmp/library/default_store.h
./route.lo: ../../include/net-snmp/library/snmp_parse_args.h
./route.lo: ../../include/net-snmp/library/snmp_enum.h
./route.lo: ../../include/net-snmp/library/vacm.h
./route.lo: ../../include/net-snmp/output_api.h
./route.lo: ../../include/net-snmp/library/snmp_debug.h
./route.lo: ../../include/net-snmp/library/snmp_logging.h
./route.lo: ../../include/net-snmp/snmpv3_api.h
./route.lo: ../../include/net-snmp/library/snmpv3.h
./route.lo: ../../include/net-snmp/library/transform_oids.h
./route.lo: ../../include/net-snmp/library/keytools.h
./route.lo: ../../include/net-snmp/library/scapi.h
./route.lo: ../../include/net-snmp/library/lcd_time.h
./route.lo: ../../include/net-snmp/library/snmp_secmod.h
./route.lo: ../../include/net-snmp/library/snmpv3-security-includes.h
./route.lo: ../../include/net-snmp/library/snmpusm.h netstat.h
./winstub.lo: ../../include/net-snmp/net-snmp-config.h
./winstub.lo: ../../include/net-snmp/system/linux.h
./winstub.lo: ../../include/net-snmp/system/sysv.h
./winstub.lo: ../../include/net-snmp/system/generic.h
./winstub.lo: ../../include/net-snmp/machine/generic.h
