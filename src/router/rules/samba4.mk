samba4-configure: gnutls
	cp $(TOP)/samba4/waf-cross-answers/$(ARCH).txt $(TOP)/samba4/cross-answers.txt
	echo 'Checking uname machine type: "$(ARCH)"' >> $(TOP)/samba4/cross-answers.txt
	echo 'Checking uname release type: "$(LINUX_VERSION)"' >> $(TOP)/samba4/cross-answers.txt
	echo 'Checking uname version type: "$(VERSION_DIST) Linux-$(LINUX_VERSION) $(shell date +%Y-%m-%d)"' >> $(TOP)/samba4/cross-answers.txt
	# NOTE: For some unknown reason this answer is not needed on some hosts/distros, yet needed on others?
	echo 'Checking whether POSIX capabilities are available: OK' >> $(TOP)/samba4/cross-answers.txt
	cd samba4 && ./configure --hostcc=gcc \
		--disable-cups \
		--disable-iprint \
		--disable-cephfs \
		--disable-fault-handling \
		--disable-glusterfs \
		--disable-rpath \
		--disable-rpath-install \
		--disable-rpath-private-install \
		--enable-fhs \
		--without-automount \
		--without-iconv \
		--without-lttng \
		--without-ntvfs-fileserver \
		--without-pam \
		--without-systemd \
		--without-utmp \
		--without-json \
		--without-dmapi \
		--without-fam \
		--without-gettext \
		--without-regedit \
		--without-gpgme \
		--builtin-libraries=replace --nonshared-binary=asn1_compile \
		--disable-avahi \
		--without-quotas --without-acl-support --without-winbind \
		--without-ad-dc --without-libarchive --disable-python --nopyc --nopyo \
		--without-dnsupdate --without-ads --without-ldap python_LDFLAGS="" python_LIBDIR="" CC=gcc LD=ld CFLAGS="-O2"
	cd samba4 && ./buildtools/bin/waf build --targets=asn1_compile,compile_et python_LDFLAGS="" python_LIBDIR="" CC=gcc LD=ld CFLAGS="-O2"
		install $(TOP)/samba4/bin/asn1_compile $(TOP)/samba4/bin/asn1_compile_host 
		install $(TOP)/samba4/bin/compile_et $(TOP)/samba4/bin/compile_et_host
	cd samba4 && ./configure \
		--hostcc="$(ARCH)-linux-uclibc-gcc" \
		--cross-compile \
		--cross-answers=cross-answers.txt \
		--disable-cups \
		--disable-iprint \
		--disable-cephfs \
		--disable-fault-handling \
		--disable-glusterfs \
		--disable-rpath \
		--disable-rpath-install \
		--disable-rpath-private-install \
		--enable-fhs \
		--without-automount \
		--without-iconv \
		--without-lttng \
		--without-ntvfs-fileserver \
		--without-pam \
		--without-systemd \
		--without-utmp \
		--without-dmapi \
		--without-fam \
		--without-gettext \
		--without-regedit \
		--without-gpgme \
		--with-lockdir=/var/lock \
		--with-logfilebase=/var/log \
		--with-piddir=/var/run \
		--with-privatedir=/etc/samba \
		--enable-avahi \
		--with-quotas \
		--without-acl-support \
		--without-ad-dc \
		--without-libarchive \
		--disable-python \
		--nopyc \
		--nopyo \
		--without-dnsupdate \
		--without-ads \
		--without-ldap \
		--with-winbind \
		--without-json \
		--prefix=/usr \
		--with-lockdir=/var \
		--destdir=$(INSTALLDIR)/samba4 \
		--private-libraries=talloc,tevent,tevent-util,texpect,tdb,ldb,tdr,cmocka,replace \
		CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF -I$(TOP)/gnutls/lib/includes -ffunction-sections -fdata-sections" \
		LDFLAGS="-Wl,--gc-sections -L$(TOP)/gnutls/lib/.libs -lgnutls -L$(TOP)/nettle -lnettle -lhogweed -L$(TOP)/gmp/.libs -lgmp"


samba4: gnutls
	make -C samba4

samba4-install:
	cd samba4 && ./buildtools/bin/waf install --destdir=$(INSTALLDIR)/samba4
	rm -rf $(INSTALLDIR)/samba4/usr/include
	rm -rf $(INSTALLDIR)/samba4/usr/lib/pkgconfig

samba4-clean:
	make -C samba4 clean