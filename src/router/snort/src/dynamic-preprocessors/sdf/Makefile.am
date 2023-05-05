## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include -I${srcdir}/../libs

dynamicpreprocessordir = ${libdir}/snort_dynamicpreprocessor

dynamicpreprocessor_LTLIBRARIES = libsf_sdf_preproc.la

libsf_sdf_preproc_la_LDFLAGS = -export-dynamic -module @XCCFLAGS@
if SO_WITH_STATIC_LIB
libsf_sdf_preproc_la_LIBADD = ../libsf_dynamic_preproc.la
else
nodist_libsf_sdf_preproc_la_SOURCES = \
../include/sf_dynamic_preproc_lib.c \
../include/sfPolicyUserData.c
endif

libsf_sdf_preproc_la_SOURCES = \
spp_sdf.c \
spp_sdf.h \
sdf_pattern_match.c \
sdf_pattern_match.h \
sdf_credit_card.c \
sdf_credit_card.h \
sdf_us_ssn.c \
sdf_us_ssn.h \
sdf_detection_option.c \
sdf_detection_option.h

EXTRA_DIST = \
sf_sdf.vcxproj \
sf_sdf.dsp

all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-dynamicpreprocessorLTLIBRARIES

