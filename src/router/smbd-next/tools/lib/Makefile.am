AM_CFLAGS = -I$(top_srcdir)/include $(GLIB_CFLAGS) -fno-common
LIBS = $(GLIB_LIBS)

lib_LTLIBRARIES = libksmbdtools.la
libksmbdtools_la_LIBADD = -lresolv
libksmbdtools_la_SOURCES = management/tree_conn.c \
			   management/user.c \
			   management/share.c \
			   management/session.c \
			   config_parser.c \
			   ksmbdtools.c
