## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include

libdir = ${exec_prefix}/lib/snort_dynamicpreprocessor

lib_LTLIBRARIES = libsf_dns_preproc.la

libsf_dns_preproc_la_LDFLAGS = -shared -export-dynamic -module @XCCFLAGS@

BUILT_SOURCES = \
sf_dynamic_preproc_lib.c \
sfPolicyUserData.c

nodist_libsf_dns_preproc_la_SOURCES = \
sf_dynamic_preproc_lib.c \
sfPolicyUserData.c

libsf_dns_preproc_la_SOURCES = \
spp_dns.c \
spp_dns.h \
sf_preproc_info.h

EXTRA_DIST = \
sf_dns.dsp

sf_dynamic_preproc_lib.c: ../include/sf_dynamic_preproc_lib.c
	cp $? $@

sfPolicyUserData.c: ../include/sfPolicyUserData.c
	cp $? $@

all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-libLTLIBRARIES

clean-local:
	rm -f sf_dynamic_preproc_lib.c sfPolicyUserData.c
