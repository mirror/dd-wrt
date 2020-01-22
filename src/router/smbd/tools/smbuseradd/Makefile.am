AM_CFLAGS = -I$(top_srcdir)/include $(GLIB_CFLAGS) $(LIBNL_CFLAGS) -fno-common
LIBS = $(GLIB_LIBS)
smbuseradd_LDADD = $(top_builddir)/lib/libusmbdtools.la

sbin_PROGRAMS = smbuseradd

smbuseradd_SOURCES = md4_hash.c user_admin.c smbuseradd.c
