AM_CFLAGS = -I$(top_srcdir)/include $(GLIB_CFLAGS) -fno-common
LIBS = $(GLIB_LIBS)

noinst_LIBRARIES = libksmbdtools.a
libksmbdtools_a_SOURCES = management/tree_conn.c \
			   management/user.c \
			   management/share.c \
			   management/session.c \
			   config_parser.c \
			   ksmbdtools.c

if HAVE_LIBKRB5
libksmbdtools_a_SOURCES += management/spnego.c \
			   asn1.c \
			   management/spnego_krb5.c \
			   management/spnego_mech.h
endif
