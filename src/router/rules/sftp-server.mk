sftp-server-configure:
	cd openssh && ./configure \
		--host=$(ARCH)-uclibc-linux \
		--prefix=/usr \
		--libdir=/usr/lib \
		--sysconfdir=/etc/ssh \
		--with-privsep-user=sshd \
		--with-privsep-path=/var/empty \
		--disable-strip \
		--disable-etc-default-login \
		--disable-lastlog \
		--disable-utmp \
		--disable-utmpx \
		--disable-wtmp \
		--disable-wtmpx \
		--without-bsd-auth \
		--without-pam \
		--without-bsd-out \
		--without-libedit \
		--without-ldns \
		--without-security-key-bsd \
		--disable-strip \
		--without-openssl \
		--without-zlib \
		--without-kerberos5 \
		--with-stackprotect \
		--with-cflags-after="-fzero-call-used-regs=skip $(LTO) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections"


sftp-server-clean:
	make -C openssh clean

sftp-server: 
	make -C openssh

sftp-server-install:
	install -D openssh/sftp-server $(INSTALLDIR)/sftp-server/usr/libexec/sftp-server	

