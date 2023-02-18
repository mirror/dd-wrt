## Process this file with automake to produce Makefile.in

OPTDIRS =

if CONFIG_NFSV4
OPTDIRS += idmapd
OPTDIRS += nfsidmap
endif

if CONFIG_NFSV4SERVER
OPTDIRS += exportd
endif

if CONFIG_NFSV41
OPTDIRS += blkmapd
endif

if CONFIG_GSS
OPTDIRS += gssd
endif

if CONFIG_MOUNT
OPTDIRS += mount
endif

if CONFIG_NFSDCLD
OPTDIRS += nfsdcld
endif

if CONFIG_NFSDCLTRACK
OPTDIRS += nfsdcltrack
endif

if CONFIG_JUNCTION
OPTDIRS += nfsref
endif

SUBDIRS = \
	exportfs \
	mountd \
	nfsd \
	nfsstat \
	showmount \
	statd \
	$(OPTDIRS)

MAINTAINERCLEANFILES = Makefile.in
