
pkglib_LTLIBRARIES = \
	libdcplugin_example_logging.la

libdcplugin_example_logging_la_LIBTOOLFLAGS = --tag=disable-static

libdcplugin_example_logging_la_SOURCES = \
	example-logging.c

libdcplugin_example_logging_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-avoid-version \
	-export-dynamic \
	-module \
	-no-undefined

libdcplugin_example_logging_la_CPPFLAGS = \
	$(LTDLINCL) \
	-I../../include
