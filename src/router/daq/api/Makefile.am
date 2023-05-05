AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4

include_HEADERS = daq.h daq_api.h daq_common.h

lib_LTLIBRARIES = libdaq.la libdaq_static.la

libdaq_la_SOURCES = daq_base.c daq_mod_ops.c daq.h daq_api.h daq_common.h daq_version.h
libdaq_la_LDFLAGS = -version-info 4:2:0 @XCCFLAGS@
libdaq_la_LIBADD = @LIBDL@

libdaq_static_la_SOURCES = daq_base.c daq_mod_ops.c daq.h daq_api.h daq_common.h daq_version.h
libdaq_static_la_CFLAGS = $(AM_CFLAGS) -DSTATIC_MODULE_LIST
libdaq_static_la_LDFLAGS = -static
