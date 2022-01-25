AM_CFLAGS = -I$(top_srcdir)/include $(GLIB_CFLAGS) -fno-common -g
LIBS = $(GLIB_LIBS)

lib_LTLIBRARIES = libksmbdtools.la
libksmbdtools_la_LIBADD = -lresolv
libksmbdtools_la_SOURCES = management/tree_conn.c \
			   management/user.c \
			   management/share.c \
			   management/session.c \
			   config_parser.c \
			   ksmbdtools.c

if HAVE_LIBKRB5
libksmbdtools_la_SOURCES += management/spnego.c \
			   asn1.c \
			   management/spnego_krb5.c
endif
