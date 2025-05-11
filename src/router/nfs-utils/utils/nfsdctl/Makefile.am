## Process this file with automake to produce Makefile.in

man8_MANS       = nfsdctl.8
EXTRA_DIST      = $(man8_MANS)

sbin_PROGRAMS	= nfsdctl

noinst_HEADERS = nfsdctl.h
nfsdctl_SOURCES = nfsdctl.c
nfsdctl_CFLAGS = $(LIBNL3_CFLAGS) $(LIBNLGENL3_CFLAGS) $(LIBREADLINE_CFLAGS)
nfsdctl_LDADD = ../../support/nfs/libnfs.la $(LIBNL3_LIBS) $(LIBNLGENL3_LIBS) $(LIBREADLINE_LIBS)

MAINTAINERCLEANFILES = Makefile.in
