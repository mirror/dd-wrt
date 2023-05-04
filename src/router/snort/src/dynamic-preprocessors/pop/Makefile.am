## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include -I${srcdir}/../ssl_common -I${srcdir}/../libs

dynamicpreprocessordir = ${libdir}/snort_dynamicpreprocessor

dynamicpreprocessor_LTLIBRARIES = libsf_pop_preproc.la

libsf_pop_preproc_la_LDFLAGS = -export-dynamic -module @XCCFLAGS@
if SO_WITH_STATIC_LIB
libsf_pop_preproc_la_LIBADD = ../libsf_dynamic_preproc.la
else
nodist_libsf_pop_preproc_la_SOURCES = \
../include/sf_dynamic_preproc_lib.c \
../include/mempool.c \
../include/sf_sdlist.c \
../include/util_unfold.c \
../include/sf_base64decode.c \
../include/sf_email_attach_decode.c \
../include/sfPolicyUserData.c \
../ssl_common/ssl.c \
../ssl_common/ssl_config.c \
../ssl_common/ssl_inspect.c \
../libs/sfparser.c
endif

libsf_pop_preproc_la_SOURCES = \
pop_config.c \
pop_config.h \
pop_log.c \
pop_log.h \
pop_paf.c \
pop_paf.h \
pop_util.c \
pop_util.h \
snort_pop.c \
snort_pop.h \
spp_pop.c \
spp_pop.h

if BUILD_BUFFER_DUMP
libsf_pop_preproc_la_SOURCES += \
pop_buffer_dump.c \
pop_buffer_dump.h
endif


EXTRA_DIST = \
sf_pop.vcxproj \
sf_pop.dsp

all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-dynamicpreprocessorLTLIBRARIES

