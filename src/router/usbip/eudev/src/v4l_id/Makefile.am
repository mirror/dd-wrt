ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

AM_CPPFLAGS = \
	-I $(top_srcdir)/src/shared

udevlibexec_PROGRAMS = \
	v4l_id

v4l_id_SOURCES = \
	v4l_id.c

v4l_id_LDADD = \
	$(top_builddir)/src/libudev/libudev-private.la \
	$(top_builddir)/src/udev/libudev-core.la
