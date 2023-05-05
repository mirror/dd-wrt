## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include -I${srcdir}/../libs

dynamicpreprocessordir = ${libdir}/snort_dynamicpreprocessor

dynamicpreprocessor_LTLIBRARIES = libsf_s7commplus_preproc.la

libsf_s7commplus_preproc_la_LDFLAGS = -export-dynamic -module @XCCFLAGS@
if SO_WITH_STATIC_LIB
libsf_s7commplus_preproc_la_LIBADD = ../libsf_dynamic_preproc.la
else
nodist_libsf_s7commplus_preproc_la_SOURCES = \
../include/sf_dynamic_preproc_lib.c \
../include/sfPolicyUserData.c
endif

libsf_s7commplus_preproc_la_SOURCES = \
spp_s7comm.c \
spp_s7comm.h \
s7comm_decode.c \
s7comm_decode.h \
s7comm_roptions.c \
s7comm_roptions.h \
s7comm_paf.c \
s7comm_paf.h

#EXTRA_DIST = \
#sf_s7comm.dsp

all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-dynamicpreprocessorLTLIBRARIES

