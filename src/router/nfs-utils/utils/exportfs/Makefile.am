## Process this file with automake to produce Makefile.in

man5_MANS	= exports.man
man7_MANS	= nfsd.man
man8_MANS	= exportfs.man

EXTRA_DIST	= $(man5_MANS) $(man7_MANS) $(man8_MANS)
sbin_PROGRAMS	= exportfs
exportfs_SOURCES = exportfs.c
exportfs_LDADD = ../../support/export/libexport.a \
	       	 ../../support/nfs/libnfs.la \
		 ../../support/misc/libmisc.a \
		 ../../support/reexport/libreexport.a \
		 $(LIBWRAP) $(LIBNSL) $(LIBPTHREAD)

exportfs_CPPFLAGS = $(AM_CPPFLAGS) $(CPPFLAGS) -I$(top_srcdir)/support/reexport

MAINTAINERCLEANFILES = Makefile.in
