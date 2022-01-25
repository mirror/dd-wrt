AM_CFLAGS = -I$(top_srcdir)/include $(GLIB_CFLAGS) $(LIBNL_CFLAGS) -fno-common
LIBS = $(GLIB_LIBS)
ksmbd_adduser_LDADD = $(top_builddir)/lib/libksmbdtools.a

sbin_PROGRAMS = ksmbd.adduser

ksmbd_adduser_SOURCES = md4_hash.c user_admin.c adduser.c md4_hash.h user_admin.h
