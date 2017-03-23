
pkglib_LTLIBRARIES = \
	libdcplugin_example.la

libdcplugin_example_la_LIBTOOLFLAGS = --tag=disable-static

libdcplugin_example_la_SOURCES = \
	example.c

libdcplugin_example_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-avoid-version \
	-export-dynamic \
	-module \
	-no-undefined

libdcplugin_example_la_CPPFLAGS = \
	$(LTDLINCL) \
	-I../../include
