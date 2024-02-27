# This Makefile.am is in the public domain
EMPTY_ITEM =

SUBDIRS = .

@HEAVY_TESTS_NOTPARALLEL@

AM_CPPFLAGS = \
  -I$(top_srcdir)/src/include \
  -I$(top_srcdir)/src/microhttpd \
  -DMHD_CPU_COUNT=$(CPU_COUNT) \
  -DSRCDIR=\"$(srcdir)\" \
  $(CPPFLAGS_ac) $(LIBCURL_CPPFLAGS) $(MHD_TLS_LIB_CPPFLAGS)

AM_CFLAGS = $(CFLAGS_ac) @LIBGCRYPT_CFLAGS@

AM_LDFLAGS = $(LDFLAGS_ac)

AM_TESTS_ENVIRONMENT = $(TESTS_ENVIRONMENT_ac)

if USE_COVERAGE
  AM_CFLAGS += --coverage
endif

$(top_builddir)/src/microhttpd/libmicrohttpd.la: $(top_builddir)/src/microhttpd/Makefile
	@echo ' cd $(top_builddir)/src/microhttpd && $(MAKE) $(AM_MAKEFLAGS) libmicrohttpd.la'; \
	$(am__cd) $(top_builddir)/src/microhttpd && $(MAKE) $(AM_MAKEFLAGS) libmicrohttpd.la

LDADD = \
  $(top_builddir)/src/microhttpd/libmicrohttpd.la \
  $(MHD_TLS_LIB_LDFLAGS) $(MHD_TLS_LIBDEPS) @LIBGCRYPT_LIBS@ @LIBCURL@

if HAVE_GNUTLS_SNI
  TEST_HTTPS_SNI = test_https_sni
endif

if !HAVE_GNUTLS_MTHREAD_BROKEN
HTTPS_PARALLEL_TESTS = \
    test_https_time_out \
    test_https_get_parallel \
    test_https_get_parallel_threads
endif

THREAD_ONLY_TESTS = \
  test_tls_options \
  test_tls_authentication \
  $(HTTPS_PARALLEL_TESTS) \
  $(TEST_HTTPS_SNI) \
  test_https_session_info \
  test_https_session_info_append \
  test_https_multi_daemon \
  test_https_get \
  test_empty_response \
  test_https_get_iovec \
  $(EMPTY_ITEM)

check_PROGRAMS = \
  test_https_get_select

if USE_THREADS
check_PROGRAMS += \
  $(THREAD_ONLY_TESTS)
endif

EXTRA_DIST = \
  test-ca.crt test-ca.key \
  mhdhost1.crt mhdhost1.key \
  mhdhost2.crt mhdhost2.key

TESTS = \
  $(check_PROGRAMS)


test_https_time_out_SOURCES = \
  test_https_time_out.c \
  tls_test_keys.h \
  tls_test_common.h \
  tls_test_common.c

test_tls_options_SOURCES = \
  test_tls_options.c \
  tls_test_keys.h \
  tls_test_common.h \
  tls_test_common.c

test_https_get_parallel_SOURCES = \
  test_https_get_parallel.c \
  tls_test_keys.h \
  tls_test_common.h \
  tls_test_common.c
test_https_get_parallel_CFLAGS = \
  $(AM_CFLAGS) $(PTHREAD_CFLAGS)
test_https_get_parallel_LDADD = \
  $(PTHREAD_LIBS) $(LDADD)

test_empty_response_SOURCES = \
  test_empty_response.c \
  tls_test_keys.h \
  tls_test_common.h \
  tls_test_common.c

test_https_get_parallel_threads_SOURCES = \
  test_https_get_parallel_threads.c \
  tls_test_keys.h \
  tls_test_common.h \
  tls_test_common.c
test_https_get_parallel_threads_CFLAGS = \
  $(AM_CFLAGS) $(PTHREAD_CFLAGS)
test_https_get_parallel_threads_LDADD = \
  $(PTHREAD_LIBS) $(LDADD)

test_tls_authentication_SOURCES = \
  test_tls_authentication.c \
  tls_test_keys.h \
  tls_test_common.h \
  tls_test_common.c

test_https_session_info_SOURCES = \
  test_https_session_info.c \
  tls_test_keys.h \
  tls_test_common.h \
  tls_test_common.c

test_https_session_info_append_SOURCES = $(test_https_session_info_SOURCES)

test_https_multi_daemon_SOURCES = \
  test_https_multi_daemon.c \
  tls_test_keys.h \
  tls_test_common.h \
  tls_test_common.c

test_https_get_SOURCES = \
  test_https_get.c \
  tls_test_keys.h \
  tls_test_common.h \
  tls_test_common.c

test_https_get_iovec_SOURCES = \
  test_https_get_iovec.c \
  tls_test_keys.h \
  tls_test_common.h \
  tls_test_common.c

test_https_sni_SOURCES = \
  test_https_sni.c \
  tls_test_keys.h \
  tls_test_common.h \
  tls_test_common.c

test_https_get_select_SOURCES = \
  test_https_get_select.c \
  tls_test_keys.h \
  tls_test_common.h \
  tls_test_common.c
