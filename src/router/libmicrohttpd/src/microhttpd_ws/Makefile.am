# This Makefile.am is in the public domain
AM_CPPFLAGS = \
  -I$(top_srcdir)/src/include \
  -I$(top_srcdir)/src/microhttpd

AM_CFLAGS = $(HIDDEN_VISIBILITY_CFLAGS)

noinst_DATA =
MOSTLYCLEANFILES =

SUBDIRS = .

lib_LTLIBRARIES = \
  libmicrohttpd_ws.la
libmicrohttpd_ws_la_SOURCES = \
  sha1.c sha1.h \
  mhd_websocket.c
libmicrohttpd_ws_la_CPPFLAGS = \
  $(AM_CPPFLAGS) $(MHD_LIB_CPPFLAGS) \
  -DBUILDING_MHD_LIB=1
libmicrohttpd_ws_la_CFLAGS = \
  $(AM_CFLAGS) $(MHD_LIB_CFLAGS)
libmicrohttpd_ws_la_LDFLAGS = \
  $(MHD_LIB_LDFLAGS) \
  $(W32_MHD_LIB_LDFLAGS) \
  -version-info 0:0:0
libmicrohttpd_ws_la_LIBADD = \
  $(MHD_LIBDEPS)

TESTS = $(check_PROGRAMS)

check_PROGRAMS = \
  test_websocket

test_websocket_SOURCES = \
  test_websocket.c
test_websocket_LDADD = \
  $(top_builddir)/src/microhttpd_ws/libmicrohttpd_ws.la \
  $(top_builddir)/src/microhttpd/libmicrohttpd.la
