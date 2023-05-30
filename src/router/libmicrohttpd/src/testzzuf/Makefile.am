# This Makefile.am is in the public domain
SUBDIRS  = .

AM_CPPFLAGS = \
  -I$(top_srcdir)/src/include \
  -DMHD_CPU_COUNT=$(CPU_COUNT) \
  $(CPPFLAGS_ac) $(LIBCURL_CPPFLAGS)

AM_CFLAGS = $(CFLAGS_ac) @LIBGCRYPT_CFLAGS@

AM_LDFLAGS = $(LDFLAGS_ac)

if USE_COVERAGE
  AM_CFLAGS += -fprofile-arcs -ftest-coverage
endif

LDADD = \
  $(top_builddir)/src/microhttpd/libmicrohttpd.la \
  @LIBCURL@

EXTRA_DIST = README socat.c

THREAD_ONLY_TESTS = \
  test_long_header

check_PROGRAMS = \
  test_get \
  test_get_chunked \
  test_post \
  test_post_form \
  test_put \
  test_put_chunked \
  test_put_large \
  test_get11 \
  test_post11 \
  test_post_form11 \
  test_put11 \
  test_put_large11

.NOTPARALLEL:


if USE_POSIX_THREADS
check_PROGRAMS += \
  $(THREAD_ONLY_TESTS)
endif
if USE_W32_THREADS
check_PROGRAMS += \
  $(THREAD_ONLY_TESTS)
endif


TESTS = $(check_PROGRAMS)

test_get_SOURCES = \
  test_get.c

test_get_chunked_SOURCES = \
  test_get_chunked.c

test_post_SOURCES = \
  test_post.c

test_post_form_SOURCES = \
  test_post_form.c

test_put_SOURCES = \
  test_put.c

test_put_chunked_SOURCES = \
  test_put_chunked.c

test_put_large_SOURCES = \
  test_put_large.c



test_get11_SOURCES = \
  test_get.c

test_post11_SOURCES = \
  test_post.c

test_post_form11_SOURCES = \
  test_post_form.c

test_put11_SOURCES = \
  test_put.c

test_put_large11_SOURCES = \
  test_put_large.c

test_long_header_SOURCES = \
  test_long_header.c
