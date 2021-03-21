AM_CFLAGS = -I$(top_srcdir)/include $(GLIB_CFLAGS) $(LIBNL_CFLAGS) -fno-common
LIBS = $(GLIB_LIBS) $(LIBNL_LIBS) $(LIBKRB5_LIBS)
ksmbd_mountd_LDADD = $(top_builddir)/lib/libksmbdtools.a

sbin_PROGRAMS = ksmbd.mountd

ksmbd_mountd_SOURCES = worker.c ipc.c rpc.c rpc_srvsvc.c rpc_wkssvc.c mountd.c \
		       smbacl.c rpc_samr.c rpc_lsarpc.c
