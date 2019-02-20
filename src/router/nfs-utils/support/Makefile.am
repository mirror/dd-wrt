## Process this file with automake to produce Makefile.in

OPTDIRS =

if CONFIG_NFSV4
OPTDIRS += nfsidmap
endif

if CONFIG_JUNCTION
OPTDIRS += junction
endif

SUBDIRS = export include misc nfs nsm $(OPTDIRS)

MAINTAINERCLEANFILES = Makefile.in

