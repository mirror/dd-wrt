
aoss2-configure:
	@t:

aoss2: libutils
	$(MAKE) -C aoss2 all

aoss2-install:
ifeq ($(CONFIG_AOSS2),y)
	mkdir -p $(INSTALLDIR)/aoss2/usr/sbin; true;
	install -D aoss2/aoss2d $(INSTALLDIR)/aoss2/usr/sbin/aoss2d
	install -D aoss2/adb $(INSTALLDIR)/aoss2/usr/sbin/adb
endif

aoss2-clean:
	$(MAKE) -C aoss2 clean
