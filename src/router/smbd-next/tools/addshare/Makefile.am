AM_CFLAGS = -I$(top_srcdir)/include $(GLIB_CFLAGS) $(LIBNL_CFLAGS) -fno-common
ksmbd_addshare_LDADD = $(top_builddir)/lib/libksmbdtools.la

sbin_PROGRAMS = ksmbd.addshare

ksmbd_addshare_SOURCES = share_admin.c addshare.c
