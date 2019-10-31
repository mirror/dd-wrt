samba4-configure:
	cp $(TOP)/samba4/waf-cross-answers/$(ARCH).txt $(TOP)/samba4/cross-answers.txt
	echo 'Checking uname machine type: "$(ARCH)"' >> $(TOP)/samba4/cross-answers.txt
	echo 'Checking uname release type: "$(LINUX_VERSION)"' >> $(TOP)/samba4/cross-answers.txt
	echo 'Checking uname version type: "$(VERSION_DIST) Linux-$(LINUX_VERSION) $(shell date +%Y-%m-%d)"' >> $(TOP)/samba4/cross-answers.txt
	# NOTE: For some unknown reason this answer is not needed on some hosts/distros, yet needed on others?
	echo 'Checking whether POSIX capabilities are available: OK' >> $(TOP)/samba4/cross-answers.txt

	cd samba4 && ./configure -v \
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
		--disable-avahi \
		--without-quotas \
		--without-acl-support \
		--without-ad-dc \
		--without-libarchive \
		--disable-python \
		--nopyc \
		--nopyo \
		--without-dnsupdate \
		--without-ads \
		--without-ldap \
		--without-winbind \
		--private-libraries=talloc,tevent,tevent-util,texpect,tdb,ldb,tdr,cmocka,replace CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF"

samba4:
	@true

samba4-install:
	@true

samba4-clean:
	@true