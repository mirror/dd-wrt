AM_CFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/smbuseradd -I$(top_srcdir)/smbshareadd $(GLIB_CFLAGS) $(LIBNL_CFLAGS) -fno-common -DMULTICALL
LIBS = $(GLIB_LIBS) $(LIBNL_LIBS)
smbd_multicall_LDADD = $(top_builddir)/lib/libusmbdtools.la

sbin_PROGRAMS = smbd_multicall

smbd_multicall_SOURCES = worker.c ipc.c rpc.c rpc_srvsvc.c rpc_wkssvc.c usmbd.c multicall.c ../smbuseradd/md4_hash.c ../smbuseradd/user_admin.c ../smbuseradd/smbuseradd.c ../smbshareadd/share_admin.c ../smbshareadd/smbshareadd.c
