AM_CFLAGS = -I$(top_srcdir)/include $(GLIB_CFLAGS) $(LIBNL_CFLAGS) -fno-common
LIBS = $(GLIB_LIBS)
smbshareadd_LDADD = $(top_builddir)/lib/libsmbdtools.la

sbin_PROGRAMS = smbshareadd

smbshareadd_SOURCES = share_admin.c smbshareadd.c
