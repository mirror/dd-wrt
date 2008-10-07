php5-configure:
	cd php5 && './configure' '--host=$(ARCH)-linux' '--program-prefix=' '--program-suffix=' '--prefix=/usr' '--exec-prefix=/usr' '--bindir=/usr/bin' '--datadir=/usr/share' '--includedir=/usr/include' '--infodir=/usr/share/info' '--libdir=/usr/lib' '--libexecdir=/usr/lib' '--localstatedir=/var' '--mandir=/usr/share/man' '--sbindir=/usr/sbin' '--sysconfdir=/etc' '--disable-nls' '--disable-shared' '--disable-static' '--disable-rpath' '--disable-debug' '--without-pear' '--disable-spl' '--with-config-file-path=/etc' '--disable-ipv6' '--enable-magic-quotes' '--enable-memory-limit' '--disable-short-tags' '--disable-ctype' '--disable-dom' '--disable-ftp' '--without-gettext' '--without-iconv' '--disable-xml' '--disable-xmlreader' '--disable-xmlwriter' '--disable-libxml' '--without-libxml-dir' '--disable-mbstring' '--disable-mbregex' '--without-openssl' '--without-sqlite' '--disable-pdo' '--with-kerberos=no' '--disable-simplexml' '--disable-soap' '--enable-sockets' '--disable-tokenizer' '--without-curl' '--without-gd' '--without-freetype-dir' '--without-xpm-dir' '--without-ttf' '--without-t1lib' '--disable-gd-jis-conv' '--enable-cli' '--disable-cgi' '--disable-fastcgi' '--enable-force-cgi-redirect' '--enable-discard-path'

php5:
	make -j 4 -C php5

php5-clean:
	if test -e "php5/Makefile"; then make -C php5 clean; fi

php5-install:
	install -D php5/sapi/cli/php $(INSTALLDIR)/php5/usr/bin/php
	$(STRIP) $(INSTALLDIR)/php5/usr/bin/php

