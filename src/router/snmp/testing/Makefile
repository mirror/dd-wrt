# Generated automatically from Makefile.in by configure.
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
#
# Makefile for testing
#

#TARG		= etest ktest misctest stest

USELIBS		= ../snmplib/libsnmp.$(LIB_EXTENSION)$(LIB_VERSION)
LIBS		= -L../snmplib $(CC_RUNTIME_ARG) -lsnmp -lm 

PARSEOBJS	=

CPPFLAGS	= -I.. -I$(srcdir)/.. -I$(srcdir)/../snmplib 
CC		= mipsel-linux-uclibc-gcc $(CPPFLAGS)

all: $(TARG)

test:
	$(srcdir)/RUNTESTS -a -D `pwd`/..

etest:    etimetest.o $(PARSEOBJS) $(USELIBS)
	${CC} -o $@ etimetest.o $(PARSEOBJS) ${LDFLAGS} ${LIBS} 

ktest:    keymanagetest.o $(PARSEOBJS) $(USELIBS)
	${CC} -o $@ keymanagetest.o $(PARSEOBJS) ${LDFLAGS} ${LIBS} 

misctest:    misctest.o $(PARSEOBJS) $(USELIBS)
	${CC} -o $@ misctest.o $(PARSEOBJS) ${LDFLAGS} ${LIBS} 

stest:    scapitest.o $(PARSEOBJS) $(USELIBS)
	${CC} -o $@ scapitest.o $(PARSEOBJS) ${LDFLAGS} ${LIBS} 

clean: testclean
	rm -f *.o core *.core $(TARG)

testclean:
	-rm -fr /tmp/snmp-test*

install: installdirs $(TARG)
	@for i in $(TARG) ; \
		do $(INSTALL) $$i $(bindir) ; \
		echo "install:  installed $$i in $(bindir)" ; \
	done

installdirs:
	@$(SHELL) $(srcdir)/../mkinstalldirs $(snmplibdir) $(bindir)

dependlocal: 
	makedepend $(CPPFLAGS) -o.lo $(srcdir)/*.c

depend: dependlocal

nosysdepend: 
	makedepend $(CPPFLAGS) -o.lo *.c
	perl -n -i.bak $(top_srcdir)/makenosysdepend.pl Makefile

makefileindepend: nosysdepend
	perl $(top_srcdir)/makefileindepend.pl


# DO NOT DELETE THIS LINE -- make depend depends on it.


etimetest.lo: ../config.h ../m/generic.h ../s/generic.h ../s/linux.h
etimetest.lo: ../snmplib/asn1.h ../snmplib/callback.h ../snmplib/lcd_time.h
etimetest.lo: ../snmplib/snmp_api.h ../snmplib/snmp_debug.h ../snmplib/tools.h
etimetest.lo: ../s/sysv.h  
keymanagetest.lo: ../config.h ../m/generic.h ../s/generic.h ../s/linux.h
keymanagetest.lo: ../snmplib/asn1.h ../snmplib/callback.h ../snmplib/keytools.h
keymanagetest.lo: ../snmplib/scapi.h ../snmplib/snmp_api.h ../snmplib/tools.h
keymanagetest.lo: ../snmplib/transform_oids.h ../s/sysv.h 
misctest.lo: ../config.h ../m/generic.h ../s/generic.h ../s/linux.h
misctest.lo: ../snmplib/asn1.h ../snmplib/callback.h ../snmplib/snmp_api.h
misctest.lo: ../snmplib/tools.h ../snmplib/transform_oids.h ../s/sysv.h
scapitest.lo: ../config.h ../m/generic.h ../s/generic.h ../s/linux.h
scapitest.lo: ../snmplib/asn1.h ../snmplib/callback.h ../snmplib/keytools.h
scapitest.lo: ../snmplib/scapi.h ../snmplib/snmp_api.h ../snmplib/tools.h
scapitest.lo: ../snmplib/transform_oids.h ../s/sysv.h 
T.lo: ../config.h ../m/generic.h ../s/generic.h ../s/linux.h ../s/sysv.h
