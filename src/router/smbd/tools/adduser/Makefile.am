AM_CFLAGS = -I$(top_srcdir)/include $(GLIB_CFLAGS) $(LIBNL_CFLAGS) -fno-common
ksmbd_adduser_LDADD = $(top_builddir)/lib/libksmbdtools.la

sbin_PROGRAMS = ksmbd.adduser

ksmbd_adduser_SOURCES = md4_hash.c user_admin.c adduser.c
