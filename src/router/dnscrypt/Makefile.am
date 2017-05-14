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
	dnscrypt-proxy.service.in \
	dnscrypt-proxy.socket \
	org.dnscrypt.osx.DNSCryptProxy.plist.in

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

doc_DATA = \
	README.markdown \
	COPYING \
	DNSCRYPT-V2-PROTOCOL.txt \
	dnscrypt-proxy.conf

if PLUGINS
doc_DATA += \
	README-PLUGINS.markdown
endif

noinst_DATA = \
	org.dnscrypt.osx.DNSCryptProxy.plist

if HAVE_SYSTEMD
noinst_DATA += \
	dnscrypt-proxy.service
endif

dnscrypt-proxy.service: dnscrypt-proxy.service.in
	$(SED) \
	-e 's|[@]sbindir@|$(sbindir)|g' \
	-e 's|[@]sysconfdir@|$(sysconfdir)|g' \
	< dnscrypt-proxy.service.in > dnscrypt-proxy.service

org.dnscrypt.osx.DNSCryptProxy.plist: org.dnscrypt.osx.DNSCryptProxy.plist.in
	$(SED) \
	-e 's|[@]sbindir@|$(sbindir)|g' \
	-e 's|[@]sysconfdir@|$(sysconfdir)|g' \
	< org.dnscrypt.osx.DNSCryptProxy.plist.in > org.dnscrypt.osx.DNSCryptProxy.plist

CLEANFILES = \
	dnscrypt-proxy.service \
	dnscrypt-update-resolvers.sh \
	org.dnscrypt.osx.DNSCryptProxy.plist

install-data-local:
	@$(mkinstalldirs) $(DESTDIR)$(sysconfdir); \
	if [ -f $(DESTDIR)$(sysconfdir)/dnscrypt-proxy.conf ]; then \
		if cmp -s $(srcdir)/dnscrypt-proxy.conf $(docdir)/dnscrypt-proxy.conf; then \
			echo "Configuration file unchanged"; \
		else \
			echo "The example configuration file [$(docdir)/dnscrypt-proxy.conf] has been updated."; \
			echo "You may want to compare it with the one at [$(DESTDIR)$(sysconfdir)/dnscrypt-proxy.conf]."; \
		fi; \
	else \
		$(INSTALL_DATA) $(srcdir)/dnscrypt-proxy.conf $(DESTDIR)$(sysconfdir)/dnscrypt-proxy.conf; \
	fi

uninstall-local:
	@if cmp -s $(srcdir)/dnscrypt-proxy.conf $(DESTDIR)$(sysconfdir)/dnscrypt-proxy.conf; then \
		echo "$(DESTDIR)$(sysconfdir)/dnscrypt-proxy.conf has not changed and will be removed."; \
		rm -f $(DESTDIR)$(sysconfdir)/dnscrypt-proxy.conf; \
        rmdir $(DESTDIR)$(sysconfdir) 2> /dev/null ||:; \
	fi
