ifeq ($(CONFIG_HOTPLUG2),y)
export SAMBA3_EXTRA:= -DHAVE_DDINOTIFY
endif
CONFIGURE_VARS += \
	ac_cv_file__proc_sys_kernel_core_pattern=yes \
	libreplace_cv_HAVE_C99_VSNPRINTF=yes \
	libreplace_cv_HAVE_GETADDRINFO=yes \
	libreplace_cv_HAVE_IFACE_IFCONF=yes \
	LINUX_LFS_SUPPORT=yes \
	samba_cv_CC_NEGATIVE_ENUM_VALUES=yes \
	samba_cv_HAVE_GETTIMEOFDAY_TZ=yes \
	samba_cv_HAVE_IFACE_IFCONF=yes \
	samba_cv_HAVE_KERNEL_OPLOCKS_LINUX=yes \
	samba_cv_HAVE_SECURE_MKSTEMP=yes \
	samba_cv_HAVE_WRFILE_KEYTAB=no \
	samba_cv_USE_SETREUID=yes \
	samba_cv_USE_SETRESUID=yes \
	samba_cv_have_setreuid=yes \
	samba_cv_have_setresuid=yes

CONFIGURE_ARGS_SMB += \
	--host=$(ARCH)-linux \
	--exec-prefix=/usr \
	--prefix=/ \
	--disable-avahi \
	--disable-cups \
	--disable-pie \
	--disable-relro \
	--disable-static \
	--disable-swat \
	--disable-shared-libs \
	--with-codepagedir=/etc/samba \
	--with-configdir=/etc/samba \
	--with-included-iniparser \
	--with-included-popt \
	--with-lockdir=/var/lock \
	--with-logfilebase=/var/log \
	--with-nmbdsocketdir=/var/nmbd \
	--with-piddir=/var/run \
	--with-privatedir=/etc/samba \
	--with-sendfile-support \
	--without-cluster-support \
	--without-ads \
	--without-krb5 \
	--without-ldap \
	--without-pam \
	--without-winbind \
	--without-libtdb \
	--without-libtalloc \
	--without-libnetapi \
	--without-libsmbclient \
	--without-libsmbsharemodes \
	--without-libaddns \
	--with-shared-modules=pdb_tdbsam,pdb_wbc_sam,idmap_nss,nss_info_template,auth_winbind,auth_wbc,auth_domain

samba3-preconfigure:
	if ! test -e "samba36/source3/Makefile"; then	cd samba36/source3 && ./configure $(CONFIGURE_VARS) $(CONFIGURE_ARGS_SMB) CFLAGS="$(COPTS) $(LTO) -DMAX_DEBUG_LEVEL=-1  -ffunction-sections -fdata-sections -Wl,--gc-sections $(SAMBA3_EXTRA)" LDFLAGS="$(COPTS) $(LDLTO) -DMAX_DEBUG_LEVEL=-1  -ffunction-sections -fdata-sections -Wl,--gc-sections $(SAMBA3_EXTRA)"; fi

samba3-configure: samba3-delete samba3-preconfigure
	touch samba36/librpc/idl/*
	

samba3-delete:
	rm -f samba36/source3/Makefile

samba3: samba3-preconfigure
	$(MAKE) -C samba36/source3 bin/libtalloc.a WITH_LFS=yes DYNEXP= PICFLAG= MODULES= 
	$(MAKE) -C samba36/source3 bin/libtdb.a WITH_LFS=yes DYNEXP= PICFLAG= MODULES= 
	$(MAKE) -C samba36/source3 bin/libtevent.a WITH_LFS=yes DYNEXP= PICFLAG= MODULES= 
	$(MAKE) -C samba36/source3 bin/libwbclient.a WITH_LFS=yes DYNEXP= PICFLAG= MODULES= 
	$(MAKE) -C samba36/source3 bin/samba_multicall WITH_LFS=yes DYNEXP= PICFLAG= MODULES= 




samba3-install:
	mkdir -p $(INSTALLDIR)/samba3
	install -D samba36/source3/bin/samba_multicall $(INSTALLDIR)/samba3/usr/sbin/samba_multicall
	install -D samba36/source3/bin/smbpasswd $(INSTALLDIR)/samba3/usr/sbin/smbpasswd
	install -D samba36/config/samba3.webnas $(INSTALLDIR)/samba3/etc/config/02samba3.webnas
	install -D samba36/config/samba3.nvramconfig $(INSTALLDIR)/samba3/etc/config/samba3.nvramconfig
	install -D filesharing/config/zfilesharing.webnas $(INSTALLDIR)/samba3/etc/config/03zfilesharing.webnas
	cd  $(INSTALLDIR)/samba3/usr/sbin && ln -sf samba_multicall smbd
	cd  $(INSTALLDIR)/samba3/usr/sbin && ln -sf samba_multicall nmbd


samba3-clean:
	-$(MAKE) -C samba36/source3 clean
	-rm -f samba36/source3/multi.o
	-rm -f samba36/auth/auth_sam_reply.o
