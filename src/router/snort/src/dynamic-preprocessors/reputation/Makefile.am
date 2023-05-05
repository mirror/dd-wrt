## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include -I${srcdir}/../libs -I$(srcdir)/includes

dynamicpreprocessordir = ${libdir}/snort_dynamicpreprocessor

dynamicpreprocessor_LTLIBRARIES = libsf_reputation_preproc.la

libsf_reputation_preproc_la_LDFLAGS = -export-dynamic -module @XCCFLAGS@
if SO_WITH_STATIC_LIB
libsf_reputation_preproc_la_LIBADD = ../libsf_dynamic_preproc.la
else
nodist_libsf_reputation_preproc_la_SOURCES = \
../include/sf_dynamic_preproc_lib.c \
../include/sf_ip.c \
../include/sfrt.c \
../include/sfrt_dir.c \
../include/sfrt_flat.c \
../include/sfrt_flat_dir.c \
../include/segment_mem.c \
../include/sfPolicyUserData.c
endif

if HAVE_SHARED_REP
libsf_reputation_preproc_la_SOURCES = \
spp_reputation.c \
spp_reputation.h \
reputation_config.c \
reputation_config.h \
reputation_utils.c \
reputation_utils.h \
reputation_debug.h \
./shmem/sflinux_helpers.c \
./shmem/sflinux_helpers.h \
./shmem/shmem_common.h \
./shmem/shmem_config.h \
./shmem/shmem_config.c \
./shmem/shmem_datamgmt.h \
./shmem/shmem_datamgmt.c \
./shmem/shmem_lib.h \
./shmem/shmem_lib.c \
./shmem/shmem_mgmt.h \
./shmem/shmem_mgmt.c 
else
libsf_reputation_preproc_la_SOURCES = \
spp_reputation.c \
spp_reputation.h \
reputation_config.c \
reputation_config.h \
reputation_utils.c \
reputation_utils.h \
reputation_debug.h 
endif 


EXTRA_DIST = \
sf_reputation.vcxproj \
sf_reputation.dsp

all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-dynamicpreprocessorLTLIBRARIES
