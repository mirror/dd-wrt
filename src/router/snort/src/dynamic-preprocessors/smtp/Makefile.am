## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include -I${srcdir}/../ssl_common -I${srcdir}/../libs

dynamicpreprocessordir = ${libdir}/snort_dynamicpreprocessor

dynamicpreprocessor_LTLIBRARIES = libsf_smtp_preproc.la

libsf_smtp_preproc_la_LDFLAGS = -export-dynamic -module @XCCFLAGS@
if SO_WITH_STATIC_LIB
libsf_smtp_preproc_la_LIBADD = ../libsf_dynamic_preproc.la
else
nodist_libsf_smtp_preproc_la_SOURCES = \
../include/sf_dynamic_preproc_lib.c \
../include/mempool.c \
../include/sf_sdlist.c \
../include/sf_base64decode.c \
../include/util_unfold.c \
../include/sf_email_attach_decode.c \
../include/sfPolicyUserData.c \
../ssl_common/ssl.c \
../ssl_common/ssl_ha.c \
../ssl_common/ssl_config.c \
../ssl_common/ssl_inspect.c \
../libs/sfparser.c

endif

libsf_smtp_preproc_la_SOURCES = \
smtp_config.c \
smtp_config.h \
smtp_log.c \
smtp_log.h \
smtp_normalize.c \
smtp_normalize.h \
smtp_util.c \
smtp_util.h \
smtp_xlink2state.c \
smtp_xlink2state.h \
smtp_paf.c \
smtp_paf.h \
snort_smtp.c \
snort_smtp.h \
spp_smtp.c \
spp_smtp.h

if BUILD_BUFFER_DUMP
libsf_smtp_preproc_la_SOURCES += \
smtp_buffer_dump.c \
smtp_buffer_dump.h
endif

EXTRA_DIST = \
sf_smtp.vcxproj \
sf_smtp.dsp

all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-dynamicpreprocessorLTLIBRARIES

