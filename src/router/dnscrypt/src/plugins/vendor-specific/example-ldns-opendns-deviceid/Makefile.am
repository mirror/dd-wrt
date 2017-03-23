
EXTRA_DIST = \
	CONFIGURATION.txt

pkglib_LTLIBRARIES = \
	libdcplugin_example_ldns_opendns_deviceid.la

libdcplugin_example_ldns_opendns_deviceid_la_LIBTOOLFLAGS = --tag=disable-static

libdcplugin_example_ldns_opendns_deviceid_la_SOURCES = \
	example-ldns-opendns-deviceid.c

libdcplugin_example_ldns_opendns_deviceid_la_LIBADD = @LDNS_LIBS@

libdcplugin_example_ldns_opendns_deviceid_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-avoid-version \
	-export-dynamic \
	-module \
	-no-undefined

libdcplugin_example_ldns_opendns_deviceid_la_CPPFLAGS = \
	$(LTDLINCL) \
	-I../../../include
