AM_CFLAGS = -I$(top_srcdir)/include $(GLIB_CFLAGS) $(LIBNL_CFLAGS) -fno-common
LIBS = $(GLIB_LIBS)
smbshareadd_LDADD = $(top_builddir)/lib/libksmbdtools.la

sbin_PROGRAMS = ksmbd.addshare

smbshareadd_SOURCES = share_admin.c addshare.c
