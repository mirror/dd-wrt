## Process this file with automake to produce Makefile.in

man8_MANS = nfsidmap.man
sbin_PROGRAMS	= nfsidmap

AM_CPPFLAGS += -I ../../support/nfsidmap

nfsidmap_SOURCES = nfsidmap.c
nfsidmap_LDADD = -lkeyutils \
		 ../../support/nfs/libnfs.la \
		 ../../support/nfsidmap/libnfsidmap.la

MAINTAINERCLEANFILES = Makefile.in
EXTRA_DIST = id_resolver.conf $(man8_MANS)
