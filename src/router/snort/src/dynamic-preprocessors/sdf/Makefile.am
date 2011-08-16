## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include

libdir = ${exec_prefix}/lib/snort_dynamicpreprocessor

lib_LTLIBRARIES = libsf_sdf_preproc.la

libsf_sdf_preproc_la_LDFLAGS = -shared -export-dynamic -module @XCCFLAGS@

BUILT_SOURCES = \
sf_dynamic_preproc_lib.c \
sfPolicyUserData.c

nodist_libsf_sdf_preproc_la_SOURCES = \
sf_dynamic_preproc_lib.c \
sfPolicyUserData.c

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
sdf_detection_option.h \
sf_preproc_info.h

EXTRA_DIST = \
sf_sdf.dsp

sf_dynamic_preproc_lib.c: ../include/sf_dynamic_preproc_lib.c
	cp $? $@

sfPolicyUserData.c: ../include/sfPolicyUserData.c
	cp $? $@

all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-libLTLIBRARIES

clean-local:
	rm -f sf_dynamic_preproc_lib.c
	rm -f sfPolicyUserData.c
