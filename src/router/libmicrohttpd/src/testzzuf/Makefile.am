# This Makefile.am is in the public domain

SUBDIRS  = .

# ZZUF_SEED can be redefined to use other initial seeds
# for extended testing. E.g.,
#  make ZZUF_SEED=1234 check
ZZUF_SEED = 0

# Additional flags for zzuf
ZZUF_FLAGS = 

# Additional flags for socat (if socat is used)
SOCAT_FLAGS = 

if FORCE_USE_ZZUF_SOCAT
TEST_RUNNER_SCRIPT = zzuf_socat_test_runner.sh
else
TEST_RUNNER_SCRIPT = zzuf_test_runner.sh
endif

AM_CPPFLAGS = \
  -I$(top_srcdir)/src/include \
  -I$(top_srcdir)/src/microhttpd \
  -DMHD_CPU_COUNT=$(CPU_COUNT) \
  $(CPPFLAGS_ac) $(LIBCURL_CPPFLAGS)

AM_CFLAGS = $(CFLAGS_ac)

AM_LDFLAGS = $(LDFLAGS_ac)

AM_TESTS_ENVIRONMENT = $(TESTS_ENVIRONMENT_ac) \
  ZZUF="$(ZZUF)" ; export ZZUF ; \
  ZZUF_SEED="$(ZZUF_SEED)" ; export ZZUF_SEED ; \
  ZZUF_FLAGS="$(ZZUF_FLAGS)" ; export ZZUF_FLAGS ; \
  SOCAT="$(SOCAT)" ; export SOCAT ; \
  SOCAT_FLAGS="$(SOCAT_FLAGS)" ; export SOCAT_FLAGS ;

if USE_COVERAGE
  AM_CFLAGS += -fprofile-arcs -ftest-coverage
endif

LDADD = \
  $(top_builddir)/src/microhttpd/libmicrohttpd.la \
  @LIBCURL@

$(top_builddir)/src/microhttpd/libmicrohttpd.la: $(top_builddir)/src/microhttpd/Makefile
	@echo ' cd $(top_builddir)/src/microhttpd && $(MAKE) $(AM_MAKEFLAGS) libmicrohttpd.la'; \
	$(am__cd) $(top_builddir)/src/microhttpd && $(MAKE) $(AM_MAKEFLAGS) libmicrohttpd.la

check_PROGRAMS = \
  test_get \
  test_get_chunked \
  test_post \
  test_post_form \
  test_put \
  test_put_chunked \
  test_put_large \
  test_get_long_uri \
  test_get_long_header \
  test_get_close \
  test_get_chunked_close \
  test_post_close \
  test_post_form_close \
  test_put_close \
  test_put_chunked_close \
  test_put_large_close \
  test_get_long_uri_close \
  test_get_long_header_close \
  test_get10 \
  test_get_chunked10 \
  test_post10 \
  test_post_form10 \
  test_put10 \
  test_put_large10 \
  test_get_long_uri10 \
  test_get_long_header10

.NOTPARALLEL:


TESTS = $(check_PROGRAMS)

dist_check_SCRIPTS = zzuf_test_runner.sh zzuf_socat_test_runner.sh

LOG_COMPILER = @SHELL@ "$(srcdir)/$(TEST_RUNNER_SCRIPT)"

if VHEAVY_TESTS
check_SCRIPTS = warn_vheavy_use

.PHONY: warn_vheavy_use
endif

warn_vheavy_use:
	@echo "NOTICE" ; \
	echo "NOTICE: Full heavy tests are enabled. Each test may take up to several minutes to complete." ; \
	echo "NOTICE"


tests_common_sources = mhd_debug_funcs.h mhd_debug_funcs.c

test_get_SOURCES = \
  test_get.c $(tests_common_sources)

test_get_chunked_SOURCES = $(test_get_SOURCES)

test_post_SOURCES = $(test_get_SOURCES)

test_post_form_SOURCES = $(test_get_SOURCES)

test_put_SOURCES = $(test_get_SOURCES)

test_put_chunked_SOURCES = $(test_get_SOURCES)

test_put_large_SOURCES = $(test_get_SOURCES)

test_get_long_uri_SOURCES = $(test_get_SOURCES)

test_get_long_header_SOURCES = $(test_get_SOURCES)

test_get_close_SOURCES = $(test_get_SOURCES)

test_get_chunked_close_SOURCES = $(test_get_chunked_SOURCES)

test_post_close_SOURCES = $(test_post_SOURCES)

test_post_form_close_SOURCES = $(test_post_form_SOURCES)

test_put_close_SOURCES = $(test_put_SOURCES)

test_put_chunked_close_SOURCES = $(test_put_chunked_SOURCES)

test_put_large_close_SOURCES = $(test_put_large_SOURCES)

test_get_long_uri_close_SOURCES = $(test_get_long_uri_SOURCES)

test_get_long_header_close_SOURCES = $(test_get_long_header_SOURCES)

test_get10_SOURCES = $(test_get_SOURCES)

test_get_chunked10_SOURCES = $(test_get_chunked_SOURCES)

test_post10_SOURCES = $(test_post_SOURCES)

test_post_form10_SOURCES = $(test_post_form_SOURCES)

test_put10_SOURCES = $(test_put_SOURCES)

test_put_large10_SOURCES = $(test_put_large_SOURCES)

test_get_long_uri10_SOURCES = $(test_get_long_uri_SOURCES)

test_get_long_header10_SOURCES = $(test_get_long_header_SOURCES)
