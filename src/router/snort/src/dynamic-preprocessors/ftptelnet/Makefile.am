## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include -I./includes

libdir = ${exec_prefix}/lib/snort_dynamicpreprocessor

lib_LTLIBRARIES = libsf_ftptelnet_preproc.la

libsf_ftptelnet_preproc_la_LDFLAGS = -shared -export-dynamic -module @XCCFLAGS@

BUILT_SOURCES = \
sf_dynamic_preproc_lib.c \
sf_ip.c \
sfrt.c \
sfrt_dir.c \
sfPolicyUserData.c 

nodist_libsf_ftptelnet_preproc_la_SOURCES = \
sf_dynamic_preproc_lib.c \
sf_ip.c \
sfrt.c \
sfrt_dir.c \
sfPolicyUserData.c 

libsf_ftptelnet_preproc_la_SOURCES = \
ftp_bounce_lookup.c \
ftp_bounce_lookup.h \
ftp_client.h \
ftp_cmd_lookup.c \
ftp_cmd_lookup.h \
ftpp_eo_events.h \
ftpp_eo.h \
ftpp_eo_log.c \
ftpp_eo_log.h \
ftpp_include.h \
ftpp_return_codes.h \
ftpp_si.c \
ftpp_si.h \
ftpp_ui_client_lookup.c \
ftpp_ui_client_lookup.h \
ftpp_ui_config.c \
ftpp_ui_config.h \
ftpp_ui_server_lookup.c \
ftpp_ui_server_lookup.h \
ftp_server.h \
hi_util_kmap.c \
hi_util_kmap.h \
hi_util_xmalloc.c \
hi_util_xmalloc.h \
pp_ftp.c \
pp_ftp.h \
pp_telnet.c \
pp_telnet.h \
snort_ftptelnet.c \
snort_ftptelnet.h \
spp_ftptelnet.c \
spp_ftptelnet.h \
sf_preproc_info.h

EXTRA_DIST = \
sf_ftptelnet.dsp

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
	rm -f sf_dynamic_preproc_lib.c sf_ip.c sfrt.c sfrt_dir.c sfPolicyUserData.c 

DIST_SUBDIRS = .

