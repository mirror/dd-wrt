java-configure:
	cd $(TOP)/java/classpath && ./configure \
	--host=$(ARCH)-linux CC="$(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(MIPS16_OPT) -DNEED_PRINTF" \
	CCASFLAGS="$(COPTS) -DNEED_PRINTF" \
	--with-gmp="$(TOP)/gmp" \
	--prefix=/usr \
	--without-x \
	--disable-gtk-peer \
	--disable-qt-peer \
	--disable-dssi \
	--disable-plugin \
	--disable-gconf-peer \
	--disable-gjdoc \
	--disable-examples \
	--with-antlr-jar=$(TOP)/java/antlr/antlr-3.4-complete.jar
	
	cd $(TOP)/java/jamvm && ./autogen.sh 
	cd $(TOP)/java/jamvm && ./configure \
	--host=$(ARCH)-linux CC="$(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) -I$(TOP)/zlib -L$(TOP)/zlib -DNEED_PRINTF" \
	CCASFLAGS="$(COPTS) -DNEED_PRINTF" \
	--with-java-runtime-library=gnuclasspath \
	--with-classpath-install-dir=/usr \
	--prefix=/usr \
	--disable-int-inlining \
	--disable-shared \
	--without-pic

java:
	-make -C $(TOP)/java/classpath
	-make -C $(TOP)/java/classpath
	make -C $(TOP)/java/jamvm GLIBJ_ZIP=$(TOP)/java/classpath/lib/glibj.zip


java-clean:
	make -C $(TOP)/java/classpath clean
	make -C $(TOP)/java/jamvm clean
	
	
java-install:
	make -C $(TOP)/java/classpath install DESTDIR=$(INSTALLDIR)/java
	rm -rf $(INSTALLDIR)/java/usr/include
	rm -rf $(INSTALLDIR)/java/usr/share/info
	rm -rf $(INSTALLDIR)/java/usr/share/man
	rm -rf $(INSTALLDIR)/java/usr/bin
	rm -f $(INSTALLDIR)/java/usr/share/classpath/tools.zip
	rm -f $(INSTALLDIR)/java/usr/lib/classpath/*.la
	make -C $(TOP)/java/jamvm install DESTDIR=$(INSTALLDIR)/java
	rm -f $(INSTALLDIR)/java/usr/lib/*.a
	rm -f $(INSTALLDIR)/java/usr/lib/*.la
	rm -rf $(INSTALLDIR)/java/usr/include
	
#	mkdir -p $(INSTALLDIR)/java/usr/lib/classpath
#	mkdir -p $(INSTALLDIR)/java/usr/share/classpath
#	cp java
#	
#	$(CP) \
#		$(PKG_INSTALL_DIR)/usr/lib/security \
#		$(PKG_INSTALL_DIR)/usr/lib/logging.properties \
#		$(1)/usr/lib/
#	$(CP) $(PKG_INSTALL_DIR)/usr/lib/classpath/*.so* $(1)/usr/lib/classpath/
#	$(CP) $(PKG_INSTALL_DIR)/usr/share/classpath/glibj.zip $(1)/usr/share/classpath/
