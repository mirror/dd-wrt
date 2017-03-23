ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = \
	DNSCRYPT-V2-PROTOCOL.txt \
	README-iOS.markdown \
	README-PLUGINS.markdown \
	README-WINDOWS.markdown \
	README.markdown \
	THANKS \
	apparmor.profile.dnscrypt-proxy \
	autogen.sh \
	dnscrypt-proxy.conf \
	dnscrypt-proxy.service \
	dnscrypt-proxy.socket \
	org.dnscrypt.osx.DNSCryptProxy.plist \
	resolvers-check.sh

SUBDIRS = \
	dist-build \
	man

if PLUGINS
SUBDIRS += \
	libltdl
endif

SUBDIRS += \
	contrib \
	src \
	test

dist_pkgdata_DATA = \
	dnscrypt-resolvers.csv \
	minisign.pub

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(sysconfdir); \
	if [ ! -f $(DESTDIR)$(sysconfdir)/dnscrypt-proxy.conf ]; then \
		$(INSTALL_DATA) $(srcdir)/dnscrypt-proxy.conf $(DESTDIR)$(sysconfdir)/dnscrypt-proxy.conf; \
	fi; \
	$(INSTALL_DATA) $(srcdir)/dnscrypt-proxy.conf $(DESTDIR)$(sysconfdir)/dnscrypt-proxy.conf.example

uninstall-local:
	if cmp -s $(DESTDIR)$(sysconfdir)/dnscrypt-proxy.conf $(srcdir)/dnscrypt-proxy.conf; then \
		echo "$(DESTDIR)$(sysconfdir)/dnscrypt-proxy.conf is not changed and will be removed."; \
		rm -f $(DESTDIR)$(sysconfdir)/dnscrypt-proxy.conf; \
	fi; \
	rm -f $(DESTDIR)$(sysconfdir)/dnscrypt-proxy.conf.example
