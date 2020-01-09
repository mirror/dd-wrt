include_HEADERS = libintl.h
lib_LTLIBRARIES = libintl.la

libintl_la_SOURCES = \
	libintl.h \
	libintl.c

EXTRA_DIST = \
	autogen.sh

MAINTAINERCLEANFILES = \
	configure \
	Makefile.in \
	aclocal.m4 \
	config.guess \
	config.sub \
	config.h.in \
	ltmain.sh \
	missing \
	depcomp \
	install-sh \
	*~ \
	$(DIST_ARCHIVES)

maintainer-clean-local:
	-chmod -R a+rw $(distdir)
	-rm -fr $(distdir)

