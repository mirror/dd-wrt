sbin_PROGRAMS = sockd
sockd_SOURCES = ../lib/addressmatch.c ../lib/authneg.c ../lib/broken.c	\
   ../lib/clientprotocol.c ../lib/config.c ../lib/config_parse.c	\
   ../lib/config_scan.c ../lib/debug.c ../lib/errorsymbols.c		\
   ../lib/fmt.c ../lib/fmt_customer.c ../lib/fmt_protocol.c		\
   ../lib/gssapi.c ../lib/hostcache.c ../lib/hostid.c			\
   ../lib/httpproxy.c ../lib/hw.c ../lib/iface.c ../lib/io.c		\
   ../lib/iobuf.c ../lib/ipv6.c ../lib/krb5.c ../lib/log.c		\
   ../lib/method.c ../lib/parse_util.c ../lib/protocol.c		\
   ../lib/serr.c ../lib/showconfig.c ../lib/sockaddr.c			\
   ../lib/socket.c ../lib/socketopt.c ../lib/sockopt.c			\
   ../lib/tcpinfo.c ../lib/time.c ../lib/tostring.c ../lib/udp_util.c	\
   ../lib/upnp.c ../lib/userio.c ../lib/util.c				\
   ../libscompat/in_cksum.c						\
									\
   accesscheck.c auth_bsd.c auth_pam.c auth_password.c bandwidth.c	\
   bandwidth_key.c checkmodule.c cpu.c cpuaffinity.c cpupolicy.c	\
   dante_udp.c fmt_sockd.c getoutaddr.c ldap.c ldap_key.c ldap_util.c	\
   math.c method_gssapi.c method_uname.c monitor.c monitor_util.c	\
   mother_util.c pac.c pac_key.c privileges.c redirect.c		\
   redirect_key.c rule.c serverconfig.c session.c shmem.c		\
   shmemconfig.c sockd.c sockd_child.c sockd_icmp.c sockd_io.c		\
   sockd_io_misc.c sockd_negotiate.c sockd_protocol.c sockd_request.c	\
   sockd_socket.c sockd_tcp.c sockd_udp.c sockd_util.c statistics.c	\
   udp_port.c

#libscompat - files common for both client and server
EXTRA_sockd_SOURCES = ../libscompat/getifaddrs.c        \
   ../libscompat/hstrerror.c ../libscompat/inet_ntoa.c  \
   ../libscompat/inet_pton.c ../libscompat/memmove.c    \
   ../libscompat/pselect.c ../libscompat/setproctitle.c \
   ../libscompat/strlcpy.c ../libscompat/strvis.c       \
   ../libscompat/vsyslog.c
#libscompat - files only used by server
EXTRA_sockd_SOURCES += ../libscompat/daemon.c ../libscompat/seteuid.c   \
   ../libscompat/sockatmark.c

#use preloading if enabled and possible
if SERVER_INTERPOSITION
SERVDL=1
sockd_SOURCES += ../dlib/interposition.c
else
SERVDL=0
endif #SERVER_INTERPOSITION

sockd_LDADD = @SOCKDCOMPAT@ @SOCKDDEPS@ @LIBSCOMPAT@
sockd_DEPENDENCIES = Makefile.am _reconf @SOCKDCOMPAT@

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/libscompat \
	   -I$(top_srcdir)/lib

CFLAGVALS = -DSOCKS_SERVER=1 -DSOCKS_CLIENT=0 -DSOCKSLIBRARY_DYNAMIC=${SERVDL}
AM_CFLAGS = $(CFLAGVALS) -DSTANDALONE_UNIT_TEST=0

#XXX hack used to ensure depcomp is rerun when Makefile.am files are changed
_reconf: ../_reconf Makefile.am
	@(cd ..; make _reconf)
	@echo "notice: autoconf regeneration complete, rerun make"
	@false

#unit test framework source file listing
dumpunitenv:
	@echo CC=\"$(CC)\"
	@echo CFLAGS=\"$(CFLAGS) $(CFLAGVALS) -DSTANDALONE_UNIT_TEST=1\"
	@echo CPP=\"$(CPP)\"
	@echo CPPFLAGS=\"$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(CPPFLAGS)\"
	@echo LDFLAGS=\"$(LDFLAGS)\"
	@echo LIBS=\"$(LIBS) $(SOCKDDEPS) @LIBSCOMPAT@\"
	@echo AMSRC=\"$(sockd_SOURCES)\"
#	@echo AMOBJ=\"$(sockd_OBJECTS)\"
	@echo AMOPTSRC=\"$(EXTRA_sockd_SOURCES)\"
	@echo AMOPTOBJ=\"@SOCKDCOMPAT@\"

#lint files
LINT = @LINT@
LINTFLAGS = @LINTFLAGS@

if RUNLINT
SUFFIXES = .ln
LINTFILESa = $(sockd_SOURCES:.c=.ln)
LINTFILESb = $(LINTFILESa:.y=.ln)
LINTFILES = $(LINTFILESb:.l=.ln)
sockd_DEPENDENCIES += $(LINTFILES) $(SOCKDCOMPAT) pass2.ln

CLEANFILES = $(LINTFILES)

.c.ln:
	-$(LINT) $(LINTPASS1) $(LINTFLAGS) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(AM_CFLAGS) $< > $@.lint 2>&1

pass2.ln: $(LINTFILES)
	-@test -s interposition.ln && cp interposition.ln ../dlib #XXX
	-$(LINT) $(LINTPASS2) $(LINTFLAGS) -L../libscompat -L../lib @LINTSCCOMPATLIB@ -lsocks $(LINTFILES) > $@.lint 2>&1
endif #RUNLINT
