## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include -I$(srcdir)/includes

libdir = ${exec_prefix}/lib/snort_dynamicpreprocessor

lib_LTLIBRARIES = libsf_dce2_preproc.la

libsf_dce2_preproc_la_LDFLAGS = -shared -export-dynamic -module @XCCFLAGS@

BUILT_SOURCES = \
sf_dynamic_preproc_lib.c \
sf_ip.c \
sfrt.c \
sfrt_dir.c \
sfPolicyUserData.c 

nodist_libsf_dce2_preproc_la_SOURCES = \
sf_dynamic_preproc_lib.c \
sf_ip.c \
sfrt.c \
sfrt_dir.c \
sfPolicyUserData.c 

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
sf_preproc_info.h

EXTRA_DIST = \
sf_dce2.dsp

sf_dynamic_preproc_lib.c: ../include/sf_dynamic_preproc_lib.c
	cp $? $@

sf_ip.c: ../include/sf_ip.c
	cp $? $@

sfrt.c: ../include/sfrt.c
	cp $? $@

sfrt_dir.c: ../include/sfrt_dir.c
	cp $? $@

sfPolicyUserData.c: ../include/sfPolicyUserData.c
	cp $? $@

all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-libLTLIBRARIES

clean-local:
	rm -f *~ sf_dynamic_preproc_lib.c sf_ip.c sfrt.c sfrt_dir.c sfPolicyUserData.c
