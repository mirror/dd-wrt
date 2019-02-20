## Process this file with automake to produce Makefile.in

man8_MANS	= nfsconf.man
EXTRA_DIST	= $(man8_MANS)

sbin_PROGRAMS = nfsconf

nfsconf_SOURCES = nfsconfcli.c
nfsconf_LDADD = ../../support/nfs/libnfsconf.la

MAINTAINERCLEANFILES = Makefile.in
