AM_CFLAGS = -I$(top_srcdir)/include $(GLIB_CFLAGS) $(LIBNL_CFLAGS) -fno-common
LIBS = $(GLIB_LIBS) $(LIBNL_LIBS)
usmbd_LDADD = $(top_builddir)/lib/libsmbdtools.la

sbin_PROGRAMS = usmbd

usmbd_SOURCES = worker.c ipc.c rpc.c rpc_srvsvc.c rpc_wkssvc.c usmbd.c
