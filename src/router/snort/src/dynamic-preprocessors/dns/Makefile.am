## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include -I${srcdir}/../libs

dynamicpreprocessordir = ${libdir}/snort_dynamicpreprocessor

dynamicpreprocessor_LTLIBRARIES = libsf_dns_preproc.la

libsf_dns_preproc_la_LDFLAGS = -export-dynamic -module @XCCFLAGS@
if SO_WITH_STATIC_LIB
libsf_dns_preproc_la_LIBADD = ../libsf_dynamic_preproc.la
else
nodist_libsf_dns_preproc_la_SOURCES = \
../include/sf_dynamic_preproc_lib.c \
../include/sfPolicyUserData.c
endif

libsf_dns_preproc_la_SOURCES = \
spp_dns.c \
spp_dns.h

if BUILD_BUFFER_DUMP
libsf_dns_preproc_la_SOURCES += \
dns_buffer_dump.c \
dns_buffer_dump.h
endif

EXTRA_DIST = \
sf_dns.vcxproj \
sf_dns.dsp

all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-dynamicpreprocessorLTLIBRARIES

