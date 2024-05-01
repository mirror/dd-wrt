freeradius3-configure: libtalloc openssl libpcap
	cd freeradius3 && mkdir -p build/make; gcc scripts/jlibtool.c -o build/make/jlibtool
	-cd freeradius3 && libtoolize -ci --force 
	-cd freeradius3 && aclocal
	-cd freeradius3 && automake --add-missing
	cd freeradius3 && autoconf && \
	sys_lib_dlsearch_path_spec="$(ARCH)-uclibc" \
	sys_lib_search_path_spec="$(ARCH)-uclibc" \
	MYSQL_CONFIG="no" CC="ccache $(ARCH)-linux-uclibc-gcc" \
	ac_cv_lib_readline=no \
	ac_cv_lib_ssl_SSL_new=yes \
	ac_cv_lib_crypto_DH_new=yes \
	ac_cv_func_strncasecmp=yes \
	ac_cv_func_strcasecmp=yes \
	ac_cv_lib_crypt_crypt=yes \
	ac_cv_func_gettimeofday=yes \
	ac_cv_func_getnameinfo=yes \
	ac_cv_func_getaddrinfo=yes \
	ac_cv_func_setlinebuf=yes \
	ax_cv_cc_builtin_choose_expr=yes \
	ax_cv_cc_builtin_types_compatible_p=yes ax_cv_cc_builtin_bswap64=yes \
	ax_cv_cc_bounded_attribute=no \
	ac_cv_lib_collectdclient_lcc_connect=no \
	ac_cv_lib_execinfo_backtrace_symbols=no \
	ac_cv_host=$(ARCH)-uclibc-linux \
	./configure  --target=$(ARCH)-linux --host=$(ARCH) CFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -fPIC -I$(TOP)/openssl/include -D_GNU_SOURCE" CPPFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -D_GNU_SOURCE -fPIC -I$(TOP)/openssl/include " LDFLAGS="$(COPTS) $(MIPS16_OPT) $(THUMB) -L$(TOP)/openssl -L$(TOP)/libpcap -lpcap -lm" --enable-shared \
	--program-prefix="" \
	--program-suffix="" \
	--prefix=/usr \
	--exec-prefix=/usr \
	--bindir=/usr/bin \
	--datadir=/usr/share \
	--disable-developer \
	--includedir=/usr/include \
	--infodir=/usr/share/info \
	--libdir=/usr/lib \
	--libexecdir=/usr/lib \
	--with-radacctdir=/var/db/radacct \
	--with-logdir=/var/log \
	--with-talloc-lib-dir=$(TOP)/libtalloc/bin/default \
	--with-talloc-include-dir=$(TOP)/libtalloc \
	--disable-openssl-version-check \
	--localstatedir=/var \
	--mandir=/usr/share/man \
	--sbindir=/usr/sbin \
	--sysconfdir=/etc \
	--enable-shared \
	--with-pcap-lib-dir=$(TOP)/libpcap \
	--with-pcap-include-dir=$(TOP)/libpcap \
	--disable-static \
	--disable-developer \
	--with-threads \
	--with-ltdl-include="$(TOP)/freeradius3/libltdl/.libs" \
	--with-ltdl-lib="$(TOP)/freeradius3/libltdl" \
	--with-openssl-includes="$(TOP)/openssl/include" \
	--with-openssl-libraries="$(TOP)/openssl" \
	--enable-strict-dependencies \
	--with-raddbdir=/etc/freeradius \
	--without-edir \
	--without-snmp \
	--with-experimental-modules \
	--without-rlm_attr-rewrite \
	--without-rlm_checkval \
	--without-rlm_counter \
	--without-rlm_dbm \
	--without-rlm_python3 \
	--without-rlm_ldap \
	--without-edir \
	--without-snmp \
	--with-rlm_expr \
	--with-rlm_eap \
	--without-rlm_eap_sim \
	--without-rlm_example \
	--without-rlm_ippool \
	--without-rlm_krb5 \
	--without-rlm_pam \
	--without-rlm_perl \
	--without-rlm_python \
	--without-rlm_smb \
	--with-rlm_sql \
	--with-rlm_sqlcounter \
	--without-rlm_sql_db2 \
	--without-rlm_sql_freetds \
	--without-rlm_sql_iodbc \
	--without-rlm_sql_oracle \
	--without-rlm_sql_sybase \
	--without-rlm_sql_unixodbc \
	--without-rlm_x99-token \
  	--without-rlm_eap_ikev2 \
  	--without-rlm_eap_tnc \
	--without-rlm_opendirectory \
	--without-rlm_wimax \
  	--with-rlm_eap_peap \
  	--with-rlm_eap_tls \
  	--with-rlm_eap_ttls \
	--with-rlm_expiration \
	--with-rlm_logintime \
	--with-rlm_attr-rewrite \
	--without-rlm_otp \
	--without-rlm_smsotp \
	--without-rlm_sqlhpwippool \
	--without-rlm_sqlippool \
	--without-rlm_sql_db2 \
	--without-rlm_sql_firebird \
	--without-rlm_sql_freetds \
	--without-rlm_sql_iodbc \
	--without-rlm_sql_oracle \
	--without-rlm_sql_sybase \
	--without-rlm_sql_unixodbc \
	--without-rlm_sql_log \
	--without-rlm_sql_sqlite \
	--without-rlm_caching \
	--without-rlm_redis \
	--without-rlm_rediswho \
	--without-rlm_eap_tnc \
	--without-rlm_eap_ikev2 \
	--without-rlm_opendirectory \
	--without-rlm_wimax \
	--without-rlm_ruby \
	--without-rlm_sql_mysql \
	--without-rlm_sql_postgresql \
	--without-edir \
	--without-snmp \
	--without-rlm_cache \
	--without-rlm_cache_memcached \
	--without-rlm_couchbase \
	--without-rlm_counter \
	--without-rlm_eap_ikev2 \
	--without-rlm_eap_pwd \
	--without-rlm_eap_sim \
	--without-rlm_eap_tnc \
	--without-rlm_example \
	--without-rlm_idn \
	--without-rlm_ippool \
	--without-rlm_krb5 \
	--without-rlm_ldap \
	--without-rlm_opendirectory \
	--without-rlm_pam \
	--without-rlm_perl \
	--without-rlm_python \
	--without-rlm_redis \
	--without-rlm_rediswho \
	--without-rlm_rest \
	--without-rlm_ruby \
	--without-rlm_securid \
	--without-rlm_smsotp \
	--without-rlm_sql \
	--without-rlm_sql_db2 \
	--without-rlm_sql_firebird \
	--without-rlm_sql_freetds \
	--without-rlm_sql_iodbc \
	--without-rlm_sql_mysql \
	--without-rlm_sql_oracle \
	--without-rlm_sql_postgresql \
	--without-rlm_sql_sqlite \
	--without-rlm_sql_unixodbc \
	--without-rlm_sqlcounter \
	--without-rlm_sqlhpwippool \
	--without-rlm_sqlippool \
	--without-rlm_unbound \
	--without-rlm_yubikey \
	--without-rlm_json
	sed -i 's/-isystem \/usr\/include/ /g' $(TOP)/freeradius3/Make.inc


freeradius3: libtalloc libpcap
	cd freeradius3 && mkdir -p build/make; gcc scripts/jlibtool.c -o build/make/jlibtool
	make -C freeradius3 R="$(INSTALLDIR)/freeradius3" INSTALLSTRIP="" all

freeradius3-clean:
	make -C freeradius3 clean

freeradius3-install:
	make -C freeradius3 install R=$(INSTALLDIR)/freeradius3
	rm -rf $(INSTALLDIR)/freeradius3/usr/share/doc
	rm -rf $(INSTALLDIR)/freeradius3/usr/share/man
	rm -rf $(INSTALLDIR)/freeradius3/usr/include
	rm -rf $(INSTALLDIR)/freeradius3/var
	rm -f $(INSTALLDIR)/freeradius3/usr/lib/*.la
	rm -f $(INSTALLDIR)/freeradius3/usr/lib/*.a
	rm -f $(INSTALLDIR)/freeradius3/etc/sql
	mkdir -p $(INSTALLDIR)/freeradius3/etc/config
	cp freeradius3/config/freeradius.nvramconfig $(INSTALLDIR)/freeradius3/etc/config
	rm -f $(INSTALLDIR)/freeradius3/usr/sbin/rc.radiusd
	rm -f $(INSTALLDIR)/freeradius3/usr/sbin/radmin
	rm -f $(INSTALLDIR)/freeradius3/usr/sbin/raddebug
	rm -f $(INSTALLDIR)/freeradius3/usr/sbin/checkrad
	rm -f $(INSTALLDIR)/freeradius3/usr/bin/dhcpclient
	rm -f $(INSTALLDIR)/freeradius3/usr/bin/map_unit
	rm -f $(INSTALLDIR)/freeradius3/usr/bin/radattr
	rm -f $(INSTALLDIR)/freeradius3/usr/bin/radcrypt
	rm -f $(INSTALLDIR)/freeradius3/usr/bin/radlast
	rm -f $(INSTALLDIR)/freeradius3/usr/bin/radsqlrelay
	rm -f $(INSTALLDIR)/freeradius3/usr/bin/radtest
	rm -f $(INSTALLDIR)/freeradius3/usr/bin/radzap
	rm -f $(INSTALLDIR)/freeradius3/usr/bin/smbencrypt
