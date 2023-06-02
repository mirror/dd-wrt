## Process this file with automake to produce Makefile.in

OPTDIRS =

if CONFIG_RPCGEN
OPTDIRS += rpcgen
endif

OPTDIRS += nfsconf

if CONFIG_NFSDCLD
OPTDIRS += nfsdclddb
endif

if CONFIG_NFSRAHEAD
OPTDIRS += nfsrahead
endif

SUBDIRS = locktest rpcdebug nlmtest mountstats nfs-iostat rpcctl nfsdclnts $(OPTDIRS)

MAINTAINERCLEANFILES = Makefile.in
