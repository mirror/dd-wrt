
pkglib_LTLIBRARIES = \
	libdcplugin_example_cache.la

libdcplugin_example_cache_la_LIBTOOLFLAGS = --tag=disable-static

libdcplugin_example_cache_la_SOURCES = \
	example-cache.c \
	example-cache.h

libdcplugin_example_cache_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-avoid-version \
	-export-dynamic \
	-module \
	-no-undefined

libdcplugin_example_cache_la_CPPFLAGS = \
	$(LTDLINCL) \
	-I../../include
