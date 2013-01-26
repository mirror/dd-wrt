
SUBDIRS = . test

bin_PROGRAMS = qmicli

qmicli_CPPFLAGS = \
	$(QMICLI_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libqmi-glib \
	-I$(top_srcdir)/libqmi-glib/generated \
	-I$(top_builddir)/libqmi-glib/generated

qmicli_SOURCES = \
	qmicli.c \
	qmicli.h \
	qmicli-helpers.c \
	qmicli-helpers.h \
	qmicli-dms.c \
	qmicli-wds.c \
	qmicli-nas.c

qmicli_LDADD = \
	$(QMICLI_LIBS) \
	$(top_builddir)/libqmi-glib/libqmi-glib.la
