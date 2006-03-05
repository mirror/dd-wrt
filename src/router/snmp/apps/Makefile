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
SED		= /home/dd-wrt/whiterussian/openwrt/staging_dir_mipsel/bin//sed
LN_S		= ln -s
AUTOCONF	= :
AUTOHEADER	= :

#
# Compiler arguments
#
CFLAGS		= -Os -mtune=mips32 -mips32 -funit-at-a-time -Delf 
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
# Makefile for snmpget, snmpwalk, snmpbulkwalk, snmptest, snmptranslate,
# snmptrapd, snmptable, snmpset, snmpgetnext, and other utilities.
#

top_builddir=..

#
# Things to install
#

INSTALLBINPROGS	= snmpget$(EXEEXT) 			\
		snmpgetnext$(EXEEXT) 			\
		snmpset$(EXEEXT) 			\
		snmpusm$(EXEEXT)			\
		snmpwalk$(EXEEXT) 			\
		snmpbulkwalk$(EXEEXT) 			\
		snmptable$(EXEEXT)			\
		snmptrap$(EXEEXT) 			\
		snmpbulkget$(EXEEXT)			\
		snmptranslate$(EXEEXT) 			\
		snmpstatus$(EXEEXT) 			\
		snmpdelta$(EXEEXT) 			\
		snmptest$(EXEEXT)			\
		encode_keychange$(EXEEXT) 		\
		snmpdf$(EXEEXT) 			\
		snmpvacm$(EXEEXT)

INSTALLSBINPROGS = snmptrapd$(EXEEXT)

SUBDIRS		= snmpnetstat

#
# build variables.
#

USELIBS		= ../snmplib/libnetsnmp.$(LIB_EXTENSION)$(LIB_VERSION) 
USETRAPLIBS	= ../agent/libnetsnmpagent.$(LIB_EXTENSION)$(LIB_VERSION) ../agent/libnetsnmpmibs.$(LIB_EXTENSION)$(LIB_VERSION) ../agent/helpers/libnetsnmphelpers.$(LIB_EXTENSION)$(LIB_VERSION) $(USELIBS)
LOCAL_LIBS	= -L../snmplib -L../agent -L../agent/helpers
LIBS		= $(USELIBS) -lm 
TRAPLIBS	= $(USETRAPLIBS) -ldl -lm 

CPPFLAGS	= -I$(top_srcdir)/include -I../include \
		  -I.. -I$(srcdir)/.. -I$(srcdir)/../snmplib \
		   -I$(srcdir)/../agent -I../agent/helpers \
		  -I$(srcdir)/../agent/mibgroup
OSUFFIX		= lo
TRAPD_OBJECTS   = snmptrapd.$(OSUFFIX) snmptrapd_handlers.$(OSUFFIX) snmptrapd_log.$(OSUFFIX) notification_log.$(OSUFFIX)
OBJS  = *.o
LOBJS = *.lo

all: standardall

OTHERINSTALL=snmpinforminstall

#
# build rules
#
snmpwalk$(EXEEXT):    snmpwalk.$(OSUFFIX) $(USELIBS)
	$(LINK) -o $@ snmpwalk.$(OSUFFIX) $(LOCAL_LIBS) ${LDFLAGS} ${LIBS} 

snmpbulkwalk$(EXEEXT):    snmpbulkwalk.$(OSUFFIX) $(USELIBS)
	$(LINK) -o $@ snmpbulkwalk.$(OSUFFIX) $(LOCAL_LIBS) ${LDFLAGS} ${LIBS} 

snmpbulkget$(EXEEXT):    snmpbulkget.$(OSUFFIX) $(USELIBS)
	$(LINK) -o $@ snmpbulkget.$(OSUFFIX) $(LOCAL_LIBS) ${LDFLAGS} ${LIBS} 

snmptranslate$(EXEEXT):    snmptranslate.$(OSUFFIX) $(USELIBS)
	$(LINK) -o $@ snmptranslate.$(OSUFFIX) $(LOCAL_LIBS) ${LDFLAGS} ${LIBS} 

snmpstatus$(EXEEXT):    snmpstatus.$(OSUFFIX) $(USELIBS)
	$(LINK) -o $@ snmpstatus.$(OSUFFIX) $(LOCAL_LIBS) ${LDFLAGS} ${LIBS} 

snmpget$(EXEEXT):    snmpget.$(OSUFFIX) $(USELIBS)
	$(LINK) -o $@ snmpget.$(OSUFFIX) $(LOCAL_LIBS) ${LDFLAGS} ${LIBS} 

snmpdelta$(EXEEXT):    snmpdelta.$(OSUFFIX) $(USELIBS)
	$(LINK) -o $@ snmpdelta.$(OSUFFIX) $(LOCAL_LIBS) ${LDFLAGS} ${LIBS} 

snmptable$(EXEEXT):    snmptable.$(OSUFFIX) $(USELIBS)
	$(LINK) -o $@ snmptable.$(OSUFFIX) $(LOCAL_LIBS) ${LDFLAGS} ${LIBS} 

snmptest$(EXEEXT):    snmptest.$(OSUFFIX) $(USELIBS)
	$(LINK) -o $@ snmptest.$(OSUFFIX) $(LOCAL_LIBS) ${LDFLAGS} ${LIBS} 

snmptrapd$(EXEEXT):    $(TRAPD_OBJECTS) $(USETRAPLIBS) 
	$(LINK) -o $@ $(TRAPD_OBJECTS) $(LOCAL_LIBS) ${LDFLAGS} ${TRAPLIBS} 

snmptrap$(EXEEXT):    snmptrap.$(OSUFFIX) $(USELIBS)
	$(LINK) -o $@ snmptrap.$(OSUFFIX) $(LOCAL_LIBS) ${LDFLAGS} ${LIBS} 

snmpinform$(EXEEXT): snmptrap$(EXEEXT)
	rm -f snmpinform
	$(LN_S) snmptrap$(EXEEXT) snmpinform$(EXEEXT)

snmpset$(EXEEXT):    snmpset.$(OSUFFIX) $(USELIBS)
	$(LINK) -o $@ snmpset.$(OSUFFIX) $(LOCAL_LIBS) ${LDFLAGS} ${LIBS} 

snmpusm$(EXEEXT):    snmpusm.$(OSUFFIX) $(USELIBS)
	$(LINK) -o $@ snmpusm.$(OSUFFIX) $(LOCAL_LIBS) ${LDFLAGS} ${LIBS} 

snmpvacm$(EXEEXT):    snmpvacm.$(OSUFFIX) $(USELIBS)
	$(LINK) -o $@ snmpvacm.$(OSUFFIX) $(LOCAL_LIBS) ${LDFLAGS} ${LIBS} 

snmpgetnext$(EXEEXT):    snmpgetnext.$(OSUFFIX) $(USELIBS)
	$(LINK) -o $@ snmpgetnext.$(OSUFFIX) $(LOCAL_LIBS) ${LDFLAGS} ${LIBS} 

encode_keychange$(EXEEXT):    encode_keychange.$(OSUFFIX) $(USELIBS)
	$(LINK) -o $@ encode_keychange.$(OSUFFIX) $(LOCAL_LIBS) ${LDFLAGS} ${LIBS} 

snmpdf$(EXEEXT):    snmpdf.$(OSUFFIX) $(USELIBS)
	$(LINK) -o $@ snmpdf.$(OSUFFIX) $(LOCAL_LIBS) ${LDFLAGS} ${LIBS} 

snmpinforminstall:
	rm -f $(bindir)/snmpinform$(EXEEXT)
	$(LN_S) snmptrap$(EXEEXT) $(bindir)/snmpinform$(EXEEXT)

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

./encode_keychange.lo: ../include/net-snmp/net-snmp-config.h
./encode_keychange.lo: ../include/net-snmp/system/linux.h
./encode_keychange.lo: ../include/net-snmp/system/sysv.h
./encode_keychange.lo: ../include/net-snmp/system/generic.h
./encode_keychange.lo: ../include/net-snmp/machine/generic.h
./encode_keychange.lo: ../include/net-snmp/net-snmp-includes.h
./encode_keychange.lo: ../include/net-snmp/definitions.h
./encode_keychange.lo: ../include/net-snmp/types.h
./encode_keychange.lo: ../include/net-snmp/library/snmp_api.h
./encode_keychange.lo: ../include/net-snmp/library/asn1.h
./encode_keychange.lo: ../include/net-snmp/library/snmp_impl.h
./encode_keychange.lo: ../include/net-snmp/library/snmp.h
./encode_keychange.lo: ../include/net-snmp/library/snmp-tc.h
./encode_keychange.lo: ../include/net-snmp/utilities.h
./encode_keychange.lo: ../include/net-snmp/library/snmp_client.h
./encode_keychange.lo: ../include/net-snmp/library/getopt.h
./encode_keychange.lo: ../include/net-snmp/library/system.h
./encode_keychange.lo: ../include/net-snmp/library/tools.h
./encode_keychange.lo: ../include/net-snmp/library/int64.h
./encode_keychange.lo: ../include/net-snmp/library/mt_support.h
./encode_keychange.lo: ../include/net-snmp/library/snmp_alarm.h
./encode_keychange.lo: ../include/net-snmp/library/data_list.h
./encode_keychange.lo: ../include/net-snmp/library/callback.h
./encode_keychange.lo: ../include/net-snmp/library/oid_stash.h
./encode_keychange.lo: ../include/net-snmp/library/check_varbind.h
./encode_keychange.lo: ../include/net-snmp/version.h
./encode_keychange.lo: ../include/net-snmp/session_api.h
./encode_keychange.lo: ../include/net-snmp/library/snmp_transport.h
./encode_keychange.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./encode_keychange.lo: ../include/net-snmp/library/snmpUnixDomain.h
./encode_keychange.lo: ../include/net-snmp/library/snmpUDPDomain.h
./encode_keychange.lo: ../include/net-snmp/library/snmpTCPDomain.h
./encode_keychange.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./encode_keychange.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./encode_keychange.lo: ../include/net-snmp/library/snmpIPXDomain.h
./encode_keychange.lo: ../include/net-snmp/library/ucd_compat.h
./encode_keychange.lo: ../include/net-snmp/pdu_api.h
./encode_keychange.lo: ../include/net-snmp/mib_api.h
./encode_keychange.lo: ../include/net-snmp/library/mib.h
./encode_keychange.lo: ../include/net-snmp/library/parse.h
./encode_keychange.lo: ../include/net-snmp/varbind_api.h
./encode_keychange.lo: ../include/net-snmp/config_api.h
./encode_keychange.lo: ../include/net-snmp/library/read_config.h
./encode_keychange.lo: ../include/net-snmp/library/default_store.h
./encode_keychange.lo: ../include/net-snmp/library/snmp_parse_args.h
./encode_keychange.lo: ../include/net-snmp/library/snmp_enum.h
./encode_keychange.lo: ../include/net-snmp/library/vacm.h
./encode_keychange.lo: ../include/net-snmp/output_api.h
./encode_keychange.lo: ../include/net-snmp/library/snmp_debug.h
./encode_keychange.lo: ../include/net-snmp/library/snmp_logging.h
./encode_keychange.lo: ../include/net-snmp/snmpv3_api.h
./encode_keychange.lo: ../include/net-snmp/library/snmpv3.h
./encode_keychange.lo: ../include/net-snmp/library/transform_oids.h
./encode_keychange.lo: ../include/net-snmp/library/keytools.h
./encode_keychange.lo: ../include/net-snmp/library/scapi.h
./encode_keychange.lo: ../include/net-snmp/library/lcd_time.h
./encode_keychange.lo: ../include/net-snmp/library/snmp_secmod.h
./encode_keychange.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./encode_keychange.lo: ../include/net-snmp/library/snmpusm.h
./notification_log.lo: ../include/net-snmp/net-snmp-config.h
./notification_log.lo: ../include/net-snmp/system/linux.h
./notification_log.lo: ../include/net-snmp/system/sysv.h
./notification_log.lo: ../include/net-snmp/system/generic.h
./notification_log.lo: ../include/net-snmp/machine/generic.h
./notification_log.lo: ../include/net-snmp/net-snmp-includes.h
./notification_log.lo: ../include/net-snmp/definitions.h
./notification_log.lo: ../include/net-snmp/types.h
./notification_log.lo: ../include/net-snmp/library/snmp_api.h
./notification_log.lo: ../include/net-snmp/library/asn1.h
./notification_log.lo: ../include/net-snmp/library/snmp_impl.h
./notification_log.lo: ../include/net-snmp/library/snmp.h
./notification_log.lo: ../include/net-snmp/library/snmp-tc.h
./notification_log.lo: ../include/net-snmp/utilities.h
./notification_log.lo: ../include/net-snmp/library/snmp_client.h
./notification_log.lo: ../include/net-snmp/library/getopt.h
./notification_log.lo: ../include/net-snmp/library/system.h
./notification_log.lo: ../include/net-snmp/library/tools.h
./notification_log.lo: ../include/net-snmp/library/int64.h
./notification_log.lo: ../include/net-snmp/library/mt_support.h
./notification_log.lo: ../include/net-snmp/library/snmp_alarm.h
./notification_log.lo: ../include/net-snmp/library/data_list.h
./notification_log.lo: ../include/net-snmp/library/callback.h
./notification_log.lo: ../include/net-snmp/library/oid_stash.h
./notification_log.lo: ../include/net-snmp/library/check_varbind.h
./notification_log.lo: ../include/net-snmp/version.h
./notification_log.lo: ../include/net-snmp/session_api.h
./notification_log.lo: ../include/net-snmp/library/snmp_transport.h
./notification_log.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./notification_log.lo: ../include/net-snmp/library/snmpUnixDomain.h
./notification_log.lo: ../include/net-snmp/library/snmpUDPDomain.h
./notification_log.lo: ../include/net-snmp/library/snmpTCPDomain.h
./notification_log.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./notification_log.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./notification_log.lo: ../include/net-snmp/library/snmpIPXDomain.h
./notification_log.lo: ../include/net-snmp/library/ucd_compat.h
./notification_log.lo: ../include/net-snmp/pdu_api.h
./notification_log.lo: ../include/net-snmp/mib_api.h
./notification_log.lo: ../include/net-snmp/library/mib.h
./notification_log.lo: ../include/net-snmp/library/parse.h
./notification_log.lo: ../include/net-snmp/varbind_api.h
./notification_log.lo: ../include/net-snmp/config_api.h
./notification_log.lo: ../include/net-snmp/library/read_config.h
./notification_log.lo: ../include/net-snmp/library/default_store.h
./notification_log.lo: ../include/net-snmp/library/snmp_parse_args.h
./notification_log.lo: ../include/net-snmp/library/snmp_enum.h
./notification_log.lo: ../include/net-snmp/library/vacm.h
./notification_log.lo: ../include/net-snmp/output_api.h
./notification_log.lo: ../include/net-snmp/library/snmp_debug.h
./notification_log.lo: ../include/net-snmp/library/snmp_logging.h
./notification_log.lo: ../include/net-snmp/snmpv3_api.h
./notification_log.lo: ../include/net-snmp/library/snmpv3.h
./notification_log.lo: ../include/net-snmp/library/transform_oids.h
./notification_log.lo: ../include/net-snmp/library/keytools.h
./notification_log.lo: ../include/net-snmp/library/scapi.h
./notification_log.lo: ../include/net-snmp/library/lcd_time.h
./notification_log.lo: ../include/net-snmp/library/snmp_secmod.h
./notification_log.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./notification_log.lo: ../include/net-snmp/library/snmpusm.h
./notification_log.lo: ../include/net-snmp/agent/net-snmp-agent-includes.h
./notification_log.lo: ../include/net-snmp/agent/snmp_agent.h
./notification_log.lo: ../include/net-snmp/agent/snmp_vars.h
./notification_log.lo: ../include/net-snmp/agent/agent_handler.h
./notification_log.lo: ../include/net-snmp/agent/var_struct.h
./notification_log.lo: ../include/net-snmp/agent/agent_registry.h
./notification_log.lo: ../include/net-snmp/agent/ds_agent.h
./notification_log.lo: ../include/net-snmp/agent/agent_read_config.h
./notification_log.lo: ../include/net-snmp/agent/agent_trap.h
./notification_log.lo: ../include/net-snmp/agent/all_helpers.h
./notification_log.lo: ../include/net-snmp/agent/instance.h
./notification_log.lo: ../include/net-snmp/agent/multiplexer.h
./notification_log.lo: ../include/net-snmp/agent/null.h
./notification_log.lo: ../include/net-snmp/agent/debug_handler.h
./notification_log.lo: ../include/net-snmp/agent/old_api.h
./notification_log.lo: ../include/net-snmp/agent/read_only.h
./notification_log.lo: ../include/net-snmp/agent/serialize.h
./notification_log.lo: ../include/net-snmp/agent/bulk_to_next.h
./notification_log.lo: ../include/net-snmp/agent/mode_end_call.h
./notification_log.lo: ../include/net-snmp/agent/table.h
./notification_log.lo: ../include/net-snmp/agent/table_data.h
./notification_log.lo: ../include/net-snmp/agent/table_dataset.h
./notification_log.lo: ../include/net-snmp/agent/table_iterator.h
./notification_log.lo: ../include/net-snmp/agent/mib_module_config.h
./notification_log.lo: notification_log.h
./snmpbulkget.lo: ../include/net-snmp/net-snmp-config.h
./snmpbulkget.lo: ../include/net-snmp/system/linux.h
./snmpbulkget.lo: ../include/net-snmp/system/sysv.h
./snmpbulkget.lo: ../include/net-snmp/system/generic.h
./snmpbulkget.lo: ../include/net-snmp/machine/generic.h 
./snmpbulkget.lo:  ../include/net-snmp/utilities.h
./snmpbulkget.lo: ../include/net-snmp/types.h 
./snmpbulkget.lo: ../include/net-snmp/definitions.h
./snmpbulkget.lo: ../include/net-snmp/library/snmp_api.h
./snmpbulkget.lo: ../include/net-snmp/library/asn1.h
./snmpbulkget.lo: ../include/net-snmp/library/snmp_impl.h
./snmpbulkget.lo: ../include/net-snmp/library/snmp.h
./snmpbulkget.lo: ../include/net-snmp/library/snmp-tc.h
./snmpbulkget.lo: ../include/net-snmp/library/snmp_client.h
./snmpbulkget.lo: ../include/net-snmp/library/getopt.h
./snmpbulkget.lo: ../include/net-snmp/library/system.h
./snmpbulkget.lo: ../include/net-snmp/library/tools.h
./snmpbulkget.lo: ../include/net-snmp/library/int64.h
./snmpbulkget.lo: ../include/net-snmp/library/mt_support.h
./snmpbulkget.lo: ../include/net-snmp/library/snmp_alarm.h
./snmpbulkget.lo: ../include/net-snmp/library/data_list.h
./snmpbulkget.lo: ../include/net-snmp/library/callback.h
./snmpbulkget.lo: ../include/net-snmp/library/oid_stash.h
./snmpbulkget.lo: ../include/net-snmp/library/check_varbind.h
./snmpbulkget.lo: ../include/net-snmp/version.h 
./snmpbulkget.lo: ../include/net-snmp/net-snmp-includes.h
./snmpbulkget.lo: ../include/net-snmp/session_api.h
./snmpbulkget.lo: ../include/net-snmp/library/snmp_transport.h
./snmpbulkget.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmpbulkget.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmpbulkget.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmpbulkget.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmpbulkget.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmpbulkget.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmpbulkget.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmpbulkget.lo: ../include/net-snmp/library/ucd_compat.h
./snmpbulkget.lo: ../include/net-snmp/pdu_api.h ../include/net-snmp/mib_api.h
./snmpbulkget.lo: ../include/net-snmp/library/mib.h
./snmpbulkget.lo: ../include/net-snmp/library/parse.h
./snmpbulkget.lo: ../include/net-snmp/varbind_api.h
./snmpbulkget.lo: ../include/net-snmp/config_api.h
./snmpbulkget.lo: ../include/net-snmp/library/read_config.h
./snmpbulkget.lo: ../include/net-snmp/library/default_store.h
./snmpbulkget.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpbulkget.lo: ../include/net-snmp/library/snmp_enum.h
./snmpbulkget.lo: ../include/net-snmp/library/vacm.h
./snmpbulkget.lo: ../include/net-snmp/output_api.h
./snmpbulkget.lo: ../include/net-snmp/library/snmp_debug.h
./snmpbulkget.lo: ../include/net-snmp/library/snmp_logging.h
./snmpbulkget.lo: ../include/net-snmp/snmpv3_api.h
./snmpbulkget.lo: ../include/net-snmp/library/snmpv3.h
./snmpbulkget.lo: ../include/net-snmp/library/transform_oids.h
./snmpbulkget.lo: ../include/net-snmp/library/keytools.h
./snmpbulkget.lo: ../include/net-snmp/library/scapi.h
./snmpbulkget.lo: ../include/net-snmp/library/lcd_time.h
./snmpbulkget.lo: ../include/net-snmp/library/snmp_secmod.h
./snmpbulkget.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./snmpbulkget.lo: ../include/net-snmp/library/snmpusm.h
./snmpbulkwalk.lo: ../include/net-snmp/net-snmp-config.h
./snmpbulkwalk.lo: ../include/net-snmp/system/linux.h
./snmpbulkwalk.lo: ../include/net-snmp/system/sysv.h
./snmpbulkwalk.lo: ../include/net-snmp/system/generic.h
./snmpbulkwalk.lo: ../include/net-snmp/machine/generic.h
./snmpbulkwalk.lo: ../include/net-snmp/net-snmp-includes.h
./snmpbulkwalk.lo: ../include/net-snmp/definitions.h
./snmpbulkwalk.lo: ../include/net-snmp/types.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmp_api.h
./snmpbulkwalk.lo: ../include/net-snmp/library/asn1.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmp_impl.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmp.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmp-tc.h
./snmpbulkwalk.lo: ../include/net-snmp/utilities.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmp_client.h
./snmpbulkwalk.lo: ../include/net-snmp/library/getopt.h
./snmpbulkwalk.lo: ../include/net-snmp/library/system.h
./snmpbulkwalk.lo: ../include/net-snmp/library/tools.h
./snmpbulkwalk.lo: ../include/net-snmp/library/int64.h
./snmpbulkwalk.lo: ../include/net-snmp/library/mt_support.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmp_alarm.h
./snmpbulkwalk.lo: ../include/net-snmp/library/data_list.h
./snmpbulkwalk.lo: ../include/net-snmp/library/callback.h
./snmpbulkwalk.lo: ../include/net-snmp/library/oid_stash.h
./snmpbulkwalk.lo: ../include/net-snmp/library/check_varbind.h
./snmpbulkwalk.lo: ../include/net-snmp/version.h
./snmpbulkwalk.lo: ../include/net-snmp/session_api.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmp_transport.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmpbulkwalk.lo: ../include/net-snmp/library/ucd_compat.h
./snmpbulkwalk.lo: ../include/net-snmp/pdu_api.h
./snmpbulkwalk.lo: ../include/net-snmp/mib_api.h
./snmpbulkwalk.lo: ../include/net-snmp/library/mib.h
./snmpbulkwalk.lo: ../include/net-snmp/library/parse.h
./snmpbulkwalk.lo: ../include/net-snmp/varbind_api.h
./snmpbulkwalk.lo: ../include/net-snmp/config_api.h
./snmpbulkwalk.lo: ../include/net-snmp/library/read_config.h
./snmpbulkwalk.lo: ../include/net-snmp/library/default_store.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmp_enum.h
./snmpbulkwalk.lo: ../include/net-snmp/library/vacm.h
./snmpbulkwalk.lo: ../include/net-snmp/output_api.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmp_debug.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmp_logging.h
./snmpbulkwalk.lo: ../include/net-snmp/snmpv3_api.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmpv3.h
./snmpbulkwalk.lo: ../include/net-snmp/library/transform_oids.h
./snmpbulkwalk.lo: ../include/net-snmp/library/keytools.h
./snmpbulkwalk.lo: ../include/net-snmp/library/scapi.h
./snmpbulkwalk.lo: ../include/net-snmp/library/lcd_time.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmp_secmod.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./snmpbulkwalk.lo: ../include/net-snmp/library/snmpusm.h
./snmpdelta.lo: ../include/net-snmp/net-snmp-config.h
./snmpdelta.lo: ../include/net-snmp/system/linux.h
./snmpdelta.lo: ../include/net-snmp/system/sysv.h
./snmpdelta.lo: ../include/net-snmp/system/generic.h
./snmpdelta.lo: ../include/net-snmp/machine/generic.h 
./snmpdelta.lo: ../include/net-snmp/net-snmp-includes.h
./snmpdelta.lo: ../include/net-snmp/definitions.h ../include/net-snmp/types.h
./snmpdelta.lo: ../include/net-snmp/library/snmp_api.h
./snmpdelta.lo: ../include/net-snmp/library/asn1.h
./snmpdelta.lo: ../include/net-snmp/library/snmp_impl.h
./snmpdelta.lo: ../include/net-snmp/library/snmp.h
./snmpdelta.lo: ../include/net-snmp/library/snmp-tc.h
./snmpdelta.lo: ../include/net-snmp/utilities.h
./snmpdelta.lo: ../include/net-snmp/library/snmp_client.h
./snmpdelta.lo: ../include/net-snmp/library/getopt.h
./snmpdelta.lo: ../include/net-snmp/library/system.h
./snmpdelta.lo: ../include/net-snmp/library/tools.h
./snmpdelta.lo: ../include/net-snmp/library/int64.h
./snmpdelta.lo: ../include/net-snmp/library/mt_support.h
./snmpdelta.lo: ../include/net-snmp/library/snmp_alarm.h
./snmpdelta.lo: ../include/net-snmp/library/data_list.h
./snmpdelta.lo: ../include/net-snmp/library/callback.h
./snmpdelta.lo: ../include/net-snmp/library/oid_stash.h
./snmpdelta.lo: ../include/net-snmp/library/check_varbind.h
./snmpdelta.lo: ../include/net-snmp/version.h
./snmpdelta.lo: ../include/net-snmp/session_api.h
./snmpdelta.lo: ../include/net-snmp/library/snmp_transport.h
./snmpdelta.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmpdelta.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmpdelta.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmpdelta.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmpdelta.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmpdelta.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmpdelta.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmpdelta.lo: ../include/net-snmp/library/ucd_compat.h
./snmpdelta.lo: ../include/net-snmp/pdu_api.h ../include/net-snmp/mib_api.h
./snmpdelta.lo: ../include/net-snmp/library/mib.h
./snmpdelta.lo: ../include/net-snmp/library/parse.h
./snmpdelta.lo: ../include/net-snmp/varbind_api.h
./snmpdelta.lo: ../include/net-snmp/config_api.h
./snmpdelta.lo: ../include/net-snmp/library/read_config.h
./snmpdelta.lo: ../include/net-snmp/library/default_store.h
./snmpdelta.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpdelta.lo: ../include/net-snmp/library/snmp_enum.h
./snmpdelta.lo: ../include/net-snmp/library/vacm.h
./snmpdelta.lo: ../include/net-snmp/output_api.h
./snmpdelta.lo: ../include/net-snmp/library/snmp_debug.h
./snmpdelta.lo: ../include/net-snmp/library/snmp_logging.h
./snmpdelta.lo: ../include/net-snmp/snmpv3_api.h
./snmpdelta.lo: ../include/net-snmp/library/snmpv3.h
./snmpdelta.lo: ../include/net-snmp/library/transform_oids.h
./snmpdelta.lo: ../include/net-snmp/library/keytools.h
./snmpdelta.lo: ../include/net-snmp/library/scapi.h
./snmpdelta.lo: ../include/net-snmp/library/lcd_time.h
./snmpdelta.lo: ../include/net-snmp/library/snmp_secmod.h
./snmpdelta.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./snmpdelta.lo: ../include/net-snmp/library/snmpusm.h
./snmpdf.lo: ../include/net-snmp/net-snmp-config.h
./snmpdf.lo: ../include/net-snmp/system/linux.h
./snmpdf.lo: ../include/net-snmp/system/sysv.h
./snmpdf.lo: ../include/net-snmp/system/generic.h
./snmpdf.lo: ../include/net-snmp/machine/generic.h 
./snmpdf.lo: ../include/net-snmp/net-snmp-includes.h
./snmpdf.lo: ../include/net-snmp/definitions.h ../include/net-snmp/types.h
./snmpdf.lo: ../include/net-snmp/library/snmp_api.h
./snmpdf.lo: ../include/net-snmp/library/asn1.h
./snmpdf.lo: ../include/net-snmp/library/snmp_impl.h
./snmpdf.lo: ../include/net-snmp/library/snmp.h
./snmpdf.lo: ../include/net-snmp/library/snmp-tc.h
./snmpdf.lo: ../include/net-snmp/utilities.h
./snmpdf.lo: ../include/net-snmp/library/snmp_client.h
./snmpdf.lo: ../include/net-snmp/library/getopt.h
./snmpdf.lo: ../include/net-snmp/library/system.h
./snmpdf.lo: ../include/net-snmp/library/tools.h
./snmpdf.lo: ../include/net-snmp/library/int64.h
./snmpdf.lo: ../include/net-snmp/library/mt_support.h
./snmpdf.lo: ../include/net-snmp/library/snmp_alarm.h
./snmpdf.lo: ../include/net-snmp/library/data_list.h
./snmpdf.lo: ../include/net-snmp/library/callback.h
./snmpdf.lo: ../include/net-snmp/library/oid_stash.h
./snmpdf.lo: ../include/net-snmp/library/check_varbind.h
./snmpdf.lo: ../include/net-snmp/version.h ../include/net-snmp/session_api.h
./snmpdf.lo: ../include/net-snmp/library/snmp_transport.h
./snmpdf.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmpdf.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmpdf.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmpdf.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmpdf.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmpdf.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmpdf.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmpdf.lo: ../include/net-snmp/library/ucd_compat.h
./snmpdf.lo: ../include/net-snmp/pdu_api.h ../include/net-snmp/mib_api.h
./snmpdf.lo: ../include/net-snmp/library/mib.h
./snmpdf.lo: ../include/net-snmp/library/parse.h
./snmpdf.lo: ../include/net-snmp/varbind_api.h
./snmpdf.lo: ../include/net-snmp/config_api.h
./snmpdf.lo: ../include/net-snmp/library/read_config.h
./snmpdf.lo: ../include/net-snmp/library/default_store.h
./snmpdf.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpdf.lo: ../include/net-snmp/library/snmp_enum.h
./snmpdf.lo: ../include/net-snmp/library/vacm.h
./snmpdf.lo: ../include/net-snmp/output_api.h
./snmpdf.lo: ../include/net-snmp/library/snmp_debug.h
./snmpdf.lo: ../include/net-snmp/library/snmp_logging.h 
./snmpdf.lo:  ../include/net-snmp/snmpv3_api.h
./snmpdf.lo: ../include/net-snmp/library/snmpv3.h
./snmpdf.lo: ../include/net-snmp/library/transform_oids.h
./snmpdf.lo: ../include/net-snmp/library/keytools.h
./snmpdf.lo: ../include/net-snmp/library/scapi.h
./snmpdf.lo: ../include/net-snmp/library/lcd_time.h
./snmpdf.lo: ../include/net-snmp/library/snmp_secmod.h
./snmpdf.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./snmpdf.lo: ../include/net-snmp/library/snmpusm.h
./snmpget.lo: ../include/net-snmp/net-snmp-config.h
./snmpget.lo: ../include/net-snmp/system/linux.h
./snmpget.lo: ../include/net-snmp/system/sysv.h
./snmpget.lo: ../include/net-snmp/system/generic.h
./snmpget.lo: ../include/net-snmp/machine/generic.h 
./snmpget.lo: ../include/net-snmp/utilities.h ../include/net-snmp/types.h
./snmpget.lo: ../include/net-snmp/definitions.h
./snmpget.lo: ../include/net-snmp/library/snmp_api.h
./snmpget.lo: ../include/net-snmp/library/asn1.h
./snmpget.lo: ../include/net-snmp/library/snmp_impl.h
./snmpget.lo: ../include/net-snmp/library/snmp.h
./snmpget.lo: ../include/net-snmp/library/snmp-tc.h
./snmpget.lo: ../include/net-snmp/library/snmp_client.h
./snmpget.lo: ../include/net-snmp/library/getopt.h
./snmpget.lo: ../include/net-snmp/library/system.h
./snmpget.lo: ../include/net-snmp/library/tools.h
./snmpget.lo: ../include/net-snmp/library/int64.h
./snmpget.lo: ../include/net-snmp/library/mt_support.h
./snmpget.lo: ../include/net-snmp/library/snmp_alarm.h
./snmpget.lo: ../include/net-snmp/library/data_list.h
./snmpget.lo: ../include/net-snmp/library/callback.h
./snmpget.lo: ../include/net-snmp/library/oid_stash.h
./snmpget.lo: ../include/net-snmp/library/check_varbind.h
./snmpget.lo: ../include/net-snmp/version.h
./snmpget.lo: ../include/net-snmp/net-snmp-includes.h
./snmpget.lo: ../include/net-snmp/session_api.h
./snmpget.lo: ../include/net-snmp/library/snmp_transport.h
./snmpget.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmpget.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmpget.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmpget.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmpget.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmpget.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmpget.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmpget.lo: ../include/net-snmp/library/ucd_compat.h
./snmpget.lo: ../include/net-snmp/pdu_api.h ../include/net-snmp/mib_api.h
./snmpget.lo: ../include/net-snmp/library/mib.h
./snmpget.lo: ../include/net-snmp/library/parse.h
./snmpget.lo: ../include/net-snmp/varbind_api.h
./snmpget.lo: ../include/net-snmp/config_api.h
./snmpget.lo: ../include/net-snmp/library/read_config.h
./snmpget.lo: ../include/net-snmp/library/default_store.h
./snmpget.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpget.lo: ../include/net-snmp/library/snmp_enum.h
./snmpget.lo: ../include/net-snmp/library/vacm.h
./snmpget.lo: ../include/net-snmp/output_api.h
./snmpget.lo: ../include/net-snmp/library/snmp_debug.h
./snmpget.lo: ../include/net-snmp/library/snmp_logging.h
./snmpget.lo: ../include/net-snmp/snmpv3_api.h
./snmpget.lo: ../include/net-snmp/library/snmpv3.h
./snmpget.lo: ../include/net-snmp/library/transform_oids.h
./snmpget.lo: ../include/net-snmp/library/keytools.h
./snmpget.lo: ../include/net-snmp/library/scapi.h
./snmpget.lo: ../include/net-snmp/library/lcd_time.h
./snmpget.lo: ../include/net-snmp/library/snmp_secmod.h
./snmpget.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./snmpget.lo: ../include/net-snmp/library/snmpusm.h
./snmpgetnext.lo: ../include/net-snmp/net-snmp-config.h
./snmpgetnext.lo: ../include/net-snmp/system/linux.h
./snmpgetnext.lo: ../include/net-snmp/system/sysv.h
./snmpgetnext.lo: ../include/net-snmp/system/generic.h
./snmpgetnext.lo: ../include/net-snmp/machine/generic.h 
./snmpgetnext.lo: ../include/net-snmp/net-snmp-includes.h
./snmpgetnext.lo: ../include/net-snmp/definitions.h
./snmpgetnext.lo: ../include/net-snmp/types.h
./snmpgetnext.lo: ../include/net-snmp/library/snmp_api.h
./snmpgetnext.lo: ../include/net-snmp/library/asn1.h
./snmpgetnext.lo: ../include/net-snmp/library/snmp_impl.h
./snmpgetnext.lo: ../include/net-snmp/library/snmp.h
./snmpgetnext.lo: ../include/net-snmp/library/snmp-tc.h
./snmpgetnext.lo: ../include/net-snmp/utilities.h
./snmpgetnext.lo: ../include/net-snmp/library/snmp_client.h
./snmpgetnext.lo: ../include/net-snmp/library/getopt.h
./snmpgetnext.lo: ../include/net-snmp/library/system.h
./snmpgetnext.lo: ../include/net-snmp/library/tools.h
./snmpgetnext.lo: ../include/net-snmp/library/int64.h
./snmpgetnext.lo: ../include/net-snmp/library/mt_support.h
./snmpgetnext.lo: ../include/net-snmp/library/snmp_alarm.h
./snmpgetnext.lo: ../include/net-snmp/library/data_list.h
./snmpgetnext.lo: ../include/net-snmp/library/callback.h
./snmpgetnext.lo: ../include/net-snmp/library/oid_stash.h
./snmpgetnext.lo: ../include/net-snmp/library/check_varbind.h
./snmpgetnext.lo: ../include/net-snmp/version.h
./snmpgetnext.lo: ../include/net-snmp/session_api.h
./snmpgetnext.lo: ../include/net-snmp/library/snmp_transport.h
./snmpgetnext.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmpgetnext.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmpgetnext.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmpgetnext.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmpgetnext.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmpgetnext.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmpgetnext.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmpgetnext.lo: ../include/net-snmp/library/ucd_compat.h
./snmpgetnext.lo: ../include/net-snmp/pdu_api.h ../include/net-snmp/mib_api.h
./snmpgetnext.lo: ../include/net-snmp/library/mib.h
./snmpgetnext.lo: ../include/net-snmp/library/parse.h
./snmpgetnext.lo: ../include/net-snmp/varbind_api.h
./snmpgetnext.lo: ../include/net-snmp/config_api.h
./snmpgetnext.lo: ../include/net-snmp/library/read_config.h
./snmpgetnext.lo: ../include/net-snmp/library/default_store.h
./snmpgetnext.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpgetnext.lo: ../include/net-snmp/library/snmp_enum.h
./snmpgetnext.lo: ../include/net-snmp/library/vacm.h
./snmpgetnext.lo: ../include/net-snmp/output_api.h
./snmpgetnext.lo: ../include/net-snmp/library/snmp_debug.h
./snmpgetnext.lo: ../include/net-snmp/library/snmp_logging.h
./snmpgetnext.lo: ../include/net-snmp/snmpv3_api.h
./snmpgetnext.lo: ../include/net-snmp/library/snmpv3.h
./snmpgetnext.lo: ../include/net-snmp/library/transform_oids.h
./snmpgetnext.lo: ../include/net-snmp/library/keytools.h
./snmpgetnext.lo: ../include/net-snmp/library/scapi.h
./snmpgetnext.lo: ../include/net-snmp/library/lcd_time.h
./snmpgetnext.lo: ../include/net-snmp/library/snmp_secmod.h
./snmpgetnext.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./snmpgetnext.lo: ../include/net-snmp/library/snmpusm.h
./snmpset.lo: ../include/net-snmp/net-snmp-config.h
./snmpset.lo: ../include/net-snmp/system/linux.h
./snmpset.lo: ../include/net-snmp/system/sysv.h
./snmpset.lo: ../include/net-snmp/system/generic.h
./snmpset.lo: ../include/net-snmp/machine/generic.h 
./snmpset.lo: ../include/net-snmp/net-snmp-includes.h
./snmpset.lo: ../include/net-snmp/definitions.h ../include/net-snmp/types.h
./snmpset.lo: ../include/net-snmp/library/snmp_api.h
./snmpset.lo: ../include/net-snmp/library/asn1.h
./snmpset.lo: ../include/net-snmp/library/snmp_impl.h
./snmpset.lo: ../include/net-snmp/library/snmp.h
./snmpset.lo: ../include/net-snmp/library/snmp-tc.h
./snmpset.lo: ../include/net-snmp/utilities.h
./snmpset.lo: ../include/net-snmp/library/snmp_client.h
./snmpset.lo: ../include/net-snmp/library/getopt.h
./snmpset.lo: ../include/net-snmp/library/system.h
./snmpset.lo: ../include/net-snmp/library/tools.h
./snmpset.lo: ../include/net-snmp/library/int64.h
./snmpset.lo: ../include/net-snmp/library/mt_support.h
./snmpset.lo: ../include/net-snmp/library/snmp_alarm.h
./snmpset.lo: ../include/net-snmp/library/data_list.h
./snmpset.lo: ../include/net-snmp/library/callback.h
./snmpset.lo: ../include/net-snmp/library/oid_stash.h
./snmpset.lo: ../include/net-snmp/library/check_varbind.h
./snmpset.lo: ../include/net-snmp/version.h ../include/net-snmp/session_api.h
./snmpset.lo: ../include/net-snmp/library/snmp_transport.h
./snmpset.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmpset.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmpset.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmpset.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmpset.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmpset.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmpset.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmpset.lo: ../include/net-snmp/library/ucd_compat.h
./snmpset.lo: ../include/net-snmp/pdu_api.h ../include/net-snmp/mib_api.h
./snmpset.lo: ../include/net-snmp/library/mib.h
./snmpset.lo: ../include/net-snmp/library/parse.h
./snmpset.lo: ../include/net-snmp/varbind_api.h
./snmpset.lo: ../include/net-snmp/config_api.h
./snmpset.lo: ../include/net-snmp/library/read_config.h
./snmpset.lo: ../include/net-snmp/library/default_store.h
./snmpset.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpset.lo: ../include/net-snmp/library/snmp_enum.h
./snmpset.lo: ../include/net-snmp/library/vacm.h
./snmpset.lo: ../include/net-snmp/output_api.h
./snmpset.lo: ../include/net-snmp/library/snmp_debug.h
./snmpset.lo: ../include/net-snmp/library/snmp_logging.h
./snmpset.lo: ../include/net-snmp/snmpv3_api.h
./snmpset.lo: ../include/net-snmp/library/snmpv3.h
./snmpset.lo: ../include/net-snmp/library/transform_oids.h
./snmpset.lo: ../include/net-snmp/library/keytools.h
./snmpset.lo: ../include/net-snmp/library/scapi.h
./snmpset.lo: ../include/net-snmp/library/lcd_time.h
./snmpset.lo: ../include/net-snmp/library/snmp_secmod.h
./snmpset.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./snmpset.lo: ../include/net-snmp/library/snmpusm.h
./snmpstatus.lo: ../include/net-snmp/net-snmp-config.h
./snmpstatus.lo: ../include/net-snmp/system/linux.h
./snmpstatus.lo: ../include/net-snmp/system/sysv.h
./snmpstatus.lo: ../include/net-snmp/system/generic.h
./snmpstatus.lo: ../include/net-snmp/machine/generic.h 
./snmpstatus.lo: ../include/net-snmp/utilities.h ../include/net-snmp/types.h
./snmpstatus.lo: ../include/net-snmp/definitions.h
./snmpstatus.lo: ../include/net-snmp/library/snmp_api.h
./snmpstatus.lo: ../include/net-snmp/library/asn1.h
./snmpstatus.lo: ../include/net-snmp/library/snmp_impl.h
./snmpstatus.lo: ../include/net-snmp/library/snmp.h
./snmpstatus.lo: ../include/net-snmp/library/snmp-tc.h
./snmpstatus.lo: ../include/net-snmp/library/snmp_client.h
./snmpstatus.lo: ../include/net-snmp/library/getopt.h
./snmpstatus.lo: ../include/net-snmp/library/system.h
./snmpstatus.lo: ../include/net-snmp/library/tools.h
./snmpstatus.lo: ../include/net-snmp/library/int64.h
./snmpstatus.lo: ../include/net-snmp/library/mt_support.h
./snmpstatus.lo: ../include/net-snmp/library/snmp_alarm.h
./snmpstatus.lo: ../include/net-snmp/library/data_list.h
./snmpstatus.lo: ../include/net-snmp/library/callback.h
./snmpstatus.lo: ../include/net-snmp/library/oid_stash.h
./snmpstatus.lo: ../include/net-snmp/library/check_varbind.h
./snmpstatus.lo: ../include/net-snmp/version.h
./snmpstatus.lo: ../include/net-snmp/net-snmp-includes.h
./snmpstatus.lo: ../include/net-snmp/session_api.h
./snmpstatus.lo: ../include/net-snmp/library/snmp_transport.h
./snmpstatus.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmpstatus.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmpstatus.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmpstatus.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmpstatus.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmpstatus.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmpstatus.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmpstatus.lo: ../include/net-snmp/library/ucd_compat.h
./snmpstatus.lo: ../include/net-snmp/pdu_api.h ../include/net-snmp/mib_api.h
./snmpstatus.lo: ../include/net-snmp/library/mib.h
./snmpstatus.lo: ../include/net-snmp/library/parse.h
./snmpstatus.lo: ../include/net-snmp/varbind_api.h
./snmpstatus.lo: ../include/net-snmp/config_api.h
./snmpstatus.lo: ../include/net-snmp/library/read_config.h
./snmpstatus.lo: ../include/net-snmp/library/default_store.h
./snmpstatus.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpstatus.lo: ../include/net-snmp/library/snmp_enum.h
./snmpstatus.lo: ../include/net-snmp/library/vacm.h
./snmpstatus.lo: ../include/net-snmp/output_api.h
./snmpstatus.lo: ../include/net-snmp/library/snmp_debug.h
./snmpstatus.lo: ../include/net-snmp/library/snmp_logging.h
./snmpstatus.lo: ../include/net-snmp/snmpv3_api.h
./snmpstatus.lo: ../include/net-snmp/library/snmpv3.h
./snmpstatus.lo: ../include/net-snmp/library/transform_oids.h
./snmpstatus.lo: ../include/net-snmp/library/keytools.h
./snmpstatus.lo: ../include/net-snmp/library/scapi.h
./snmpstatus.lo: ../include/net-snmp/library/lcd_time.h
./snmpstatus.lo: ../include/net-snmp/library/snmp_secmod.h
./snmpstatus.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./snmpstatus.lo: ../include/net-snmp/library/snmpusm.h
./snmptable.lo: ../include/net-snmp/net-snmp-config.h
./snmptable.lo: ../include/net-snmp/system/linux.h
./snmptable.lo: ../include/net-snmp/system/sysv.h
./snmptable.lo: ../include/net-snmp/system/generic.h
./snmptable.lo: ../include/net-snmp/machine/generic.h 
./snmptable.lo: ../include/net-snmp/net-snmp-includes.h
./snmptable.lo: ../include/net-snmp/definitions.h ../include/net-snmp/types.h
./snmptable.lo: ../include/net-snmp/library/snmp_api.h
./snmptable.lo: ../include/net-snmp/library/asn1.h
./snmptable.lo: ../include/net-snmp/library/snmp_impl.h
./snmptable.lo: ../include/net-snmp/library/snmp.h
./snmptable.lo: ../include/net-snmp/library/snmp-tc.h
./snmptable.lo: ../include/net-snmp/utilities.h
./snmptable.lo: ../include/net-snmp/library/snmp_client.h
./snmptable.lo: ../include/net-snmp/library/getopt.h
./snmptable.lo: ../include/net-snmp/library/system.h
./snmptable.lo: ../include/net-snmp/library/tools.h
./snmptable.lo: ../include/net-snmp/library/int64.h
./snmptable.lo: ../include/net-snmp/library/mt_support.h
./snmptable.lo: ../include/net-snmp/library/snmp_alarm.h
./snmptable.lo: ../include/net-snmp/library/data_list.h
./snmptable.lo: ../include/net-snmp/library/callback.h
./snmptable.lo: ../include/net-snmp/library/oid_stash.h
./snmptable.lo: ../include/net-snmp/library/check_varbind.h
./snmptable.lo: ../include/net-snmp/version.h
./snmptable.lo: ../include/net-snmp/session_api.h
./snmptable.lo: ../include/net-snmp/library/snmp_transport.h
./snmptable.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmptable.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmptable.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmptable.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmptable.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmptable.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmptable.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmptable.lo: ../include/net-snmp/library/ucd_compat.h
./snmptable.lo: ../include/net-snmp/pdu_api.h ../include/net-snmp/mib_api.h
./snmptable.lo: ../include/net-snmp/library/mib.h
./snmptable.lo: ../include/net-snmp/library/parse.h
./snmptable.lo: ../include/net-snmp/varbind_api.h
./snmptable.lo: ../include/net-snmp/config_api.h
./snmptable.lo: ../include/net-snmp/library/read_config.h
./snmptable.lo: ../include/net-snmp/library/default_store.h
./snmptable.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmptable.lo: ../include/net-snmp/library/snmp_enum.h
./snmptable.lo: ../include/net-snmp/library/vacm.h
./snmptable.lo: ../include/net-snmp/output_api.h
./snmptable.lo: ../include/net-snmp/library/snmp_debug.h
./snmptable.lo: ../include/net-snmp/library/snmp_logging.h
./snmptable.lo: ../include/net-snmp/snmpv3_api.h
./snmptable.lo: ../include/net-snmp/library/snmpv3.h
./snmptable.lo: ../include/net-snmp/library/transform_oids.h
./snmptable.lo: ../include/net-snmp/library/keytools.h
./snmptable.lo: ../include/net-snmp/library/scapi.h
./snmptable.lo: ../include/net-snmp/library/lcd_time.h
./snmptable.lo: ../include/net-snmp/library/snmp_secmod.h
./snmptable.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./snmptable.lo: ../include/net-snmp/library/snmpusm.h
./snmptest.lo: ../include/net-snmp/net-snmp-config.h
./snmptest.lo: ../include/net-snmp/system/linux.h
./snmptest.lo: ../include/net-snmp/system/sysv.h
./snmptest.lo: ../include/net-snmp/system/generic.h
./snmptest.lo: ../include/net-snmp/machine/generic.h 
./snmptest.lo: ../include/net-snmp/net-snmp-includes.h
./snmptest.lo: ../include/net-snmp/definitions.h ../include/net-snmp/types.h
./snmptest.lo: ../include/net-snmp/library/snmp_api.h
./snmptest.lo: ../include/net-snmp/library/asn1.h
./snmptest.lo: ../include/net-snmp/library/snmp_impl.h
./snmptest.lo: ../include/net-snmp/library/snmp.h
./snmptest.lo: ../include/net-snmp/library/snmp-tc.h
./snmptest.lo: ../include/net-snmp/utilities.h
./snmptest.lo: ../include/net-snmp/library/snmp_client.h
./snmptest.lo: ../include/net-snmp/library/getopt.h
./snmptest.lo: ../include/net-snmp/library/system.h
./snmptest.lo: ../include/net-snmp/library/tools.h
./snmptest.lo: ../include/net-snmp/library/int64.h
./snmptest.lo: ../include/net-snmp/library/mt_support.h
./snmptest.lo: ../include/net-snmp/library/snmp_alarm.h
./snmptest.lo: ../include/net-snmp/library/data_list.h
./snmptest.lo: ../include/net-snmp/library/callback.h
./snmptest.lo: ../include/net-snmp/library/oid_stash.h
./snmptest.lo: ../include/net-snmp/library/check_varbind.h
./snmptest.lo: ../include/net-snmp/version.h
./snmptest.lo: ../include/net-snmp/session_api.h
./snmptest.lo: ../include/net-snmp/library/snmp_transport.h
./snmptest.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmptest.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmptest.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmptest.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmptest.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmptest.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmptest.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmptest.lo: ../include/net-snmp/library/ucd_compat.h
./snmptest.lo: ../include/net-snmp/pdu_api.h ../include/net-snmp/mib_api.h
./snmptest.lo: ../include/net-snmp/library/mib.h
./snmptest.lo: ../include/net-snmp/library/parse.h
./snmptest.lo: ../include/net-snmp/varbind_api.h
./snmptest.lo: ../include/net-snmp/config_api.h
./snmptest.lo: ../include/net-snmp/library/read_config.h
./snmptest.lo: ../include/net-snmp/library/default_store.h
./snmptest.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmptest.lo: ../include/net-snmp/library/snmp_enum.h
./snmptest.lo: ../include/net-snmp/library/vacm.h
./snmptest.lo: ../include/net-snmp/output_api.h
./snmptest.lo: ../include/net-snmp/library/snmp_debug.h
./snmptest.lo: ../include/net-snmp/library/snmp_logging.h
./snmptest.lo: ../include/net-snmp/snmpv3_api.h
./snmptest.lo: ../include/net-snmp/library/snmpv3.h
./snmptest.lo: ../include/net-snmp/library/transform_oids.h
./snmptest.lo: ../include/net-snmp/library/keytools.h
./snmptest.lo: ../include/net-snmp/library/scapi.h
./snmptest.lo: ../include/net-snmp/library/lcd_time.h
./snmptest.lo: ../include/net-snmp/library/snmp_secmod.h
./snmptest.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./snmptest.lo: ../include/net-snmp/library/snmpusm.h
./snmptranslate.lo: ../include/net-snmp/net-snmp-config.h
./snmptranslate.lo: ../include/net-snmp/system/linux.h
./snmptranslate.lo: ../include/net-snmp/system/sysv.h
./snmptranslate.lo: ../include/net-snmp/system/generic.h
./snmptranslate.lo: ../include/net-snmp/machine/generic.h
./snmptranslate.lo: ../include/net-snmp/utilities.h
./snmptranslate.lo: ../include/net-snmp/types.h 
./snmptranslate.lo: ../include/net-snmp/definitions.h
./snmptranslate.lo: ../include/net-snmp/library/snmp_api.h
./snmptranslate.lo: ../include/net-snmp/library/asn1.h
./snmptranslate.lo: ../include/net-snmp/library/snmp_impl.h
./snmptranslate.lo: ../include/net-snmp/library/snmp.h
./snmptranslate.lo: ../include/net-snmp/library/snmp-tc.h
./snmptranslate.lo: ../include/net-snmp/library/snmp_client.h
./snmptranslate.lo: ../include/net-snmp/library/getopt.h
./snmptranslate.lo: ../include/net-snmp/library/system.h
./snmptranslate.lo: ../include/net-snmp/library/tools.h
./snmptranslate.lo: ../include/net-snmp/library/int64.h
./snmptranslate.lo: ../include/net-snmp/library/mt_support.h
./snmptranslate.lo: ../include/net-snmp/library/snmp_alarm.h
./snmptranslate.lo: ../include/net-snmp/library/data_list.h
./snmptranslate.lo: ../include/net-snmp/library/callback.h
./snmptranslate.lo: ../include/net-snmp/library/oid_stash.h
./snmptranslate.lo: ../include/net-snmp/library/check_varbind.h
./snmptranslate.lo: ../include/net-snmp/version.h
./snmptranslate.lo: ../include/net-snmp/config_api.h
./snmptranslate.lo: ../include/net-snmp/library/read_config.h
./snmptranslate.lo: ../include/net-snmp/library/default_store.h
./snmptranslate.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmptranslate.lo: ../include/net-snmp/library/snmp_enum.h
./snmptranslate.lo: ../include/net-snmp/library/vacm.h
./snmptranslate.lo: ../include/net-snmp/output_api.h
./snmptranslate.lo: ../include/net-snmp/library/snmp_debug.h
./snmptranslate.lo: ../include/net-snmp/library/snmp_logging.h
./snmptranslate.lo: ../include/net-snmp/mib_api.h
./snmptranslate.lo: ../include/net-snmp/library/mib.h
./snmptranslate.lo: ../include/net-snmp/library/parse.h
./snmptranslate.lo: ../include/net-snmp/library/ucd_compat.h
./snmptrap.lo: ../include/net-snmp/net-snmp-config.h
./snmptrap.lo: ../include/net-snmp/system/linux.h
./snmptrap.lo: ../include/net-snmp/system/sysv.h
./snmptrap.lo: ../include/net-snmp/system/generic.h
./snmptrap.lo: ../include/net-snmp/machine/generic.h 
./snmptrap.lo: ../include/net-snmp/net-snmp-includes.h
./snmptrap.lo: ../include/net-snmp/definitions.h ../include/net-snmp/types.h
./snmptrap.lo: ../include/net-snmp/library/snmp_api.h
./snmptrap.lo: ../include/net-snmp/library/asn1.h
./snmptrap.lo: ../include/net-snmp/library/snmp_impl.h
./snmptrap.lo: ../include/net-snmp/library/snmp.h
./snmptrap.lo: ../include/net-snmp/library/snmp-tc.h
./snmptrap.lo: ../include/net-snmp/utilities.h
./snmptrap.lo: ../include/net-snmp/library/snmp_client.h
./snmptrap.lo: ../include/net-snmp/library/getopt.h
./snmptrap.lo: ../include/net-snmp/library/system.h
./snmptrap.lo: ../include/net-snmp/library/tools.h
./snmptrap.lo: ../include/net-snmp/library/int64.h
./snmptrap.lo: ../include/net-snmp/library/mt_support.h
./snmptrap.lo: ../include/net-snmp/library/snmp_alarm.h
./snmptrap.lo: ../include/net-snmp/library/data_list.h
./snmptrap.lo: ../include/net-snmp/library/callback.h
./snmptrap.lo: ../include/net-snmp/library/oid_stash.h
./snmptrap.lo: ../include/net-snmp/library/check_varbind.h
./snmptrap.lo: ../include/net-snmp/version.h
./snmptrap.lo: ../include/net-snmp/session_api.h
./snmptrap.lo: ../include/net-snmp/library/snmp_transport.h
./snmptrap.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmptrap.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmptrap.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmptrap.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmptrap.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmptrap.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmptrap.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmptrap.lo: ../include/net-snmp/library/ucd_compat.h
./snmptrap.lo: ../include/net-snmp/pdu_api.h ../include/net-snmp/mib_api.h
./snmptrap.lo: ../include/net-snmp/library/mib.h
./snmptrap.lo: ../include/net-snmp/library/parse.h
./snmptrap.lo: ../include/net-snmp/varbind_api.h
./snmptrap.lo: ../include/net-snmp/config_api.h
./snmptrap.lo: ../include/net-snmp/library/read_config.h
./snmptrap.lo: ../include/net-snmp/library/default_store.h
./snmptrap.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmptrap.lo: ../include/net-snmp/library/snmp_enum.h
./snmptrap.lo: ../include/net-snmp/library/vacm.h
./snmptrap.lo: ../include/net-snmp/output_api.h
./snmptrap.lo: ../include/net-snmp/library/snmp_debug.h
./snmptrap.lo: ../include/net-snmp/library/snmp_logging.h
./snmptrap.lo: ../include/net-snmp/snmpv3_api.h
./snmptrap.lo: ../include/net-snmp/library/snmpv3.h
./snmptrap.lo: ../include/net-snmp/library/transform_oids.h
./snmptrap.lo: ../include/net-snmp/library/keytools.h
./snmptrap.lo: ../include/net-snmp/library/scapi.h
./snmptrap.lo: ../include/net-snmp/library/lcd_time.h
./snmptrap.lo: ../include/net-snmp/library/snmp_secmod.h
./snmptrap.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./snmptrap.lo: ../include/net-snmp/library/snmpusm.h
./snmptrapd.lo: ../include/net-snmp/net-snmp-config.h
./snmptrapd.lo: ../include/net-snmp/system/linux.h
./snmptrapd.lo: ../include/net-snmp/system/sysv.h
./snmptrapd.lo: ../include/net-snmp/system/generic.h
./snmptrapd.lo: ../include/net-snmp/machine/generic.h 
./snmptrapd.lo: ../include/net-snmp/net-snmp-includes.h
./snmptrapd.lo: ../include/net-snmp/definitions.h ../include/net-snmp/types.h
./snmptrapd.lo: ../include/net-snmp/library/snmp_api.h
./snmptrapd.lo: ../include/net-snmp/library/asn1.h
./snmptrapd.lo: ../include/net-snmp/library/snmp_impl.h
./snmptrapd.lo: ../include/net-snmp/library/snmp.h
./snmptrapd.lo: ../include/net-snmp/library/snmp-tc.h
./snmptrapd.lo: ../include/net-snmp/utilities.h
./snmptrapd.lo: ../include/net-snmp/library/snmp_client.h
./snmptrapd.lo: ../include/net-snmp/library/getopt.h
./snmptrapd.lo: ../include/net-snmp/library/system.h
./snmptrapd.lo: ../include/net-snmp/library/tools.h
./snmptrapd.lo: ../include/net-snmp/library/int64.h
./snmptrapd.lo: ../include/net-snmp/library/mt_support.h
./snmptrapd.lo: ../include/net-snmp/library/snmp_alarm.h
./snmptrapd.lo: ../include/net-snmp/library/data_list.h
./snmptrapd.lo: ../include/net-snmp/library/callback.h
./snmptrapd.lo: ../include/net-snmp/library/oid_stash.h
./snmptrapd.lo: ../include/net-snmp/library/check_varbind.h
./snmptrapd.lo: ../include/net-snmp/version.h
./snmptrapd.lo: ../include/net-snmp/session_api.h
./snmptrapd.lo: ../include/net-snmp/library/snmp_transport.h
./snmptrapd.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmptrapd.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmptrapd.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmptrapd.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmptrapd.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmptrapd.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmptrapd.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmptrapd.lo: ../include/net-snmp/library/ucd_compat.h
./snmptrapd.lo: ../include/net-snmp/pdu_api.h ../include/net-snmp/mib_api.h
./snmptrapd.lo: ../include/net-snmp/library/mib.h
./snmptrapd.lo: ../include/net-snmp/library/parse.h
./snmptrapd.lo: ../include/net-snmp/varbind_api.h
./snmptrapd.lo: ../include/net-snmp/config_api.h
./snmptrapd.lo: ../include/net-snmp/library/read_config.h
./snmptrapd.lo: ../include/net-snmp/library/default_store.h
./snmptrapd.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmptrapd.lo: ../include/net-snmp/library/snmp_enum.h
./snmptrapd.lo: ../include/net-snmp/library/vacm.h
./snmptrapd.lo: ../include/net-snmp/output_api.h
./snmptrapd.lo: ../include/net-snmp/library/snmp_debug.h
./snmptrapd.lo: ../include/net-snmp/library/snmp_logging.h
./snmptrapd.lo: ../include/net-snmp/snmpv3_api.h
./snmptrapd.lo: ../include/net-snmp/library/snmpv3.h
./snmptrapd.lo: ../include/net-snmp/library/transform_oids.h
./snmptrapd.lo: ../include/net-snmp/library/keytools.h
./snmptrapd.lo: ../include/net-snmp/library/scapi.h
./snmptrapd.lo: ../include/net-snmp/library/lcd_time.h
./snmptrapd.lo: ../include/net-snmp/library/snmp_secmod.h
./snmptrapd.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./snmptrapd.lo: ../include/net-snmp/library/snmpusm.h
./snmptrapd.lo: ../include/net-snmp/agent/net-snmp-agent-includes.h
./snmptrapd.lo: ../include/net-snmp/agent/snmp_agent.h
./snmptrapd.lo: ../include/net-snmp/agent/snmp_vars.h
./snmptrapd.lo: ../include/net-snmp/agent/agent_handler.h
./snmptrapd.lo: ../include/net-snmp/agent/var_struct.h
./snmptrapd.lo: ../include/net-snmp/agent/agent_registry.h
./snmptrapd.lo: ../include/net-snmp/agent/ds_agent.h
./snmptrapd.lo: ../include/net-snmp/agent/agent_read_config.h
./snmptrapd.lo: ../include/net-snmp/agent/agent_trap.h
./snmptrapd.lo: ../include/net-snmp/agent/all_helpers.h
./snmptrapd.lo: ../include/net-snmp/agent/instance.h
./snmptrapd.lo: ../include/net-snmp/agent/multiplexer.h
./snmptrapd.lo: ../include/net-snmp/agent/null.h
./snmptrapd.lo: ../include/net-snmp/agent/debug_handler.h
./snmptrapd.lo: ../include/net-snmp/agent/old_api.h
./snmptrapd.lo: ../include/net-snmp/agent/read_only.h
./snmptrapd.lo: ../include/net-snmp/agent/serialize.h
./snmptrapd.lo: ../include/net-snmp/agent/bulk_to_next.h
./snmptrapd.lo: ../include/net-snmp/agent/mode_end_call.h
./snmptrapd.lo: ../include/net-snmp/agent/table.h
./snmptrapd.lo: ../include/net-snmp/agent/table_data.h
./snmptrapd.lo: ../include/net-snmp/agent/table_dataset.h
./snmptrapd.lo: ../include/net-snmp/agent/table_iterator.h
./snmptrapd.lo: ../include/net-snmp/agent/mib_module_config.h
./snmptrapd.lo: snmptrapd_handlers.h snmptrapd_log.h notification_log.h
./snmptrapd_handlers.lo: ../include/net-snmp/net-snmp-config.h
./snmptrapd_handlers.lo: ../include/net-snmp/system/linux.h
./snmptrapd_handlers.lo: ../include/net-snmp/system/sysv.h
./snmptrapd_handlers.lo: ../include/net-snmp/system/generic.h
./snmptrapd_handlers.lo: ../include/net-snmp/machine/generic.h
./snmptrapd_handlers.lo: ../include/net-snmp/config_api.h
./snmptrapd_handlers.lo: ../include/net-snmp/types.h 
./snmptrapd_handlers.lo: ../include/net-snmp/definitions.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/snmp_api.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/asn1.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/snmp_impl.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/snmp.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/snmp-tc.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/read_config.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/default_store.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/snmp_enum.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/vacm.h
./snmptrapd_handlers.lo: ../include/net-snmp/output_api.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/snmp_client.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/snmp_debug.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/snmp_logging.h
./snmptrapd_handlers.lo: ../include/net-snmp/mib_api.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/mib.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/parse.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/callback.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/oid_stash.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/ucd_compat.h
./snmptrapd_handlers.lo: ../include/net-snmp/utilities.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/getopt.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/system.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/tools.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/int64.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/mt_support.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/snmp_alarm.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/data_list.h
./snmptrapd_handlers.lo: ../include/net-snmp/library/check_varbind.h
./snmptrapd_handlers.lo: ../include/net-snmp/version.h
./snmptrapd_log.lo: ../include/net-snmp/net-snmp-config.h
./snmptrapd_log.lo: ../include/net-snmp/system/linux.h
./snmptrapd_log.lo: ../include/net-snmp/system/sysv.h
./snmptrapd_log.lo: ../include/net-snmp/system/generic.h
./snmptrapd_log.lo: ../include/net-snmp/machine/generic.h
./snmptrapd_log.lo: ../include/net-snmp/net-snmp-includes.h
./snmptrapd_log.lo: ../include/net-snmp/definitions.h
./snmptrapd_log.lo: ../include/net-snmp/types.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmp_api.h
./snmptrapd_log.lo: ../include/net-snmp/library/asn1.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmp_impl.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmp.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmp-tc.h
./snmptrapd_log.lo: ../include/net-snmp/utilities.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmp_client.h
./snmptrapd_log.lo: ../include/net-snmp/library/getopt.h
./snmptrapd_log.lo: ../include/net-snmp/library/system.h
./snmptrapd_log.lo: ../include/net-snmp/library/tools.h
./snmptrapd_log.lo: ../include/net-snmp/library/int64.h
./snmptrapd_log.lo: ../include/net-snmp/library/mt_support.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmp_alarm.h
./snmptrapd_log.lo: ../include/net-snmp/library/data_list.h
./snmptrapd_log.lo: ../include/net-snmp/library/callback.h
./snmptrapd_log.lo: ../include/net-snmp/library/oid_stash.h
./snmptrapd_log.lo: ../include/net-snmp/library/check_varbind.h
./snmptrapd_log.lo: ../include/net-snmp/version.h
./snmptrapd_log.lo: ../include/net-snmp/session_api.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmp_transport.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmptrapd_log.lo: ../include/net-snmp/library/ucd_compat.h
./snmptrapd_log.lo: ../include/net-snmp/pdu_api.h
./snmptrapd_log.lo: ../include/net-snmp/mib_api.h
./snmptrapd_log.lo: ../include/net-snmp/library/mib.h
./snmptrapd_log.lo: ../include/net-snmp/library/parse.h
./snmptrapd_log.lo: ../include/net-snmp/varbind_api.h
./snmptrapd_log.lo: ../include/net-snmp/config_api.h
./snmptrapd_log.lo: ../include/net-snmp/library/read_config.h
./snmptrapd_log.lo: ../include/net-snmp/library/default_store.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmp_enum.h
./snmptrapd_log.lo: ../include/net-snmp/library/vacm.h
./snmptrapd_log.lo: ../include/net-snmp/output_api.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmp_debug.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmp_logging.h
./snmptrapd_log.lo: ../include/net-snmp/snmpv3_api.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmpv3.h
./snmptrapd_log.lo: ../include/net-snmp/library/transform_oids.h
./snmptrapd_log.lo: ../include/net-snmp/library/keytools.h
./snmptrapd_log.lo: ../include/net-snmp/library/scapi.h
./snmptrapd_log.lo: ../include/net-snmp/library/lcd_time.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmp_secmod.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./snmptrapd_log.lo: ../include/net-snmp/library/snmpusm.h snmptrapd_log.h
./snmpusm.lo: ../include/net-snmp/net-snmp-config.h
./snmpusm.lo: ../include/net-snmp/system/linux.h
./snmpusm.lo: ../include/net-snmp/system/sysv.h
./snmpusm.lo: ../include/net-snmp/system/generic.h
./snmpusm.lo: ../include/net-snmp/machine/generic.h 
./snmpusm.lo: ../include/net-snmp/net-snmp-includes.h
./snmpusm.lo: ../include/net-snmp/definitions.h ../include/net-snmp/types.h
./snmpusm.lo: ../include/net-snmp/library/snmp_api.h
./snmpusm.lo: ../include/net-snmp/library/asn1.h
./snmpusm.lo: ../include/net-snmp/library/snmp_impl.h
./snmpusm.lo: ../include/net-snmp/library/snmp.h
./snmpusm.lo: ../include/net-snmp/library/snmp-tc.h
./snmpusm.lo: ../include/net-snmp/utilities.h
./snmpusm.lo: ../include/net-snmp/library/snmp_client.h
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
./snmpusm.lo: ../include/net-snmp/version.h ../include/net-snmp/session_api.h
./snmpusm.lo: ../include/net-snmp/library/snmp_transport.h
./snmpusm.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmpusm.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmpusm.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmpusm.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmpusm.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmpusm.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmpusm.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmpusm.lo: ../include/net-snmp/library/ucd_compat.h
./snmpusm.lo: ../include/net-snmp/pdu_api.h ../include/net-snmp/mib_api.h
./snmpusm.lo: ../include/net-snmp/library/mib.h
./snmpusm.lo: ../include/net-snmp/library/parse.h
./snmpusm.lo: ../include/net-snmp/varbind_api.h
./snmpusm.lo: ../include/net-snmp/config_api.h
./snmpusm.lo: ../include/net-snmp/library/read_config.h
./snmpusm.lo: ../include/net-snmp/library/default_store.h
./snmpusm.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpusm.lo: ../include/net-snmp/library/snmp_enum.h
./snmpusm.lo: ../include/net-snmp/library/vacm.h
./snmpusm.lo: ../include/net-snmp/output_api.h
./snmpusm.lo: ../include/net-snmp/library/snmp_debug.h
./snmpusm.lo: ../include/net-snmp/library/snmp_logging.h
./snmpusm.lo: ../include/net-snmp/snmpv3_api.h
./snmpusm.lo: ../include/net-snmp/library/snmpv3.h
./snmpusm.lo: ../include/net-snmp/library/transform_oids.h
./snmpusm.lo: ../include/net-snmp/library/keytools.h
./snmpusm.lo: ../include/net-snmp/library/scapi.h
./snmpusm.lo: ../include/net-snmp/library/lcd_time.h
./snmpusm.lo: ../include/net-snmp/library/snmp_secmod.h
./snmpusm.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./snmpusm.lo: ../include/net-snmp/library/snmpusm.h
./snmpvacm.lo: ../include/net-snmp/net-snmp-config.h
./snmpvacm.lo: ../include/net-snmp/system/linux.h
./snmpvacm.lo: ../include/net-snmp/system/sysv.h
./snmpvacm.lo: ../include/net-snmp/system/generic.h
./snmpvacm.lo: ../include/net-snmp/machine/generic.h 
./snmpvacm.lo: ../include/net-snmp/net-snmp-includes.h
./snmpvacm.lo: ../include/net-snmp/definitions.h ../include/net-snmp/types.h
./snmpvacm.lo: ../include/net-snmp/library/snmp_api.h
./snmpvacm.lo: ../include/net-snmp/library/asn1.h
./snmpvacm.lo: ../include/net-snmp/library/snmp_impl.h
./snmpvacm.lo: ../include/net-snmp/library/snmp.h
./snmpvacm.lo: ../include/net-snmp/library/snmp-tc.h
./snmpvacm.lo: ../include/net-snmp/utilities.h
./snmpvacm.lo: ../include/net-snmp/library/snmp_client.h
./snmpvacm.lo: ../include/net-snmp/library/getopt.h
./snmpvacm.lo: ../include/net-snmp/library/system.h
./snmpvacm.lo: ../include/net-snmp/library/tools.h
./snmpvacm.lo: ../include/net-snmp/library/int64.h
./snmpvacm.lo: ../include/net-snmp/library/mt_support.h
./snmpvacm.lo: ../include/net-snmp/library/snmp_alarm.h
./snmpvacm.lo: ../include/net-snmp/library/data_list.h
./snmpvacm.lo: ../include/net-snmp/library/callback.h
./snmpvacm.lo: ../include/net-snmp/library/oid_stash.h
./snmpvacm.lo: ../include/net-snmp/library/check_varbind.h
./snmpvacm.lo: ../include/net-snmp/version.h
./snmpvacm.lo: ../include/net-snmp/session_api.h
./snmpvacm.lo: ../include/net-snmp/library/snmp_transport.h
./snmpvacm.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmpvacm.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmpvacm.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmpvacm.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmpvacm.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmpvacm.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmpvacm.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmpvacm.lo: ../include/net-snmp/library/ucd_compat.h
./snmpvacm.lo: ../include/net-snmp/pdu_api.h ../include/net-snmp/mib_api.h
./snmpvacm.lo: ../include/net-snmp/library/mib.h
./snmpvacm.lo: ../include/net-snmp/library/parse.h
./snmpvacm.lo: ../include/net-snmp/varbind_api.h
./snmpvacm.lo: ../include/net-snmp/config_api.h
./snmpvacm.lo: ../include/net-snmp/library/read_config.h
./snmpvacm.lo: ../include/net-snmp/library/default_store.h
./snmpvacm.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpvacm.lo: ../include/net-snmp/library/snmp_enum.h
./snmpvacm.lo: ../include/net-snmp/library/vacm.h
./snmpvacm.lo: ../include/net-snmp/output_api.h
./snmpvacm.lo: ../include/net-snmp/library/snmp_debug.h
./snmpvacm.lo: ../include/net-snmp/library/snmp_logging.h
./snmpvacm.lo: ../include/net-snmp/snmpv3_api.h
./snmpvacm.lo: ../include/net-snmp/library/snmpv3.h
./snmpvacm.lo: ../include/net-snmp/library/transform_oids.h
./snmpvacm.lo: ../include/net-snmp/library/keytools.h
./snmpvacm.lo: ../include/net-snmp/library/scapi.h
./snmpvacm.lo: ../include/net-snmp/library/lcd_time.h
./snmpvacm.lo: ../include/net-snmp/library/snmp_secmod.h
./snmpvacm.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./snmpvacm.lo: ../include/net-snmp/library/snmpusm.h
./snmpwalk.lo: ../include/net-snmp/net-snmp-config.h
./snmpwalk.lo: ../include/net-snmp/system/linux.h
./snmpwalk.lo: ../include/net-snmp/system/sysv.h
./snmpwalk.lo: ../include/net-snmp/system/generic.h
./snmpwalk.lo: ../include/net-snmp/machine/generic.h 
./snmpwalk.lo: ../include/net-snmp/net-snmp-includes.h
./snmpwalk.lo: ../include/net-snmp/definitions.h ../include/net-snmp/types.h
./snmpwalk.lo: ../include/net-snmp/library/snmp_api.h
./snmpwalk.lo: ../include/net-snmp/library/asn1.h
./snmpwalk.lo: ../include/net-snmp/library/snmp_impl.h
./snmpwalk.lo: ../include/net-snmp/library/snmp.h
./snmpwalk.lo: ../include/net-snmp/library/snmp-tc.h
./snmpwalk.lo: ../include/net-snmp/utilities.h
./snmpwalk.lo: ../include/net-snmp/library/snmp_client.h
./snmpwalk.lo: ../include/net-snmp/library/getopt.h
./snmpwalk.lo: ../include/net-snmp/library/system.h
./snmpwalk.lo: ../include/net-snmp/library/tools.h
./snmpwalk.lo: ../include/net-snmp/library/int64.h
./snmpwalk.lo: ../include/net-snmp/library/mt_support.h
./snmpwalk.lo: ../include/net-snmp/library/snmp_alarm.h
./snmpwalk.lo: ../include/net-snmp/library/data_list.h
./snmpwalk.lo: ../include/net-snmp/library/callback.h
./snmpwalk.lo: ../include/net-snmp/library/oid_stash.h
./snmpwalk.lo: ../include/net-snmp/library/check_varbind.h
./snmpwalk.lo: ../include/net-snmp/version.h
./snmpwalk.lo: ../include/net-snmp/session_api.h
./snmpwalk.lo: ../include/net-snmp/library/snmp_transport.h
./snmpwalk.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmpwalk.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmpwalk.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmpwalk.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmpwalk.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmpwalk.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmpwalk.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmpwalk.lo: ../include/net-snmp/library/ucd_compat.h
./snmpwalk.lo: ../include/net-snmp/pdu_api.h ../include/net-snmp/mib_api.h
./snmpwalk.lo: ../include/net-snmp/library/mib.h
./snmpwalk.lo: ../include/net-snmp/library/parse.h
./snmpwalk.lo: ../include/net-snmp/varbind_api.h
./snmpwalk.lo: ../include/net-snmp/config_api.h
./snmpwalk.lo: ../include/net-snmp/library/read_config.h
./snmpwalk.lo: ../include/net-snmp/library/default_store.h
./snmpwalk.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpwalk.lo: ../include/net-snmp/library/snmp_enum.h
./snmpwalk.lo: ../include/net-snmp/library/vacm.h
./snmpwalk.lo: ../include/net-snmp/output_api.h
./snmpwalk.lo: ../include/net-snmp/library/snmp_debug.h
./snmpwalk.lo: ../include/net-snmp/library/snmp_logging.h
./snmpwalk.lo: ../include/net-snmp/snmpv3_api.h
./snmpwalk.lo: ../include/net-snmp/library/snmpv3.h
./snmpwalk.lo: ../include/net-snmp/library/transform_oids.h
./snmpwalk.lo: ../include/net-snmp/library/keytools.h
./snmpwalk.lo: ../include/net-snmp/library/scapi.h
./snmpwalk.lo: ../include/net-snmp/library/lcd_time.h
./snmpwalk.lo: ../include/net-snmp/library/snmp_secmod.h
./snmpwalk.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./snmpwalk.lo: ../include/net-snmp/library/snmpusm.h
./snmpnetstat/if.lo: ../include/net-snmp/net-snmp-config.h
./snmpnetstat/if.lo: ../include/net-snmp/system/linux.h
./snmpnetstat/if.lo: ../include/net-snmp/system/sysv.h
./snmpnetstat/if.lo: ../include/net-snmp/system/generic.h
./snmpnetstat/if.lo: ../include/net-snmp/machine/generic.h
./snmpnetstat/if.lo:  ./snmpnetstat/main.h
./snmpnetstat/if.lo: ../include/net-snmp/net-snmp-includes.h
./snmpnetstat/if.lo: ../include/net-snmp/definitions.h
./snmpnetstat/if.lo: ../include/net-snmp/types.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmp_api.h
./snmpnetstat/if.lo: ../include/net-snmp/library/asn1.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmp_impl.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmp.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmp-tc.h
./snmpnetstat/if.lo: ../include/net-snmp/utilities.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmp_client.h
./snmpnetstat/if.lo: ../include/net-snmp/library/getopt.h
./snmpnetstat/if.lo: ../include/net-snmp/library/system.h
./snmpnetstat/if.lo: ../include/net-snmp/library/tools.h
./snmpnetstat/if.lo: ../include/net-snmp/library/int64.h
./snmpnetstat/if.lo: ../include/net-snmp/library/mt_support.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmp_alarm.h
./snmpnetstat/if.lo: ../include/net-snmp/library/data_list.h
./snmpnetstat/if.lo: ../include/net-snmp/library/callback.h
./snmpnetstat/if.lo: ../include/net-snmp/library/oid_stash.h
./snmpnetstat/if.lo: ../include/net-snmp/library/check_varbind.h
./snmpnetstat/if.lo: ../include/net-snmp/version.h
./snmpnetstat/if.lo: ../include/net-snmp/session_api.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmp_transport.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmpnetstat/if.lo: ../include/net-snmp/library/ucd_compat.h
./snmpnetstat/if.lo: ../include/net-snmp/pdu_api.h
./snmpnetstat/if.lo: ../include/net-snmp/mib_api.h
./snmpnetstat/if.lo: ../include/net-snmp/library/mib.h
./snmpnetstat/if.lo: ../include/net-snmp/library/parse.h
./snmpnetstat/if.lo: ../include/net-snmp/varbind_api.h
./snmpnetstat/if.lo: ../include/net-snmp/config_api.h
./snmpnetstat/if.lo: ../include/net-snmp/library/read_config.h
./snmpnetstat/if.lo: ../include/net-snmp/library/default_store.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmp_enum.h
./snmpnetstat/if.lo: ../include/net-snmp/library/vacm.h
./snmpnetstat/if.lo: ../include/net-snmp/output_api.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmp_debug.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmp_logging.h
./snmpnetstat/if.lo: ../include/net-snmp/snmpv3_api.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmpv3.h
./snmpnetstat/if.lo: ../include/net-snmp/library/transform_oids.h
./snmpnetstat/if.lo: ../include/net-snmp/library/keytools.h
./snmpnetstat/if.lo: ../include/net-snmp/library/scapi.h
./snmpnetstat/if.lo: ../include/net-snmp/library/lcd_time.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmp_secmod.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./snmpnetstat/if.lo: ../include/net-snmp/library/snmpusm.h
./snmpnetstat/if.lo: ./snmpnetstat/netstat.h
./snmpnetstat/inet6.lo: ../include/net-snmp/net-snmp-config.h
./snmpnetstat/inet6.lo: ../include/net-snmp/system/linux.h
./snmpnetstat/inet6.lo: ../include/net-snmp/system/sysv.h
./snmpnetstat/inet6.lo: ../include/net-snmp/system/generic.h
./snmpnetstat/inet6.lo: ../include/net-snmp/machine/generic.h
./snmpnetstat/inet.lo: ../include/net-snmp/net-snmp-config.h
./snmpnetstat/inet.lo: ../include/net-snmp/system/linux.h
./snmpnetstat/inet.lo: ../include/net-snmp/system/sysv.h
./snmpnetstat/inet.lo: ../include/net-snmp/system/generic.h
./snmpnetstat/inet.lo: ../include/net-snmp/machine/generic.h
./snmpnetstat/inet.lo:  ./snmpnetstat/main.h
./snmpnetstat/inet.lo: ../include/net-snmp/net-snmp-includes.h
./snmpnetstat/inet.lo: ../include/net-snmp/definitions.h
./snmpnetstat/inet.lo: ../include/net-snmp/types.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmp_api.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/asn1.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmp_impl.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmp.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmp-tc.h
./snmpnetstat/inet.lo: ../include/net-snmp/utilities.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmp_client.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/getopt.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/system.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/tools.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/int64.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/mt_support.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmp_alarm.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/data_list.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/callback.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/oid_stash.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/check_varbind.h
./snmpnetstat/inet.lo: ../include/net-snmp/version.h
./snmpnetstat/inet.lo: ../include/net-snmp/session_api.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmp_transport.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/ucd_compat.h
./snmpnetstat/inet.lo: ../include/net-snmp/pdu_api.h
./snmpnetstat/inet.lo: ../include/net-snmp/mib_api.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/mib.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/parse.h
./snmpnetstat/inet.lo: ../include/net-snmp/varbind_api.h
./snmpnetstat/inet.lo: ../include/net-snmp/config_api.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/read_config.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/default_store.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmp_enum.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/vacm.h
./snmpnetstat/inet.lo: ../include/net-snmp/output_api.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmp_debug.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmp_logging.h
./snmpnetstat/inet.lo: ../include/net-snmp/snmpv3_api.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmpv3.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/transform_oids.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/keytools.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/scapi.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/lcd_time.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmp_secmod.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./snmpnetstat/inet.lo: ../include/net-snmp/library/snmpusm.h
./snmpnetstat/inet.lo: ./snmpnetstat/netstat.h
./snmpnetstat/main.lo: ../include/net-snmp/net-snmp-config.h
./snmpnetstat/main.lo: ../include/net-snmp/system/linux.h
./snmpnetstat/main.lo: ../include/net-snmp/system/sysv.h
./snmpnetstat/main.lo: ../include/net-snmp/system/generic.h
./snmpnetstat/main.lo: ../include/net-snmp/machine/generic.h
./snmpnetstat/main.lo:  ../include/net-snmp/utilities.h
./snmpnetstat/main.lo: ../include/net-snmp/types.h 
./snmpnetstat/main.lo: ../include/net-snmp/definitions.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmp_api.h
./snmpnetstat/main.lo: ../include/net-snmp/library/asn1.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmp_impl.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmp.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmp-tc.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmp_client.h
./snmpnetstat/main.lo: ../include/net-snmp/library/getopt.h
./snmpnetstat/main.lo: ../include/net-snmp/library/system.h
./snmpnetstat/main.lo: ../include/net-snmp/library/tools.h
./snmpnetstat/main.lo: ../include/net-snmp/library/int64.h
./snmpnetstat/main.lo: ../include/net-snmp/library/mt_support.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmp_alarm.h
./snmpnetstat/main.lo: ../include/net-snmp/library/data_list.h
./snmpnetstat/main.lo: ../include/net-snmp/library/callback.h
./snmpnetstat/main.lo: ../include/net-snmp/library/oid_stash.h
./snmpnetstat/main.lo: ../include/net-snmp/library/check_varbind.h
./snmpnetstat/main.lo: ../include/net-snmp/version.h 
./snmpnetstat/main.lo: ../include/net-snmp/net-snmp-includes.h
./snmpnetstat/main.lo: ../include/net-snmp/session_api.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmp_transport.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmpnetstat/main.lo: ../include/net-snmp/library/ucd_compat.h
./snmpnetstat/main.lo: ../include/net-snmp/pdu_api.h
./snmpnetstat/main.lo: ../include/net-snmp/mib_api.h
./snmpnetstat/main.lo: ../include/net-snmp/library/mib.h
./snmpnetstat/main.lo: ../include/net-snmp/library/parse.h
./snmpnetstat/main.lo: ../include/net-snmp/varbind_api.h
./snmpnetstat/main.lo: ../include/net-snmp/config_api.h
./snmpnetstat/main.lo: ../include/net-snmp/library/read_config.h
./snmpnetstat/main.lo: ../include/net-snmp/library/default_store.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmp_enum.h
./snmpnetstat/main.lo: ../include/net-snmp/library/vacm.h
./snmpnetstat/main.lo: ../include/net-snmp/output_api.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmp_debug.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmp_logging.h
./snmpnetstat/main.lo: ../include/net-snmp/snmpv3_api.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmpv3.h
./snmpnetstat/main.lo: ../include/net-snmp/library/transform_oids.h
./snmpnetstat/main.lo: ../include/net-snmp/library/keytools.h
./snmpnetstat/main.lo: ../include/net-snmp/library/scapi.h
./snmpnetstat/main.lo: ../include/net-snmp/library/lcd_time.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmp_secmod.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./snmpnetstat/main.lo: ../include/net-snmp/library/snmpusm.h
./snmpnetstat/main.lo: ./snmpnetstat/netstat.h
./snmpnetstat/route.lo: ../include/net-snmp/net-snmp-config.h
./snmpnetstat/route.lo: ../include/net-snmp/system/linux.h
./snmpnetstat/route.lo: ../include/net-snmp/system/sysv.h
./snmpnetstat/route.lo: ../include/net-snmp/system/generic.h
./snmpnetstat/route.lo: ../include/net-snmp/machine/generic.h
./snmpnetstat/route.lo: ./snmpnetstat/main.h
./snmpnetstat/route.lo: ../include/net-snmp/net-snmp-includes.h
./snmpnetstat/route.lo: ../include/net-snmp/definitions.h
./snmpnetstat/route.lo: ../include/net-snmp/types.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmp_api.h
./snmpnetstat/route.lo: ../include/net-snmp/library/asn1.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmp_impl.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmp.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmp-tc.h
./snmpnetstat/route.lo: ../include/net-snmp/utilities.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmp_client.h
./snmpnetstat/route.lo: ../include/net-snmp/library/getopt.h
./snmpnetstat/route.lo: ../include/net-snmp/library/system.h
./snmpnetstat/route.lo: ../include/net-snmp/library/tools.h
./snmpnetstat/route.lo: ../include/net-snmp/library/int64.h
./snmpnetstat/route.lo: ../include/net-snmp/library/mt_support.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmp_alarm.h
./snmpnetstat/route.lo: ../include/net-snmp/library/data_list.h
./snmpnetstat/route.lo: ../include/net-snmp/library/callback.h
./snmpnetstat/route.lo: ../include/net-snmp/library/oid_stash.h
./snmpnetstat/route.lo: ../include/net-snmp/library/check_varbind.h
./snmpnetstat/route.lo: ../include/net-snmp/version.h
./snmpnetstat/route.lo: ../include/net-snmp/session_api.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmp_transport.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmpCallbackDomain.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmpUnixDomain.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmpUDPDomain.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmpTCPDomain.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmpUDPIPv6Domain.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmpTCPIPv6Domain.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmpIPXDomain.h
./snmpnetstat/route.lo: ../include/net-snmp/library/ucd_compat.h
./snmpnetstat/route.lo: ../include/net-snmp/pdu_api.h
./snmpnetstat/route.lo: ../include/net-snmp/mib_api.h
./snmpnetstat/route.lo: ../include/net-snmp/library/mib.h
./snmpnetstat/route.lo: ../include/net-snmp/library/parse.h
./snmpnetstat/route.lo: ../include/net-snmp/varbind_api.h
./snmpnetstat/route.lo: ../include/net-snmp/config_api.h
./snmpnetstat/route.lo: ../include/net-snmp/library/read_config.h
./snmpnetstat/route.lo: ../include/net-snmp/library/default_store.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmp_parse_args.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmp_enum.h
./snmpnetstat/route.lo: ../include/net-snmp/library/vacm.h
./snmpnetstat/route.lo: ../include/net-snmp/output_api.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmp_debug.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmp_logging.h
./snmpnetstat/route.lo: ../include/net-snmp/snmpv3_api.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmpv3.h
./snmpnetstat/route.lo: ../include/net-snmp/library/transform_oids.h
./snmpnetstat/route.lo: ../include/net-snmp/library/keytools.h
./snmpnetstat/route.lo: ../include/net-snmp/library/scapi.h
./snmpnetstat/route.lo: ../include/net-snmp/library/lcd_time.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmp_secmod.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmpv3-security-includes.h
./snmpnetstat/route.lo: ../include/net-snmp/library/snmpusm.h
./snmpnetstat/route.lo: ./snmpnetstat/netstat.h
./snmpnetstat/winstub.lo: ../include/net-snmp/net-snmp-config.h
./snmpnetstat/winstub.lo: ../include/net-snmp/system/linux.h
./snmpnetstat/winstub.lo: ../include/net-snmp/system/sysv.h
./snmpnetstat/winstub.lo: ../include/net-snmp/system/generic.h
./snmpnetstat/winstub.lo: ../include/net-snmp/machine/generic.h
