## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include -I${srcdir}/../ssl_common -I${srcdir}/../libs -I${srcdir}/../libs/ssl_common

dynamicpreprocessordir = ${libdir}/snort_dynamicpreprocessor

dynamicpreprocessor_LTLIBRARIES = libsf_ssl_preproc.la

libsf_ssl_preproc_la_LDFLAGS = -export-dynamic -module @XCCFLAGS@
if SO_WITH_STATIC_LIB
libsf_ssl_preproc_la_LIBADD = ../libsf_dynamic_preproc.la
else
nodist_libsf_ssl_preproc_la_SOURCES = \
../include/sf_dynamic_preproc_lib.c \
../include/sfPolicyUserData.c \
../ssl_common/ssl.c \
../ssl_common/ssl_config.c \
../ssl_common/ssl_inspect.c \
../libs/sfparser.c

endif

libsf_ssl_preproc_la_SOURCES = \
ssl_setup.c \
ssl_setup.h

if BUILD_BUFFER_DUMP
libsf_ssl_preproc_la_SOURCES += \
ssl_buffer_dump.c \
ssl_buffer_dump.h
endif

EXTRA_DIST = \
sf_ssl.vcxproj \
sf_ssl.dsp

all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-dynamicpreprocessorLTLIBRARIES

