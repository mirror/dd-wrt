iproute2:
	$(MAKE) -C iproute2 all

iproute2-install:
	chmod 0644 iproute2/tc/tc
	install -D iproute2/tc/tc $(INSTALLDIR)/iproute2/usr/sbin/tc
ifeq ($(CONFIG_WSHAPER),y)
	chmod 0644 iproute2/wshaper.htb
	install -D iproute2/wshaper.htb $(INSTALLDIR)/iproute2/usr/sbin/wshaper
else
	@true
endif

ifeq ($(CONFIG_SVQOS),y)
	chmod 0644 iproute2/svqos.htb
	install -D iproute2/svqos $(INSTALLDIR)/iproute2/usr/sbin/svqos
else
	@true
endif
