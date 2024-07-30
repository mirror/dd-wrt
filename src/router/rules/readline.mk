readline-configure: 
	cd readline && ./configure --host=$(ARCH)-uclibc-linux --prefix=/usr --libdir=/usr/lib --disable-static CFLAGS="$(COPTS) $(MIPS16_OPT) -fPIC -fno-strict-aliasing" LDFLAGS="$(COPTS)"
	make -C readline clean all
	cd readline/shlib && rm -f libreadline.so
	cd readline/shlib && rm -f libhistory.so

readline:
	make -C readline
	cd readline/shlib && rm -f libreadline.so
	cd readline/shlib && rm -f libhistory.so
	cd readline/shlib && ln -s libreadline.so.7.0 libreadline.so
	cd readline/shlib && ln -s libhistory.so.7.0 libhistory.so

readline-install:
	rm -rf $(INSTALLDIR)/readline/usr/lib
	make -C readline DESTDIR=$(INSTALLDIR)/readline install
	rm -rf $(INSTALLDIR)/readline/usr/include
	rm -rf $(INSTALLDIR)/readline/usr/info
	rm -rf $(INSTALLDIR)/readline/usr/man
	rm -rf $(INSTALLDIR)/readline/usr/share
	rm -rf $(INSTALLDIR)/readline/usr/bin
	rm -f $(INSTALLDIR)/readline/usr/lib/*.la
	rm -f $(INSTALLDIR)/readline/usr/lib/*.a
ifneq ($(CONFIG_SOFTETHER),y)
	rm -f $(INSTALLDIR)/readline/usr/lib/libhistory*
endif

readline-clean:
	-if test -e "readline/Makefile"; then make -C readline clean; fi
