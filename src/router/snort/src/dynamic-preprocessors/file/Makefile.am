## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I../include -I${srcdir}/../libs -I./include   

dynamicpreprocessordir = ${libdir}/snort_dynamicpreprocessor

dynamicpreprocessor_LTLIBRARIES = libsf_file_preproc.la

libsf_file_preproc_la_LDFLAGS = -export-dynamic -module @XCCFLAGS@
if SO_WITH_STATIC_LIB
libsf_file_preproc_la_LIBADD = ../libsf_dynamic_preproc.la
else
nodist_libsf_file_preproc_la_SOURCES = \
../include/sf_dynamic_preproc_lib.c \
../include/sfPolicyUserData.c
endif

libsf_file_preproc_la_SOURCES = \
spp_file.c \
spp_file.h \
file_agent.c \
file_agent.h  \
file_event_log.c \
file_event_log.h  \
file_inspect_config.c  \
file_inspect_config.h  \
file_sha.c \
file_sha.h  \
./include/output_lib.c \
./include/circular_buffer.c

BUILT_SOURCES = \
include/output_lib.c \
include/output_api.h \
include/output_lib.h \
include/output_common.h \
include/sf_textlog.h \
include/circular_buffer.c \
include/circular_buffer.h

EXTRA_DIST = \
sf_file.dsp

copy_headers = \
	mkdir -p include; \
	if test -f $$dst_header; then \
		x=`diff $$src_header $$dst_header.new.new >> /dev/null`; \
		if test "$$x" != "0"; then \
			echo "Updating " $$dst_header; \
			cp $$src_header $$dst_header; \
		fi \
	else \
		echo "Updating " $$dst_header; \
		cp $$src_header $$dst_header; \
	fi
	
include/output_lib.c: ${top_srcdir}/src/dynamic-output/libs/output_lib.c
	@src_header=$?; dst_header=$@; $(copy_headers)
include/output_api.h: ${top_srcdir}/src/dynamic-output/plugins/output_api.h
	@src_header=$?; dst_header=$@; $(copy_headers)
include/output_lib.h: ${top_srcdir}/src/dynamic-output/plugins/output_lib.h
	@src_header=$?; dst_header=$@; $(copy_headers)
include/output_common.h: ${top_srcdir}/src/dynamic-output/plugins/output_common.h
	@src_header=$?; dst_header=$@; $(copy_headers)
include/sf_textlog.h: ${top_srcdir}/src/sfutil/sf_textlog.h
	@src_header=$?; dst_header=$@; $(copy_headers)
include/circular_buffer.h: ${top_srcdir}/src/file-process/circular_buffer.h
	@src_header=$?; dst_header=$@; $(copy_headers)
include/circular_buffer.c: ${top_srcdir}/src/file-process/circular_buffer.c
	@src_header=$?; dst_header=$@; $(copy_headers)
	
clean-local:
	rm -rf include
	
all-local: $(LTLIBRARIES)
	$(MAKE) DESTDIR=`pwd`/../build install-dynamicpreprocessorLTLIBRARIES

