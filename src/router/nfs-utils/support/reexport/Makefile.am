## Process this file with automake to produce Makefile.in

noinst_LIBRARIES = libreexport.a
libreexport_a_SOURCES = reexport.c

sbin_PROGRAMS	= fsidd

fsidd_SOURCES = fsidd.c backend_sqlite.c

fsidd_LDADD = ../../support/misc/libmisc.a \
	      ../../support/nfs/libnfs.la \
	       $(LIBPTHREAD) $(LIBEVENT) $(LIBSQLITE) \
	       $(OPTLIBS)

fsidd_CPPFLAGS = $(AM_CPPFLAGS) $(CPPFLAGS) \
		  -I$(top_builddir)/support/include

MAINTAINERCLEANFILES = Makefile.in
