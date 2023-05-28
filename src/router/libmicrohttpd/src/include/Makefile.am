# This Makefile.am is in the public domain
SUBDIRS = .

include_HEADERS = microhttpd.h
noinst_HEADERS = microhttpd2.h microhttpd_tls.h

if HAVE_EXPERIMENTAL
include_HEADERS += microhttpd_ws.h
endif

EXTRA_DIST = platform.h autoinit_funcs.h mhd_options.h
