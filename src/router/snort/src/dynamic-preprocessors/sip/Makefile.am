## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include -I${srcdir}/../libs -I$(srcdir)/includes

dynamicpreprocessordir = ${libdir}/snort_dynamicpreprocessor

dynamicpreprocessor_LTLIBRARIES = libsf_sip_preproc.la

libsf_sip_preproc_la_LDFLAGS = -export-dynamic -module @XCCFLAGS@
if SO_WITH_STATIC_LIB
libsf_sip_preproc_la_LIBADD = ../libsf_dynamic_preproc.la
if BUILD_SNORT_RELOAD
libsf_sip_preproc_la_LIBADD += ../libsf_dynamic_utils.la
endif
else
nodist_libsf_sip_preproc_la_SOURCES = \
../include/sf_dynamic_preproc_lib.c \
../include/sf_ip.c \
../include/sfPolicyUserData.c
if BUILD_SNORT_RELOAD
nodist_libsf_sip_preproc_la_SOURCES += ../include/appdata_adjuster.c ../include/sfxhash.c ../include/sfhashfcn.c ../include/sfmemcap.c ../include/sfprimetable.c ../include/reg_test.h ../include/reg_test.c
endif
endif

libsf_sip_preproc_la_SOURCES = \
spp_sip.c \
spp_sip.h \
sip_config.c \
sip_config.h \
sip_parser.c \
sip_parser.h \
sip_dialog.c \
sip_dialog.h \
sip_roptions.c \
sip_roptions.h \
sip_utils.c \
sip_utils.h \
sip_debug.h \
sip_paf.c \
sip_paf.h

if BUILD_BUFFER_DUMP
libsf_sip_preproc_la_SOURCES += \
sip_buffer_dump.c \
sip_buffer_dump.h
endif

EXTRA_DIST = \
sf_sip.vcxproj \
sf_sip.dsp


all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-dynamicpreprocessorLTLIBRARIES
