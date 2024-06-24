ifeq ($(ARCH),x86_64)
	SAMBA4_AES_ARGS:= --accel-aes=intelaesni
else
	SAMBA4_AES_ARGS:= --accel-aes=none
endif

samba4-configure: gnutls icu zlib
	cp $(TOP)/samba4/waf-cross-answers/$(ARCH).txt $(TOP)/samba4/cross-answers.txt
	echo 'Checking uname machine type: "$(ARCH)"' >> $(TOP)/samba4/cross-answers.txt
	echo 'Checking uname release type: "$(LINUX_VERSION)"' >> $(TOP)/samba4/cross-answers.txt
	echo 'Checking uname version type: "$(VERSION_DIST) Linux-$(LINUX_VERSION) $(shell date +%Y-%m-%d)"' >> $(TOP)/samba4/cross-answers.txt
	# NOTE: For some unknown reason this answer is not needed on some hosts/distros, yet needed on others?
	echo 'Checking whether POSIX capabilities are available: OK' >> $(TOP)/samba4/cross-answers.txt
		cd samba4 && ./configure  --hostcc=gcc \
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
		--without-pam \
		--without-systemd \
		--without-utmp \
		--without-json \
		--without-dmapi \
		--without-fam \
		--without-gettext \
		--without-regedit \
		--without-gpgme \
		--with-shared-modules='!vfs_snapper' \
		--builtin-libraries=replace --nonshared-binary=asn1_compile \
		--disable-avahi \
		--without-quotas --without-acl-support --without-winbind \
		--without-ad-dc --without-libarchive --disable-python --nopyc --nopyo \
		--without-ads --without-ldap python_LDFLAGS="" python_LIBDIR="" CC=gcc LD=ld CFLAGS="-O2"
		-cd samba4 && ./buildtools/bin/waf build --targets=asn1_compile,compile_et python_LDFLAGS="" python_LIBDIR="" CC=gcc LD=ld CFLAGS="-O2"
		install $(TOP)/samba4/bin/asn1_compile $(TOP)/samba4/bin/asn1_compile_host 
		install $(TOP)/samba4/bin/compile_et $(TOP)/samba4/bin/compile_et_host
		cd samba4 && ./buildtools/bin/waf build --targets=asn1_compile,compile_et python_LDFLAGS="" python_LIBDIR="" CC=gcc LD=ld CFLAGS="-O2"
		export CPPFLAGS="$(COPTS) $(MIPS16_OPT) -DLIBREPLACE_NETWORK_CHECKS -DNODEBUG -DNEED_PRINTF -I$(TOP)/libtirpc/tirpc -I$(TOP)/gnutls/lib/includes -I$(TOP)/zlib/include -I$(TOP)/icu/target_staging/include -L$(TOP)/icu/target_staging/lib -ffunction-sections -fdata-sections -fPIC" && \
		export CXXFLAGS="$(COPTS) $(MIPS16_OPT) -DLIBREPLACE_NETWORK_CHECKS -DNODEBUG -DNEED_PRINTF -I$(TOP)/libtirpc/tirpc  -I$(TOP)/gnutls/lib/includes -I$(TOP)/zlib/include -I$(TOP)/icu/target_staging/include -L$(TOP)/icu/target_staging/lib -ffunction-sections -fdata-sections -fPIC" && \
		export CFLAGS="$(COPTS) $(MIPS16_OPT) -DLIBREPLACE_NETWORK_CHECKS -DNODEBUG -DNEED_PRINTF -I$(TOP)/libtirpc/tirpc  -I$(TOP)/gnutls/lib/includes -I$(TOP)/zlib/include -I$(TOP)/icu/target_staging/include -L$(TOP)/icu/target_staging/lib -ffunction-sections -fdata-sections -fPIC" && \
		export LDFLAGS="-Wl,--gc-sections -L$(TOP)/libtirpc/src/.libs -L$(TOP)/gnutls/lib/.libs -lgnutls -L$(TOP)/nettle -lnettle -lhogweed  -L$(TOP)/gmp/.libs -lgmp -L$(TOP)/icu/target_staging/lib -L$(TOP)/zlib  -fPIC" && \
		export AR_FLAGS="cru $(LTOPLUGIN)" && \
		export RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)" && \
		cd samba4 && ./buildtools/bin/waf configure \
		--hostcc="ccache $(ARCH)-linux-uclibc-gcc" \
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
		--with-shared-modules='!vfs_snapper' \
		--enable-fhs \
		--without-automount \
		--without-iconv \
		--without-lttng \
		--without-pam \
		--without-systemd \
		--without-utmp \
		--without-dmapi \
		--without-fam \
		--without-gettext \
		--without-regedit \
		--without-gpgme \
		--with-lockdir=/tmp/var/lock \
		--with-logfilebase=/tmp/var/log \
		--with-piddir=/tmp/var/run \
		--with-privatedir=/etc/samba \
		--with-privatelibdir=/usr/lib \
		--localstatedir=/tmp/var \
		--enable-avahi \
		--with-quotas \
		--without-acl-support \
		--without-ad-dc \
		--without-libarchive \
		--disable-python \
		--nopyc \
		--nopyo \
		--without-ads \
		--without-ldap \
		--with-winbind \
		--without-json \
		$(SAMBA4_AES_ARGS) \
		--prefix=/usr \
		--with-lockdir=/tmp/var \
		--destdir=$(INSTALLDIR)/samba4 \
		--private-libraries=talloc,tevent,tevent-util,texpect,tdb,ldb,tdr,cmocka,replace \
		
		-make -C samba4
		sed -i 's/\/USR\/BIN\/PYTHON3/PYTHON3/g' $(TOP)/samba4/bin/default/source3/smbd/build_options.c
		-make -C samba4
		sed -i 's/\/USR\/BIN\/PYTHON3/PYTHON3/g' $(TOP)/samba4/bin/default/source3/smbd/build_options.c


samba4: gnutls icu zlib
	install -D samba4/config/samba4.webnas httpd/ej_temp/02samba4.webnas
	install -D filesharing/config/zfilesharing.webnas httpd/ej_temp/03zfilesharing.webnas
	-sed -i 's/\/USR\/BIN\/PYTHON3/PYTHON3/g' $(TOP)/samba4/bin/default/source3/smbd/build_options.c
	make -C samba4

samba4-install:
	cd samba4 && ./buildtools/bin/waf install --destdir=$(INSTALLDIR)/samba4
	cp $(INSTALLDIR)/samba4/usr/bin/smbpasswd $(INSTALLDIR)/samba4/usr/sbin/smbpasswd
	rm -f $(INSTALLDIR)/samba4/usr/bin/*
	mv $(INSTALLDIR)/samba4/usr/sbin/smbpasswd $(INSTALLDIR)/samba4/usr/bin/smbpasswd
	rm -f $(INSTALLDIR)/samba4/usr/sbin/samba-gpupdate
	rm -f $(INSTALLDIR)/samba4/usr/sbin/eventlogadm
#	install -D samba36/source3/bin/smbpasswd $(INSTALLDIR)/samba4/usr/bin/smbpasswd
	rm -rf $(INSTALLDIR)/samba4/usr/include
	rm -rf $(INSTALLDIR)/samba4/usr/etc
	rm -rf $(INSTALLDIR)/samba4/usr/lib/pkgconfig
	rm -rf $(INSTALLDIR)/samba4/var
	rm -rf $(INSTALLDIR)/samba4/tmp
	install -D samba4/config/samba4.webnas $(INSTALLDIR)/samba4/etc/config/02samba4.webnas
	install -D samba4/config/samba4.nvramconfig $(INSTALLDIR)/samba4/etc/config/samba4.nvramconfig
	install -D filesharing/config/zfilesharing.webnas $(INSTALLDIR)/samba4/etc/config/03zfilesharing.webnas


samba4-clean:
	make -C samba4 clean