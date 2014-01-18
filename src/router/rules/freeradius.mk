freeradius-configure: openssl
	cd freeradius && \
	sys_lib_dlsearch_path_spec="$(ARCH)-uclibc" \
	sys_lib_search_path_spec="$(ARCH)-uclibc" \
	MYSQL_CONFIG="no" \
	ac_cv_lib_readline=no \
	./configure --host=$(ARCH)-linux-uclibc CFLAGS="$(COPTS) $(MIPS16_OPT) -I$(TOP)/openssl/include " LTCFLAGS="$(COPTS) $(MIPS16_OPT)" LDFLAGS="$(COPTS) $(MIPS16_OPT) -L$(TOP)/openssl" --enable-shared \
	--program-prefix="" \
	--program-suffix="" \
	--prefix=/usr \
	--exec-prefix=/usr \
	--bindir=/usr/bin \
	--datadir=/usr/share \
	--includedir=/usr/include \
	--infodir=/usr/share/info \
	--libdir=/usr/lib \
	--libexecdir=/usr/lib \
	--with-raddbdir=/etc/freeradius2 \
	--with-radacctdir=/var/db/radacct \
	--with-logdir=/var/log \
	--localstatedir=/var \
	--mandir=/usr/share/man \
	--sbindir=/usr/sbin \
	--sysconfdir=/etc \
	--enable-shared \
	--disable-static \
	--disable-developer \
	--with-threads \
	--with-ltdl-include="$(TOP)/freeradius/libltdl/.libs" \
	--with-ltdl-lib="$(TOP)/freeradius/libltdl" \
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
	--without-rlm_sql_postgresql

freeradius:
	make -C freeradius R="$(INSTALLDIR)/freeradius" INSTALLSTRIP="" all

freeradius-clean:
	make -C freeradius clean

freeradius-install:
	make -C freeradius install R=$(INSTALLDIR)/freeradius
	rm -rf $(INSTALLDIR)/freeradius/usr/share/doc
	rm -rf $(INSTALLDIR)/freeradius/usr/share/man
	rm -rf $(INSTALLDIR)/freeradius/usr/include
	rm -rf $(INSTALLDIR)/freeradius/var
	rm -f $(INSTALLDIR)/freeradius/usr/lib/*.la
	rm -f $(INSTALLDIR)/freeradius/etc/sql
	mkdir -p $(INSTALLDIR)/freeradius/etc/config
	cp freeradius/config/freeradius.nvramconfig $(INSTALLDIR)/freeradius/etc/config



