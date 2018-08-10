readline-configure: 
	cd readline && ./configure --host=$(ARCH)-uclibc-linux --prefix=/usr --disable-static CFLAGS="$(COPTS) $(MIPS16_OPT) -fPIC -fno-strict-aliasing"
	$(MAKE) -C readline clean all
	cd readline/shlib && rm -f libreadline.so

readline:
	$(MAKE) -C readline
	cd readline/shlib && rm -f libreadline.so
	cd readline/shlib && ln -s libreadline.so.7.0 libreadline.so

readline-install:
	rm -rf $(INSTALLDIR)/readline/usr/lib
	$(MAKE) -C readline DESTDIR=$(INSTALLDIR)/readline install
	rm -rf $(INSTALLDIR)/readline/usr/include
	rm -rf $(INSTALLDIR)/readline/usr/info
	rm -rf $(INSTALLDIR)/readline/usr/man
	rm -f $(INSTALLDIR)/readline/usr/lib/*.la
	rm -f $(INSTALLDIR)/readline/usr/lib/*.a
	rm -f $(INSTALLDIR)/readline/usr/lib/libhistory*

readline-clean:
	-if test -e "readline/Makefile"; then $(MAKE) -C readline clean; fi
