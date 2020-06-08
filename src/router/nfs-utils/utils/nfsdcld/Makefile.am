## Process this file with automake to produce Makefile.in

man8_MANS	= nfsdcld.man
EXTRA_DIST	= $(man8_MANS)

AM_CFLAGS	+= -D_LARGEFILE64_SOURCE
sbin_PROGRAMS	= nfsdcld

nfsdcld_SOURCES = nfsdcld.c sqlite.c legacy.c
nfsdcld_LDADD = ../../support/nfs/libnfs.la $(LIBEVENT) $(LIBSQLITE) $(LIBCAP)

noinst_HEADERS	= sqlite.h cld-internal.h legacy.h

MAINTAINERCLEANFILES = Makefile.in

