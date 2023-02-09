include libpsl-srcs.mk

# suffixes.c must be created before psl.c is compiled
BUILT_SOURCES = suffixes_dafsa.h

# suffixes.c is a built source that must be cleaned
CLEANFILES = suffixes_dafsa.h

lib_LTLIBRARIES = libpsl.la

libpsl_la_SOURCES = $(LIBPSL_SRCS)
libpsl_la_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/include -DPSL_DISTFILE=\"$(PSL_DISTFILE)\" \
  $(CFLAG_VISIBILITY) -DBUILDING_PSL
libpsl_la_LIBADD = $(LTLIBICONV) $(LTLIBINTL) @INTL_MACOSX_LIBS@

# include ABI version information
libpsl_la_LDFLAGS = -no-undefined -version-info $(LIBPSL_SO_VERSION)

# Build rule for suffix_dafsa.c
# PSL_FILE can be set by ./configure --with-psl-file=[PATH]
suffixes_dafsa.h: $(PSL_FILE) $(srcdir)/psl-make-dafsa
	$(PYTHON) $(srcdir)/psl-make-dafsa --output-format=cxx+ "$(PSL_FILE)" suffixes_dafsa.h

EXTRA_DIST = psl-make-dafsa LICENSE.chromium meson.build

dist_man_MANS = psl-make-dafsa.1
