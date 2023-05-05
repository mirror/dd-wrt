## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include -I${srcdir}/../libs

dynamicpreprocessordir = ${libdir}/snort_dynamicpreprocessor

dynamicpreprocessor_LTLIBRARIES = libsf_ssh_preproc.la

libsf_ssh_preproc_la_LDFLAGS = -export-dynamic -module @XCCFLAGS@
if SO_WITH_STATIC_LIB
libsf_ssh_preproc_la_LIBADD = ../libsf_dynamic_preproc.la
else
nodist_libsf_ssh_preproc_la_SOURCES = \
../include/sf_dynamic_preproc_lib.c \
../include/sfPolicyUserData.c
endif

libsf_ssh_preproc_la_SOURCES = \
spp_ssh.c \
spp_ssh.h

if BUILD_BUFFER_DUMP
libsf_ssh_preproc_la_SOURCES += \
ssh_buffer_dump.c \
ssh_buffer_dump.h
endif

EXTRA_DIST = \
sf_ssh.vcxproj \
sf_ssh.dsp

all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-dynamicpreprocessorLTLIBRARIES

