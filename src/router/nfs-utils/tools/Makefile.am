## Process this file with automake to produce Makefile.in

OPTDIRS =

if CONFIG_RPCGEN
OPTDIRS += rpcgen
endif

OPTDIRS += nfsconf

if CONFIG_NFSDCLD
OPTDIRS += nfsdclddb
endif

SUBDIRS = locktest rpcdebug nlmtest mountstats nfs-iostat nfsdclnts $(OPTDIRS)

MAINTAINERCLEANFILES = Makefile.in
