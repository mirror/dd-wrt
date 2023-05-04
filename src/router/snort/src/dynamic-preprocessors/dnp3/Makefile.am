# $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include -I${srcdir}/../libs

dynamicpreprocessordir = ${libdir}/snort_dynamicpreprocessor

dynamicpreprocessor_LTLIBRARIES = libsf_dnp3_preproc.la

libsf_dnp3_preproc_la_LDFLAGS = -export-dynamic -module @XCCFLAGS@
if SO_WITH_STATIC_LIB
libsf_dnp3_preproc_la_LIBADD = ../libsf_dynamic_preproc.la
if BUILD_SNORT_RELOAD
libsf_dnp3_preproc_la_LIBADD += ../libsf_dynamic_utils.la
endif
else
nodist_libsf_dnp3_preproc_la_SOURCES = \
../include/sf_dynamic_preproc_lib.c \
../include/sfPolicyUserData.c \
../include/mempool.c \
../include/sf_sdlist.c
if BUILD_SNORT_RELOAD
nodist_libsf_dnp3_preproc_la_SOURCES += ../include/appdata_adjuster.c ../include/sfxhash.c ../include/sfhashfcn.c ../include/sfmemcap.c ../include/sfprimetable.c ../include/reg_test.h ../include/reg_test.c
endif
endif

libsf_dnp3_preproc_la_SOURCES = \
spp_dnp3.c \
spp_dnp3.h \
dnp3_paf.c \
dnp3_paf.h \
dnp3_reassembly.c \
dnp3_reassembly.h \
dnp3_roptions.c \
dnp3_roptions.h \
dnp3_map.c \
dnp3_map.h

if BUILD_BUFFER_DUMP
libsf_dnp3_preproc_la_SOURCES += \
dnp3_buffer_dump.c \
dnp3_buffer_dump.h
endif


EXTRA_DIST = \
sf_dnp3.vcxproj \
sf_dnp3.dsp

all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-dynamicpreprocessorLTLIBRARIES

