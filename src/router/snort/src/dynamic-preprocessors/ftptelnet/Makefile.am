## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include -I${srcdir}/../ssl_common -I${srcdir}/../libs -I${srcdir}/includes

dynamicpreprocessordir = ${libdir}/snort_dynamicpreprocessor

dynamicpreprocessor_LTLIBRARIES = libsf_ftptelnet_preproc.la

libsf_ftptelnet_preproc_la_LDFLAGS = -export-dynamic -module @XCCFLAGS@
if SO_WITH_STATIC_LIB
libsf_ftptelnet_preproc_la_LIBADD = ../libsf_dynamic_preproc.la ../libsf_dynamic_utils.la
else
nodist_libsf_ftptelnet_preproc_la_SOURCES = \
../include/sf_dynamic_preproc_lib.c \
../include/sf_ip.c \
../include/sfrt.c \
../include/sfrt_dir.c \
../include/sfPolicyUserData.c \
../include/sfmemcap.c \
../ssl_common/ssl.c \
../ssl_common/ssl_config.c \
../ssl_common/ssl_inspect.c \
../libs/sfparser.c
endif

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
spp_ftptelnet.h

if BUILD_BUFFER_DUMP
libsf_ftptelnet_preproc_la_SOURCES += \
ftptelnet_buffer_dump.c \
ftptelnet_buffer_dump.h
endif


EXTRA_DIST = \
sf_ftptelnet.vcxproj \
sf_ftptelnet.dsp

all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-dynamicpreprocessorLTLIBRARIES

