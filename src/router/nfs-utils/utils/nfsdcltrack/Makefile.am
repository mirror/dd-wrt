## Process this file with automake to produce Makefile.in

# These binaries go in /sbin (not /usr/sbin) as the kernel "knows" the
# /sbin name.  If /sbin is a symlink, CONFIG_SBIN_OVERRIDE can be
# disabled to install in /usr/sbin anyway.
# Note that we don't use "if CONFIG_SBIN_OVERRIDE" as that
# causes autotools to notice the override and disable it.
@CONFIG_SBIN_OVERRIDE_TRUE@sbindir = /sbin

man8_MANS	= nfsdcltrack.man
EXTRA_DIST	= $(man8_MANS)

AM_CFLAGS	+= -D_LARGEFILE64_SOURCE
sbin_PROGRAMS	= nfsdcltrack

noinst_HEADERS	= sqlite.h

nfsdcltrack_SOURCES = nfsdcltrack.c sqlite.c
nfsdcltrack_LDADD = ../../support/nfs/libnfs.la $(LIBSQLITE) $(LIBCAP)

MAINTAINERCLEANFILES = Makefile.in

