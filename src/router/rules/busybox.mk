busybox-config: 
	cd busybox && rm -f Config.h && ln -sf configs/$(CONFIG_BUSYBOX_CONFIG).h Config.h

busybox: busybox-config net-tools bird dhcpforwarder
ifeq ($(ARCH),mipsel)
ifeq ($(ARCHITECTURE),adm5120)
ifeq ($(CONFIG_DIST),"micro")
	cp busybox/.config_micro_atheros busybox/.config
else
	cp busybox/.config_fonera busybox/.config
endif
ifeq ($(CONFIG_MMC),y)
	echo CONFIG_MKE2FS=y >> busybox/.config
else
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
endif
else
ifeq ($(CONFIG_DIST),"micro")
	cp busybox/.config_micro busybox/.config
else
ifeq ($(CONFIG_DIST),"micro-special")
	cp busybox/.config_micro busybox/.config
else
ifeq ($(CONFIG_DIST),"mini")
	cp busybox/.config_mini busybox/.config
else
	cp busybox/.config_std busybox/.config
endif
endif
endif
ifeq ($(CONFIG_BBOX),"mini")
	cp busybox/.config_mini busybox/.config
endif
	cd busybox && make oldconfig
endif
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
ifeq ($(CONFIG_MMC),y)
	echo CONFIG_MKE2FS=y >> busybox/.config
else
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
endif
else
ifeq ($(ARCHITECTURE),mr3202a)
ifeq ($(CONFIG_DIST),"micro")
	cp busybox/.config_micro_atheros busybox/.config
else
	cp busybox/.config_fonera busybox/.config
endif
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),dir300)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),whrag108)
ifeq ($(CONFIG_DIST),"micro")
	cp busybox/.config_micro_atheros busybox/.config
else
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
endif
else
ifeq ($(ARCHITECTURE),ca8)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),rcaa01)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),ls5)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),ls2)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),eoc2610)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),lsx)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),danube)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
else
	cp busybox/.config_3com busybox/.config
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif
ifeq ($(ARCH),arm)
ifeq ($(ARCHITECTURE),storm)
	cp busybox/.config_storm busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
else
	cp busybox/.config_xscale busybox/.config
endif
endif
ifeq ($(ARCH),armeb)
ifeq ($(ARCHITECTURE),wrt300nv2)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
else
	cp busybox/.config_xscale busybox/.config
endif
endif
ifeq ($(ARCH),powerpc)
	cp busybox/.config_powerpc busybox/.config
endif
	cd busybox && make oldconfig

	
	make  -C busybox clean
	rm -f busybox/busybox
	$(MAKE) -j 4 -C busybox STRIPTOOL=$(STRIP) PREFIX=$(INSTALLDIR)/busybox 

busybox-install:
	$(MAKE) -C busybox STRIPTOOL=$(STRIP) PREFIX=$(INSTALLDIR)/busybox install

busybox-clean: busybox-config
	$(MAKE) -C busybox clean PREFIX=$(INSTALLDIR)/busybox 

busybox-distclean: busybox-config
	$(MAKE) -C busybox clean
	$(MAKE) -C busybox/scripts/config clean
	rm -f busybox/.depend busybox/include/config.h busybox.rb500/scripts/mkdep