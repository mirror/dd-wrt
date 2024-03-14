AUTOMAKE_OPTIONS=foreign subdir-objects

SUBDIRS = 
EXTRA_DIST = cmdline.ggo cmdline.patch \
linux/Makefile linux/xt_coova.c linux/xt_coova.h \
linux/ipt_coova.h linux/libxt_coova.c \
cmdline.mini.sh cmdline.mini.gen 

chilliincludedir = $(includedir)/chilli
chilliinclude_HEADERS = \
chilli.h session.h garden.h radius.h options.h \
tun.h ippool.h md5.h SHA256.h redir.h dhcp.h iphash.h \
radius_wispr.h radius_coovachilli.h ssl.h dns.h net.h \
pkt.h conn.h lookup.h chilli_limits.h cmdline.h debug.h \
radius_pkt.h ../bstring/bstrlib.h ../config.h system.h 

lib_LTLIBRARIES = libchilli.la
sbin_PROGRAMS = chilli_multicall

suid_programs =

libchilli_la_SOURCES = \
chilli.c tun.c ippool.c radius.c md5.c SHA256.c redir.c dhcp.c \
iphash.c lookup.c system.h util.c options.c statusfile.c conn.c sig.c \
garden.c dns.c session.c pkt.c chksum.c net.c safe.c

AM_CFLAGS = -D_GNU_SOURCE -Wall -fno-builtin -fno-strict-aliasing \
  -fomit-frame-pointer -funroll-loops -pipe -I$(top_builddir)/bstring \
 -DDEFCHILLICONF='"$(sysconfdir)/chilli.conf"'\
 -DDEFPIDFILE='"$(localstatedir)/run/chilli.pid"'\
 -DDEFSTATEDIR='"$(localstatedir)/run"'\
 -DSBINDIR='"$(sbindir)"' 

chilli_multicall_SOURCES = multicall.c main.c main-radconfig.c main-response.c main-query.c main-opt.c

libchilli_la_LIBADD = $(top_builddir)/bstring/libbstring.la 
LDADD = libchilli.la ${LIBRT} $(top_builddir)/bstring/libbstring.la ${LIBJSON}

chilli_multicall_LDADD = $(LDADD)

if WITH_MINICONFIG
chilli_multicall_SOURCES += cmdline.mini.c
else
chilli_multicall_SOURCES += cmdline.c 
endif

if WITH_PATRICIA
libchilli_la_SOURCES += patricia.c patricia.h
endif

if WITH_JSONLIB
AM_CFLAGS += -I$(top_builddir)/json
libchilli_la_LIBADD += $(top_builddir)/json/libjson.la 
LDADD += $(top_builddir)/json/libjson.la 
endif

if WITH_EWTAPI
libchilli_la_SOURCES += ewt.c ewt.h
AM_CFLAGS += -std=gnu99 
endif

if WITH_LOCATION
libchilli_la_SOURCES += location.c 
endif

if WITH_SFHASH
libchilli_la_SOURCES += sfhash.c
endif

if WITH_LOOKUP3
libchilli_la_SOURCES += lookup3.c
endif

if WITH_OPENSSL
libchilli_la_LIBADD += ${LIBSSL}
LDADD += ${LIBSSL}
endif

if WITH_CYASSL
libchilli_la_LIBADD += ${LIBSSL}
LDADD += ${LIBSSL}
endif

if WITH_MATRIXSSL
libchilli_la_SOURCES += mssl.c mssl.h
libchilli_la_LIBADD += -lmatrixssl
LDADD += -lmatrixssl
if WITH_MATRIXSSL_CLI
SUBDIRS += mssl
endif
endif

if WITH_AVL
libchilli_la_SOURCES += avl/avl.c avl/avl.h avl/list.h \
avl/common_types.h avl/container_of.h
endif

if WITH_SSL
libchilli_la_SOURCES += ssl.c ms_chap.c 
endif

if WITH_NETFILTER_QUEUE
libchilli_la_SOURCES += queue.c
LDADD += ${LIBNETFILTER_QUEUE}
endif

if WITH_NETNAT
libchilli_la_SOURCES += nat.c nat.h
endif

if WITH_PCAP
LDADD += ${LIBPCAP}
endif

if WITH_MULTIROUTE
chilli_multicall_SOURCES += main-rtmon.c
libchilli_la_SOURCES += rtmon.c rtmon.h
endif

if WITH_CHILLIQUERY
libchilli_la_SOURCES += cmdsock.c
chilliinclude_HEADERS += cmdsock.h
endif

if WITH_CHILLIPROXY
chilli_multicall_SOURCES = main-proxy.c 
chilli_multicall_LDADD += ${LDADD}
if WITH_CURL
chilli_multicall_LDADD += ${LIBCURL} 
endif
endif

if WITH_CHILLIRADSEC
chilli_multicall_SOURCES = main-radsec.c 
chilli_multicall_LDADD = ${LDADD}
endif

if WITH_CHILLISCRIPT
suid_programs += chilli_script
chilli_multicall_SOURCES = main-script.c 
chilli_multicall_LDADD = ${LDADD}
endif

if WITH_CHILLIREDIR
chilli_multicall_SOURCES = main-redir.c 
chilli_multicall_LDADD = ${LDADD}
if WITH_CURL
chilli_multicall_LDADD += ${LIBCURL} 
endif
endif

if WITH_NETFILTER_COOVA
SUBDIRS += linux
libchilli_la_SOURCES += kcoova.c
endif

if WITH_MODULES
sample_la_SOURCES = sample-mod.c
sample_la_LDFLAGS = -module -avoid-version
samplelibdir = $(libdir)/coova-chilli
samplelib_LTLIBRARIES = sample.la
libchilli_la_SOURCES += chilli_module.c
libchilli_la_LIBADD += -ldl
LDADD += -ldl
chilliinclude_HEADERS += chilli_module.h
AM_CFLAGS += -DDEFLIBDIR='"$(libdir)/coova-chilli"'
endif

if !HAVE_STRLCPY
libchilli_la_SOURCES += ../extern/strlcpy.c
endif

CMDLINE = cmdline.ggo
if WITH_CONFIG
CMDLINE += `[ -e ../../cmdline.ggo ] && echo ../../cmdline.ggo`
endif

install-exec-hook:
	@for f in $(suid_programs); do \
		echo "Setting SUID on $(DESTDIR)$(sbindir)/$$f !"; \
		chown root $(DESTDIR)$(sbindir)/$$f; \
		chmod u+s $(DESTDIR)$(sbindir)/$$f; \
	done

main-opt.c: cmdline.c cmdline.h

cmdline.c: cmdline.ggo cmdline.patch
	cat $(CMDLINE) | gengetopt -C 
	cp cmdline.c cmdline.orig
	patch -p0 < cmdline.patch

cmdline.mini.gen: cmdline.ggo 
	sh cmdline.mini.sh > cmdline.mini.gen

cmdline.mini.c: cmdline.mini.gen
