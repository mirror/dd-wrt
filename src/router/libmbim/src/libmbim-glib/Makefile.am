
SUBDIRS = generated . test

# Core library, built as a noinst
noinst_LTLIBRARIES = libmbim-glib-core.la
libmbim_glib_core_la_CPPFLAGS = \
	$(LIBMBIM_GLIB_CFLAGS) \
	$(GUDEV_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	-I$(top_srcdir)/src/libmbim-glib \
	-I$(top_srcdir)/src/libmbim-glib/generated \
	-I$(top_builddir)/src/libmbim-glib \
	-I$(top_builddir)/src/libmbim-glib/generated \
	-DLIBMBIM_GLIB_COMPILATION \
	-DLIBEXEC_PATH=\""$(libexecdir)"\" \
	-DG_LOG_DOMAIN=\"Mbim\"
libmbim_glib_core_la_SOURCES = \
	mbim-version.h \
	mbim-errors.h \
	mbim-enums.h \
	mbim-utils.h mbim-utils.c \
	mbim-uuid.h mbim-uuid.c \
	mbim-cid.h mbim-cid.c \
	mbim-message-private.h mbim-message.h mbim-message.c \
	mbim-device.h mbim-device.c \
	mbim-compat.h mbim-compat.c \
	mbim-proxy.h mbim-proxy.c \
	mbim-proxy-helpers.h mbim-proxy-helpers.c

# Final installable library
lib_LTLIBRARIES = libmbim-glib.la

libmbim_glib_la_SOURCES = \
	libmbim-glib.h

libmbim_glib_la_LIBADD = \
	libmbim-glib-core.la \
	${top_builddir}/src/libmbim-glib/generated/libmbim-glib-generated.la \
	$(LIBMBIM_GLIB_LIBS) \
	$(GUDEV_LIBS)

libmbim_glib_la_LDFLAGS = \
	-version-info $(MBIM_GLIB_LT_CURRENT):$(MBIM_GLIB_LT_REVISION):$(MBIM_GLIB_LT_AGE) \
	-export-symbols-regex '^mbim_.*'

includedir = @includedir@/libmbim-glib
include_HEADERS = \
	libmbim-glib.h \
	mbim-version.h \
	mbim-errors.h \
	mbim-enums.h \
	mbim-utils.h \
	mbim-uuid.h \
	mbim-cid.h \
	mbim-message.h \
	mbim-device.h \
	mbim-compat.h \
	mbim-proxy.h

EXTRA_DIST = \
	mbim-version.h.in
