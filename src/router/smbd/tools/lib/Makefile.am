AM_CFLAGS = -I$(top_srcdir)/include $(GLIB_CFLAGS) -fno-common
LIBS = $(GLIB_LIBS)

lib_LTLIBRARIES = libusmbdtools.la
libusmbdtools_la_LIBADD = -lresolv
libusmbdtools_la_SOURCES = management/tree_conn.c \
			   management/user.c \
			   management/share.c \
			   management/session.c \
			   config_parser.c \
			   usmbdtools.c
