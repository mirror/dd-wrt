ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

AM_CPPFLAGS = \
	-I $(top_srcdir)/src/shared \
	-I $(top_srcdir)/src/libudev

udevlibexec_PROGRAMS = \
	cdrom_id

cdrom_id_SOURCES = \
	cdrom_id.c

cdrom_id_LDADD = \
	$(top_builddir)/src/libudev/libudev-private.la \
	$(top_builddir)/src/udev/libudev-core.la
