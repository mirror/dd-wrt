ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

LIBUDEV_CURRENT=7
LIBUDEV_REVISION=3
LIBUDEV_AGE=6

define move-to-rootlibdir
	if test "$(libdir)" != "$(rootlibdir)"; then \
		$(MKDIR_P) $(DESTDIR)$(rootlibdir) && \
		so_img_name=$$(readlink $(DESTDIR)$(libdir)/$$libname) && \
		so_img_rel_target_prefix=$$(echo $(libdir) | sed 's,\(^/\|\)[^/][^/]*,..,g') && \
		$(LN_S) -f $$so_img_rel_target_prefix$(rootlibdir)/$$so_img_name $(DESTDIR)$(libdir)/$$libname && \
		mv $(DESTDIR)$(libdir)/$$libname.* $(DESTDIR)$(rootlibdir); \
	fi
endef

AM_CPPFLAGS = \
	-include $(top_builddir)/config.h \
	-DUDEV_ROOT_RUN=\"$(rootrundir)\" \
	-DUDEV_CONF_FILE=\"$(udevconffile)\" \
	-DUDEV_HWDB_BIN=\"$(udevhwdbbin)\" \
	-DUDEV_VERSION=$(UDEV_VERSION) \
	-I $(top_srcdir)/src/shared

lib_LTLIBRARIES = \
	libudev.la

libudev_la_SOURCES =\
	libudev.sym \
	libudev-private.h \
	libudev.c \
	libudev-list.c \
	libudev-util.c \
	libudev-device.c \
	libudev-enumerate.c \
	libudev-monitor.c \
	libudev-queue.c \
	libudev-hwdb-def.h \
	libudev-hwdb.c

libudev_la_CFLAGS = \
	$(AM_CFLAGS) \
	-fvisibility=hidden

noinst_HEADERS = \
	libudev-hwdb-def.h \
	libudev-private.h

include_HEADERS = \
	libudev.h

libudev_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-version-info $(LIBUDEV_CURRENT):$(LIBUDEV_REVISION):$(LIBUDEV_AGE) \
	-Wl,--version-script=$(top_srcdir)/src/libudev/libudev.sym

libudev_la_LIBADD = \
	$(top_builddir)/src/shared/libudev_shared.la \
	$(SELINUX_LIBS)

libudev_private_la_SOURCES = \
	$(libudev_la_SOURCES) \
	libudev-device-private.c

libudev_private_la_CFLAGS = \
	$(AM_CFLAGS) \
	-fvisibility=default

noinst_LTLIBRARIES = \
	libudev-private.la

pkgconfiglib_DATA = \
	libudev.pc

EXTRA_DIST = \
	libudev.pc.in \
	libudev.sym

CLEANFILES = \
	libudev.pc

install-exec-hook:
	libname=libudev.so && $(move-to-rootlibdir)

uninstall-hook:
	rm -f $(DESTDIR)$(rootlibdir)/libudev.so*
