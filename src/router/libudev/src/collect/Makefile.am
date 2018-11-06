ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

AM_CPPFLAGS = \
	-DUDEV_ROOT_RUN=\"$(rootrundir)\" \
	-I $(top_srcdir)/src/shared \
	-I $(top_srcdir)/src/libudev

udevlibexec_PROGRAMS = \
	collect

collect_SOURCES = \
	collect.c

collect_LDADD = \
	$(top_builddir)/src/libudev/libudev-private.la \
	$(top_builddir)/src/udev/libudev-core.la
