lib_LTLIBRARIES = libsocks.la
#config_parse/scan compiled first to get yacc/lex run as early as possible
#(other dirs rely on yacc/lex being run here).
libsocks_la_SOURCES = config_parse.c config_scan.c                      \
   Raccept.c Rbind.c Rbindresvport.c Rcompat.c Rconnect.c               \
   Rgethostbyname.c Rgetpeername.c Rgetsockname.c Rgetsockopt.c         \
   Rlisten.c Rrresvport.c address.c addressmatch.c authneg.c broken.c   \
   client.c clientconfig.c clientprotocol.c config.c connectchild.c     \
   debug.c fmt.c fmt_customer.c fmt_protocol.c gssapi.c hostcache.c     \
   hostid.c httpproxy.c hw.c iface.c io.c iobuf.c ipv6.c log.c          \
   method.c parse_util.c protocol.c serr.c showconfig.c sockaddr.c      \
   socket.c socketopt.c sockopt.c time.c tostring.c udp.c udp_util.c    \
   upnp.c userio.c util.c

#libscompat - files common for both client and server
EXTRA_libsocks_la_SOURCES = ../libscompat/getifaddrs.c  \
   ../libscompat/hstrerror.c ../libscompat/inet_ntoa.c  \
   ../libscompat/inet_pton.c ../libscompat/memmove.c    \
   ../libscompat/pselect.c ../libscompat/setproctitle.c \
   ../libscompat/strlcpy.c ../libscompat/strvis.c       \
   ../libscompat/vsyslog.c
#libscompat - files only used by client
EXTRA_libsocks_la_SOURCES += ../libscompat/issetugid.c

#libsocks version info, interfaceversion:revision:old interfaces supported
#XXX SPEC/dante.spec.in must have the same value as version info
libsocks_la_LDFLAGS = -version-info 1:1:1 @MAPOPT@
libsocks_la_LIBADD = @LIBSCSRC@ @DLIBDEPS@ @LIBSCOMPAT@
libsocks_la_DEPENDENCIES = Makefile.am _reconf @LIBSCSRC@

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/libscompat

CFLAGVALS = -DSOCKS_CLIENT=1 -DSOCKS_SERVER=0 -DSOCKSLIBRARY_DYNAMIC=0
AM_CFLAGS = $(CFLAGVALS) -DSTANDALONE_UNIT_TEST=0

EXTRA_DIST = config_parse.y config_scan.l
noinst_HEADERS = config_parse.h
MAINTAINERCLEANFILES = config_parse.h config_parse.c config_scan.c

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
	@echo LIBS=\"$(LIBS) $(DLIBDEPS)\"
	@echo AMSRC=\"$(libsocks_la_SOURCES)\"
#	@echo AMOBJ=\"$(libsocks_la_OBJECTS)\"
	@echo AMOPTSRC=\"$(EXTRA_libsocks_la_SOURCES)\"
	@echo AMOPTOBJ=\"@LIBSCSRC@\"

PREFIX = socks_yy
AM_YFLAGS = -d -p${PREFIX}
AM_LFLAGS = -P${PREFIX} -olex.yy.c
YFLAGS = @YFLAGS@
#LFLAGS = @LFLAGS@

#XXX ensure common.h is included before any other headers for lex/yacc;
#    keep the generation rules here, with a slight modification
#    XXX likely no longer needed
config_parse.c: config_parse.y
	$(YACC) $(YFLAGS) $(AM_YFLAGS) config_parse.y
	@if test -f y.tab.h; then \
	  to=`echo "$*_H" | sed \
                -e 'y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/' \
                -e 's/[^ABCDEFGHIJKLMNOPQRSTUVWXYZ]/_/g'`; \
	  sed -e "/^#/!b" -e "s/Y_TAB_H/$$to/g" -e "s|y\.tab\.h|$*.h|" \
            y.tab.h >$*.ht; \
	  rm -f y.tab.h; \
	  if cmp -s $*.ht $*.h; then \
	    rm -f $*.ht ;\
	  else \
	    mv $*.ht $*.h; \
	  fi; \
	fi
	@if test -f y.output; then \
	  mv y.output $*.output; \
	fi
	@echo '#include "common.h"' > $@
	@sed '/^#/ s|y\.tab\.c|$@|' y.tab.c >$@t && cat $@t >> $@ && \
	   rm -f $@t
	@rm -f y.tab.c

config_scan.c: config_scan.l
	$(LEX) $(LFLAGS) $(AM_LFLAGS) config_scan.l
	@echo '#include "common.h"' > $@
	@sed '/^#/ s|$(LEX_OUTPUT_ROOT)\.c|$@|' $(LEX_OUTPUT_ROOT).c >> $@
	@rm -f $(LEX_OUTPUT_ROOT).c

#lint files
LINT = @LINT@
LINTFLAGS = @LINTFLAGS@

if RUNLINT
SUFFIXES = .ln
LINTFILESa = $(libsocks_la_SOURCES:.c=.ln)
LINTFILESb = $(LINTFILESa:.y=.ln)
LINTFILES = $(LINTFILESb:.l=.ln)
libsocks_la_DEPENDENCIES += $(LINTFILES) llib-lsocks.ln

CLEANFILES = $(LINTFILES)

.c.ln:
	-$(LINT) $(LINTPASS1) $(LINTFLAGS) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(AM_CFLAGS) $< > $@.lint 2>&1

llib-lsocks.ln: $(LINTFILES)
	-$(LINT) $(LINTPASS2) $(LINTFLAGS) $(LINTLIBOPT)socks $(LINTFILES) > $@.lint 2>&1
endif #RUNLINT
