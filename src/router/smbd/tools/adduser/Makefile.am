AM_CFLAGS = -I$(top_srcdir)/include $(GLIB_CFLAGS) $(LIBNL_CFLAGS) -fno-common
LIBS = $(GLIB_LIBS)
smbuseradd_LDADD = $(top_builddir)/lib/libksmbdtools.la

sbin_PROGRAMS = ksmbd.adduser

smbuseradd_SOURCES = md4_hash.c user_admin.c useradd.c
