## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include -I${srcdir}/../libs -I$(srcdir)/includes

dynamicpreprocessordir = ${libdir}/snort_dynamicpreprocessor

dynamicpreprocessor_LTLIBRARIES = libsf_dce2_preproc.la

libsf_dce2_preproc_la_LDFLAGS = -export-dynamic -module @XCCFLAGS@
if SO_WITH_STATIC_LIB
libsf_dce2_preproc_la_LIBADD = ../libsf_dynamic_preproc.la
if BUILD_SNORT_RELOAD
libsf_dce2_preproc_la_LIBADD += ../libsf_dynamic_utils.la
endif
else
nodist_libsf_dce2_preproc_la_SOURCES = \
../include/sf_dynamic_preproc_lib.c \
../include/sf_ip.c \
../include/sfrt.c \
../include/sfrt_dir.c \
../include/sfPolicyUserData.c
if BUILD_SNORT_RELOAD
nodist_libsf_dce2_preproc_la_SOURCES += ../include/appdata_adjuster.c ../include/sfxhash.c ../include/sfhashfcn.c ../include/sfmemcap.c ../include/sfprimetable.c ../include/reg_test.h ../include/reg_test.c
endif
endif

libsf_dce2_preproc_la_SOURCES = \
includes/dcerpc.h \
includes/smb.h \
dce2_debug.c \
dce2_debug.h \
dce2_utils.c \
dce2_utils.h \
dce2_list.c \
dce2_list.h \
dce2_memory.c \
dce2_memory.h \
dce2_stats.c \
dce2_stats.h \
dce2_event.c \
dce2_event.h \
dce2_config.c \
dce2_config.h \
dce2_roptions.c \
dce2_roptions.h \
dce2_session.h \
spp_dce2.c \
spp_dce2.h \
snort_dce2.c \
snort_dce2.h \
dce2_smb.c \
dce2_smb.h \
dce2_smb2.c \
dce2_smb2.h \
dce2_tcp.c \
dce2_tcp.h \
dce2_co.c \
dce2_co.h \
dce2_udp.c \
dce2_udp.h \
dce2_cl.c \
dce2_cl.h \
dce2_http.c \
dce2_http.h \
dce2_paf.c \
dce2_paf.h

if BUILD_BUFFER_DUMP
libsf_dce2_preproc_la_SOURCES += \
dcerpc2_buffer_dump.c \
dcerpc2_buffer_dump.h
endif


EXTRA_DIST = \
sf_dce2.vcxproj \
sf_dce2.dsp

all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-dynamicpreprocessorLTLIBRARIES

