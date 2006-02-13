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
SED		= /home/backup/mikrotik/toolchain/bin//sed
LN_S		= ln -s
AUTOCONF	= :
AUTOHEADER	= :

#
# Compiler arguments
#
CFLAGS		= -Os -mtune=mips32 -mips32 -pipe -funit-at-a-time -Delf 
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
top_builddir=../..

#
# What to install
#

INSTALLLIBS=libnetsnmphelpers.$(LIB_EXTENSION)$(LIB_VERSION)
INCLUDESUBDIR=agent
INCLUDESUBDIRHEADERS=all_helpers.h \
	instance.h \
	scalar.h \
	watcher.h \
        multiplexer.h \
	bulk_to_next.h \
        null.h \
        debug_handler.h \
        old_api.h \
        read_only.h \
        serialize.h \
        set_helper.h \
        table_data.h \
        table_dataset.h \
	table_array.h \
        table.h \
        table_iterator.h \
	mode_end_call.h


#
# Build info
#
CPPFLAGS=-I../../include -I$(top_srcdir)/include \
	-I. -I.. -I../.. -I$(srcdir) -I$(srcdir)/..  \
	-I$(srcdir)/../mibgroup -I$(srcdir)/../.. -I$(srcdir)/../../snmplib \
	 $(LIB_CFLAGS)

OBJS =  all_helpers.o \
	multiplexer.o \
	read_only.o \
	bulk_to_next.o \
	instance.o \
	scalar.o \
	watcher.o \
	null.o \
	debug_handler.o \
	old_api.o \
	serialize.o \
	table_array.o \
	table_iterator.o \
	table_data.o \
	table_dataset.o \
	table.o \
	mode_end_call.o

LOBJS = all_helpers.lo \
	multiplexer.lo \
	read_only.lo \
	bulk_to_next.lo \
	instance.lo \
	scalar.lo \
	watcher.lo \
	null.lo \
	debug_handler.lo \
	old_api.lo \
	serialize.lo \
	table_array.lo \
	table_iterator.lo \
	table_data.lo \
	table_dataset.lo \
	table.lo \
	mode_end_call.lo

SRCS =  all_helpers.c \
	multiplexer.c \
	read_only.c \
	bulk_to_next.c \
	instance.c \
	scalar.c \
	watcher.c \
	null.c \
	debug_handler.c \
	old_api.c \
	serialize.c \
	table_array.c \
	table_iterator.c \
	table_data.c \
	table_dataset.c \
	table.c \
	mode_end_call.c

all: standardall

libnetsnmphelpers.$(LIB_EXTENSION)$(LIB_VERSION):    ${LOBJS}
	$(LIB_LD_CMD) libnetsnmphelpers.$(LIB_EXTENSION)$(LIB_VERSION) ${LOBJS}  $(LIB_LD_LIBS)
	$(RANLIB) libnetsnmphelpers.$(LIB_EXTENSION)$(LIB_VERSION)

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

./all_helpers.lo: ../../include/net-snmp/net-snmp-config.h
./all_helpers.lo: ../../include/net-snmp/system/linux.h
./all_helpers.lo: ../../include/net-snmp/system/sysv.h
./all_helpers.lo: ../../include/net-snmp/system/generic.h
./all_helpers.lo: ../../include/net-snmp/machine/generic.h
./bulk_to_next.lo: ../../include/net-snmp/net-snmp-config.h
./bulk_to_next.lo: ../../include/net-snmp/system/linux.h
./bulk_to_next.lo: ../../include/net-snmp/system/sysv.h
./bulk_to_next.lo: ../../include/net-snmp/system/generic.h
./bulk_to_next.lo: ../../include/net-snmp/machine/generic.h
./bulk_to_next.lo: ../../include/net-snmp/net-snmp-includes.h
./bulk_to_next.lo: ../../include/net-snmp/definitions.h
./bulk_to_next.lo: ../../include/net-snmp/types.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmp_api.h
./bulk_to_next.lo: ../../include/net-snmp/library/asn1.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmp_impl.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmp.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmp-tc.h
./bulk_to_next.lo: ../../include/net-snmp/utilities.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmp_client.h
./bulk_to_next.lo: ../../include/net-snmp/library/getopt.h
./bulk_to_next.lo: ../../include/net-snmp/library/system.h
./bulk_to_next.lo: ../../include/net-snmp/library/tools.h
./bulk_to_next.lo: ../../include/net-snmp/library/int64.h
./bulk_to_next.lo: ../../include/net-snmp/library/mt_support.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmp_alarm.h
./bulk_to_next.lo: ../../include/net-snmp/library/data_list.h
./bulk_to_next.lo: ../../include/net-snmp/library/callback.h
./bulk_to_next.lo: ../../include/net-snmp/library/oid_stash.h
./bulk_to_next.lo: ../../include/net-snmp/library/check_varbind.h
./bulk_to_next.lo: ../../include/net-snmp/version.h
./bulk_to_next.lo: ../../include/net-snmp/session_api.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmp_transport.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmpCallbackDomain.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmpUnixDomain.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmpUDPDomain.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmpTCPDomain.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmpUDPIPv6Domain.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmpTCPIPv6Domain.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmpIPXDomain.h
./bulk_to_next.lo: ../../include/net-snmp/library/ucd_compat.h
./bulk_to_next.lo: ../../include/net-snmp/pdu_api.h
./bulk_to_next.lo: ../../include/net-snmp/mib_api.h
./bulk_to_next.lo: ../../include/net-snmp/library/mib.h
./bulk_to_next.lo: ../../include/net-snmp/library/parse.h
./bulk_to_next.lo: ../../include/net-snmp/varbind_api.h
./bulk_to_next.lo: ../../include/net-snmp/config_api.h
./bulk_to_next.lo: ../../include/net-snmp/library/read_config.h
./bulk_to_next.lo: ../../include/net-snmp/library/default_store.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmp_parse_args.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmp_enum.h
./bulk_to_next.lo: ../../include/net-snmp/library/vacm.h
./bulk_to_next.lo: ../../include/net-snmp/output_api.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmp_debug.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmp_logging.h
./bulk_to_next.lo: ../../include/net-snmp/snmpv3_api.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmpv3.h
./bulk_to_next.lo: ../../include/net-snmp/library/transform_oids.h
./bulk_to_next.lo: ../../include/net-snmp/library/keytools.h
./bulk_to_next.lo: ../../include/net-snmp/library/scapi.h
./bulk_to_next.lo: ../../include/net-snmp/library/lcd_time.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmp_secmod.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmpv3-security-includes.h
./bulk_to_next.lo: ../../include/net-snmp/library/snmpusm.h
./bulk_to_next.lo: ../../include/net-snmp/agent/net-snmp-agent-includes.h
./bulk_to_next.lo: ../../include/net-snmp/agent/snmp_agent.h
./bulk_to_next.lo: ../../include/net-snmp/agent/snmp_vars.h
./bulk_to_next.lo: ../../include/net-snmp/agent/agent_handler.h
./bulk_to_next.lo: ../../include/net-snmp/agent/var_struct.h
./bulk_to_next.lo: ../../include/net-snmp/agent/agent_registry.h
./bulk_to_next.lo: ../../include/net-snmp/agent/ds_agent.h
./bulk_to_next.lo: ../../include/net-snmp/agent/agent_read_config.h
./bulk_to_next.lo: ../../include/net-snmp/agent/agent_trap.h
./bulk_to_next.lo: ../../include/net-snmp/agent/all_helpers.h
./bulk_to_next.lo: ../../include/net-snmp/agent/instance.h
./bulk_to_next.lo: ../../include/net-snmp/agent/multiplexer.h
./bulk_to_next.lo: ../../include/net-snmp/agent/null.h
./bulk_to_next.lo: ../../include/net-snmp/agent/debug_handler.h
./bulk_to_next.lo: ../../include/net-snmp/agent/old_api.h
./bulk_to_next.lo: ../../include/net-snmp/agent/read_only.h
./bulk_to_next.lo: ../../include/net-snmp/agent/serialize.h
./bulk_to_next.lo: ../../include/net-snmp/agent/bulk_to_next.h
./bulk_to_next.lo: ../../include/net-snmp/agent/mode_end_call.h
./bulk_to_next.lo: ../../include/net-snmp/agent/table.h
./bulk_to_next.lo: ../../include/net-snmp/agent/table_data.h
./bulk_to_next.lo: ../../include/net-snmp/agent/table_dataset.h
./bulk_to_next.lo: ../../include/net-snmp/agent/table_iterator.h
./bulk_to_next.lo: ../../include/net-snmp/agent/mib_module_config.h
./debug_handler.lo: ../../include/net-snmp/net-snmp-config.h
./debug_handler.lo: ../../include/net-snmp/system/linux.h
./debug_handler.lo: ../../include/net-snmp/system/sysv.h
./debug_handler.lo: ../../include/net-snmp/system/generic.h
./debug_handler.lo: ../../include/net-snmp/machine/generic.h
./debug_handler.lo: ../../include/net-snmp/net-snmp-includes.h
./debug_handler.lo: ../../include/net-snmp/definitions.h
./debug_handler.lo: ../../include/net-snmp/types.h
./debug_handler.lo: ../../include/net-snmp/library/snmp_api.h
./debug_handler.lo: ../../include/net-snmp/library/asn1.h
./debug_handler.lo: ../../include/net-snmp/library/snmp_impl.h
./debug_handler.lo: ../../include/net-snmp/library/snmp.h
./debug_handler.lo: ../../include/net-snmp/library/snmp-tc.h
./debug_handler.lo: ../../include/net-snmp/utilities.h
./debug_handler.lo: ../../include/net-snmp/library/snmp_client.h
./debug_handler.lo: ../../include/net-snmp/library/getopt.h
./debug_handler.lo: ../../include/net-snmp/library/system.h
./debug_handler.lo: ../../include/net-snmp/library/tools.h
./debug_handler.lo: ../../include/net-snmp/library/int64.h
./debug_handler.lo: ../../include/net-snmp/library/mt_support.h
./debug_handler.lo: ../../include/net-snmp/library/snmp_alarm.h
./debug_handler.lo: ../../include/net-snmp/library/data_list.h
./debug_handler.lo: ../../include/net-snmp/library/callback.h
./debug_handler.lo: ../../include/net-snmp/library/oid_stash.h
./debug_handler.lo: ../../include/net-snmp/library/check_varbind.h
./debug_handler.lo: ../../include/net-snmp/version.h
./debug_handler.lo: ../../include/net-snmp/session_api.h
./debug_handler.lo: ../../include/net-snmp/library/snmp_transport.h
./debug_handler.lo: ../../include/net-snmp/library/snmpCallbackDomain.h
./debug_handler.lo: ../../include/net-snmp/library/snmpUnixDomain.h
./debug_handler.lo: ../../include/net-snmp/library/snmpUDPDomain.h
./debug_handler.lo: ../../include/net-snmp/library/snmpTCPDomain.h
./debug_handler.lo: ../../include/net-snmp/library/snmpUDPIPv6Domain.h
./debug_handler.lo: ../../include/net-snmp/library/snmpTCPIPv6Domain.h
./debug_handler.lo: ../../include/net-snmp/library/snmpIPXDomain.h
./debug_handler.lo: ../../include/net-snmp/library/ucd_compat.h
./debug_handler.lo: ../../include/net-snmp/pdu_api.h
./debug_handler.lo: ../../include/net-snmp/mib_api.h
./debug_handler.lo: ../../include/net-snmp/library/mib.h
./debug_handler.lo: ../../include/net-snmp/library/parse.h
./debug_handler.lo: ../../include/net-snmp/varbind_api.h
./debug_handler.lo: ../../include/net-snmp/config_api.h
./debug_handler.lo: ../../include/net-snmp/library/read_config.h
./debug_handler.lo: ../../include/net-snmp/library/default_store.h
./debug_handler.lo: ../../include/net-snmp/library/snmp_parse_args.h
./debug_handler.lo: ../../include/net-snmp/library/snmp_enum.h
./debug_handler.lo: ../../include/net-snmp/library/vacm.h
./debug_handler.lo: ../../include/net-snmp/output_api.h
./debug_handler.lo: ../../include/net-snmp/library/snmp_debug.h
./debug_handler.lo: ../../include/net-snmp/library/snmp_logging.h
./debug_handler.lo: ../../include/net-snmp/snmpv3_api.h
./debug_handler.lo: ../../include/net-snmp/library/snmpv3.h
./debug_handler.lo: ../../include/net-snmp/library/transform_oids.h
./debug_handler.lo: ../../include/net-snmp/library/keytools.h
./debug_handler.lo: ../../include/net-snmp/library/scapi.h
./debug_handler.lo: ../../include/net-snmp/library/lcd_time.h
./debug_handler.lo: ../../include/net-snmp/library/snmp_secmod.h
./debug_handler.lo: ../../include/net-snmp/library/snmpv3-security-includes.h
./debug_handler.lo: ../../include/net-snmp/library/snmpusm.h
./debug_handler.lo: ../../include/net-snmp/agent/net-snmp-agent-includes.h
./debug_handler.lo: ../../include/net-snmp/agent/snmp_agent.h
./debug_handler.lo: ../../include/net-snmp/agent/snmp_vars.h
./debug_handler.lo: ../../include/net-snmp/agent/agent_handler.h
./debug_handler.lo: ../../include/net-snmp/agent/var_struct.h
./debug_handler.lo: ../../include/net-snmp/agent/agent_registry.h
./debug_handler.lo: ../../include/net-snmp/agent/ds_agent.h
./debug_handler.lo: ../../include/net-snmp/agent/agent_read_config.h
./debug_handler.lo: ../../include/net-snmp/agent/agent_trap.h
./debug_handler.lo: ../../include/net-snmp/agent/all_helpers.h
./debug_handler.lo: ../../include/net-snmp/agent/instance.h
./debug_handler.lo: ../../include/net-snmp/agent/multiplexer.h
./debug_handler.lo: ../../include/net-snmp/agent/null.h
./debug_handler.lo: ../../include/net-snmp/agent/debug_handler.h
./debug_handler.lo: ../../include/net-snmp/agent/old_api.h
./debug_handler.lo: ../../include/net-snmp/agent/read_only.h
./debug_handler.lo: ../../include/net-snmp/agent/serialize.h
./debug_handler.lo: ../../include/net-snmp/agent/bulk_to_next.h
./debug_handler.lo: ../../include/net-snmp/agent/mode_end_call.h
./debug_handler.lo: ../../include/net-snmp/agent/table.h
./debug_handler.lo: ../../include/net-snmp/agent/table_data.h
./debug_handler.lo: ../../include/net-snmp/agent/table_dataset.h
./debug_handler.lo: ../../include/net-snmp/agent/table_iterator.h
./debug_handler.lo: ../../include/net-snmp/agent/mib_module_config.h
./instance.lo: ../../include/net-snmp/net-snmp-config.h
./instance.lo: ../../include/net-snmp/system/linux.h
./instance.lo: ../../include/net-snmp/system/sysv.h
./instance.lo: ../../include/net-snmp/system/generic.h
./instance.lo: ../../include/net-snmp/machine/generic.h 
./instance.lo: ../../include/net-snmp/net-snmp-includes.h
./instance.lo:  ../../include/net-snmp/definitions.h
./instance.lo: ../../include/net-snmp/types.h
./instance.lo: ../../include/net-snmp/library/snmp_api.h
./instance.lo: ../../include/net-snmp/library/asn1.h
./instance.lo: ../../include/net-snmp/library/snmp_impl.h
./instance.lo: ../../include/net-snmp/library/snmp.h
./instance.lo: ../../include/net-snmp/library/snmp-tc.h
./instance.lo: ../../include/net-snmp/utilities.h
./instance.lo: ../../include/net-snmp/library/snmp_client.h
./instance.lo: ../../include/net-snmp/library/getopt.h
./instance.lo: ../../include/net-snmp/library/system.h
./instance.lo: ../../include/net-snmp/library/tools.h
./instance.lo: ../../include/net-snmp/library/int64.h
./instance.lo: ../../include/net-snmp/library/mt_support.h
./instance.lo: ../../include/net-snmp/library/snmp_alarm.h
./instance.lo: ../../include/net-snmp/library/data_list.h
./instance.lo: ../../include/net-snmp/library/callback.h
./instance.lo: ../../include/net-snmp/library/oid_stash.h
./instance.lo: ../../include/net-snmp/library/check_varbind.h
./instance.lo: ../../include/net-snmp/version.h
./instance.lo: ../../include/net-snmp/session_api.h
./instance.lo: ../../include/net-snmp/library/snmp_transport.h
./instance.lo: ../../include/net-snmp/library/snmpCallbackDomain.h
./instance.lo: ../../include/net-snmp/library/snmpUnixDomain.h
./instance.lo: ../../include/net-snmp/library/snmpUDPDomain.h
./instance.lo: ../../include/net-snmp/library/snmpTCPDomain.h
./instance.lo: ../../include/net-snmp/library/snmpUDPIPv6Domain.h
./instance.lo: ../../include/net-snmp/library/snmpTCPIPv6Domain.h
./instance.lo: ../../include/net-snmp/library/snmpIPXDomain.h
./instance.lo: ../../include/net-snmp/library/ucd_compat.h
./instance.lo: ../../include/net-snmp/pdu_api.h
./instance.lo: ../../include/net-snmp/mib_api.h
./instance.lo: ../../include/net-snmp/library/mib.h
./instance.lo: ../../include/net-snmp/library/parse.h
./instance.lo: ../../include/net-snmp/varbind_api.h
./instance.lo: ../../include/net-snmp/config_api.h
./instance.lo: ../../include/net-snmp/library/read_config.h
./instance.lo: ../../include/net-snmp/library/default_store.h
./instance.lo: ../../include/net-snmp/library/snmp_parse_args.h
./instance.lo: ../../include/net-snmp/library/snmp_enum.h
./instance.lo: ../../include/net-snmp/library/vacm.h
./instance.lo: ../../include/net-snmp/output_api.h
./instance.lo: ../../include/net-snmp/library/snmp_debug.h
./instance.lo: ../../include/net-snmp/library/snmp_logging.h
./instance.lo: ../../include/net-snmp/snmpv3_api.h
./instance.lo: ../../include/net-snmp/library/snmpv3.h
./instance.lo: ../../include/net-snmp/library/transform_oids.h
./instance.lo: ../../include/net-snmp/library/keytools.h
./instance.lo: ../../include/net-snmp/library/scapi.h
./instance.lo: ../../include/net-snmp/library/lcd_time.h
./instance.lo: ../../include/net-snmp/library/snmp_secmod.h
./instance.lo: ../../include/net-snmp/library/snmpv3-security-includes.h
./instance.lo: ../../include/net-snmp/library/snmpusm.h
./instance.lo: ../../include/net-snmp/agent/net-snmp-agent-includes.h
./instance.lo: ../../include/net-snmp/agent/snmp_agent.h
./instance.lo: ../../include/net-snmp/agent/snmp_vars.h
./instance.lo: ../../include/net-snmp/agent/agent_handler.h
./instance.lo: ../../include/net-snmp/agent/var_struct.h
./instance.lo: ../../include/net-snmp/agent/agent_registry.h
./instance.lo: ../../include/net-snmp/agent/ds_agent.h
./instance.lo: ../../include/net-snmp/agent/agent_read_config.h
./instance.lo: ../../include/net-snmp/agent/agent_trap.h
./instance.lo: ../../include/net-snmp/agent/all_helpers.h
./instance.lo: ../../include/net-snmp/agent/instance.h
./instance.lo: ../../include/net-snmp/agent/multiplexer.h
./instance.lo: ../../include/net-snmp/agent/null.h
./instance.lo: ../../include/net-snmp/agent/debug_handler.h
./instance.lo: ../../include/net-snmp/agent/old_api.h
./instance.lo: ../../include/net-snmp/agent/read_only.h
./instance.lo: ../../include/net-snmp/agent/serialize.h
./instance.lo: ../../include/net-snmp/agent/bulk_to_next.h
./instance.lo: ../../include/net-snmp/agent/mode_end_call.h
./instance.lo: ../../include/net-snmp/agent/table.h
./instance.lo: ../../include/net-snmp/agent/table_data.h
./instance.lo: ../../include/net-snmp/agent/table_dataset.h
./instance.lo: ../../include/net-snmp/agent/table_iterator.h
./instance.lo: ../../include/net-snmp/agent/mib_module_config.h
./mode_end_call.lo: ../../include/net-snmp/net-snmp-config.h
./mode_end_call.lo: ../../include/net-snmp/system/linux.h
./mode_end_call.lo: ../../include/net-snmp/system/sysv.h
./mode_end_call.lo: ../../include/net-snmp/system/generic.h
./mode_end_call.lo: ../../include/net-snmp/machine/generic.h
./mode_end_call.lo: ../../include/net-snmp/net-snmp-includes.h
./mode_end_call.lo: ../../include/net-snmp/definitions.h
./mode_end_call.lo: ../../include/net-snmp/types.h
./mode_end_call.lo: ../../include/net-snmp/library/snmp_api.h
./mode_end_call.lo: ../../include/net-snmp/library/asn1.h
./mode_end_call.lo: ../../include/net-snmp/library/snmp_impl.h
./mode_end_call.lo: ../../include/net-snmp/library/snmp.h
./mode_end_call.lo: ../../include/net-snmp/library/snmp-tc.h
./mode_end_call.lo: ../../include/net-snmp/utilities.h
./mode_end_call.lo: ../../include/net-snmp/library/snmp_client.h
./mode_end_call.lo: ../../include/net-snmp/library/getopt.h
./mode_end_call.lo: ../../include/net-snmp/library/system.h
./mode_end_call.lo: ../../include/net-snmp/library/tools.h
./mode_end_call.lo: ../../include/net-snmp/library/int64.h
./mode_end_call.lo: ../../include/net-snmp/library/mt_support.h
./mode_end_call.lo: ../../include/net-snmp/library/snmp_alarm.h
./mode_end_call.lo: ../../include/net-snmp/library/data_list.h
./mode_end_call.lo: ../../include/net-snmp/library/callback.h
./mode_end_call.lo: ../../include/net-snmp/library/oid_stash.h
./mode_end_call.lo: ../../include/net-snmp/library/check_varbind.h
./mode_end_call.lo: ../../include/net-snmp/version.h
./mode_end_call.lo: ../../include/net-snmp/session_api.h
./mode_end_call.lo: ../../include/net-snmp/library/snmp_transport.h
./mode_end_call.lo: ../../include/net-snmp/library/snmpCallbackDomain.h
./mode_end_call.lo: ../../include/net-snmp/library/snmpUnixDomain.h
./mode_end_call.lo: ../../include/net-snmp/library/snmpUDPDomain.h
./mode_end_call.lo: ../../include/net-snmp/library/snmpTCPDomain.h
./mode_end_call.lo: ../../include/net-snmp/library/snmpUDPIPv6Domain.h
./mode_end_call.lo: ../../include/net-snmp/library/snmpTCPIPv6Domain.h
./mode_end_call.lo: ../../include/net-snmp/library/snmpIPXDomain.h
./mode_end_call.lo: ../../include/net-snmp/library/ucd_compat.h
./mode_end_call.lo: ../../include/net-snmp/pdu_api.h
./mode_end_call.lo: ../../include/net-snmp/mib_api.h
./mode_end_call.lo: ../../include/net-snmp/library/mib.h
./mode_end_call.lo: ../../include/net-snmp/library/parse.h
./mode_end_call.lo: ../../include/net-snmp/varbind_api.h
./mode_end_call.lo: ../../include/net-snmp/config_api.h
./mode_end_call.lo: ../../include/net-snmp/library/read_config.h
./mode_end_call.lo: ../../include/net-snmp/library/default_store.h
./mode_end_call.lo: ../../include/net-snmp/library/snmp_parse_args.h
./mode_end_call.lo: ../../include/net-snmp/library/snmp_enum.h
./mode_end_call.lo: ../../include/net-snmp/library/vacm.h
./mode_end_call.lo: ../../include/net-snmp/output_api.h
./mode_end_call.lo: ../../include/net-snmp/library/snmp_debug.h
./mode_end_call.lo: ../../include/net-snmp/library/snmp_logging.h
./mode_end_call.lo: ../../include/net-snmp/snmpv3_api.h
./mode_end_call.lo: ../../include/net-snmp/library/snmpv3.h
./mode_end_call.lo: ../../include/net-snmp/library/transform_oids.h
./mode_end_call.lo: ../../include/net-snmp/library/keytools.h
./mode_end_call.lo: ../../include/net-snmp/library/scapi.h
./mode_end_call.lo: ../../include/net-snmp/library/lcd_time.h
./mode_end_call.lo: ../../include/net-snmp/library/snmp_secmod.h
./mode_end_call.lo: ../../include/net-snmp/library/snmpv3-security-includes.h
./mode_end_call.lo: ../../include/net-snmp/library/snmpusm.h
./mode_end_call.lo: ../../include/net-snmp/agent/net-snmp-agent-includes.h
./mode_end_call.lo: ../../include/net-snmp/agent/snmp_agent.h
./mode_end_call.lo: ../../include/net-snmp/agent/snmp_vars.h
./mode_end_call.lo: ../../include/net-snmp/agent/agent_handler.h
./mode_end_call.lo: ../../include/net-snmp/agent/var_struct.h
./mode_end_call.lo: ../../include/net-snmp/agent/agent_registry.h
./mode_end_call.lo: ../../include/net-snmp/agent/ds_agent.h
./mode_end_call.lo: ../../include/net-snmp/agent/agent_read_config.h
./mode_end_call.lo: ../../include/net-snmp/agent/agent_trap.h
./mode_end_call.lo: ../../include/net-snmp/agent/all_helpers.h
./mode_end_call.lo: ../../include/net-snmp/agent/instance.h
./mode_end_call.lo: ../../include/net-snmp/agent/multiplexer.h
./mode_end_call.lo: ../../include/net-snmp/agent/null.h
./mode_end_call.lo: ../../include/net-snmp/agent/debug_handler.h
./mode_end_call.lo: ../../include/net-snmp/agent/old_api.h
./mode_end_call.lo: ../../include/net-snmp/agent/read_only.h
./mode_end_call.lo: ../../include/net-snmp/agent/serialize.h
./mode_end_call.lo: ../../include/net-snmp/agent/bulk_to_next.h
./mode_end_call.lo: ../../include/net-snmp/agent/mode_end_call.h
./mode_end_call.lo: ../../include/net-snmp/agent/table.h
./mode_end_call.lo: ../../include/net-snmp/agent/table_data.h
./mode_end_call.lo: ../../include/net-snmp/agent/table_dataset.h
./mode_end_call.lo: ../../include/net-snmp/agent/table_iterator.h
./mode_end_call.lo: ../../include/net-snmp/agent/mib_module_config.h
./multiplexer.lo: ../../include/net-snmp/net-snmp-config.h
./multiplexer.lo: ../../include/net-snmp/system/linux.h
./multiplexer.lo: ../../include/net-snmp/system/sysv.h
./multiplexer.lo: ../../include/net-snmp/system/generic.h
./multiplexer.lo: ../../include/net-snmp/machine/generic.h
./multiplexer.lo: ../../include/net-snmp/net-snmp-includes.h
./multiplexer.lo: ../../include/net-snmp/definitions.h
./multiplexer.lo: ../../include/net-snmp/types.h
./multiplexer.lo: ../../include/net-snmp/library/snmp_api.h
./multiplexer.lo: ../../include/net-snmp/library/asn1.h
./multiplexer.lo: ../../include/net-snmp/library/snmp_impl.h
./multiplexer.lo: ../../include/net-snmp/library/snmp.h
./multiplexer.lo: ../../include/net-snmp/library/snmp-tc.h
./multiplexer.lo: ../../include/net-snmp/utilities.h
./multiplexer.lo: ../../include/net-snmp/library/snmp_client.h
./multiplexer.lo: ../../include/net-snmp/library/getopt.h
./multiplexer.lo: ../../include/net-snmp/library/system.h
./multiplexer.lo: ../../include/net-snmp/library/tools.h
./multiplexer.lo: ../../include/net-snmp/library/int64.h
./multiplexer.lo: ../../include/net-snmp/library/mt_support.h
./multiplexer.lo: ../../include/net-snmp/library/snmp_alarm.h
./multiplexer.lo: ../../include/net-snmp/library/data_list.h
./multiplexer.lo: ../../include/net-snmp/library/callback.h
./multiplexer.lo: ../../include/net-snmp/library/oid_stash.h
./multiplexer.lo: ../../include/net-snmp/library/check_varbind.h
./multiplexer.lo: ../../include/net-snmp/version.h
./multiplexer.lo: ../../include/net-snmp/session_api.h
./multiplexer.lo: ../../include/net-snmp/library/snmp_transport.h
./multiplexer.lo: ../../include/net-snmp/library/snmpCallbackDomain.h
./multiplexer.lo: ../../include/net-snmp/library/snmpUnixDomain.h
./multiplexer.lo: ../../include/net-snmp/library/snmpUDPDomain.h
./multiplexer.lo: ../../include/net-snmp/library/snmpTCPDomain.h
./multiplexer.lo: ../../include/net-snmp/library/snmpUDPIPv6Domain.h
./multiplexer.lo: ../../include/net-snmp/library/snmpTCPIPv6Domain.h
./multiplexer.lo: ../../include/net-snmp/library/snmpIPXDomain.h
./multiplexer.lo: ../../include/net-snmp/library/ucd_compat.h
./multiplexer.lo: ../../include/net-snmp/pdu_api.h
./multiplexer.lo: ../../include/net-snmp/mib_api.h
./multiplexer.lo: ../../include/net-snmp/library/mib.h
./multiplexer.lo: ../../include/net-snmp/library/parse.h
./multiplexer.lo: ../../include/net-snmp/varbind_api.h
./multiplexer.lo: ../../include/net-snmp/config_api.h
./multiplexer.lo: ../../include/net-snmp/library/read_config.h
./multiplexer.lo: ../../include/net-snmp/library/default_store.h
./multiplexer.lo: ../../include/net-snmp/library/snmp_parse_args.h
./multiplexer.lo: ../../include/net-snmp/library/snmp_enum.h
./multiplexer.lo: ../../include/net-snmp/library/vacm.h
./multiplexer.lo: ../../include/net-snmp/output_api.h
./multiplexer.lo: ../../include/net-snmp/library/snmp_debug.h
./multiplexer.lo: ../../include/net-snmp/library/snmp_logging.h
./multiplexer.lo: ../../include/net-snmp/snmpv3_api.h
./multiplexer.lo: ../../include/net-snmp/library/snmpv3.h
./multiplexer.lo: ../../include/net-snmp/library/transform_oids.h
./multiplexer.lo: ../../include/net-snmp/library/keytools.h
./multiplexer.lo: ../../include/net-snmp/library/scapi.h
./multiplexer.lo: ../../include/net-snmp/library/lcd_time.h
./multiplexer.lo: ../../include/net-snmp/library/snmp_secmod.h
./multiplexer.lo: ../../include/net-snmp/library/snmpv3-security-includes.h
./multiplexer.lo: ../../include/net-snmp/library/snmpusm.h
./multiplexer.lo: ../../include/net-snmp/agent/net-snmp-agent-includes.h
./multiplexer.lo: ../../include/net-snmp/agent/snmp_agent.h
./multiplexer.lo: ../../include/net-snmp/agent/snmp_vars.h
./multiplexer.lo: ../../include/net-snmp/agent/agent_handler.h
./multiplexer.lo: ../../include/net-snmp/agent/var_struct.h
./multiplexer.lo: ../../include/net-snmp/agent/agent_registry.h
./multiplexer.lo: ../../include/net-snmp/agent/ds_agent.h
./multiplexer.lo: ../../include/net-snmp/agent/agent_read_config.h
./multiplexer.lo: ../../include/net-snmp/agent/agent_trap.h
./multiplexer.lo: ../../include/net-snmp/agent/all_helpers.h
./multiplexer.lo: ../../include/net-snmp/agent/instance.h
./multiplexer.lo: ../../include/net-snmp/agent/multiplexer.h
./multiplexer.lo: ../../include/net-snmp/agent/null.h
./multiplexer.lo: ../../include/net-snmp/agent/debug_handler.h
./multiplexer.lo: ../../include/net-snmp/agent/old_api.h
./multiplexer.lo: ../../include/net-snmp/agent/read_only.h
./multiplexer.lo: ../../include/net-snmp/agent/serialize.h
./multiplexer.lo: ../../include/net-snmp/agent/bulk_to_next.h
./multiplexer.lo: ../../include/net-snmp/agent/mode_end_call.h
./multiplexer.lo: ../../include/net-snmp/agent/table.h
./multiplexer.lo: ../../include/net-snmp/agent/table_data.h
./multiplexer.lo: ../../include/net-snmp/agent/table_dataset.h
./multiplexer.lo: ../../include/net-snmp/agent/table_iterator.h
./multiplexer.lo: ../../include/net-snmp/agent/mib_module_config.h
./null.lo: ../../include/net-snmp/net-snmp-config.h
./null.lo: ../../include/net-snmp/system/linux.h
./null.lo: ../../include/net-snmp/system/sysv.h
./null.lo: ../../include/net-snmp/system/generic.h
./null.lo: ../../include/net-snmp/machine/generic.h 
./null.lo:  ../../include/net-snmp/net-snmp-includes.h
./null.lo:  ../../include/net-snmp/definitions.h
./null.lo: ../../include/net-snmp/types.h
./null.lo: ../../include/net-snmp/library/snmp_api.h
./null.lo: ../../include/net-snmp/library/asn1.h
./null.lo: ../../include/net-snmp/library/snmp_impl.h
./null.lo: ../../include/net-snmp/library/snmp.h
./null.lo: ../../include/net-snmp/library/snmp-tc.h
./null.lo: ../../include/net-snmp/utilities.h
./null.lo: ../../include/net-snmp/library/snmp_client.h
./null.lo: ../../include/net-snmp/library/getopt.h
./null.lo: ../../include/net-snmp/library/system.h
./null.lo: ../../include/net-snmp/library/tools.h
./null.lo: ../../include/net-snmp/library/int64.h
./null.lo: ../../include/net-snmp/library/mt_support.h
./null.lo: ../../include/net-snmp/library/snmp_alarm.h
./null.lo: ../../include/net-snmp/library/data_list.h
./null.lo: ../../include/net-snmp/library/callback.h
./null.lo: ../../include/net-snmp/library/oid_stash.h
./null.lo: ../../include/net-snmp/library/check_varbind.h
./null.lo: ../../include/net-snmp/version.h
./null.lo: ../../include/net-snmp/session_api.h
./null.lo: ../../include/net-snmp/library/snmp_transport.h
./null.lo: ../../include/net-snmp/library/snmpCallbackDomain.h
./null.lo: ../../include/net-snmp/library/snmpUnixDomain.h
./null.lo: ../../include/net-snmp/library/snmpUDPDomain.h
./null.lo: ../../include/net-snmp/library/snmpTCPDomain.h
./null.lo: ../../include/net-snmp/library/snmpUDPIPv6Domain.h
./null.lo: ../../include/net-snmp/library/snmpTCPIPv6Domain.h
./null.lo: ../../include/net-snmp/library/snmpIPXDomain.h
./null.lo: ../../include/net-snmp/library/ucd_compat.h
./null.lo: ../../include/net-snmp/pdu_api.h ../../include/net-snmp/mib_api.h
./null.lo: ../../include/net-snmp/library/mib.h
./null.lo: ../../include/net-snmp/library/parse.h
./null.lo: ../../include/net-snmp/varbind_api.h
./null.lo: ../../include/net-snmp/config_api.h
./null.lo: ../../include/net-snmp/library/read_config.h
./null.lo: ../../include/net-snmp/library/default_store.h
./null.lo: ../../include/net-snmp/library/snmp_parse_args.h
./null.lo: ../../include/net-snmp/library/snmp_enum.h
./null.lo: ../../include/net-snmp/library/vacm.h
./null.lo: ../../include/net-snmp/output_api.h
./null.lo: ../../include/net-snmp/library/snmp_debug.h
./null.lo: ../../include/net-snmp/library/snmp_logging.h
./null.lo: ../../include/net-snmp/snmpv3_api.h
./null.lo: ../../include/net-snmp/library/snmpv3.h
./null.lo: ../../include/net-snmp/library/transform_oids.h
./null.lo: ../../include/net-snmp/library/keytools.h
./null.lo: ../../include/net-snmp/library/scapi.h
./null.lo: ../../include/net-snmp/library/lcd_time.h
./null.lo: ../../include/net-snmp/library/snmp_secmod.h
./null.lo: ../../include/net-snmp/library/snmpv3-security-includes.h
./null.lo: ../../include/net-snmp/library/snmpusm.h
./null.lo: ../../include/net-snmp/agent/net-snmp-agent-includes.h
./null.lo: ../../include/net-snmp/agent/snmp_agent.h
./null.lo: ../../include/net-snmp/agent/snmp_vars.h
./null.lo: ../../include/net-snmp/agent/agent_handler.h
./null.lo: ../../include/net-snmp/agent/var_struct.h
./null.lo: ../../include/net-snmp/agent/agent_registry.h
./null.lo: ../../include/net-snmp/agent/ds_agent.h
./null.lo: ../../include/net-snmp/agent/agent_read_config.h
./null.lo: ../../include/net-snmp/agent/agent_trap.h
./null.lo: ../../include/net-snmp/agent/all_helpers.h
./null.lo: ../../include/net-snmp/agent/instance.h
./null.lo: ../../include/net-snmp/agent/multiplexer.h
./null.lo: ../../include/net-snmp/agent/null.h
./null.lo: ../../include/net-snmp/agent/debug_handler.h
./null.lo: ../../include/net-snmp/agent/old_api.h
./null.lo: ../../include/net-snmp/agent/read_only.h
./null.lo: ../../include/net-snmp/agent/serialize.h
./null.lo: ../../include/net-snmp/agent/bulk_to_next.h
./null.lo: ../../include/net-snmp/agent/mode_end_call.h
./null.lo: ../../include/net-snmp/agent/table.h
./null.lo: ../../include/net-snmp/agent/table_data.h
./null.lo: ../../include/net-snmp/agent/table_dataset.h
./null.lo: ../../include/net-snmp/agent/table_iterator.h
./null.lo: ../../include/net-snmp/agent/mib_module_config.h
./old_api.lo: ../../include/net-snmp/net-snmp-config.h
./old_api.lo: ../../include/net-snmp/system/linux.h
./old_api.lo: ../../include/net-snmp/system/sysv.h
./old_api.lo: ../../include/net-snmp/system/generic.h
./old_api.lo: ../../include/net-snmp/machine/generic.h 
./old_api.lo: ../../include/net-snmp/net-snmp-includes.h 
./old_api.lo:  ../../include/net-snmp/definitions.h
./old_api.lo: ../../include/net-snmp/types.h
./old_api.lo: ../../include/net-snmp/library/snmp_api.h
./old_api.lo: ../../include/net-snmp/library/asn1.h
./old_api.lo: ../../include/net-snmp/library/snmp_impl.h
./old_api.lo: ../../include/net-snmp/library/snmp.h
./old_api.lo: ../../include/net-snmp/library/snmp-tc.h
./old_api.lo: ../../include/net-snmp/utilities.h
./old_api.lo: ../../include/net-snmp/library/snmp_client.h
./old_api.lo: ../../include/net-snmp/library/getopt.h
./old_api.lo: ../../include/net-snmp/library/system.h
./old_api.lo: ../../include/net-snmp/library/tools.h
./old_api.lo: ../../include/net-snmp/library/int64.h
./old_api.lo: ../../include/net-snmp/library/mt_support.h
./old_api.lo: ../../include/net-snmp/library/snmp_alarm.h
./old_api.lo: ../../include/net-snmp/library/data_list.h
./old_api.lo: ../../include/net-snmp/library/callback.h
./old_api.lo: ../../include/net-snmp/library/oid_stash.h
./old_api.lo: ../../include/net-snmp/library/check_varbind.h
./old_api.lo: ../../include/net-snmp/version.h
./old_api.lo: ../../include/net-snmp/session_api.h
./old_api.lo: ../../include/net-snmp/library/snmp_transport.h
./old_api.lo: ../../include/net-snmp/library/snmpCallbackDomain.h
./old_api.lo: ../../include/net-snmp/library/snmpUnixDomain.h
./old_api.lo: ../../include/net-snmp/library/snmpUDPDomain.h
./old_api.lo: ../../include/net-snmp/library/snmpTCPDomain.h
./old_api.lo: ../../include/net-snmp/library/snmpUDPIPv6Domain.h
./old_api.lo: ../../include/net-snmp/library/snmpTCPIPv6Domain.h
./old_api.lo: ../../include/net-snmp/library/snmpIPXDomain.h
./old_api.lo: ../../include/net-snmp/library/ucd_compat.h
./old_api.lo: ../../include/net-snmp/pdu_api.h
./old_api.lo: ../../include/net-snmp/mib_api.h
./old_api.lo: ../../include/net-snmp/library/mib.h
./old_api.lo: ../../include/net-snmp/library/parse.h
./old_api.lo: ../../include/net-snmp/varbind_api.h
./old_api.lo: ../../include/net-snmp/config_api.h
./old_api.lo: ../../include/net-snmp/library/read_config.h
./old_api.lo: ../../include/net-snmp/library/default_store.h
./old_api.lo: ../../include/net-snmp/library/snmp_parse_args.h
./old_api.lo: ../../include/net-snmp/library/snmp_enum.h
./old_api.lo: ../../include/net-snmp/library/vacm.h
./old_api.lo: ../../include/net-snmp/output_api.h
./old_api.lo: ../../include/net-snmp/library/snmp_debug.h
./old_api.lo: ../../include/net-snmp/library/snmp_logging.h
./old_api.lo: ../../include/net-snmp/snmpv3_api.h
./old_api.lo: ../../include/net-snmp/library/snmpv3.h
./old_api.lo: ../../include/net-snmp/library/transform_oids.h
./old_api.lo: ../../include/net-snmp/library/keytools.h
./old_api.lo: ../../include/net-snmp/library/scapi.h
./old_api.lo: ../../include/net-snmp/library/lcd_time.h
./old_api.lo: ../../include/net-snmp/library/snmp_secmod.h
./old_api.lo: ../../include/net-snmp/library/snmpv3-security-includes.h
./old_api.lo: ../../include/net-snmp/library/snmpusm.h
./old_api.lo: ../../include/net-snmp/agent/net-snmp-agent-includes.h
./old_api.lo: ../../include/net-snmp/agent/snmp_agent.h
./old_api.lo: ../../include/net-snmp/agent/snmp_vars.h
./old_api.lo: ../../include/net-snmp/agent/agent_handler.h
./old_api.lo: ../../include/net-snmp/agent/var_struct.h
./old_api.lo: ../../include/net-snmp/agent/agent_registry.h
./old_api.lo: ../../include/net-snmp/agent/ds_agent.h
./old_api.lo: ../../include/net-snmp/agent/agent_read_config.h
./old_api.lo: ../../include/net-snmp/agent/agent_trap.h
./old_api.lo: ../../include/net-snmp/agent/all_helpers.h
./old_api.lo: ../../include/net-snmp/agent/instance.h
./old_api.lo: ../../include/net-snmp/agent/multiplexer.h
./old_api.lo: ../../include/net-snmp/agent/null.h
./old_api.lo: ../../include/net-snmp/agent/debug_handler.h
./old_api.lo: ../../include/net-snmp/agent/old_api.h
./old_api.lo: ../../include/net-snmp/agent/read_only.h
./old_api.lo: ../../include/net-snmp/agent/serialize.h
./old_api.lo: ../../include/net-snmp/agent/bulk_to_next.h
./old_api.lo: ../../include/net-snmp/agent/mode_end_call.h
./old_api.lo: ../../include/net-snmp/agent/table.h
./old_api.lo: ../../include/net-snmp/agent/table_data.h
./old_api.lo: ../../include/net-snmp/agent/table_dataset.h
./old_api.lo: ../../include/net-snmp/agent/table_iterator.h
./old_api.lo: ../../include/net-snmp/agent/mib_module_config.h
./old_api.lo: ../../include/net-snmp/agent/agent_callbacks.h
./read_only.lo: ../../include/net-snmp/net-snmp-config.h
./read_only.lo: ../../include/net-snmp/system/linux.h
./read_only.lo: ../../include/net-snmp/system/sysv.h
./read_only.lo: ../../include/net-snmp/system/generic.h
./read_only.lo: ../../include/net-snmp/machine/generic.h
./read_only.lo: ../../include/net-snmp/net-snmp-includes.h
./read_only.lo:  ../../include/net-snmp/definitions.h
./read_only.lo: ../../include/net-snmp/types.h
./read_only.lo: ../../include/net-snmp/library/snmp_api.h
./read_only.lo: ../../include/net-snmp/library/asn1.h
./read_only.lo: ../../include/net-snmp/library/snmp_impl.h
./read_only.lo: ../../include/net-snmp/library/snmp.h
./read_only.lo: ../../include/net-snmp/library/snmp-tc.h
./read_only.lo: ../../include/net-snmp/utilities.h
./read_only.lo: ../../include/net-snmp/library/snmp_client.h
./read_only.lo: ../../include/net-snmp/library/getopt.h
./read_only.lo: ../../include/net-snmp/library/system.h
./read_only.lo: ../../include/net-snmp/library/tools.h
./read_only.lo: ../../include/net-snmp/library/int64.h
./read_only.lo: ../../include/net-snmp/library/mt_support.h
./read_only.lo: ../../include/net-snmp/library/snmp_alarm.h
./read_only.lo: ../../include/net-snmp/library/data_list.h
./read_only.lo: ../../include/net-snmp/library/callback.h
./read_only.lo: ../../include/net-snmp/library/oid_stash.h
./read_only.lo: ../../include/net-snmp/library/check_varbind.h
./read_only.lo: ../../include/net-snmp/version.h
./read_only.lo: ../../include/net-snmp/session_api.h
./read_only.lo: ../../include/net-snmp/library/snmp_transport.h
./read_only.lo: ../../include/net-snmp/library/snmpCallbackDomain.h
./read_only.lo: ../../include/net-snmp/library/snmpUnixDomain.h
./read_only.lo: ../../include/net-snmp/library/snmpUDPDomain.h
./read_only.lo: ../../include/net-snmp/library/snmpTCPDomain.h
./read_only.lo: ../../include/net-snmp/library/snmpUDPIPv6Domain.h
./read_only.lo: ../../include/net-snmp/library/snmpTCPIPv6Domain.h
./read_only.lo: ../../include/net-snmp/library/snmpIPXDomain.h
./read_only.lo: ../../include/net-snmp/library/ucd_compat.h
./read_only.lo: ../../include/net-snmp/pdu_api.h
./read_only.lo: ../../include/net-snmp/mib_api.h
./read_only.lo: ../../include/net-snmp/library/mib.h
./read_only.lo: ../../include/net-snmp/library/parse.h
./read_only.lo: ../../include/net-snmp/varbind_api.h
./read_only.lo: ../../include/net-snmp/config_api.h
./read_only.lo: ../../include/net-snmp/library/read_config.h
./read_only.lo: ../../include/net-snmp/library/default_store.h
./read_only.lo: ../../include/net-snmp/library/snmp_parse_args.h
./read_only.lo: ../../include/net-snmp/library/snmp_enum.h
./read_only.lo: ../../include/net-snmp/library/vacm.h
./read_only.lo: ../../include/net-snmp/output_api.h
./read_only.lo: ../../include/net-snmp/library/snmp_debug.h
./read_only.lo: ../../include/net-snmp/library/snmp_logging.h
./read_only.lo: ../../include/net-snmp/snmpv3_api.h
./read_only.lo: ../../include/net-snmp/library/snmpv3.h
./read_only.lo: ../../include/net-snmp/library/transform_oids.h
./read_only.lo: ../../include/net-snmp/library/keytools.h
./read_only.lo: ../../include/net-snmp/library/scapi.h
./read_only.lo: ../../include/net-snmp/library/lcd_time.h
./read_only.lo: ../../include/net-snmp/library/snmp_secmod.h
./read_only.lo: ../../include/net-snmp/library/snmpv3-security-includes.h
./read_only.lo: ../../include/net-snmp/library/snmpusm.h
./read_only.lo: ../../include/net-snmp/agent/net-snmp-agent-includes.h
./read_only.lo: ../../include/net-snmp/agent/snmp_agent.h
./read_only.lo: ../../include/net-snmp/agent/snmp_vars.h
./read_only.lo: ../../include/net-snmp/agent/agent_handler.h
./read_only.lo: ../../include/net-snmp/agent/var_struct.h
./read_only.lo: ../../include/net-snmp/agent/agent_registry.h
./read_only.lo: ../../include/net-snmp/agent/ds_agent.h
./read_only.lo: ../../include/net-snmp/agent/agent_read_config.h
./read_only.lo: ../../include/net-snmp/agent/agent_trap.h
./read_only.lo: ../../include/net-snmp/agent/all_helpers.h
./read_only.lo: ../../include/net-snmp/agent/instance.h
./read_only.lo: ../../include/net-snmp/agent/multiplexer.h
./read_only.lo: ../../include/net-snmp/agent/null.h
./read_only.lo: ../../include/net-snmp/agent/debug_handler.h
./read_only.lo: ../../include/net-snmp/agent/old_api.h
./read_only.lo: ../../include/net-snmp/agent/read_only.h
./read_only.lo: ../../include/net-snmp/agent/serialize.h
./read_only.lo: ../../include/net-snmp/agent/bulk_to_next.h
./read_only.lo: ../../include/net-snmp/agent/mode_end_call.h
./read_only.lo: ../../include/net-snmp/agent/table.h
./read_only.lo: ../../include/net-snmp/agent/table_data.h
./read_only.lo: ../../include/net-snmp/agent/table_dataset.h
./read_only.lo: ../../include/net-snmp/agent/table_iterator.h
./read_only.lo: ../../include/net-snmp/agent/mib_module_config.h
./scalar1.lo: ../../include/net-snmp/net-snmp-config.h
./scalar1.lo: ../../include/net-snmp/system/linux.h
./scalar1.lo: ../../include/net-snmp/system/sysv.h
./scalar1.lo: ../../include/net-snmp/system/generic.h
./scalar1.lo: ../../include/net-snmp/machine/generic.h 
./scalar1.lo: ../../include/net-snmp/net-snmp-includes.h 
./scalar1.lo:  ../../include/net-snmp/definitions.h
./scalar1.lo: ../../include/net-snmp/types.h
./scalar1.lo: ../../include/net-snmp/library/snmp_api.h
./scalar1.lo: ../../include/net-snmp/library/asn1.h
./scalar1.lo: ../../include/net-snmp/library/snmp_impl.h
./scalar1.lo: ../../include/net-snmp/library/snmp.h
./scalar1.lo: ../../include/net-snmp/library/snmp-tc.h
./scalar1.lo: ../../include/net-snmp/utilities.h
./scalar1.lo: ../../include/net-snmp/library/snmp_client.h
./scalar1.lo: ../../include/net-snmp/library/getopt.h
./scalar1.lo: ../../include/net-snmp/library/system.h
./scalar1.lo: ../../include/net-snmp/library/tools.h
./scalar1.lo: ../../include/net-snmp/library/int64.h
./scalar1.lo: ../../include/net-snmp/library/mt_support.h
./scalar1.lo: ../../include/net-snmp/library/snmp_alarm.h
./scalar1.lo: ../../include/net-snmp/library/data_list.h
./scalar1.lo: ../../include/net-snmp/library/callback.h
./scalar1.lo: ../../include/net-snmp/library/oid_stash.h
./scalar1.lo: ../../include/net-snmp/library/check_varbind.h
./scalar1.lo: ../../include/net-snmp/version.h
./scalar1.lo: ../../include/net-snmp/session_api.h
./scalar1.lo: ../../include/net-snmp/library/snmp_transport.h
./scalar1.lo: ../../include/net-snmp/library/snmpCallbackDomain.h
./scalar1.lo: ../../include/net-snmp/library/snmpUnixDomain.h
./scalar1.lo: ../../include/net-snmp/library/snmpUDPDomain.h
./scalar1.lo: ../../include/net-snmp/library/snmpTCPDomain.h
./scalar1.lo: ../../include/net-snmp/library/snmpUDPIPv6Domain.h
./scalar1.lo: ../../include/net-snmp/library/snmpTCPIPv6Domain.h
./scalar1.lo: ../../include/net-snmp/library/snmpIPXDomain.h
./scalar1.lo: ../../include/net-snmp/library/ucd_compat.h
./scalar1.lo: ../../include/net-snmp/pdu_api.h
./scalar1.lo: ../../include/net-snmp/mib_api.h
./scalar1.lo: ../../include/net-snmp/library/mib.h
./scalar1.lo: ../../include/net-snmp/library/parse.h
./scalar1.lo: ../../include/net-snmp/varbind_api.h
./scalar1.lo: ../../include/net-snmp/config_api.h
./scalar1.lo: ../../include/net-snmp/library/read_config.h
./scalar1.lo: ../../include/net-snmp/library/default_store.h
./scalar1.lo: ../../include/net-snmp/library/snmp_parse_args.h
./scalar1.lo: ../../include/net-snmp/library/snmp_enum.h
./scalar1.lo: ../../include/net-snmp/library/vacm.h
./scalar1.lo: ../../include/net-snmp/output_api.h
./scalar1.lo: ../../include/net-snmp/library/snmp_debug.h
./scalar1.lo: ../../include/net-snmp/library/snmp_logging.h
./scalar1.lo: ../../include/net-snmp/snmpv3_api.h
./scalar1.lo: ../../include/net-snmp/library/snmpv3.h
./scalar1.lo: ../../include/net-snmp/library/transform_oids.h
./scalar1.lo: ../../include/net-snmp/library/keytools.h
./scalar1.lo: ../../include/net-snmp/library/scapi.h
./scalar1.lo: ../../include/net-snmp/library/lcd_time.h
./scalar1.lo: ../../include/net-snmp/library/snmp_secmod.h
./scalar1.lo: ../../include/net-snmp/library/snmpv3-security-includes.h
./scalar1.lo: ../../include/net-snmp/library/snmpusm.h
./scalar1.lo: ../../include/net-snmp/agent/net-snmp-agent-includes.h
./scalar1.lo: ../../include/net-snmp/agent/snmp_agent.h
./scalar1.lo: ../../include/net-snmp/agent/snmp_vars.h
./scalar1.lo: ../../include/net-snmp/agent/agent_handler.h
./scalar1.lo: ../../include/net-snmp/agent/var_struct.h
./scalar1.lo: ../../include/net-snmp/agent/agent_registry.h
./scalar1.lo: ../../include/net-snmp/agent/ds_agent.h
./scalar1.lo: ../../include/net-snmp/agent/agent_read_config.h
./scalar1.lo: ../../include/net-snmp/agent/agent_trap.h
./scalar1.lo: ../../include/net-snmp/agent/all_helpers.h
./scalar1.lo: ../../include/net-snmp/agent/instance.h
./scalar1.lo: ../../include/net-snmp/agent/multiplexer.h
./scalar1.lo: ../../include/net-snmp/agent/null.h
./scalar1.lo: ../../include/net-snmp/agent/debug_handler.h
./scalar1.lo: ../../include/net-snmp/agent/old_api.h
./scalar1.lo: ../../include/net-snmp/agent/read_only.h
./scalar1.lo: ../../include/net-snmp/agent/serialize.h
./scalar1.lo: ../../include/net-snmp/agent/bulk_to_next.h
./scalar1.lo: ../../include/net-snmp/agent/mode_end_call.h
./scalar1.lo: ../../include/net-snmp/agent/table.h
./scalar1.lo: ../../include/net-snmp/agent/table_data.h
./scalar1.lo: ../../include/net-snmp/agent/table_dataset.h
./scalar1.lo: ../../include/net-snmp/agent/table_iterator.h
./scalar1.lo: ../../include/net-snmp/agent/mib_module_config.h
./scalar1.lo: ../../include/net-snmp/agent/scalar.h
./scalar.lo: ../../include/net-snmp/net-snmp-config.h
./scalar.lo: ../../include/net-snmp/system/linux.h
./scalar.lo: ../../include/net-snmp/system/sysv.h
./scalar.lo: ../../include/net-snmp/system/generic.h
./scalar.lo: ../../include/net-snmp/machine/generic.h 
./scalar.lo:  ../../include/net-snmp/net-snmp-includes.h
./scalar.lo:  ../../include/net-snmp/definitions.h
./scalar.lo: ../../include/net-snmp/types.h
./scalar.lo: ../../include/net-snmp/library/snmp_api.h
./scalar.lo: ../../include/net-snmp/library/asn1.h
./scalar.lo: ../../include/net-snmp/library/snmp_impl.h
./scalar.lo: ../../include/net-snmp/library/snmp.h
./scalar.lo: ../../include/net-snmp/library/snmp-tc.h
./scalar.lo: ../../include/net-snmp/utilities.h
./scalar.lo: ../../include/net-snmp/library/snmp_client.h
./scalar.lo: ../../include/net-snmp/library/getopt.h
./scalar.lo: ../../include/net-snmp/library/system.h
./scalar.lo: ../../include/net-snmp/library/tools.h
./scalar.lo: ../../include/net-snmp/library/int64.h
./scalar.lo: ../../include/net-snmp/library/mt_support.h
./scalar.lo: ../../include/net-snmp/library/snmp_alarm.h
./scalar.lo: ../../include/net-snmp/library/data_list.h
./scalar.lo: ../../include/net-snmp/library/callback.h
./scalar.lo: ../../include/net-snmp/library/oid_stash.h
./scalar.lo: ../../include/net-snmp/library/check_varbind.h
./scalar.lo: ../../include/net-snmp/version.h
./scalar.lo: ../../include/net-snmp/session_api.h
./scalar.lo: ../../include/net-snmp/library/snmp_transport.h
./scalar.lo: ../../include/net-snmp/library/snmpCallbackDomain.h
./scalar.lo: ../../include/net-snmp/library/snmpUnixDomain.h
./scalar.lo: ../../include/net-snmp/library/snmpUDPDomain.h
./scalar.lo: ../../include/net-snmp/library/snmpTCPDomain.h
./scalar.lo: ../../include/net-snmp/library/snmpUDPIPv6Domain.h
./scalar.lo: ../../include/net-snmp/library/snmpTCPIPv6Domain.h
./scalar.lo: ../../include/net-snmp/library/snmpIPXDomain.h
./scalar.lo: ../../include/net-snmp/library/ucd_compat.h
./scalar.lo: ../../include/net-snmp/pdu_api.h
./scalar.lo: ../../include/net-snmp/mib_api.h
./scalar.lo: ../../include/net-snmp/library/mib.h
./scalar.lo: ../../include/net-snmp/library/parse.h
./scalar.lo: ../../include/net-snmp/varbind_api.h
./scalar.lo: ../../include/net-snmp/config_api.h
./scalar.lo: ../../include/net-snmp/library/read_config.h
./scalar.lo: ../../include/net-snmp/library/default_store.h
./scalar.lo: ../../include/net-snmp/library/snmp_parse_args.h
./scalar.lo: ../../include/net-snmp/library/snmp_enum.h
./scalar.lo: ../../include/net-snmp/library/vacm.h
./scalar.lo: ../../include/net-snmp/output_api.h
./scalar.lo: ../../include/net-snmp/library/snmp_debug.h
./scalar.lo: ../../include/net-snmp/library/snmp_logging.h
./scalar.lo: ../../include/net-snmp/snmpv3_api.h
./scalar.lo: ../../include/net-snmp/library/snmpv3.h
./scalar.lo: ../../include/net-snmp/library/transform_oids.h
./scalar.lo: ../../include/net-snmp/library/keytools.h
./scalar.lo: ../../include/net-snmp/library/scapi.h
./scalar.lo: ../../include/net-snmp/library/lcd_time.h
./scalar.lo: ../../include/net-snmp/library/snmp_secmod.h
./scalar.lo: ../../include/net-snmp/library/snmpv3-security-includes.h
./scalar.lo: ../../include/net-snmp/library/snmpusm.h
./scalar.lo: ../../include/net-snmp/agent/net-snmp-agent-includes.h
./scalar.lo: ../../include/net-snmp/agent/snmp_agent.h
./scalar.lo: ../../include/net-snmp/agent/snmp_vars.h
./scalar.lo: ../../include/net-snmp/agent/agent_handler.h
./scalar.lo: ../../include/net-snmp/agent/var_struct.h
./scalar.lo: ../../include/net-snmp/agent/agent_registry.h
./scalar.lo: ../../include/net-snmp/agent/ds_agent.h
./scalar.lo: ../../include/net-snmp/agent/agent_read_config.h
./scalar.lo: ../../include/net-snmp/agent/agent_trap.h
./scalar.lo: ../../include/net-snmp/agent/all_helpers.h
./scalar.lo: ../../include/net-snmp/agent/instance.h
./scalar.lo: ../../include/net-snmp/agent/multiplexer.h
./scalar.lo: ../../include/net-snmp/agent/null.h
./scalar.lo: ../../include/net-snmp/agent/debug_handler.h
./scalar.lo: ../../include/net-snmp/agent/old_api.h
./scalar.lo: ../../include/net-snmp/agent/read_only.h
./scalar.lo: ../../include/net-snmp/agent/serialize.h
./scalar.lo: ../../include/net-snmp/agent/bulk_to_next.h
./scalar.lo: ../../include/net-snmp/agent/mode_end_call.h
./scalar.lo: ../../include/net-snmp/agent/table.h
./scalar.lo: ../../include/net-snmp/agent/table_data.h
./scalar.lo: ../../include/net-snmp/agent/table_dataset.h
./scalar.lo: ../../include/net-snmp/agent/table_iterator.h
./scalar.lo: ../../include/net-snmp/agent/mib_module_config.h
./scalar.lo: ../../include/net-snmp/agent/scalar.h
./serialize.lo: ../../include/net-snmp/net-snmp-config.h
./serialize.lo: ../../include/net-snmp/system/linux.h
./serialize.lo: ../../include/net-snmp/system/sysv.h
./serialize.lo: ../../include/net-snmp/system/generic.h
./serialize.lo: ../../include/net-snmp/machine/generic.h
./serialize.lo: ../../include/net-snmp/net-snmp-includes.h
./serialize.lo:  ../../include/net-snmp/definitions.h
./serialize.lo: ../../include/net-snmp/types.h
./serialize.lo: ../../include/net-snmp/library/snmp_api.h
./serialize.lo: ../../include/net-snmp/library/asn1.h
./serialize.lo: ../../include/net-snmp/library/snmp_impl.h
./serialize.lo: ../../include/net-snmp/library/snmp.h
./serialize.lo: ../../include/net-snmp/library/snmp-tc.h
./serialize.lo: ../../include/net-snmp/utilities.h
./serialize.lo: ../../include/net-snmp/library/snmp_client.h
./serialize.lo: ../../include/net-snmp/library/getopt.h
./serialize.lo: ../../include/net-snmp/library/system.h
./serialize.lo: ../../include/net-snmp/library/tools.h
./serialize.lo: ../../include/net-snmp/library/int64.h
./serialize.lo: ../../include/net-snmp/library/mt_support.h
./serialize.lo: ../../include/net-snmp/library/snmp_alarm.h
./serialize.lo: ../../include/net-snmp/library/data_list.h
./serialize.lo: ../../include/net-snmp/library/callback.h
./serialize.lo: ../../include/net-snmp/library/oid_stash.h
./serialize.lo: ../../include/net-snmp/library/check_varbind.h
./serialize.lo: ../../include/net-snmp/version.h
./serialize.lo: ../../include/net-snmp/session_api.h
./serialize.lo: ../../include/net-snmp/library/snmp_transport.h
./serialize.lo: ../../include/net-snmp/library/snmpCallbackDomain.h
./serialize.lo: ../../include/net-snmp/library/snmpUnixDomain.h
./serialize.lo: ../../include/net-snmp/library/snmpUDPDomain.h
./serialize.lo: ../../include/net-snmp/library/snmpTCPDomain.h
./serialize.lo: ../../include/net-snmp/library/snmpUDPIPv6Domain.h
./serialize.lo: ../../include/net-snmp/library/snmpTCPIPv6Domain.h
./serialize.lo: ../../include/net-snmp/library/snmpIPXDomain.h
./serialize.lo: ../../include/net-snmp/library/ucd_compat.h
./serialize.lo: ../../include/net-snmp/pdu_api.h
./serialize.lo: ../../include/net-snmp/mib_api.h
./serialize.lo: ../../include/net-snmp/library/mib.h
./serialize.lo: ../../include/net-snmp/library/parse.h
./serialize.lo: ../../include/net-snmp/varbind_api.h
./serialize.lo: ../../include/net-snmp/config_api.h
./serialize.lo: ../../include/net-snmp/library/read_config.h
./serialize.lo: ../../include/net-snmp/library/default_store.h
./serialize.lo: ../../include/net-snmp/library/snmp_parse_args.h
./serialize.lo: ../../include/net-snmp/library/snmp_enum.h
./serialize.lo: ../../include/net-snmp/library/vacm.h
./serialize.lo: ../../include/net-snmp/output_api.h
./serialize.lo: ../../include/net-snmp/library/snmp_debug.h
./serialize.lo: ../../include/net-snmp/library/snmp_logging.h
./serialize.lo: ../../include/net-snmp/snmpv3_api.h
./serialize.lo: ../../include/net-snmp/library/snmpv3.h
./serialize.lo: ../../include/net-snmp/library/transform_oids.h
./serialize.lo: ../../include/net-snmp/library/keytools.h
./serialize.lo: ../../include/net-snmp/library/scapi.h
./serialize.lo: ../../include/net-snmp/library/lcd_time.h
./serialize.lo: ../../include/net-snmp/library/snmp_secmod.h
./serialize.lo: ../../include/net-snmp/library/snmpv3-security-includes.h
./serialize.lo: ../../include/net-snmp/library/snmpusm.h
./serialize.lo: ../../include/net-snmp/agent/net-snmp-agent-includes.h
./serialize.lo: ../../include/net-snmp/agent/snmp_agent.h
./serialize.lo: ../../include/net-snmp/agent/snmp_vars.h
./serialize.lo: ../../include/net-snmp/agent/agent_handler.h
./serialize.lo: ../../include/net-snmp/agent/var_struct.h
./serialize.lo: ../../include/net-snmp/agent/agent_registry.h
./serialize.lo: ../../include/net-snmp/agent/ds_agent.h
./serialize.lo: ../../include/net-snmp/agent/agent_read_config.h
./serialize.lo: ../../include/net-snmp/agent/agent_trap.h
./serialize.lo: ../../include/net-snmp/agent/all_helpers.h
./serialize.lo: ../../include/net-snmp/agent/instance.h
./serialize.lo: ../../include/net-snmp/agent/multiplexer.h
./serialize.lo: ../../include/net-snmp/agent/null.h
./serialize.lo: ../../include/net-snmp/agent/debug_handler.h
./serialize.lo: ../../include/net-snmp/agent/old_api.h
./serialize.lo: ../../include/net-snmp/agent/read_only.h
./serialize.lo: ../../include/net-snmp/agent/serialize.h
./serialize.lo: ../../include/net-snmp/agent/bulk_to_next.h
./serialize.lo: ../../include/net-snmp/agent/mode_end_call.h
./serialize.lo: ../../include/net-snmp/agent/table.h
./serialize.lo: ../../include/net-snmp/agent/table_data.h
./serialize.lo: ../../include/net-snmp/agent/table_dataset.h
./serialize.lo: ../../include/net-snmp/agent/table_iterator.h
./serialize.lo: ../../include/net-snmp/agent/mib_module_config.h
./table_array.lo: ../../include/net-snmp/net-snmp-config.h
./table_array.lo: ../../include/net-snmp/system/linux.h
./table_array.lo: ../../include/net-snmp/system/sysv.h
./table_array.lo: ../../include/net-snmp/system/generic.h
./table_array.lo: ../../include/net-snmp/machine/generic.h
./table_array.lo: ../../include/net-snmp/net-snmp-includes.h
./table_array.lo: ../../include/net-snmp/definitions.h
./table_array.lo: ../../include/net-snmp/types.h
./table_array.lo: ../../include/net-snmp/library/snmp_api.h
./table_array.lo: ../../include/net-snmp/library/asn1.h
./table_array.lo: ../../include/net-snmp/library/snmp_impl.h
./table_array.lo: ../../include/net-snmp/library/snmp.h
./table_array.lo: ../../include/net-snmp/library/snmp-tc.h
./table_array.lo: ../../include/net-snmp/utilities.h
./table_array.lo: ../../include/net-snmp/library/snmp_client.h
./table_array.lo: ../../include/net-snmp/library/getopt.h
./table_array.lo: ../../include/net-snmp/library/system.h
./table_array.lo: ../../include/net-snmp/library/tools.h
./table_array.lo: ../../include/net-snmp/library/int64.h
./table_array.lo: ../../include/net-snmp/library/mt_support.h
./table_array.lo: ../../include/net-snmp/library/snmp_alarm.h
./table_array.lo: ../../include/net-snmp/library/data_list.h
./table_array.lo: ../../include/net-snmp/library/callback.h
./table_array.lo: ../../include/net-snmp/library/oid_stash.h
./table_array.lo: ../../include/net-snmp/library/check_varbind.h
./table_array.lo: ../../include/net-snmp/version.h
./table_array.lo: ../../include/net-snmp/session_api.h
./table_array.lo: ../../include/net-snmp/library/snmp_transport.h
./table_array.lo: ../../include/net-snmp/library/snmpCallbackDomain.h
./table_array.lo: ../../include/net-snmp/library/snmpUnixDomain.h
./table_array.lo: ../../include/net-snmp/library/snmpUDPDomain.h
./table_array.lo: ../../include/net-snmp/library/snmpTCPDomain.h
./table_array.lo: ../../include/net-snmp/library/snmpUDPIPv6Domain.h
./table_array.lo: ../../include/net-snmp/library/snmpTCPIPv6Domain.h
./table_array.lo: ../../include/net-snmp/library/snmpIPXDomain.h
./table_array.lo: ../../include/net-snmp/library/ucd_compat.h
./table_array.lo: ../../include/net-snmp/pdu_api.h
./table_array.lo: ../../include/net-snmp/mib_api.h
./table_array.lo: ../../include/net-snmp/library/mib.h
./table_array.lo: ../../include/net-snmp/library/parse.h
./table_array.lo: ../../include/net-snmp/varbind_api.h
./table_array.lo: ../../include/net-snmp/config_api.h
./table_array.lo: ../../include/net-snmp/library/read_config.h
./table_array.lo: ../../include/net-snmp/library/default_store.h
./table_array.lo: ../../include/net-snmp/library/snmp_parse_args.h
./table_array.lo: ../../include/net-snmp/library/snmp_enum.h
./table_array.lo: ../../include/net-snmp/library/vacm.h
./table_array.lo: ../../include/net-snmp/output_api.h
./table_array.lo: ../../include/net-snmp/library/snmp_debug.h
./table_array.lo: ../../include/net-snmp/library/snmp_logging.h
./table_array.lo: ../../include/net-snmp/snmpv3_api.h
./table_array.lo: ../../include/net-snmp/library/snmpv3.h
./table_array.lo: ../../include/net-snmp/library/transform_oids.h
./table_array.lo: ../../include/net-snmp/library/keytools.h
./table_array.lo: ../../include/net-snmp/library/scapi.h
./table_array.lo: ../../include/net-snmp/library/lcd_time.h
./table_array.lo: ../../include/net-snmp/library/snmp_secmod.h
./table_array.lo: ../../include/net-snmp/library/snmpv3-security-includes.h
./table_array.lo: ../../include/net-snmp/library/snmpusm.h
./table_array.lo: ../../include/net-snmp/agent/net-snmp-agent-includes.h
./table_array.lo: ../../include/net-snmp/agent/snmp_agent.h
./table_array.lo: ../../include/net-snmp/agent/snmp_vars.h
./table_array.lo: ../../include/net-snmp/agent/agent_handler.h
./table_array.lo: ../../include/net-snmp/agent/var_struct.h
./table_array.lo: ../../include/net-snmp/agent/agent_registry.h
./table_array.lo: ../../include/net-snmp/agent/ds_agent.h
./table_array.lo: ../../include/net-snmp/agent/agent_read_config.h
./table_array.lo: ../../include/net-snmp/agent/agent_trap.h
./table_array.lo: ../../include/net-snmp/agent/all_helpers.h
./table_array.lo: ../../include/net-snmp/agent/instance.h
./table_array.lo: ../../include/net-snmp/agent/multiplexer.h
./table_array.lo: ../../include/net-snmp/agent/null.h
./table_array.lo: ../../include/net-snmp/agent/debug_handler.h
./table_array.lo: ../../include/net-snmp/agent/old_api.h
./table_array.lo: ../../include/net-snmp/agent/read_only.h
./table_array.lo: ../../include/net-snmp/agent/serialize.h
./table_array.lo: ../../include/net-snmp/agent/bulk_to_next.h
./table_array.lo: ../../include/net-snmp/agent/mode_end_call.h
./table_array.lo: ../../include/net-snmp/agent/table.h
./table_array.lo: ../../include/net-snmp/agent/table_data.h
./table_array.lo: ../../include/net-snmp/agent/table_dataset.h
./table_array.lo: ../../include/net-snmp/agent/table_iterator.h
./table_array.lo: ../../include/net-snmp/agent/mib_module_config.h
./table_array.lo: ../../include/net-snmp/agent/table_array.h
./table_array.lo: ../../include/net-snmp/library/container.h
./table_array.lo: ../../include/net-snmp/library/factory.h
./table_array.lo: ../../include/net-snmp/library/snmp_assert.h
./table.lo: ../../include/net-snmp/net-snmp-config.h
./table.lo: ../../include/net-snmp/system/linux.h
./table.lo: ../../include/net-snmp/system/sysv.h
./table.lo: ../../include/net-snmp/system/generic.h
./table.lo: ../../include/net-snmp/machine/generic.h 
./table.lo:  ../../include/net-snmp/net-snmp-includes.h
./table.lo:  ../../include/net-snmp/definitions.h
./table.lo: ../../include/net-snmp/types.h
./table.lo: ../../include/net-snmp/library/snmp_api.h
./table.lo: ../../include/net-snmp/library/asn1.h
./table.lo: ../../include/net-snmp/library/snmp_impl.h
./table.lo: ../../include/net-snmp/library/snmp.h
./table.lo: ../../include/net-snmp/library/snmp-tc.h
./table.lo: ../../include/net-snmp/utilities.h
./table.lo: ../../include/net-snmp/library/snmp_client.h
./table.lo: ../../include/net-snmp/library/getopt.h
./table.lo: ../../include/net-snmp/library/system.h
./table.lo: ../../include/net-snmp/library/tools.h
./table.lo: ../../include/net-snmp/library/int64.h
./table.lo: ../../include/net-snmp/library/mt_support.h
./table.lo: ../../include/net-snmp/library/snmp_alarm.h
./table.lo: ../../include/net-snmp/library/data_list.h
./table.lo: ../../include/net-snmp/library/callback.h
./table.lo: ../../include/net-snmp/library/oid_stash.h
./table.lo: ../../include/net-snmp/library/check_varbind.h
./table.lo: ../../include/net-snmp/version.h
./table.lo: ../../include/net-snmp/session_api.h
./table.lo: ../../include/net-snmp/library/snmp_transport.h
./table.lo: ../../include/net-snmp/library/snmpCallbackDomain.h
./table.lo: ../../include/net-snmp/library/snmpUnixDomain.h
./table.lo: ../../include/net-snmp/library/snmpUDPDomain.h
./table.lo: ../../include/net-snmp/library/snmpTCPDomain.h
./table.lo: ../../include/net-snmp/library/snmpUDPIPv6Domain.h
./table.lo: ../../include/net-snmp/library/snmpTCPIPv6Domain.h
./table.lo: ../../include/net-snmp/library/snmpIPXDomain.h
./table.lo: ../../include/net-snmp/library/ucd_compat.h
./table.lo: ../../include/net-snmp/pdu_api.h ../../include/net-snmp/mib_api.h
./table.lo: ../../include/net-snmp/library/mib.h
./table.lo: ../../include/net-snmp/library/parse.h
./table.lo: ../../include/net-snmp/varbind_api.h
./table.lo: ../../include/net-snmp/config_api.h
./table.lo: ../../include/net-snmp/library/read_config.h
./table.lo: ../../include/net-snmp/library/default_store.h
./table.lo: ../../include/net-snmp/library/snmp_parse_args.h
./table.lo: ../../include/net-snmp/library/snmp_enum.h
./table.lo: ../../include/net-snmp/library/vacm.h
./table.lo: ../../include/net-snmp/output_api.h
./table.lo: ../../include/net-snmp/library/snmp_debug.h
./table.lo: ../../include/net-snmp/library/snmp_logging.h
./table.lo: ../../include/net-snmp/snmpv3_api.h
./table.lo: ../../include/net-snmp/library/snmpv3.h
./table.lo: ../../include/net-snmp/library/transform_oids.h
./table.lo: ../../include/net-snmp/library/keytools.h
./table.lo: ../../include/net-snmp/library/scapi.h
./table.lo: ../../include/net-snmp/library/lcd_time.h
./table.lo: ../../include/net-snmp/library/snmp_secmod.h
./table.lo: ../../include/net-snmp/library/snmpv3-security-includes.h
./table.lo: ../../include/net-snmp/library/snmpusm.h
./table.lo: ../../include/net-snmp/agent/net-snmp-agent-includes.h
./table.lo: ../../include/net-snmp/agent/snmp_agent.h
./table.lo: ../../include/net-snmp/agent/snmp_vars.h
./table.lo: ../../include/net-snmp/agent/agent_handler.h
./table.lo: ../../include/net-snmp/agent/var_struct.h
./table.lo: ../../include/net-snmp/agent/agent_registry.h
./table.lo: ../../include/net-snmp/agent/ds_agent.h
./table.lo: ../../include/net-snmp/agent/agent_read_config.h
./table.lo: ../../include/net-snmp/agent/agent_trap.h
./table.lo: ../../include/net-snmp/agent/all_helpers.h
./table.lo: ../../include/net-snmp/agent/instance.h
./table.lo: ../../include/net-snmp/agent/multiplexer.h
./table.lo: ../../include/net-snmp/agent/null.h
./table.lo: ../../include/net-snmp/agent/debug_handler.h
./table.lo: ../../include/net-snmp/agent/old_api.h
./table.lo: ../../include/net-snmp/agent/read_only.h
./table.lo: ../../include/net-snmp/agent/serialize.h
./table.lo: ../../include/net-snmp/agent/bulk_to_next.h
./table.lo: ../../include/net-snmp/agent/mode_end_call.h
./table.lo: ../../include/net-snmp/agent/table.h
./table.lo: ../../include/net-snmp/agent/table_data.h
./table.lo: ../../include/net-snmp/agent/table_dataset.h
./table.lo: ../../include/net-snmp/agent/table_iterator.h
./table.lo: ../../include/net-snmp/agent/mib_module_config.h
./table.lo: ../../include/net-snmp/library/snmp_assert.h
./table_data.lo: ../../include/net-snmp/net-snmp-config.h
./table_data.lo: ../../include/net-snmp/system/linux.h
./table_data.lo: ../../include/net-snmp/system/sysv.h
./table_data.lo: ../../include/net-snmp/system/generic.h
./table_data.lo: ../../include/net-snmp/machine/generic.h
./table_data.lo: ../../include/net-snmp/net-snmp-includes.h
./table_data.lo:  ../../include/net-snmp/definitions.h
./table_data.lo: ../../include/net-snmp/types.h
./table_data.lo: ../../include/net-snmp/library/snmp_api.h
./table_data.lo: ../../include/net-snmp/library/asn1.h
./table_data.lo: ../../include/net-snmp/library/snmp_impl.h
./table_data.lo: ../../include/net-snmp/library/snmp.h
./table_data.lo: ../../include/net-snmp/library/snmp-tc.h
./table_data.lo: ../../include/net-snmp/utilities.h
./table_data.lo: ../../include/net-snmp/library/snmp_client.h
./table_data.lo: ../../include/net-snmp/library/getopt.h
./table_data.lo: ../../include/net-snmp/library/system.h
./table_data.lo: ../../include/net-snmp/library/tools.h
./table_data.lo: ../../include/net-snmp/library/int64.h
./table_data.lo: ../../include/net-snmp/library/mt_support.h
./table_data.lo: ../../include/net-snmp/library/snmp_alarm.h
./table_data.lo: ../../include/net-snmp/library/data_list.h
./table_data.lo: ../../include/net-snmp/library/callback.h
./table_data.lo: ../../include/net-snmp/library/oid_stash.h
./table_data.lo: ../../include/net-snmp/library/check_varbind.h
./table_data.lo: ../../include/net-snmp/version.h
./table_data.lo: ../../include/net-snmp/session_api.h
./table_data.lo: ../../include/net-snmp/library/snmp_transport.h
./table_data.lo: ../../include/net-snmp/library/snmpCallbackDomain.h
./table_data.lo: ../../include/net-snmp/library/snmpUnixDomain.h
./table_data.lo: ../../include/net-snmp/library/snmpUDPDomain.h
./table_data.lo: ../../include/net-snmp/library/snmpTCPDomain.h
./table_data.lo: ../../include/net-snmp/library/snmpUDPIPv6Domain.h
./table_data.lo: ../../include/net-snmp/library/snmpTCPIPv6Domain.h
./table_data.lo: ../../include/net-snmp/library/snmpIPXDomain.h
./table_data.lo: ../../include/net-snmp/library/ucd_compat.h
./table_data.lo: ../../include/net-snmp/pdu_api.h
./table_data.lo: ../../include/net-snmp/mib_api.h
./table_data.lo: ../../include/net-snmp/library/mib.h
./table_data.lo: ../../include/net-snmp/library/parse.h
./table_data.lo: ../../include/net-snmp/varbind_api.h
./table_data.lo: ../../include/net-snmp/config_api.h
./table_data.lo: ../../include/net-snmp/library/read_config.h
./table_data.lo: ../../include/net-snmp/library/default_store.h
./table_data.lo: ../../include/net-snmp/library/snmp_parse_args.h
./table_data.lo: ../../include/net-snmp/library/snmp_enum.h
./table_data.lo: ../../include/net-snmp/library/vacm.h
./table_data.lo: ../../include/net-snmp/output_api.h
./table_data.lo: ../../include/net-snmp/library/snmp_debug.h
./table_data.lo: ../../include/net-snmp/library/snmp_logging.h
./table_data.lo: ../../include/net-snmp/snmpv3_api.h
./table_data.lo: ../../include/net-snmp/library/snmpv3.h
./table_data.lo: ../../include/net-snmp/library/transform_oids.h
./table_data.lo: ../../include/net-snmp/library/keytools.h
./table_data.lo: ../../include/net-snmp/library/scapi.h
./table_data.lo: ../../include/net-snmp/library/lcd_time.h
./table_data.lo: ../../include/net-snmp/library/snmp_secmod.h
./table_data.lo: ../../include/net-snmp/library/snmpv3-security-includes.h
./table_data.lo: ../../include/net-snmp/library/snmpusm.h
./table_data.lo: ../../include/net-snmp/agent/net-snmp-agent-includes.h
./table_data.lo: ../../include/net-snmp/agent/snmp_agent.h
./table_data.lo: ../../include/net-snmp/agent/snmp_vars.h
./table_data.lo: ../../include/net-snmp/agent/agent_handler.h
./table_data.lo: ../../include/net-snmp/agent/var_struct.h
./table_data.lo: ../../include/net-snmp/agent/agent_registry.h
./table_data.lo: ../../include/net-snmp/agent/ds_agent.h
./table_data.lo: ../../include/net-snmp/agent/agent_read_config.h
./table_data.lo: ../../include/net-snmp/agent/agent_trap.h
./table_data.lo: ../../include/net-snmp/agent/all_helpers.h
./table_data.lo: ../../include/net-snmp/agent/instance.h
./table_data.lo: ../../include/net-snmp/agent/multiplexer.h
./table_data.lo: ../../include/net-snmp/agent/null.h
./table_data.lo: ../../include/net-snmp/agent/debug_handler.h
./table_data.lo: ../../include/net-snmp/agent/old_api.h
./table_data.lo: ../../include/net-snmp/agent/read_only.h
./table_data.lo: ../../include/net-snmp/agent/serialize.h
./table_data.lo: ../../include/net-snmp/agent/bulk_to_next.h
./table_data.lo: ../../include/net-snmp/agent/mode_end_call.h
./table_data.lo: ../../include/net-snmp/agent/table.h
./table_data.lo: ../../include/net-snmp/agent/table_data.h
./table_data.lo: ../../include/net-snmp/agent/table_dataset.h
./table_data.lo: ../../include/net-snmp/agent/table_iterator.h
./table_data.lo: ../../include/net-snmp/agent/mib_module_config.h
./table_dataset.lo: ../../include/net-snmp/net-snmp-config.h
./table_dataset.lo: ../../include/net-snmp/system/linux.h
./table_dataset.lo: ../../include/net-snmp/system/sysv.h
./table_dataset.lo: ../../include/net-snmp/system/generic.h
./table_dataset.lo: ../../include/net-snmp/machine/generic.h
./table_dataset.lo: ../../include/net-snmp/net-snmp-includes.h
./table_dataset.lo: ../../include/net-snmp/definitions.h
./table_dataset.lo: ../../include/net-snmp/types.h
./table_dataset.lo: ../../include/net-snmp/library/snmp_api.h
./table_dataset.lo: ../../include/net-snmp/library/asn1.h
./table_dataset.lo: ../../include/net-snmp/library/snmp_impl.h
./table_dataset.lo: ../../include/net-snmp/library/snmp.h
./table_dataset.lo: ../../include/net-snmp/library/snmp-tc.h
./table_dataset.lo: ../../include/net-snmp/utilities.h
./table_dataset.lo: ../../include/net-snmp/library/snmp_client.h
./table_dataset.lo: ../../include/net-snmp/library/getopt.h
./table_dataset.lo: ../../include/net-snmp/library/system.h
./table_dataset.lo: ../../include/net-snmp/library/tools.h
./table_dataset.lo: ../../include/net-snmp/library/int64.h
./table_dataset.lo: ../../include/net-snmp/library/mt_support.h
./table_dataset.lo: ../../include/net-snmp/library/snmp_alarm.h
./table_dataset.lo: ../../include/net-snmp/library/data_list.h
./table_dataset.lo: ../../include/net-snmp/library/callback.h
./table_dataset.lo: ../../include/net-snmp/library/oid_stash.h
./table_dataset.lo: ../../include/net-snmp/library/check_varbind.h
./table_dataset.lo: ../../include/net-snmp/version.h
./table_dataset.lo: ../../include/net-snmp/session_api.h
./table_dataset.lo: ../../include/net-snmp/library/snmp_transport.h
./table_dataset.lo: ../../include/net-snmp/library/snmpCallbackDomain.h
./table_dataset.lo: ../../include/net-snmp/library/snmpUnixDomain.h
./table_dataset.lo: ../../include/net-snmp/library/snmpUDPDomain.h
./table_dataset.lo: ../../include/net-snmp/library/snmpTCPDomain.h
./table_dataset.lo: ../../include/net-snmp/library/snmpUDPIPv6Domain.h
./table_dataset.lo: ../../include/net-snmp/library/snmpTCPIPv6Domain.h
./table_dataset.lo: ../../include/net-snmp/library/snmpIPXDomain.h
./table_dataset.lo: ../../include/net-snmp/library/ucd_compat.h
./table_dataset.lo: ../../include/net-snmp/pdu_api.h
./table_dataset.lo: ../../include/net-snmp/mib_api.h
./table_dataset.lo: ../../include/net-snmp/library/mib.h
./table_dataset.lo: ../../include/net-snmp/library/parse.h
./table_dataset.lo: ../../include/net-snmp/varbind_api.h
./table_dataset.lo: ../../include/net-snmp/config_api.h
./table_dataset.lo: ../../include/net-snmp/library/read_config.h
./table_dataset.lo: ../../include/net-snmp/library/default_store.h
./table_dataset.lo: ../../include/net-snmp/library/snmp_parse_args.h
./table_dataset.lo: ../../include/net-snmp/library/snmp_enum.h
./table_dataset.lo: ../../include/net-snmp/library/vacm.h
./table_dataset.lo: ../../include/net-snmp/output_api.h
./table_dataset.lo: ../../include/net-snmp/library/snmp_debug.h
./table_dataset.lo: ../../include/net-snmp/library/snmp_logging.h
./table_dataset.lo: ../../include/net-snmp/snmpv3_api.h
./table_dataset.lo: ../../include/net-snmp/library/snmpv3.h
./table_dataset.lo: ../../include/net-snmp/library/transform_oids.h
./table_dataset.lo: ../../include/net-snmp/library/keytools.h
./table_dataset.lo: ../../include/net-snmp/library/scapi.h
./table_dataset.lo: ../../include/net-snmp/library/lcd_time.h
./table_dataset.lo: ../../include/net-snmp/library/snmp_secmod.h
./table_dataset.lo: ../../include/net-snmp/library/snmpv3-security-includes.h
./table_dataset.lo: ../../include/net-snmp/library/snmpusm.h
./table_dataset.lo: ../../include/net-snmp/agent/net-snmp-agent-includes.h
./table_dataset.lo: ../../include/net-snmp/agent/snmp_agent.h
./table_dataset.lo: ../../include/net-snmp/agent/snmp_vars.h
./table_dataset.lo: ../../include/net-snmp/agent/agent_handler.h
./table_dataset.lo: ../../include/net-snmp/agent/var_struct.h
./table_dataset.lo: ../../include/net-snmp/agent/agent_registry.h
./table_dataset.lo: ../../include/net-snmp/agent/ds_agent.h
./table_dataset.lo: ../../include/net-snmp/agent/agent_read_config.h
./table_dataset.lo: ../../include/net-snmp/agent/agent_trap.h
./table_dataset.lo: ../../include/net-snmp/agent/all_helpers.h
./table_dataset.lo: ../../include/net-snmp/agent/instance.h
./table_dataset.lo: ../../include/net-snmp/agent/multiplexer.h
./table_dataset.lo: ../../include/net-snmp/agent/null.h
./table_dataset.lo: ../../include/net-snmp/agent/debug_handler.h
./table_dataset.lo: ../../include/net-snmp/agent/old_api.h
./table_dataset.lo: ../../include/net-snmp/agent/read_only.h
./table_dataset.lo: ../../include/net-snmp/agent/serialize.h
./table_dataset.lo: ../../include/net-snmp/agent/bulk_to_next.h
./table_dataset.lo: ../../include/net-snmp/agent/mode_end_call.h
./table_dataset.lo: ../../include/net-snmp/agent/table.h
./table_dataset.lo: ../../include/net-snmp/agent/table_data.h
./table_dataset.lo: ../../include/net-snmp/agent/table_dataset.h
./table_dataset.lo: ../../include/net-snmp/agent/table_iterator.h
./table_dataset.lo: ../../include/net-snmp/agent/mib_module_config.h
./table_iterator.lo: ../../include/net-snmp/net-snmp-config.h
./table_iterator.lo: ../../include/net-snmp/system/linux.h
./table_iterator.lo: ../../include/net-snmp/system/sysv.h
./table_iterator.lo: ../../include/net-snmp/system/generic.h
./table_iterator.lo: ../../include/net-snmp/machine/generic.h
./table_iterator.lo: ../../include/net-snmp/net-snmp-includes.h
./table_iterator.lo: ../../include/net-snmp/definitions.h
./table_iterator.lo: ../../include/net-snmp/types.h
./table_iterator.lo: ../../include/net-snmp/library/snmp_api.h
./table_iterator.lo: ../../include/net-snmp/library/asn1.h
./table_iterator.lo: ../../include/net-snmp/library/snmp_impl.h
./table_iterator.lo: ../../include/net-snmp/library/snmp.h
./table_iterator.lo: ../../include/net-snmp/library/snmp-tc.h
./table_iterator.lo: ../../include/net-snmp/utilities.h
./table_iterator.lo: ../../include/net-snmp/library/snmp_client.h
./table_iterator.lo: ../../include/net-snmp/library/getopt.h
./table_iterator.lo: ../../include/net-snmp/library/system.h
./table_iterator.lo: ../../include/net-snmp/library/tools.h
./table_iterator.lo: ../../include/net-snmp/library/int64.h
./table_iterator.lo: ../../include/net-snmp/library/mt_support.h
./table_iterator.lo: ../../include/net-snmp/library/snmp_alarm.h
./table_iterator.lo: ../../include/net-snmp/library/data_list.h
./table_iterator.lo: ../../include/net-snmp/library/callback.h
./table_iterator.lo: ../../include/net-snmp/library/oid_stash.h
./table_iterator.lo: ../../include/net-snmp/library/check_varbind.h
./table_iterator.lo: ../../include/net-snmp/version.h
./table_iterator.lo: ../../include/net-snmp/session_api.h
./table_iterator.lo: ../../include/net-snmp/library/snmp_transport.h
./table_iterator.lo: ../../include/net-snmp/library/snmpCallbackDomain.h
./table_iterator.lo: ../../include/net-snmp/library/snmpUnixDomain.h
./table_iterator.lo: ../../include/net-snmp/library/snmpUDPDomain.h
./table_iterator.lo: ../../include/net-snmp/library/snmpTCPDomain.h
./table_iterator.lo: ../../include/net-snmp/library/snmpUDPIPv6Domain.h
./table_iterator.lo: ../../include/net-snmp/library/snmpTCPIPv6Domain.h
./table_iterator.lo: ../../include/net-snmp/library/snmpIPXDomain.h
./table_iterator.lo: ../../include/net-snmp/library/ucd_compat.h
./table_iterator.lo: ../../include/net-snmp/pdu_api.h
./table_iterator.lo: ../../include/net-snmp/mib_api.h
./table_iterator.lo: ../../include/net-snmp/library/mib.h
./table_iterator.lo: ../../include/net-snmp/library/parse.h
./table_iterator.lo: ../../include/net-snmp/varbind_api.h
./table_iterator.lo: ../../include/net-snmp/config_api.h
./table_iterator.lo: ../../include/net-snmp/library/read_config.h
./table_iterator.lo: ../../include/net-snmp/library/default_store.h
./table_iterator.lo: ../../include/net-snmp/library/snmp_parse_args.h
./table_iterator.lo: ../../include/net-snmp/library/snmp_enum.h
./table_iterator.lo: ../../include/net-snmp/library/vacm.h
./table_iterator.lo: ../../include/net-snmp/output_api.h
./table_iterator.lo: ../../include/net-snmp/library/snmp_debug.h
./table_iterator.lo: ../../include/net-snmp/library/snmp_logging.h
./table_iterator.lo: ../../include/net-snmp/snmpv3_api.h
./table_iterator.lo: ../../include/net-snmp/library/snmpv3.h
./table_iterator.lo: ../../include/net-snmp/library/transform_oids.h
./table_iterator.lo: ../../include/net-snmp/library/keytools.h
./table_iterator.lo: ../../include/net-snmp/library/scapi.h
./table_iterator.lo: ../../include/net-snmp/library/lcd_time.h
./table_iterator.lo: ../../include/net-snmp/library/snmp_secmod.h
./table_iterator.lo: ../../include/net-snmp/library/snmpv3-security-includes.h
./table_iterator.lo: ../../include/net-snmp/library/snmpusm.h
./table_iterator.lo: ../../include/net-snmp/agent/net-snmp-agent-includes.h
./table_iterator.lo: ../../include/net-snmp/agent/snmp_agent.h
./table_iterator.lo: ../../include/net-snmp/agent/snmp_vars.h
./table_iterator.lo: ../../include/net-snmp/agent/agent_handler.h
./table_iterator.lo: ../../include/net-snmp/agent/var_struct.h
./table_iterator.lo: ../../include/net-snmp/agent/agent_registry.h
./table_iterator.lo: ../../include/net-snmp/agent/ds_agent.h
./table_iterator.lo: ../../include/net-snmp/agent/agent_read_config.h
./table_iterator.lo: ../../include/net-snmp/agent/agent_trap.h
./table_iterator.lo: ../../include/net-snmp/agent/all_helpers.h
./table_iterator.lo: ../../include/net-snmp/agent/instance.h
./table_iterator.lo: ../../include/net-snmp/agent/multiplexer.h
./table_iterator.lo: ../../include/net-snmp/agent/null.h
./table_iterator.lo: ../../include/net-snmp/agent/debug_handler.h
./table_iterator.lo: ../../include/net-snmp/agent/old_api.h
./table_iterator.lo: ../../include/net-snmp/agent/read_only.h
./table_iterator.lo: ../../include/net-snmp/agent/serialize.h
./table_iterator.lo: ../../include/net-snmp/agent/bulk_to_next.h
./table_iterator.lo: ../../include/net-snmp/agent/mode_end_call.h
./table_iterator.lo: ../../include/net-snmp/agent/table.h
./table_iterator.lo: ../../include/net-snmp/agent/table_data.h
./table_iterator.lo: ../../include/net-snmp/agent/table_dataset.h
./table_iterator.lo: ../../include/net-snmp/agent/table_iterator.h
./table_iterator.lo: ../../include/net-snmp/agent/mib_module_config.h
./watcher.lo: ../../include/net-snmp/net-snmp-config.h
./watcher.lo: ../../include/net-snmp/system/linux.h
./watcher.lo: ../../include/net-snmp/system/sysv.h
./watcher.lo: ../../include/net-snmp/system/generic.h
./watcher.lo: ../../include/net-snmp/machine/generic.h 
./watcher.lo: ../../include/net-snmp/net-snmp-includes.h 
./watcher.lo:  ../../include/net-snmp/definitions.h
./watcher.lo: ../../include/net-snmp/types.h
./watcher.lo: ../../include/net-snmp/library/snmp_api.h
./watcher.lo: ../../include/net-snmp/library/asn1.h
./watcher.lo: ../../include/net-snmp/library/snmp_impl.h
./watcher.lo: ../../include/net-snmp/library/snmp.h
./watcher.lo: ../../include/net-snmp/library/snmp-tc.h
./watcher.lo: ../../include/net-snmp/utilities.h
./watcher.lo: ../../include/net-snmp/library/snmp_client.h
./watcher.lo: ../../include/net-snmp/library/getopt.h
./watcher.lo: ../../include/net-snmp/library/system.h
./watcher.lo: ../../include/net-snmp/library/tools.h
./watcher.lo: ../../include/net-snmp/library/int64.h
./watcher.lo: ../../include/net-snmp/library/mt_support.h
./watcher.lo: ../../include/net-snmp/library/snmp_alarm.h
./watcher.lo: ../../include/net-snmp/library/data_list.h
./watcher.lo: ../../include/net-snmp/library/callback.h
./watcher.lo: ../../include/net-snmp/library/oid_stash.h
./watcher.lo: ../../include/net-snmp/library/check_varbind.h
./watcher.lo: ../../include/net-snmp/version.h
./watcher.lo: ../../include/net-snmp/session_api.h
./watcher.lo: ../../include/net-snmp/library/snmp_transport.h
./watcher.lo: ../../include/net-snmp/library/snmpCallbackDomain.h
./watcher.lo: ../../include/net-snmp/library/snmpUnixDomain.h
./watcher.lo: ../../include/net-snmp/library/snmpUDPDomain.h
./watcher.lo: ../../include/net-snmp/library/snmpTCPDomain.h
./watcher.lo: ../../include/net-snmp/library/snmpUDPIPv6Domain.h
./watcher.lo: ../../include/net-snmp/library/snmpTCPIPv6Domain.h
./watcher.lo: ../../include/net-snmp/library/snmpIPXDomain.h
./watcher.lo: ../../include/net-snmp/library/ucd_compat.h
./watcher.lo: ../../include/net-snmp/pdu_api.h
./watcher.lo: ../../include/net-snmp/mib_api.h
./watcher.lo: ../../include/net-snmp/library/mib.h
./watcher.lo: ../../include/net-snmp/library/parse.h
./watcher.lo: ../../include/net-snmp/varbind_api.h
./watcher.lo: ../../include/net-snmp/config_api.h
./watcher.lo: ../../include/net-snmp/library/read_config.h
./watcher.lo: ../../include/net-snmp/library/default_store.h
./watcher.lo: ../../include/net-snmp/library/snmp_parse_args.h
./watcher.lo: ../../include/net-snmp/library/snmp_enum.h
./watcher.lo: ../../include/net-snmp/library/vacm.h
./watcher.lo: ../../include/net-snmp/output_api.h
./watcher.lo: ../../include/net-snmp/library/snmp_debug.h
./watcher.lo: ../../include/net-snmp/library/snmp_logging.h
./watcher.lo: ../../include/net-snmp/snmpv3_api.h
./watcher.lo: ../../include/net-snmp/library/snmpv3.h
./watcher.lo: ../../include/net-snmp/library/transform_oids.h
./watcher.lo: ../../include/net-snmp/library/keytools.h
./watcher.lo: ../../include/net-snmp/library/scapi.h
./watcher.lo: ../../include/net-snmp/library/lcd_time.h
./watcher.lo: ../../include/net-snmp/library/snmp_secmod.h
./watcher.lo: ../../include/net-snmp/library/snmpv3-security-includes.h
./watcher.lo: ../../include/net-snmp/library/snmpusm.h
./watcher.lo: ../../include/net-snmp/agent/net-snmp-agent-includes.h
./watcher.lo: ../../include/net-snmp/agent/snmp_agent.h
./watcher.lo: ../../include/net-snmp/agent/snmp_vars.h
./watcher.lo: ../../include/net-snmp/agent/agent_handler.h
./watcher.lo: ../../include/net-snmp/agent/var_struct.h
./watcher.lo: ../../include/net-snmp/agent/agent_registry.h
./watcher.lo: ../../include/net-snmp/agent/ds_agent.h
./watcher.lo: ../../include/net-snmp/agent/agent_read_config.h
./watcher.lo: ../../include/net-snmp/agent/agent_trap.h
./watcher.lo: ../../include/net-snmp/agent/all_helpers.h
./watcher.lo: ../../include/net-snmp/agent/instance.h
./watcher.lo: ../../include/net-snmp/agent/multiplexer.h
./watcher.lo: ../../include/net-snmp/agent/null.h
./watcher.lo: ../../include/net-snmp/agent/debug_handler.h
./watcher.lo: ../../include/net-snmp/agent/old_api.h
./watcher.lo: ../../include/net-snmp/agent/read_only.h
./watcher.lo: ../../include/net-snmp/agent/serialize.h
./watcher.lo: ../../include/net-snmp/agent/bulk_to_next.h
./watcher.lo: ../../include/net-snmp/agent/mode_end_call.h
./watcher.lo: ../../include/net-snmp/agent/table.h
./watcher.lo: ../../include/net-snmp/agent/table_data.h
./watcher.lo: ../../include/net-snmp/agent/table_dataset.h
./watcher.lo: ../../include/net-snmp/agent/table_iterator.h
./watcher.lo: ../../include/net-snmp/agent/mib_module_config.h
./watcher.lo: ../../include/net-snmp/agent/watcher.h
./watcher.lo: ../../include/net-snmp/agent/scalar.h
