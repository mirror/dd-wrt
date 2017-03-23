
pkglib_LTLIBRARIES = \
	libdcplugin_example_ldns_blocking.la

libdcplugin_example_ldns_blocking_la_LIBTOOLFLAGS = --tag=disable-static

libdcplugin_example_ldns_blocking_la_SOURCES = \
	example-ldns-blocking.c fpst.c fpst.h

libdcplugin_example_ldns_blocking_la_LIBADD = @LDNS_LIBS@

libdcplugin_example_ldns_blocking_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-avoid-version \
	-export-dynamic \
	-module \
	-no-undefined

libdcplugin_example_ldns_blocking_la_CPPFLAGS = \
	$(LTDLINCL) \
	-I../../include
