iproute2:
	$(MAKE) -C iproute2 all

iproute2-install:
	chmod 0644 iproute2/tc/tc
	install -D iproute2/tc/tc $(INSTALLDIR)/iproute2/usr/sbin/tc
ifneq ($(CONFIG_DIST),"micro")
ifneq ($(CONFIG_DIST),"micro-special")
	chmod 0644 iproute2/ip/ip
	install -D iproute2/ip/ip $(INSTALLDIR)/iproute2/usr/sbin/ip
endif
endif
ifeq ($(CONFIG_WSHAPER),y)
	chmod 0644 iproute2/wshaper.htb
	install -D iproute2/wshaper.htb $(INSTALLDIR)/iproute2/usr/sbin/wshaper
else
	@true
endif

ifeq ($(CONFIG_SVQOS),y)
	chmod 0644 iproute2/svqos.htb
	install -D iproute2/svqos.htb $(INSTALLDIR)/iproute2/usr/sbin/svqos
	chmod 0644 iproute2/svqos.hfsc
	install -D iproute2/svqos.hfsc $(INSTALLDIR)/iproute2/usr/sbin/svqos2
else
	@true
endif
