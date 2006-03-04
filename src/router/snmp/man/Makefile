# Generated automatically from Makefile.in:Makefile.rules by configure.
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
LDFLAGS		=  
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
#
# Makefile for the man page directory
#
top_builddir=..

#
# install stuff
#

OTHERINSTALL=maninstall
#
# local stuff
#

MAN1  = snmpnetstat.1 snmpdelta.1 snmpinform.1 snmpdf.1
MAN1G = snmpbulkget.1 snmpcmd.1 snmpget.1 snmpset.1 snmpwalk.1 \
	snmpbulkwalk.1 snmpgetnext.1 snmptest.1 snmptranslate.1 snmptrap.1 \
	snmpusm.1 snmptable.1 snmpstatus.1 snmpconf.1 mib2c.1
MAN3  = snmp_close.3 snmp_free_pdu.3 snmp_open.3 snmp_read.3 \
	snmp_select_info.3 snmp_send.3 snmp_timeout.3 \
	snmp_sess_async_send.3 snmp_sess_close.3 snmp_sess_error.3 \
	snmp_sess_init.3 snmp_sess_open.3 snmp_sess_read.3 \
	snmp_sess_select_info.3 snmp_sess_send.3 snmp_sess_timeout.3 \
	snmp_sess_session.3 snmp_perror.3 snmp_sess_perror.3 snmp_error.3 \
	snmp_api_errstring.3 init_mib.3 add_mibdir.3 init_mib_internals.3 \
	snmp_set_mib_warnings.3 read_module.3 read_all_mibs.3 read_mib.3 \
	read_module_node.3 read_objid.3 add_module_replacement.3 \
	shutdown_mib.3 print_mib.3 print_variable.3 print_value.3 \
	print_objid.3 print_description.3 snmp_set_save_descriptions.3 \
	get_module_node.3 \
	netsnmp_agent.3 netsnmp_bulk_to_next.3 netsnmp_debug.3 \
	netsnmp_example_scalar_int.3 netsnmp_handler.3 netsnmp_instance.3 \
	netsnmp_mib_handler_methods.3 netsnmp_multiplexer.3 netsnmp_old_api.3 \
	netsnmp_read_only.3 netsnmp_serialize.3 netsnmp_table.3 \
	netsnmp_table_array.3 netsnmp_table_data.3 netsnmp_table_dataset.3 \
	netsnmp_table_iterator.3 netsnmp_todo.3 netsnmp_scalar.3 \
	netsnmp_watcher.3 netsnmp_mode_end_call.3 netsnmp_library.3 \
	netsnmp_mib_utilities.3
MAN3G = default_store.3 mib_api.3 read_config.3 snmp_agent_api.3 snmp_alarm.3 \
	snmp_api.3 snmp_sess_api.3 snmp_trap_api.3
MAN5G = snmpd.conf.5 snmptrapd.conf.5 snmp.conf.5 snmp_config.5 variables.5
MAN8G = snmptrapd.8 snmpd.8

MANALL = $(MAN1) $(MAN1G) $(MAN3) $(MAN3G) $(MAN5G) $(MAN8G)

TARGETS = $(MAN5G) $(MAN1G) $(MAN3G) $(MAN8G)
OTHERCLEANTARGETS=$(TARGETS) default_store.3.h

all: $(TARGETS) standardall

snmpbulkget.1: snmpbulkget.1.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmpbulkget.1.def > snmpbulkget.1

snmpbulkwalk.1: snmpbulkwalk.1.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmpbulkwalk.1.def > snmpbulkwalk.1

snmpcmd.1: snmpcmd.1.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmpcmd.1.def > snmpcmd.1

snmpconf.1: snmpconf.1.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmpconf.1.def > snmpconf.1

snmpd.8: snmpd.8.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmpd.8.def > snmpd.8

snmpget.1: snmpget.1.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmpget.1.def > snmpget.1

snmpgetnext.1: snmpgetnext.1.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmpgetnext.1.def > snmpgetnext.1

snmpset.1: snmpset.1.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmpset.1.def > snmpset.1

snmpstatus.1: snmpstatus.1.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmpstatus.1.def > snmpstatus.1

snmptable.1: snmptable.1.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmptable.1.def > snmptable.1

snmptest.1: snmptest.1.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmptest.1.def > snmptest.1

snmptranslate.1: snmptranslate.1.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmptranslate.1.def > snmptranslate.1

snmptrap.1: snmptrap.1.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmptrap.1.def > snmptrap.1

snmpusm.1: snmpusm.1.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmpusm.1.def > snmpusm.1

mib2c.1: mib2c.1.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/mib2c.1.def > mib2c.1

snmpwalk.1: snmpwalk.1.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmpwalk.1.def > snmpwalk.1

mib_api.3: mib_api.3.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/mib_api.3.def > mib_api.3

read_config.3: read_config.3.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/read_config.3.def > read_config.3

snmp_agent_api.3: snmp_agent_api.3.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmp_agent_api.3.def > snmp_agent_api.3

snmp_alarm.3: snmp_alarm.3.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmp_alarm.3.def > snmp_alarm.3

snmp_api.3: snmp_api.3.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmp_api.3.def > snmp_api.3

snmp_sess_api.3: snmp_sess_api.3.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmp_sess_api.3.def > snmp_sess_api.3

snmp_trap_api.3: snmp_trap_api.3.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmp_trap_api.3.def > snmp_trap_api.3

snmp.conf.5: snmp.conf.5.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmp.conf.5.def > snmp.conf.5

snmp_config.5: snmp_config.5.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmp_config.5.def > snmp_config.5

snmpd.conf.5: snmpd.conf.5.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmpd.conf.5.def > snmpd.conf.5

snmptrapd.conf.5: snmptrapd.conf.5.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmptrapd.conf.5.def > snmptrapd.conf.5

variables.5: variables.5.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/variables.5.def > variables.5

snmptrapd.8: snmptrapd.8.def ../sedscript
	$(SED) -f ../sedscript < $(srcdir)/snmptrapd.8.def > snmptrapd.8

default_store.3.h: $(srcdir)/../include/net-snmp/library/default_store.h
	awk '{ if ($$0 == "     * begin storage definitions ") { p = 1 } else if ($$0 == "     * end storage definitions ") { p = 0 } else if (p) { print $$0}}' < $(srcdir)/../include/net-snmp/library/default_store.h > default_store.3.h

default_store.3: $(srcdir)/default_store.3.top default_store.3.h $(srcdir)/default_store.3.bot
	$(SED) -f ../sedscript < $(srcdir)/default_store.3.top > default_store.3
	cat default_store.3.h $(srcdir)/default_store.3.bot >> default_store.3

maninstall:  maninstalldirs $(MAN1) $(MAN1G) $(MAN3) $(MAN5G) $(MAN8)
	@for i in $(MAN1) ; do $(INSTALL) $(srcdir)/$$i $(man1dir) ; echo "install:  installed $$i in $(man1dir)" ; done
	@$(INSTALL_DATA) $(MAN1G) $(man1dir)
	@for i in $(MAN1G) ; do echo "install:  installed $$i in $(man1dir)" ; done
	@for i in $(MAN3) ; do $(INSTALL) $(srcdir)/$$i $(man3dir) ; echo "install:  installed $$i in $(man3dir)" ; done
	@$(INSTALL_DATA) $(MAN3G) $(man3dir)
	@for i in $(MAN3G) ; do echo "install:  installed $$i in $(man3dir)" ; done
	@$(INSTALL_DATA) $(MAN5G) $(man5dir)
	@for i in $(MAN5G) ; do echo "install:  installed $$i in $(man5dir)" ; done
	@$(INSTALL_DATA) $(MAN8G) $(man8dir)
	@for i in $(MAN8G) ; do echo "install:  installed $$i in $(man8dir)" ; done

maninstalldirs:
	@$(SHELL) $(srcdir)/../mkinstalldirs $(man1dir) $(man3dir) $(man5dir) $(man8dir)


#
# internal administrative
#
html: $(MANALL)
	for i in $(MANALL); do base=`echo $$i | sed 's/.[0-9]$$//;'` ; echo "<HTML><BODY bgcolor=\"#ffffff\" background=\"../ucd-snmp-bg3.gif\"><PRE>" > $$base.html; nroff -man $$i | man2html -bare -title "$$base manual page" >> $$base.html; done

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
