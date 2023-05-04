## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include -I${srcdir}/../ssl_common -I${srcdir}/../libs

dynamicpreprocessordir = ${libdir}/snort_dynamicpreprocessor

dynamicpreprocessor_LTLIBRARIES = libsf_imap_preproc.la

libsf_imap_preproc_la_LDFLAGS = -export-dynamic -module @XCCFLAGS@
if SO_WITH_STATIC_LIB
libsf_imap_preproc_la_LIBADD = ../libsf_dynamic_preproc.la
else
nodist_libsf_imap_preproc_la_SOURCES = \
../include/sf_dynamic_preproc_lib.c \
../include/mempool.c \
../include/sf_sdlist.c \
../include/sf_base64decode.c \
../include/util_unfold.c \
../include/sf_email_attach_decode.c \
../include/sfPolicyUserData.c \
../ssl_common/ssl.c \
../ssl_common/ssl_config.c \
../ssl_common/ssl_inspect.c \
../libs/sfparser.c
endif

libsf_imap_preproc_la_SOURCES = \
imap_config.c \
imap_config.h \
imap_log.c \
imap_log.h \
imap_util.c \
imap_util.h \
imap_paf.c \
imap_paf.h \
snort_imap.c \
snort_imap.h \
spp_imap.c \
spp_imap.h 

if BUILD_BUFFER_DUMP
libsf_imap_preproc_la_SOURCES += \
imap_buffer_dump.c \
imap_buffer_dump.h
endif


EXTRA_DIST = \
sf_imap.vcxproj \
sf_imap.dsp

all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-dynamicpreprocessorLTLIBRARIES

