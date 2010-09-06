busybox-config: 
	cd busybox && rm -f Config.h && ln -sf configs/$(CONFIG_BUSYBOX_CONFIG).h Config.h

busybox: busybox-config net-tools bird dhcpforwarder
ifeq ($(ARCH),mipsel)
	cp busybox/.config_std busybox/.config
ifeq ($(CONFIG_MMC),y)
	echo CONFIG_MKE2FS=y >> busybox/.config
else
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
endif

ifeq ($(CONFIG_DIST),"micro")
	cp busybox/.config_micro busybox/.config
endif

ifeq ($(CONFIG_DIST),"micro-special")
	cp busybox/.config_micro busybox/.config
endif

ifeq ($(CONFIG_DIST),"mini")
	cp busybox/.config_mini busybox/.config
ifeq ($(CONFIG_MMC),y)
	echo CONFIG_MKE2FS=y >> busybox/.config
else
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
endif
else
ifeq ($(CONFIG_BBOX),"mini")
	cp busybox/.config_mini busybox/.config
endif
endif

ifeq ($(CONFIG_BCMMODERN),y)
ifeq ($(CONFIG_DIST),"mini")
	cp busybox/.config_bcmmodern_mini busybox/.config
else
	cp busybox/.config_bcmmodern_std busybox/.config
ifeq ($(CONFIG_BBOX),"mini")
	cp busybox/.config_bcmmodern_mini busybox/.config
endif
endif
endif
	
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
endif
ifeq ($(ARCHITECTURE),rt2880)
	cp busybox/.config_fonera busybox/.config
ifeq ($(CONFIG_USB),y)
	echo CONFIG_MKE2FS=y >> busybox/.config
else
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
endif
endif
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
ifeq ($(ARCHITECTURE),ja76pf)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),ap83)
	cp busybox/.config_ap83 busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),dir825)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),wrt400)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),wndr3700)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),ubntm)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),whrhpgn)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),dir615e)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),wr741)
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
cp busybox/.config_xscale busybox/.config
    ifeq ($(ARCHITECTURE),storm)
	cp busybox/.config_storm busybox/.config
	ifeq ($(CONFIG_WBD222),y)
	    echo "CONFIG_MKE2FS=y" >> busybox/.config
	else
	    echo "# CONFIG_MKE2FS is not set" >> busybox/.config
	endif
    endif
    ifeq ($(ARCHITECTURE),laguna)
	cp busybox/.config_laguna busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
    endif
    ifeq ($(ARCHITECTURE),openrisc)
	cp busybox/.config_storm busybox/.config
	echo "CONFIG_MKE2FS=y" >> busybox/.config
    endif
endif
ifeq ($(ARCH),armeb)
    ifeq ($(ARCHITECTURE),wrt300nv2)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
    else
	ifneq ($(CONFIG_WP18),y)
	    cp busybox/.config_xscale busybox/.config
	else
	    cp busybox/.config_xscale_wp18 busybox/.config
	endif
    endif
endif
ifeq ($(ARCH),powerpc)
	cp busybox/.config_powerpc busybox/.config
endif
ifeq ($(CONFIG_IPV6),y)
	echo "CONFIG_PING6=y" >> busybox/.config
	echo "CONFIG_FEATURE_IPV6=y" >> busybox/.config
	echo "CONFIG_FEATURE_PREFER_IPV4_ADDRESS=y" >> busybox/.config
else
	echo "# CONFIG_PING6 is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_IPV6 is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_PREFER_IPV4_ADDRESS is not set" >> busybox/.config
endif
	cd busybox && make oldconfig
	
	$(MAKE) -j 4 -C busybox STRIPTOOL=$(STRIP) PREFIX=$(INSTALLDIR)/busybox ; \

busybox-install:
	$(MAKE) -j 4 -C busybox STRIPTOOL=$(STRIP) PREFIX=$(INSTALLDIR)/busybox install

busybox-clean: busybox-config
	$(MAKE) -j 4  -C busybox clean PREFIX=$(INSTALLDIR)/busybox 

busybox-distclean: busybox-config
	$(MAKE) -C busybox clean
	$(MAKE) -C busybox/scripts/config clean
	rm -f busybox/.depend busybox/include/config.h busybox.rb500/scripts/mkdep