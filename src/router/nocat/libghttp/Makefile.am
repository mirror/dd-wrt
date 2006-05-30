## Process this file with automake to produce Makefile.in

EXTRA_DIST = libghttp.spec.in doc/ghttp.html

dist-hook: libghttp.spec
	cp libghttp.spec $(distdir)

lib_LTLIBRARIES = libghttp.la

libghttp_la_SOURCES = \
	ghttp.c \
	http_date.c \
	http_hdrs.c \
	http_req.c \
	http_resp.c \
	http_trans.c \
	http_uri.c \
	http_base64.c

include_HEADERS = \
	ghttp.h \
	ghttp_constants.h

EXTRA_DIST += \
	http_date.h \
	http_global.h \
	http_hdrs.h \
	http_req.h \
	http_resp.h \
	http_trans.h \
	http_uri.h \
	http_base64.h \
	ghttpConf.sh.in

libghttp_la_LDFLAGS = -version-info 1:0:0

## We create ghttpConf.sh here and not from configure because we want
## to get the paths expanded correctly.  Macros like srcdir are given
## the value NONE in configure if the user doesn't specify them (this
## is an autoconf feature, not a bug).

confexecdir=$(libdir)
confexec_DATA = ghttpConf.sh

CLEANFILES=ghttpConf.sh

confexecdir=$(libdir)
confexec_DATA = ghttpConf.sh

ghttpConf.sh: ghttpConf.sh.in Makefile
## Use sed and then mv to avoid problems if the user interrupts.
	sed -e 's?\@GHTTP_LIBDIR\@?$(GHTTP_LIBDIR)?g' \
            -e 's?\@GHTTP_INCLUDEDIR\@?$(GHTTP_INCLUDEDIR)?g' \
            -e 's?\@GHTTP_LIBS\@?$(GHTTP_LIBS)?g' \
            -e 's?\@VERSION\@?$(VERSION)?g' \
              < $(srcdir)/ghttpConf.sh.in > ghttpConf.tmp \
          && mv ghttpConf.tmp ghttpConf.sh
