# Generated automatically from Makefile.top:Makefile.in:Makefile.rules by configure.
#
# Minimum environment and virtual path setup
#
SHELL		= /bin/sh
srcdir		= .
top_srcdir	= .
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
# Makefile.in (at the root of net-snmp)
#

top_builddir	= .

SUBDIRS		= snmplib agent apps local man mibs
TESTDIRS	= testing

CPP		= mipsel-linux-uclibc-gcc -E 					        \
		-Iinclude -I$(srcdir)/include -I$(srcdir)/agent/mibgroup -I. -I$(srcdir)	\
		-DDONT_INC_STRUCTS -DBINDIR=$(bindir) 		        \
		$(EXTRACPPFLAGS)

INSTALLHEADERS=version.h
INCLUDESUBDIR=system
INCLUDESUBDIRHEADERS= aix.h bsd.h bsdi3.h bsdi4.h bsdi.h cygwin.h darwin.h \
	dynix.h freebsd2.h freebsd3.h freebsd4.h freebsd.h generic.h hpux.h \
	irix.h linux.h mips.h netbsd.h openbsd.h solaris2.6.h solaris2.7.h \
	solaris2.8.h solaris2.9.h solaris.h sunos.h svr5.h sysv.h ultrix4.h
INCLUDESUBDIR2=machine
INCLUDESUBDIRHEADERS2=generic.h
INSTALLBUILTHEADERS=include/net-snmp/net-snmp-config.h
INSTALLBINSCRIPTS=net-snmp-config
INSTALLUCDHEADERS=ucd-snmp-config.h version.h mib_module_config.h

#
# other install rules.
#
OTHERINSTALL=copypersistentfiles 
COPY_PERSISTENT_FILES=no
PERSISTENT_DIRECTORY=/tmp/snmp-persist
UCDPERSISTENT_DIRECTORY=/var/ucd-snmp

#
# perl specific
#
# yes, order matters here.  default_store must occur before anything else
PERLMODULES=default_store SNMP ASN OID agent
PERLARGS=

#
# targets
#
all:    sedscript EXAMPLE.conf standardall  

start-flag:
	@touch build-in-progress-flag

end-flag:
	@rm -f build-in-progress-flag > /dev/null 2>&1 

#
# local build rules
#
sedscript: sedscript.in include/net-snmp/net-snmp-config.h $(srcdir)/agent/mibgroup/mibdefs.h
	$(CPP) $(srcdir)/sedscript.in | egrep '^s[/#]' | sed 's/REMOVEME//g;s# */#/#g;s/ *#/#/g;s#/ *#/#g;s/# g/#g/;' > sedscript
	echo 's#DATADIR#$(datadir)#g' >> sedscript
	echo 's#LIBDIR#$(libdir)#g' >> sedscript
	echo 's#BINDIR#$(bindir)#g' >> sedscript
	echo 's#PERSISTENT_DIRECTORY#$(PERSISTENT_DIRECTORY)#g' >> sedscript
	echo 's#SYSCONFDIR#$(sysconfdir)#g' >> sedscript

EXAMPLE.conf: sedscript EXAMPLE.conf.def
	$(SED) -f sedscript $(srcdir)/EXAMPLE.conf.def > EXAMPLE.conf

docs: docsdir mancp

docsdir: docsdox docsmunge

docsdox: doxygen.conf
	srcdir=$(srcdir) doxygen $(srcdir)/doxygen.conf

docsmunge:
	find docs/html -name \*.html -exec perl -p -i -e 's/Generated on/<!--#include virtual=\"\/sfbutton.html\" --><br>Generated on\n/; s/<body/<body bgcolor=white/' {} \;


mancp:
	@for i in docs/man/man3/* ; do \
	     cp $$i man/netsnmp_`basename $$i` ; \
	done

#
# extra install rules
#

copypersistentfiles:
	@if test "$(COPY_PERSISTENT_FILES)" = "yes" -a -d $(UCDPERSISTENT_DIRECTORY) -a ! -d $(PERSISTENT_DIRECTORY) ; then \
		cp -pr $(UCDPERSISTENT_DIRECTORY) $(PERSISTENT_DIRECTORY) ; \
		echo "copying $(UCDPERSISTENT_DIRECTORY) to $(PERSISTENT_DIRECTORY)" ; \
	fi
#
# test targets
#
test: all testdirs
	( cd testing; $(MAKE) test )

testdirs:
	for i in $(TESTDIRS) ; do	\
           ( cd $$i ; $(MAKE) ) ;		\
           if test $$? != 0 ; then \
              exit 1 ; \
           fi  \
	done

distall: ${srcdir}/configure ${srcdir}/include/net-snmp/net-snmp-config.h 

OTHERCLEANTARGETS=EXAMPLE.conf sedscript
OTHERCLEANTODOS=perlclean

#
# perl specific build rules
#
perlmodules: perlmakefiles
	@(cd perl ; $(MAKE)) ; \
        if test $$? != 0 ; then \
           exit 1 ; \
        fi

perlmakefiles:
	@if test ! -f perl/Makefile; then \
	  (dir=`pwd`; \
	   cd perl ; \
	   perl Makefile.PL -NET-SNMP-IN-SOURCE=true -NET-SNMP-CONFIG="sh $$dir/net-snmp-config") $(PERLARGS) ; \
        fi

perlinstall:
	@(cd perl ; $(MAKE) install) ; \
        if test $$? != 0 ; then \
           exit 1 ; \
        fi

perltest:
	@(cd perl ; $(MAKE) test) ; \
	if test $$? != 0 ; then \
	   exit 1 ; \
	fi

perlclean:
	@if test -f perl/Makefile; then \
	   ( cd perl ; $(MAKE) clean ) ; \
	fi

#
# make distclean completely removes all traces of building including
# any files generated by configure itself.
#
distclean: clean configclean tarclean

makefileclean:
	rm -f Makefile snmplib/Makefile				\
		agent/Makefile agent/mibgroup/Makefile		\
		agent/helpers/Makefile				\
		apps/Makefile  apps/snmpnetstat/Makefile	\
		man/Makefile mibs/Makefile ov/Makefile		\
		local/Makefile testing/Makefile

configclean: makefileclean
	rm -f config.cache config.status config.log \
		libtool include/net-snmp/net-snmp-config.h \
		net-snmp-config configure-summary
	rm -f mibs/.index
	rm -f include/net-snmp/agent/mib_module_config.h		\
		snmplib/snmpsm_init.h                   \
		agent/mibgroup/mib_module_includes.h 	\
		agent/mibgroup/mib_module_inits.h 	\
		agent/mibgroup/mib_module_shutdown.h 	\
		agent/mibgroup/mib_module_dot_conf.h
	rm -f *.core

#
# Configure script related targets
#
touchit:
	touch configure include/net-snmp/net-snmp-config.h.in
	touch config.status
	touch stamp-h stamp-h.in

Makefile: Makefile.in config.status Makefile.rules Makefile.top
	@if test "x$(NOAUTODEPS)" = "x"; then \
	    ./config.status; \
	else \
	    echo "WARNING: not running config.status"; \
	fi

$(srcdir)/include/net-snmp/net-snmp-config.h.in: stamp-h.in
$(srcdir)/stamp-h.in: configure.in acconfig.h
	@if test "x$(NOAUTODEPS)" = "x" -a "x$(AUTOHEADER)" != "x:"; then \
	    cd ${srcdir} && LC_COLLATE=C $(AUTOHEADER); \
	    echo timestamp > ${srcdir}/stamp-h.in; \
	else \
	    echo "WARNING: not running autoheader"; \
	fi

include/net-snmp/net-snmp-config.h: stamp-h
stamp-h: include/net-snmp/net-snmp-config.h.in config.status
	@if test "x$(NOAUTODEPS)" = "x"; then \
	    ./config.status; \
	    echo timestamp > stamp-h; \
	else \
	    echo "WARNING: not running config.status"; \
	fi

$(srcdir)/configure: configure.in aclocal.m4
	@if test "x$(NOAUTODEPS)" = "x" -a "x$(AUTOCONF)" != "x:"; then \
	    cd ${srcdir} && $(AUTOCONF); \
	    echo "Please run configure now."; \
	    sh -c exit 2; \
	else \
	    echo "WARNING: not running autoconf"; \
	fi

config.status: configure
	@if test "x$(NOAUTODEPS)" = "x"; then \
	    ./config.status --recheck; \
	else \
	    echo "WARNING: not running config.status --recheck"; \
	fi

#
# Emacs TAGS file
#
TAGS:
	find $(srcdir) -name '*.[ch]' -print | etags -

#
# Internal distribution packaging, etc.
#
version:
	@if test "x$(VERSION)" = "x"; then \
	  echo "you need to supply a VERSION string."; \
	  exit 2; \
	fi
	${srcdir}/agent/mibgroup/versiontag $(VERSION) reverse

tag:
	@if test "x$(VERSION)" = "x"; then \
	  echo "you need to supply a VERSION string."; \
	  exit 2; \
	fi
	${srcdir}/agent/mibgroup/versiontag $(VERSION) tag

tar:
	@if test "x$(VERSION)" = "x"; then \
	  echo "you need to supply a VERSION string."; \
	  exit 2; \
	fi
	${srcdir}/agent/mibgroup/versiontag $(VERSION) tar

tarclean:
	@if test -x ${srcdir}/agent/mibgroup/versiontag ; then \
	  ${srcdir}/agent/mibgroup/versiontag Ext clean ; \
	fi

commentcheck:
	grep -n // `find $(srcdir)/ -name \*.\[ch\]` | egrep -v "(ftp|http|/win32/)"

dist: version tag tar

FAQ.html:
	local/FAQ2HTML FAQ

.PHONY: docs docsdir mancp testdirs test TAGS
# note: tags and docs are phony to force rebulding

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
