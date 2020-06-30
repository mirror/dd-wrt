## Process this file with automake to produce Makefile.in

OPTDIRS =

if CONFIG_RPCGEN
OPTDIRS += rpcgen
endif

OPTDIRS += nfsconf

if CONFIG_NFSDCLD
OPTDIRS += clddb-tool
endif

SUBDIRS = locktest rpcdebug nlmtest mountstats nfs-iostat $(OPTDIRS)

MAINTAINERCLEANFILES = Makefile.in
