## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include

noinst_dynamicpreprocessordir = ${libdir}/snort_dynamicpreprocessor

noinst_dynamicpreprocessor_LTLIBRARIES = lib_sfdynamic_preprocessor_example.la

lib_sfdynamic_preprocessor_example_la_LDFLAGS = -export-dynamic

BUILT_SOURCES = \
sf_dynamic_preproc_lib.c  \
sfPolicyUserData.c

nodist_lib_sfdynamic_preprocessor_example_la_SOURCES = \
sf_dynamic_preproc_lib.c \
sfPolicyUserData.c \
spp_example.c \
sf_preproc_info.h

EXTRA_DIST = \
spp_example.c \
sf_preproc_info.h

sf_dynamic_preproc_lib.c: ../include/sf_dynamic_preproc_lib.c
	cp $? $@

sfPolicyUserData.c: ../include/sfPolicyUserData.c
	cp $? $@

clean-local:
	rm -f sf_dynamic_preproc_lib.c sfPolicyUserData.c
