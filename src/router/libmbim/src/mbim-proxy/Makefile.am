
libexec_PROGRAMS = mbim-proxy

mbim_proxy_CPPFLAGS = \
	$(MBIMPROXY_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src/libmbim-glib \
	-I$(top_srcdir)/src/libmbim-glib/generated \
	-I$(top_builddir)/src/libmbim-glib \
	-I$(top_builddir)/src/libmbim-glib/generated

mbim_proxy_SOURCES = mbim-proxy.c

mbim_proxy_LDADD = \
	$(MBIMPROXY_LIBS) \
	$(top_builddir)/src/libmbim-glib/libmbim-glib.la

#Install udev rules only if configured with --enable-mbim-username
if MBIM_USERNAME_ENABLED
udevrulesdir = $(UDEV_BASE_DIR)/rules.d
udevrules_DATA = 76-mbim-proxy-device-ownership.rules
endif

EXTRA_DIST = \
	76-mbim-proxy-device-ownership.rules.in
