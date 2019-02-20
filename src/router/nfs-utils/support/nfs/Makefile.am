## Process this file with automake to produce Makefile.in

noinst_LIBRARIES = 
noinst_LTLIBRARIES = libnfs.la libnfsconf.la

libnfs_la_SOURCES = exports.c rmtab.c xio.c rpcmisc.c rpcdispatch.c \
		   xcommon.c wildmat.c mydaemon.c \
		   rpc_socket.c getport.c \
		   svc_socket.c cacheio.c closeall.c nfs_mntent.c \
		   svc_create.c atomicio.c strlcat.c strlcpy.c
libnfs_la_LIBADD = libnfsconf.la

libnfsconf_la_SOURCES = conffile.c xlog.c

MAINTAINERCLEANFILES = Makefile.in

