## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include -I${srcdir}/../libs -I$(srcdir)/includes

dynamicpreprocessordir = ${libdir}/snort_dynamicpreprocessor

dynamicpreprocessor_LTLIBRARIES = libsf_gtp_preproc.la

libsf_gtp_preproc_la_LDFLAGS = -export-dynamic -module @XCCFLAGS@
if SO_WITH_STATIC_LIB
libsf_gtp_preproc_la_LIBADD = ../libsf_dynamic_preproc.la
else
nodist_libsf_gtp_preproc_la_SOURCES = \
../include/sf_dynamic_preproc_lib.c \
../include/sfPolicyUserData.c
endif

libsf_gtp_preproc_la_SOURCES = \
spp_gtp.c \
spp_gtp.h \
gtp_config.c \
gtp_config.h \
gtp_parser.c \
gtp_parser.h \
gtp_roptions.c \
gtp_roptions.h \
gtp_debug.h

if BUILD_BUFFER_DUMP
libsf_gtp_preproc_la_SOURCES += \
gtp_buffer_dump.h \
gtp_buffer_dump.c
endif

EXTRA_DIST = \
sf_gtp.vcxproj \
sf_gtp.dsp


all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-dynamicpreprocessorLTLIBRARIES
