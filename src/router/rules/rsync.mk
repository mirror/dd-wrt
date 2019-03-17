rsync-configure:
	cd rsync && autoconf
	cd rsync && ./configure --host=$(ARCH)-linux --prefix=/usr --disable-locate --disable-iconv CFLAGS="$(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -Drpl_malloc=malloc" 

rsync:
	$(MAKE) -C rsync

rsync-clean:
	$(MAKE) -C rsync clean

rsync-install:
	$(MAKE) -C rsync install DESTDIR=$(INSTALLDIR)/rsync
	rm -rf $(INSTALLDIR)/rsync/usr/share
	mkdir -p $(INSTALLDIR)/rsync/usr/sbin
	cd $(INSTALLDIR)/rsync/usr/sbin && ln -s ../bin/rsync rsyncd
	install -D rsync/config/rsync.webnas $(INSTALLDIR)/rsync/etc/config/05rsync.webnas
	install -D rsync/config/rsync.nvramconfig $(INSTALLDIR)/rsync/etc/config/rsync.nvramconfig
