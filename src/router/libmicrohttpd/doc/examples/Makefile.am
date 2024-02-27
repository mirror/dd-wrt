# This Makefile.am is in the public domain
SUBDIRS  = .

AM_CPPFLAGS = \
  -I$(top_srcdir)/src/include \
  $(CPPFLAGS_ac)

AM_CFLAGS = $(CFLAGS_ac) @LIBGCRYPT_CFLAGS@

AM_LDFLAGS = $(LDFLAGS_ac)

AM_TESTS_ENVIRONMENT = $(TESTS_ENVIRONMENT_ac)

if USE_COVERAGE
  AM_CFLAGS += --coverage
endif

$(top_builddir)/src/microhttpd/libmicrohttpd.la: $(top_builddir)/src/microhttpd/Makefile
	@echo ' cd $(top_builddir)/src/microhttpd && $(MAKE) $(AM_MAKEFLAGS) libmicrohttpd.la'; \
	$(am__cd) $(top_builddir)/src/microhttpd && $(MAKE) $(AM_MAKEFLAGS) libmicrohttpd.la

# example programs
noinst_PROGRAMS = \
  hellobrowser \
  logging \
  responseheaders
  
if ENABLE_BAUTH
noinst_PROGRAMS += \
  basicauthentication
if ENABLE_HTTPS
noinst_PROGRAMS += \
  tlsauthentication
endif
endif

if HAVE_POSTPROCESSOR
noinst_PROGRAMS += simplepost largepost sessions 
endif

if HAVE_W32
AM_CPPFLAGS += -DWINDOWS
endif

if HAVE_EXPERIMENTAL
noinst_PROGRAMS += websocket
endif

basicauthentication_SOURCES = \
 basicauthentication.c 
basicauthentication_LDADD = \
 $(top_builddir)/src/microhttpd/libmicrohttpd.la 

hellobrowser_SOURCES = \
 hellobrowser.c 
hellobrowser_LDADD = \
 $(top_builddir)/src/microhttpd/libmicrohttpd.la 

logging_SOURCES = \
 logging.c 
logging_LDADD = \
 $(top_builddir)/src/microhttpd/libmicrohttpd.la

responseheaders_SOURCES = \
 responseheaders.c 
responseheaders_LDADD = \
 $(top_builddir)/src/microhttpd/libmicrohttpd.la 

sessions_SOURCES = \
 sessions.c 
sessions_LDADD = \
 $(top_builddir)/src/microhttpd/libmicrohttpd.la 

tlsauthentication_SOURCES = \
 tlsauthentication.c 
tlsauthentication_LDADD = \
 $(top_builddir)/src/microhttpd/libmicrohttpd.la 

simplepost_SOURCES = \
 simplepost.c 
simplepost_LDADD = \
 $(top_builddir)/src/microhttpd/libmicrohttpd.la 

largepost_SOURCES = \
 largepost.c 
largepost_LDADD = \
 $(top_builddir)/src/microhttpd/libmicrohttpd.la 

websocket_SOURCES = \
 websocket.c
websocket_LDADD = \
 $(top_builddir)/src/microhttpd/libmicrohttpd.la \
 $(top_builddir)/src/microhttpd_ws/libmicrohttpd_ws.la
