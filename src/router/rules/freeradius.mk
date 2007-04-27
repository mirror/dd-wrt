freeradius:
	cd freeradius && \
	sys_lib_dlsearch_path_spec="$(ARCH)-uclibc" \
	sys_lib_search_path_spec="$(ARCH)-uclibc" \
	MYSQL_CONFIG="no" \
	./configure --host=$(ARCH)-linux-uclibc --enable-shared \
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
	--localstatedir=/var \
	--mandir=/usr/share/man \
	--sbindir=/usr/sbin \
	--sysconfdir=/etc \
	--disable-static \
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
	--with-rlm_eap \
	--without-rlm_eap_sim \
	--without-rlm_example \
	--without-rlm_ippool \
	--without-rlm_krb5 \
	--without-rlm_pam \
	--without-rlm_perl \
	--without-rlm_python \
	--without-rlm_radutmp \
	--without-rlm_smb \
	--with-rlm_sql \
	--with-rlm_sqlcounter \
	--without-rlm_sql_db2 \
	--without-rlm_sql_freetds \
	--without-rlm_sql_iodbc \
	--without-rlm_sql_oracle \
	--without-rlm_sql_sybase \
	--without-rlm_sql_unixodbc \
	--without-rlm_unix \
	--without-rlm_x99-token \
	--without-rlm_sql_postgresql
	make -C freeradius

freeradius-clean:
	make -C freeradius distclean

freeradius-install:
	make -C freeradius install R=$(INSTALLDIR)/freeradius
	rm -rf $(INSTALLDIR)/freeradius/usr/share
	rm -rf $(INSTALLDIR)/freeradius/usr/include
	rm -rf $(INSTALLDIR)/freeradius/var
	rm -f $(INSTALLDIR)/freeradius/usr/lib/*.la



