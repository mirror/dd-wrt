java-configure: zlib
	cd $(TOP)/java/classpath && ./configure \
	--host=$(ARCH)-linux CC="$(ARCH)-linux-uclibc-gcc" \
	CFLAGS=" $(COPTS) $(LTO) $(MIPS16_OPT) -DNEED_PRINTF" \
	CCASFLAGS="$(COPTS) -DNEED_PRINTF" \
	--with-gmp="$(TOP)/gmp" \
	--prefix=/usr \
	--disable-Werror \
	--without-x \
	--disable-gtk-peer \
	--disable-qt-peer \
	--disable-dssi \
	--libdir=/usr/lib \
	--disable-plugin \
	--disable-gconf-peer \
	--disable-gjdoc \
	--disable-examples \
	--with-antlr-jar=$(TOP)/java/antlr/antlr-3.4-complete.jar \
	AR_FLAGS="cru $(LTOPLUGIN)" \
	RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"
	
	cd $(TOP)/java/jamvm && ./autogen.sh 
	cd $(TOP)/java/jamvm && ./configure \
	--host=$(ARCH)-linux CC="$(ARCH)-linux-uclibc-gcc" \
	CFLAGS="$(COPTS) $(LTO) -I$(TOP)/zlib -L$(TOP)/zlib -DNEED_PRINTF" \
	CCASFLAGS="$(COPTS) -DNEED_PRINTF" \
	--with-java-runtime-library=gnuclasspath \
	--with-classpath-install-dir=/usr \
	--prefix=/usr \
	--libdir=/usr/lib \
	--disable-int-inlining \
	--disable-shared \
	--without-pic \
	AR_FLAGS="cru $(LTOPLUGIN)" \
	RANLIB="$(ARCH)-linux-ranlib $(LTOPLUGIN)"

#	cd $(TOP)/java/cacao && ./configure \
#	--host=$(ARCH)-linux CC="$(ARCH)-linux-uclibc-gcc" \
#	CFLAGS="$(COPTS) -I$(TOP)/zlib -L$(TOP)/zlib -DNEED_PRINTF -fPIC" \
#	CXXFLAGS="$(COPTS) -I$(TOP)/zlib -L$(TOP)/zlib -DNEED_PRINTF -fPIC" \
#	CPPFLAGS="$(COPTS) -I$(TOP)/zlib -L$(TOP)/zlib -DNEED_PRINTF -fPIC" \
#	CCASFLAGS="$(COPTS) -DNEED_PRINTF" \
#	--with-java-runtime-library=gnuclasspath \
#	--with-java-runtime-library-classes=$(TOP)/java/classpath/lib/glibj.zip \
#	--with-classpath-install-dir=/usr \
#	--prefix=/usr \
#	--enable-softfloat \
#	--libdir=/usr/lib \
#	--disable-int-inlining \
#	--disable-shared \
#	--without-pic \
#	--with-jasmin-jar=$(TOP)/java/jasmin/cup.jar:$(TOP)/java/jasmin/jasmin-sable.jar \
#	--with-junit-jar=$(TOP)/java/junit/junit4.jar

java: zlib
	make -C $(TOP)/java/classpath
	make -C $(TOP)/java/classpath
	make -C $(TOP)/java/jamvm GLIBJ_ZIP=$(TOP)/java/classpath/lib/glibj.zip
	#make -C $(TOP)/java/cacao


java-clean:
	make -C $(TOP)/java/classpath clean
	make -C $(TOP)/java/jamvm clean
	-make -C $(TOP)/java/cacao clean
	
	
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

#	make -C $(TOP)/java/cacao install DESTDIR=$(INSTALLDIR)/java
#	rm -rf $(INSTALLDIR)/java/usr/include
#	rm -f $(INSTALLDIR)/java/usr/lib/*.a
#	rm -f $(INSTALLDIR)/java/usr/lib/*.la
#	rm -rf $(INSTALLDIR)/java/usr/lib/pkgconfig
#	rm -rf $(INSTALLDIR)/java/usr/share/gc
#	rm -rf $(INSTALLDIR)/java/usr/share/libatomic_ops
#	rm -rf $(INSTALLDIR)/java/usr/share/man
