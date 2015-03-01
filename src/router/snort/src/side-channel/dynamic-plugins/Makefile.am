## $Id$
AUTOMAKE_OPTIONS=foreign no-dependencies

if SO_WITH_STATIC_LIB

sidechannellibdir=$(pkglibdir)/dynamic_side_channel
sidechannellibincludedir=$(pkgincludedir)/dynamic_side_channel

sidechannellib_LTLIBRARIES = libsf_dynamic_side_channel.la
libsf_dynamic_side_channel_la_CFLAGS = -fPIC -DPIC
libsf_dynamic_side_channel_la_LDFLAGS = -static

libsf_dynamic_side_channel_la_SOURCES = \
    sf_dynamic_side_channel_lib.c

nodist_sidechannellibinclude_HEADERS = \
    ../sidechannel_define.h \
    ../../dynamic-plugins/sf_dynamic_common.h \
    ../../dynamic-plugins/sf_dynamic_define.h \
    ../../dynamic-plugins/sf_dynamic_meta.h \
    ../../dynamic-plugins/sf_dynamic_side_channel.h

sidechannellibinclude_HEADERS = \
    sf_dynamic_side_channel_lib.h \
    sf_side_channel_info.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = snort_side_channel.pc

endif
