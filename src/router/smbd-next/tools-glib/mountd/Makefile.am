AM_CFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/adduser -I$(top_srcdir)/addshare  $(GLIB_CFLAGS) $(LIBNL_CFLAGS) -fno-common -DMULTICALL
LIBS = $(GLIB_LIBS) $(LIBNL_LIBS) $(LIBKRB5_LIBS)

smbd_multicall_LDADD = $(top_builddir)/lib/libksmbdtools.a

sbin_PROGRAMS = smbd_multicall

smbd_multicall_SOURCES = worker.c ipc.c rpc.c rpc_srvsvc.c rpc_wkssvc.c mountd.c \
		       smbacl.c rpc_samr.c rpc_lsarpc.c multicall.c ../adduser/md4_hash.c ../adduser/user_admin.c ../adduser/adduser.c ../addshare/share_admin.c ../addshare/addshare.c
