lib_LTLIBRARIES = libdsocks.la
#NOTE: file additions below also need to be added to dlib64/Makefile.am
#the interposition.c file is compiled first as it historically has had
#most compile problems.
libdsocks_la_SOURCES = interposition.c int_osf1.c int_osf2.c int_osf3.c \
   ../lib/Raccept.c ../lib/Rbind.c ../lib/Rbindresvport.c               \
   ../lib/Rcompat.c ../lib/Rconnect.c ../lib/Rgethostbyname.c           \
   ../lib/Rgetpeername.c ../lib/Rgetsockname.c ../lib/Rgetsockopt.c     \
   ../lib/Rlisten.c ../lib/Rrresvport.c ../lib/address.c                \
   ../lib/addressmatch.c ../lib/authneg.c ../lib/broken.c               \
   ../lib/client.c ../lib/clientconfig.c ../lib/clientprotocol.c        \
   ../lib/config.c ../lib/config_parse.c ../lib/config_scan.c           \
   ../lib/connectchild.c ../lib/debug.c ../lib/fmt.c                    \
   ../lib/fmt_customer.c ../lib/fmt_protocol.c ../lib/gssapi.c          \
   ../lib/hostcache.c ../lib/hostid.c ../lib/httpproxy.c ../lib/hw.c    \
   ../lib/iface.c ../lib/io.c ../lib/iobuf.c ../lib/ipv6.c              \
   ../lib/log.c ../lib/method.c ../lib/parse_util.c ../lib/protocol.c   \
   ../lib/serr.c ../lib/showconfig.c ../lib/sockaddr.c ../lib/socket.c  \
   ../lib/socketopt.c ../lib/sockopt.c ../lib/time.c ../lib/tostring.c  \
   ../lib/udp.c ../lib/udp_util.c ../lib/upnp.c ../lib/userio.c         \
   ../lib/util.c

#libscompat - files common for both client and server
EXTRA_libdsocks_la_SOURCES = ../libscompat/getifaddrs.c \
   ../libscompat/hstrerror.c ../libscompat/inet_ntoa.c  \
   ../libscompat/inet_pton.c ../libscompat/memmove.c    \
   ../libscompat/pselect.c ../libscompat/setproctitle.c \
   ../libscompat/strlcpy.c ../libscompat/strvis.c       \
   ../libscompat/vsyslog.c
#libscompat - files only used by client
EXTRA_libdsocks_la_SOURCES += ../libscompat/issetugid.c

libdsocks_la_LDFLAGS = -avoid-version -all-dynamic @MAPOPT@
libdsocks_la_LIBADD = @LIBDSCSRC@ @DLIBDEPS@ @LIBSCOMPAT@
libdsocks_la_DEPENDENCIES = Makefile.am _reconf @LIBDSCSRC@
libdsocks_la_LIBTOOLFLAGS = --tag=disable-static

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/libscompat \
	   -I$(top_srcdir)/lib

AM_CFLAGS = -DSOCKS_CLIENT=1 -DSOCKS_SERVER=0 -DSOCKSLIBRARY_DYNAMIC=1 -DSTANDALONE_UNIT_TEST=0

#XXX hack used to ensure depcomp is rerun when Makefile.am files are changed
_reconf: ../_reconf Makefile.am
	@(cd ..; make _reconf)
	@echo "notice: autoconf regeneration complete, rerun make"
	@false

#lint files
LINT = @LINT@
LINTFLAGS = @LINTFLAGS@

if RUNLINT
SUFFIXES = .ln
LINTFILESa = $(libdsocks_la_SOURCES:.c=.ln)
LINTFILESb = $(LINTFILESa:.y=.ln)
LINTFILES = $(LINTFILESb:.l=.ln)
libdsocks_la_DEPENDENCIES += $(LINTFILES) llib-ldsocks.ln

CLEANFILES = $(LINTFILES)

.c.ln:
	-$(LINT) $(LINTPASS1) $(LINTFLAGS) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(AM_CFLAGS) $< > $@.lint 2>&1

llib-ldsocks.ln: $(LINTFILES)
	-$(LINT) $(LINTPASS2) $(LINTFLAGS) $(LINTLIBOPT)dsocks $(LINTFILES) > $@.lint 2>&1
endif #RUNLINT
