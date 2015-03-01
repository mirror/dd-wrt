AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I$(top_srcdir)/src/dynamic-output/plugins -I$(top_srcdir)/src -I$(top_srcdir)/src/sfutil -I$(top_srcdir)/src/dynamic-plugins -I$(top_srcdir)/src/preprocessors

if SO_WITH_STATIC_LIB

outputlibdir=$(pkglibdir)/dynamic_output
outputlibincludedir=$(pkgincludedir)/dynamic_output

outputlib_LTLIBRARIES = libsf_dynamic_output.la
libsf_dynamic_output_la_CFLAGS = -fPIC -DPIC
libsf_dynamic_output_la_LDFLAGS = -static

libsf_dynamic_output_la_SOURCES = \
output_lib.c

preprocincludedir = $(top_builddir)/src/dynamic-preprocessors/include
external_headers = \
$(top_srcdir)/src/dynamic-output/plugins/output_lib.h \
$(top_srcdir)/src/dynamic-output/plugins/output_api.h \
$(top_srcdir)/src/dynamic-output/plugins/output_common.h \
$(preprocincludedir)/obfuscation.h \
$(preprocincludedir)/ipv6_port.h \
$(preprocincludedir)/sf_ip.h \
$(preprocincludedir)/snort_debug.h \
$(preprocincludedir)/sfPolicy.h \
$(preprocincludedir)/sf_dynamic_common.h \
$(preprocincludedir)/stream_api.h \
$(preprocincludedir)/preprocids.h \
$(preprocincludedir)/bitop.h \
$(preprocincludedir)/sf_snort_packet.h \
$(preprocincludedir)/sf_protocols.h \
$(preprocincludedir)/sfrt.h \
$(preprocincludedir)/sfrt_dir.h \
$(preprocincludedir)/sfrt_trie.h

nodist_libsf_dynamic_output_la_SOURCES = $(external_headers)

nodist_outputlibinclude_HEADERS = $(external_headers)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = snort_output.pc

endif

EXTRA_DIST = \
snort_output.pc.in
