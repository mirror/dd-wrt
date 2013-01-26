
SUBDIRS = generated . test

lib_LTLIBRARIES = libqmi-glib.la

libqmi_glib_la_CPPFLAGS = \
	$(LIBQMI_GLIB_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	-I$(top_srcdir)/libqmi-glib \
	-I$(top_srcdir)/libqmi-glib/generated \
	-I$(top_builddir)/libqmi-glib/generated \
	-DLIBQMI_GLIB_COMPILATION \
	-DG_LOG_DOMAIN=\"Qmi\"

libqmi_glib_la_SOURCES = \
	libqmi-glib.h \
	qmi-errors.h \
	qmi-enums-wds.h qmi-enums-wds.c \
	qmi-enums-dms.h qmi-flags64-dms.h \
	qmi-enums-nas.h qmi-flags64-nas.h\
	qmi-enums-wms.h \
	qmi-enums-pds.h \
	qmi-enums.h qmi-enums-private.h \
	qmi-utils.h qmi-utils.c \
	qmi-message.h qmi-message.c \
	qmi-device.h qmi-device.c \
	qmi-client.h qmi-client.c

libqmi_glib_la_LIBADD = \
	${top_builddir}/libqmi-glib/generated/libqmi-glib-generated.la \
	$(LIBQMI_GLIB_LIBS)

includedir = @includedir@/libqmi-glib
include_HEADERS = \
	libqmi-glib.h \
	qmi-errors.h \
	qmi-enums.h \
	qmi-enums-private.h \
	qmi-enums-dms.h qmi-flags64-dms.h \
	qmi-enums-wds.h \
	qmi-enums-nas.h qmi-flags64-nas.h \
	qmi-enums-wms.h \
	qmi-enums-pds.h \
	qmi-utils.h \
	qmi-message.h \
	qmi-device.h \
	qmi-client.h
