# This Makefile.am is in the public domain
SUBDIRS = .

AM_CPPFLAGS = \
  -I$(top_srcdir)/src/include \
  -DMHD_REAL_CPU_COUNT=@MHD_REAL_CPU_COUNT@ \
  -DMHD_CPU_COUNT=$(CPU_COUNT) \
  $(CPPFLAGS_ac) \
  -DDATA_DIR=\"$(top_srcdir)/src/datadir/\"

AM_CFLAGS = $(CFLAGS_ac) @LIBGCRYPT_CFLAGS@

AM_LDFLAGS = $(LDFLAGS_ac)

AM_TESTS_ENVIRONMENT = $(TESTS_ENVIRONMENT_ac)

if USE_COVERAGE
  AM_CFLAGS += --coverage
endif

LDADD = \
  $(top_builddir)/src/microhttpd/libmicrohttpd.la

$(top_builddir)/src/microhttpd/libmicrohttpd.la: $(top_builddir)/src/microhttpd/Makefile
	@echo ' cd $(top_builddir)/src/microhttpd && $(MAKE) $(AM_MAKEFLAGS) libmicrohttpd.la'; \
	$(am__cd) $(top_builddir)/src/microhttpd && $(MAKE) $(AM_MAKEFLAGS) libmicrohttpd.la


# Tools
noinst_PROGRAMS = 

if USE_THREADS
noinst_PROGRAMS += \
    perf_replies
endif


perf_replies_SOURCES = \
    perf_replies.c mhd_tool_str_to_uint.h \
    mhd_tool_get_cpu_count.h mhd_tool_get_cpu_count.c
