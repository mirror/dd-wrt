## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

include $(srcdir)/Makefile_defs

dynamicpreprocessordir = ${libdir}/snort_dynamicpreprocessor

dynamicpreprocessor_LTLIBRARIES = libsf_appid_preproc.la


libsf_appid_preproc_la_LDFLAGS = -export-dynamic -module @XCCFLAGS@
if SO_WITH_STATIC_LIB
libsf_appid_preproc_la_LIBADD = ../libsf_dynamic_preproc.la ../libsf_dynamic_utils.la $(LUA_LIBS)
else
nodist_libsf_appid_preproc_la_SOURCES = \
../include/sf_dynamic_preproc_lib.c \
../include/sf_ip.c \
../include/sfPolicyUserData.c \
../include/sfxhash.c \
../include/sfghash.c \
../include/sflsq.c \
../include/sfhashfcn.c \
../include/sfmemcap.c \
../include/sfprimetable.c

libsf_appid_preproc_la_LIBADD = $(LUA_LIBS)
endif

libsf_appid_preproc_la_CFLAGS = -DDYNAMIC_PREPROC_CONTEXT -DSTATIC=static -DINLINE=inline $(LUA_CFLAGS)

libsf_appid_preproc_la_SOURCES = $(APPID_SOURCES)

all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-dynamicpreprocessorLTLIBRARIES
