busybox-config: 
	cd busybox && rm -f Config.h && ln -sf configs/$(CONFIG_BUSYBOX_CONFIG).h Config.h

busybox: busybox-config rc net-tools
ifeq ($(CONFIG_DIST),"micro")
	cp busybox/.config_micro busybox/.config
	cd busybox && make oldconfig
endif
ifeq ($(CONFIG_DIST),"mini")
	cp busybox/.config_mini busybox/.config
	cd busybox && make oldconfig
else
	cp busybox/.config_std busybox/.config
	cd busybox && make oldconfig
endif
ifeq ($(ARCH),i386)
	cp busybox/.config_wrap busybox/.config
endif
ifeq ($(ARCHITECURE),rb532)
	cp busybox/.config_rb532 busybox/.config
endif
ifeq ($(ARCH),mips)
ifeq ($(ARCHITECTURE),fonera)
	cp busybox/.config_fonera busybox/.config
else
ifeq ($(ARCHITECTURE),whrag108)
	cp busybox/.config_fonera busybox/.config
else
	cp busybox/.config_3com busybox/.config
endif
endif
endif
ifeq ($(ARCH),armeb)
	cp busybox/.config_xscale busybox/.config
endif
	cd busybox && make oldconfig

	
	make  -C busybox clean
	rm -f busybox/busybox
	$(MAKE) -C busybox STRIPTOOL=$(STRIP)

busybox-install:
	$(MAKE) -C busybox STRIPTOOL=$(STRIP) PREFIX=$(INSTALLDIR)/busybox install

busybox-clean: busybox-config
	$(MAKE) -C busybox clean

busybox-distclean: busybox-config
	$(MAKE) -C busybox clean
	$(MAKE) -C busybox/scripts/config clean
	rm -f busybox/.depend busybox/include/config.h busybox.rb500/scripts/mkdep