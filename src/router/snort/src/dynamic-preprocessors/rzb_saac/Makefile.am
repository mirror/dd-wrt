## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I${top_builddir}/src/dynamic-preprocessors/include -I${top_srcdir}/src/dynamic-preprocessors/libs

pkglibdir = ${exec_prefix}/lib/snort_dynamicpreprocessor

pkglib_LTLIBRARIES = sf_rzb_saac_preproc.la

sf_rzb_saac_preproc_la_LDFLAGS = -shared -export-dynamic -module @XCCFLAGS@
sf_rzb_saac_preproc_la_LIBADD = @RZBLIBS@
sf_rzb_saac_preproc_la_CFLAGS = @RZBCFLAGS@ -Werror

nodist_sf_rzb_saac_preproc_la_SOURCES = \
${top_builddir}/src/dynamic-preprocessors/include/sf_ip.c \
${top_builddir}/src/dynamic-preprocessors/include/sf_ip.h \
${top_builddir}/src/dynamic-preprocessors/include/sf_dynamic_preproc_lib.c \
${top_builddir}/src/dynamic-preprocessors/include/sf_dynamic_preproc_lib.h \
${top_builddir}/src/dynamic-preprocessors/include/sfPolicyUserData.c \
${top_builddir}/src/dynamic-preprocessors/include/sfPolicyUserData.h \
${top_srcdir}/src/dynamic-preprocessors/libs/sfparser.c \
${top_srcdir}/src/dynamic-preprocessors/libs/sfcommon.h

sf_rzb_saac_preproc_la_SOURCES = \
rzb_debug.c \
rzb_debug.h \
rzb_http-client.c \
rzb_http-client.h \
rzb_http.h \
rzb_http-fileinfo.c \
rzb_http-fileinfo.h \
rzb_http-server.c \
rzb_http-server.h \
rzb_smtp-collector.c \
rzb_smtp-collector.h \
spp_rzb-saac.c \
sf_preproc_info.h

#EXTRA_DIST = \
#sf_ssl.dsp

all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-pkglibLTLIBRARIES

