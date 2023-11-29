icu-configure:
	-make -C icu clean
	rm -f icu/config.cache
	rm -rf icu/autom4te.cach
	cd icu && autoconf
	cd icu &&  ./runConfigureICU Linux/gcc CFLAGS= CXXFLAGS= \
	--disable-debug \
	--enable-release \
	--enable-shared \
	--enable-static \
	--enable-draft \
	--enable-renaming \
	--disable-tracing \
	--disable-extras \
	--enable-dyload \
	--libdir=$(TOP)/icu/staging/lib \
	--prefix=$(TOP)/icu/staging
	make -C icu
	make -C icu install
	mkdir -p $(TOP)/icu/staging/share/icu/68.1/lib/
	mkdir -p $(TOP)/icu/staging/share/icu/68.1/bin/
	mkdir -p $(TOP)/icu/staging/share/icu/68.1/config/
	cp -fpR  $(TOP)/icu/config/icucross.* $(TOP)/icu/staging/share/icu/68.1/config/
	cp -fpR  $(TOP)/icu/bin/icupkg $(TOP)/icu/staging/share/icu/68.1/bin/
	cp -fpR  $(TOP)/icu/bin/pkgdata $(TOP)/icu/staging/share/icu/68.1/bin/
	cp -fpR  $(TOP)/icu/lib/*.so* $(TOP)/icu/staging/share/icu/68.1/lib/

	make -C icu clean
	rm -f icu/config.cache
	rm -rf icu/autom4te.cach
	cd icu && autoconf
	cd icu && ./configure CFLAGS="$(COPTS)  $(MIPS16_OPT) -DNEED_PRINTF"  CXXFLAGS="$(COPTS)  $(MIPS16_OPT) -DNEED_PRINTF" \
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	CXX="ccache $(ARCH)-linux-uclibc-g++" \
	--target=$(ARCH)-linux-uclibc \
	--host=$(ARCH)-linux-uclibc \
	--disable-debug \
	--enable-release \
	--enable-shared \
	--enable-static \
	--enable-draft \
	--enable-renaming \
	--disable-tracing \
	--disable-extras \
	--enable-dyload \
	--with-data-packaging=archive \
	--disable-tools \
	--disable-tests \
	--disable-samples \
	--libdir=$(TOP)/icu/target_staging/lib \
	--with-cross-build="$(TOP)/icu/staging/share/icu/68.1" \
	--prefix=$(TOP)/icu/target_staging
	make -C icu install

#	make -C icu clean
#	rm -f icu/config.cache
#	rm -rf icu/autom4te.cach
#	cd icu && autoconf
#	cd icu && ./configure CFLAGS="$(COPTS)  $(MIPS16_OPT) -DNEED_PRINTF"  CXXFLAGS="$(COPTS)  $(MIPS16_OPT) -DNEED_PRINTF" \
#	CC="ccache $(ARCH)-linux-uclibc-gcc" \
#	CXX="ccache $(ARCH)-linux-uclibc-g++" \
#	--target=$(ARCH)-linux-uclibc \
#	--host=$(ARCH)-linux-uclibc \
#	--disable-debug \
#	--enable-release \
#	--enable-shared \
#	--enable-static \
#	--enable-draft \
#	--enable-renaming \
#	--disable-tracing \
#	--disable-extras \
#	--enable-dyload \
#	--disable-tools \
#	--disable-tests \
#	--disable-samples \
#	--with-cross-build="$(TOP)/icu/staging/share/icu/68.1" \
#	--prefix=/usr

icu:
	make -C icu
	make -C icu install

icu-clean:
	make -C icu clean

icu-install:
	mkdir -p $(INSTALLDIR)/icu/usr/lib
	-cp -fpR $(TOP)/icu/target_staging/lib/*.so* $(INSTALLDIR)/icu/usr/lib/


php8: libxml2 libmcrypt glib20 zlib libzip openssl sqlite libgd
	CC="ccache $(ARCH)-linux-uclibc-gcc" \
	PROF_FLAGS="$(COPTS) -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CFLAGS="$(COPTS) -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -I$(TOP)/libpng -fno-builtin -I$(TOP)/libxml2/include -I$(TOP)/curl/include -I$(TOP)/zlib/include -I$(TOP)/openssl/include -I$(TOP)/libzip -I$(TOP)/libzip/lib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	CPPFLAGS="$(COPTS) -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -I$(TOP)/libpng -fno-builtin -I$(TOP)/libxml2/include -I$(TOP)/curl/include -I$(TOP)/zlib/include -I$(TOP)/openssl/include -I$(TOP)/libzip -I$(TOP)/libzip/lib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	LDFLAGS="$(COPTS) -ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/libpng/.libs -L$(TOP)/libxml2/.libs -L$(TOP)/zlib -L$(TOP)/openssl -L$(TOP)/zlib -L$(TOP)/curl/build/lib/.libs -fPIC" \
	LIBS="-lssl -lcrypto" \
	$(MAKE) -C php8
	
	
PHP_CONFIGURE_ARGS= \
	--program-prefix= \
	--program-suffix= \
	--prefix=/usr \
	--exec-prefix=/usr \
	--bindir=/usr/bin \
	--datadir=/usr/share \
	--infodir=/usr/share/info \
	--includedir=/ \
	--oldincludedir=/ \
	--libdir=/usr/lib \
	--libexecdir=/usr/lib \
	--localstatedir=/var \
	--mandir=/usr/share/man \
	--sbindir=/usr/sbin \
	--sysconfdir=/etc \
	--with-config-file-scan-dir=/jffs/etc \
	--enable-shared \
	--enable-static \
	--disable-rpath \
	--disable-debug \
	--without-pear \
	--with-libxml-dir="$(TOP)/libxml2" \
	--with-config-file-path=/etc \
	--disable-short-tags \
	--disable-ftp \
	--without-gettext \
	--disable-mbregex \
	--with-openssl-dir="$(TOP)/openssl" \
	--with-openssl=shared,"$(TOP)/openssl" \
	--with-kerberos=no \
	--disable-phar \
	--with-kerberos=no \
	--disable-soap \
	--enable-sockets \
	--disable-tokenizer \
	--without-freetype-dir \
	--without-xpm-dir \
	--without-t1lib \
	--disable-gd-jis-conv \
	--enable-cli \
	--enable-cgi \
	--enable-zip \
	--with-zip \
	--enable-gd \
	--with-libzip=$(TOP)/libzip/lib \
	--disable-intl \
	--enable-mbstring \
	--enable-maintainer-zts \
	--with-tsrm-pthreads \
	--with-gd \
	--with-zlib \
	--with-zlib-dir="$(TOP)/zlib" \
	--with-png-dir="$(TOP)/libpng/.libs" \
	--with-jpeg-dir="$(TOP)/minidlna/jpeg-8" \
	--with-mcrypt="$(TOP)/libmcrypt" \
	--with-curl="$(TOP)/curl" \
	php_cv_cc_rpath="no" \
	iconv_impl_name="glibc" \
	ac_cv_lib_png_png_write_image="yes" \
	ac_cv_lib_crypt_crypt="yes" \
	ac_cv_lib_z_gzgets="yes" \
	ac_cv_php_xml2_config_path="$(TOP)/libxml2/xml2-config" \
	ac_cv_lib_z_gzgets="yes" \
	ac_cv_lib_crypto_X509_free="yes" \
	ac_cv_lib_ssl_DSA_get_default_method="yes" \
	ac_cv_func_crypt="yes" \
	ac_cv_lib_crypto_CRYPTO_free="yes" \
	ac_cv_header_openssl_crypto_h="yes" \
	ac_cv_lib_ssl_SSL_CTX_set_ssl_version="yes" \
	ac_cv_glob="yes" \
	ac_cv_lib_mcrypt_mcrypt_module_open="yes" \
	SQLITE_CFLAGS="-I$(TOP)/sqlite" \
	SQLITE_LIBS="-L$(TOP)/sqlite/.libs -lsqlite3" \
	LIBXML_CFLAGS="-I$(TOP)/libxml2/include" \
	LIBXML_LIBS="-L$(TOP)/libxml2/.libs -lxml2" \
	ZLIB_CFLAGS="-I$(TOP)/zlib/include" \
	ZLIB_LIBS="-L$(TOP)/zlib -lz" \
	CURL_CFLAGS="-I$(TOP)/curl/include -I$(TOP)/libpng" \
	CURL_LIBS="-L$(TOP)/curl/build/lib/.libs -lcurl" \
	OPENSSL_LIBDIR="$(TOP)/openssl" \
	OPENSSL_LIBS="-L$(TOP)/openssl -lssl -lcrypto" \
	OPENSSL_CFLAGS="-I$(TOP)/openssl/include" \
	LIBZIP_CFLAGS="-I$(TOP)/libzip/lib -I$(TOP)/libzip" \
	LIBZIP_LIBS="-L$(TOP)/libzip/lib -lzip" \
	PHP_OPENSSL_DIR="$(TOP)/openssl" \
	PHP_SETUP_OPENSSL="$(TOP)/openssl" \
	PHP_CURL="$(TOP)/curl" \
	LIBS="-lc -lpthread -lm -lssl -lcrypto" \
	CFLAGS="$(COPTS) -DNEED_PRINTF -fno-builtin -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include" \
	CXXFLAGS="$(COPTS) -DNEED_PRINTF -fno-builtin -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include" \
	EXTRA_CFLAGS="-fno-builtin -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -I$(TOP)/sqlite -I$(TOP)/libmcrypt -I$(TOP)/zlib  -I$(TOP)/zlib/include -I$(TOP)/libpng -L$(TOP)/openssl -I$(TOP)/openssl/include  -I$(TOP)/curl/include -I$(TOP)/libzip -I$(TOP)/libzip/lib -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	EXTRA_LIBS="-L$(TOP)/openssl -lsqlite3 -lcurl -lcrypto -lssl -lcrypt -lxml2 -lmcrypt -lpng16 -lgd -lz" \
	EXTRA_LDFLAGS="-L$(TOP)/libmcrypt/lib/.libs -L$(TOP)/sqlite/.libs -L$(TOP)/minidlna/jpeg-8/.libs -L$(TOP)/libxml2/.libs -L$(TOP)/zlib -L$(TOP)/libpng/.libs -L$(TOP)/libgd/src/.libs -L$(TOP)/openssl -L$(TOP)/zlib -L$(TOP)/curl/build/lib/.libs -ffunction-sections -fdata-sections -Wl,--gc-sections" \
	EXTRA_LDFLAGS_PROGRAM="-L$(TOP)/libmcrypt/lib/.libs -L$(TOP)/sqlite/.libs -L$(TOP)/libxml2/.libs -L$(TOP)/libpng/.libs -L$(TOP)/libgd/src/.libs -L$(TOP)/openssl -L$(TOP)/zlib -L$(TOP)/curl/build/lib/.libs"

ifeq ($(ARCH),mips64)
PHP_ENDIAN=ac_cv_c_bigendian_php="yes"
endif
ifeq ($(ARCH),mips)
PHP_ENDIAN=ac_cv_c_bigendian_php="yes"
endif
ifeq ($(ARCH),armeb)
PHP_ENDIAN=ac_cv_c_bigendian_php="yes"
endif
ifeq ($(ARCH),powerpc)
PHP_ENDIAN=ac_cv_c_bigendian_php="yes"
endif

	
php8-configure: libpng-configure libpng libgd-configure libgd libxml2-configure libxml2 zlib-configure zlib curl-configure curl glib20-configure glib20 libzip-configure libzip openssl sqlite
	rm -f php8/config.cache
	rm -rf php8/autom4te.cach
	cd php8 && touch configure.ac && autoconf
	cd php8 && './configure'  '--host=$(ARCH)-linux-uclibc' $(PHP_ENDIAN) $(PHP_CONFIGURE_ARGS) \
	'CFLAGS=$(COPTS) -fno-builtin -ffunction-sections -fdata-sections -Wl,--gc-sections -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -I$(TOP)/minidlna/jpeg-8 -I$(TOP)/zlib/include  -L$(TOP)/sqlite/.libs -I$(TOP)/libmcrypt -I$(TOP)/libpng -I$(TOP)/libxml2/include -I$(TOP)/openssl/include -I$(TOP)/curl/include -DNEED_PRINTF -L$(TOP)/zlib -L$(TOP)/curl/build/lib/.libs  -I$(TOP)/libzip -I$(TOP)/libzip/lib' \
	'LDFLAGS=-ffunction-sections -fdata-sections -Wl,--gc-sections -L$(TOP)/minidlna/jpeg-8/.libs -L$(TOP)/sqlite/.libs -L$(TOP)/libmcrypt/lib/.libs -L$(TOP)/libxml2/.libs -L$(TOP)/zlib -L$(TOP)/libpng/.libs -L$(TOP)/libgd/src/.libs -L$(TOP)/openssl -L$(TOP)/zlib -L$(TOP)/curl/build/lib/.libs' \
	'CXXFLAGS=$(COPTS) -fno-builtin -std=c++0x -DNEED_PRINTF'
	printf "#define HAVE_GLOB 1\n" >>$(TOP)/php8/main/php_config.h
	sed -i 's/-L\/lib/-L\/dummy\/lib/g' $(TOP)/php8/Makefile
	sed -i 's/-lltdl/ /g' $(TOP)/php8/Makefile
	sed -i 's/-I\/usr\/include/-I\/dummy\/usr\/include/g' $(TOP)/php8/Makefile

php8-clean:
	if test -e "php8/Makefile"; then make -C php8 clean; fi

php8-install:
	install -D php8/sapi/cli/.libs/php $(INSTALLDIR)/php8/usr/bin/php
	cd $(INSTALLDIR)/php8/usr/bin && ln -sf php php-cgi
	mkdir -p $(INSTALLDIR)/php8/etc/php/modules
	cp php8/modules/*.so $(INSTALLDIR)/php8/etc/php/modules
	printf "short_open_tag=on\ncgi.fix_pathinfo=1\n" >$(INSTALLDIR)/php8/etc/php.ini
	printf "post_max_size = 32M\n" >>$(INSTALLDIR)/php8/etc/php.ini
	printf "upload_max_filesize = 32M\n" >>$(INSTALLDIR)/php8/etc/php.ini
	printf "output_buffering = Off\n" >>$(INSTALLDIR)/php8/etc/php.ini
	printf "extension_dir = /etc/php/modules\n" >>$(INSTALLDIR)/php8/etc/php.ini
	printf "extension = openssl.so\n" >>$(INSTALLDIR)/php8/etc/php.ini
	printf "zend_extension = opcache.so\n" >>$(INSTALLDIR)/php8/etc/php.ini
