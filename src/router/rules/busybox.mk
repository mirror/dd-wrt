busybox-config: 
	cd busybox && rm -f Config.h && ln -sf configs/$(CONFIG_BUSYBOX_CONFIG).h Config.h

busybox: busybox-config net-tools bird dhcpforwarder
ifeq ($(ARCH),mipsel)
	cp busybox/.config_std busybox/.config
ifeq ($(CONFIG_MMC),y)
	echo CONFIG_MKFS_EXT2=y >> busybox/.config
else
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
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
	echo CONFIG_MKFS_EXT2=y >> busybox/.config
else
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
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
	echo CONFIG_MKFS_EXT2=y >> busybox/.config
else
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
endif
endif
ifeq ($(ARCHITECTURE),rt2880)
	cp busybox/.config_fonera busybox/.config
ifeq ($(CONFIG_USB),y)
	echo CONFIG_MKFS_EXT2=y >> busybox/.config
else
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
endif
endif
endif


ifeq ($(ARCH),i386)
	cp busybox/.config_wrap busybox/.config
endif
ifeq ($(ARCH),x86_64)
	cp busybox/.config_wrap busybox/.config
endif
ifeq ($(ARCHITECURE),rb532)
	cp busybox/.config_rb532 busybox/.config
endif
ifeq ($(ARCH),mips)
ifeq ($(ARCHITECTURE),fonera)
	cp busybox/.config_fonera busybox/.config
ifeq ($(CONFIG_MMC),y)
	echo CONFIG_MKFS_EXT2=y >> busybox/.config
else
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
endif
else
ifeq ($(ARCHITECTURE),mr3202a)
ifeq ($(CONFIG_DIST),"micro")
	cp busybox/.config_micro_atheros busybox/.config
else
	cp busybox/.config_fonera busybox/.config
endif
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),dir300)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),whrag108)
ifeq ($(CONFIG_DIST),"micro")
	cp busybox/.config_micro_atheros busybox/.config
else
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
endif
else
ifeq ($(ARCHITECTURE),ca8)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),rcaa01)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),ls5)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),eoc5610)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),eoc2610)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),ls2)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),wrt54g2v11)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),bs2)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),bwrg1000)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),eoc2610)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),lsx)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),ja76pf)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),ap83)
	cp busybox/.config_ap83 busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),dir825)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),wrt400)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),wndr3700)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),wzrag300nh)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),wzrg450)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),hornet)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),wasp)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),ubntm)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),unifi)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),whrhpgn)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),jjap93)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),dir615e)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),wr741)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
else
ifeq ($(ARCHITECTURE),danube)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
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
	echo "CONFIG_MKFS_EXT2=y" >> busybox/.config
else
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
endif
endif
ifeq ($(ARCHITECTURE),laguna)
	cp busybox/.config_laguna busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
endif
ifeq ($(ARCHITECTURE),ventana)
	cp busybox/.config_laguna busybox/.config
	sed -i 's/\# CONFIG_UBIUPDATEVOL is not set/CONFIG_UBIUPDATEVOL=y/g' busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
endif
ifeq ($(ARCHITECTURE),northstar)
	cp busybox/.config_laguna busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
endif
ifeq ($(ARCHITECTURE),openrisc)
	cp busybox/.config_storm busybox/.config
	echo "CONFIG_MKFS_EXT2=y" >> busybox/.config
endif
endif
ifeq ($(ARCH),armeb)
ifeq ($(ARCHITECTURE),wrt300nv2)
	cp busybox/.config_fonera busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
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
ifneq ($(CONFIG_DIST),"micro")
	sed -i 's/\# CONFIG_FEATURE_WGET_TIMEOUT is not set/CONFIG_FEATURE_WGET_TIMEOUT=y/g' busybox/.config
endif

#ifeq ($(CONFIG_BUSYBOX_UDHCPC),y)
	sed -i 's/\# CONFIG_UDHCPC is not set/CONFIG_UDHCPC=y/g' busybox/.config
	sed -i 's/\# CONFIG_UDHCP_DEBUG is not set/CONFIG_UDHCP_DEBUG=0/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_UDHCP_RFC3397 is not set/CONFIG_FEATURE_UDHCP_RFC3397=y/g' busybox/.config
	sed -i 's/CONFIG_UDHCPC_DEFAULT_SCRIPT=""/CONFIG_UDHCPC_DEFAULT_SCRIPT=\"\/tmp\/udhcpc\"/g' busybox/.config
	sed -i 's/CONFIG_UDHCPC_SLACK_FOR_BUGGY_SERVERS=0/CONFIG_UDHCPC_SLACK_FOR_BUGGY_SERVERS=80/g' busybox/.config
	sed -i 's/\# CONFIG_IFUPDOWN_UDHCPC_CMD_OPTIONS is not set/CONFIG_IFUPDOWN_UDHCPC_CMD_OPTIONS=""/g' busybox/.config
#endif

ifeq ($(CONFIG_IPV6),y)
	echo "CONFIG_TRACEROUTE6=y" >> busybox/.config
	echo "CONFIG_PING6=y" >> busybox/.config
	echo "CONFIG_FEATURE_IPV6=y" >> busybox/.config
	echo "CONFIG_FEATURE_PREFER_IPV4_ADDRESS=y" >> busybox/.config
else
	echo "# CONFIG_TRACEROUTE6 is not set" >> busybox/.config
	echo "# CONFIG_PING6 is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_IPV6 is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_PREFER_IPV4_ADDRESS is not set" >> busybox/.config
endif
ifeq ($(CONFIG_USB_ADVANCED),y)
	echo "CONFIG_HDPARM=y" >> busybox/.config
	echo "CONFIG_FEATURE_HDPARM_GET_IDENTITY=y" >> busybox/.config
	echo "CONFIG_BLKID=y" >> busybox/.config
	echo "CONFIG_FEATURE_BLKID_TYPE=n" >> busybox/.config
	echo "CONFIG_VOLUMEID=y" >> busybox/.config
	echo "CONFIG_FEATURE_VOLUMEID_EXT=y" >> busybox/.config
	echo "CONFIG_FEATURE_VOLUMEID_BTRFS=y" >> busybox/.config
	echo "CONFIG_FEATURE_VOLUMEID_REISERFS=y" >> busybox/.config
	echo "CONFIG_FEATURE_VOLUMEID_FAT=y" >> busybox/.config
	echo "CONFIG_FEATURE_VOLUMEID_EXFAT=y" >> busybox/.config
	echo "CONFIG_FEATURE_VOLUMEID_HFS=y" >> busybox/.config
	echo "CONFIG_FEATURE_VOLUMEID_JFS=y" >> busybox/.config
	echo "CONFIG_FEATURE_VOLUMEID_XFS=y" >> busybox/.config
	echo "CONFIG_FEATURE_VOLUMEID_NILFS=y" >> busybox/.config
	echo "CONFIG_FEATURE_VOLUMEID_NTFS=y" >> busybox/.config
	echo "CONFIG_FEATURE_VOLUMEID_ISO9660=y" >> busybox/.config
	echo "CONFIG_FEATURE_VOLUMEID_UDF=y" >> busybox/.config
	echo "CONFIG_FEATURE_VOLUMEID_LUKS=y" >> busybox/.config
	echo "CONFIG_FEATURE_VOLUMEID_LINUXSWAP=y" >> busybox/.config
	echo "CONFIG_FEATURE_VOLUMEID_CRAMFS=y" >> busybox/.config
	echo "CONFIG_FEATURE_VOLUMEID_ROMFS=y" >> busybox/.config
	echo "CONFIG_FEATURE_VOLUMEID_SQUASHFS=y" >> busybox/.config
	echo "CONFIG_FEATURE_VOLUMEID_SYSV=y" >> busybox/.config
	echo "CONFIG_FEATURE_VOLUMEID_OCFS2=y" >> busybox/.config
	echo "CONFIG_FEATURE_VOLUMEID_LINUXRAID=y" >> busybox/.config
	echo "CONFIG_FEATURE_MOUNT_LABEL=y" >> busybox/.config	
	echo "CONFIG_FEATURE_MKSWAP_UUID=y" >> busybox/.config
else
	echo "# CONFIG_HDPARM is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_HDPARM_GET_IDENTITY is not set" >> busybox/.config
	echo "# CONFIG_BLKID is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_BLKID_TYPE is not set" >> busybox/.config
	echo "# CONFIG_VOLUMEID is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_EXT is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_BTRFS is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_REISERFS is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_FAT is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_EXFAT is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_HFS is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_JFS is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_XFS is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_NILFS is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_NTFS is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_ISO9660 is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_UDF is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_LUKS is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_LINUXSWAP is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_CRAMFS is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_ROMFS is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_SQUASHFS is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_SYSV is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_OCFS2 is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_LINUXRAID is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_MKSWAP_UUID is not set" >> busybox/.config
endif
ifeq ($(CONFIG_SWAP),y)
	echo "CONFIG_MKSWAP=y" >> busybox/.config
	echo "CONFIG_SWAPONOFF=y" >> busybox/.config
	echo "CONFIG_FEATURE_SWAPON_PRI=y" >> busybox/.config
else
	echo "# CONFIG_MKSWAP is not set" >> busybox/.config
	echo "# CONFIG_SWAPONOFF is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_SWAPON_PRI is not set" >> busybox/.config
endif
	echo "# CONFIG_SHA3SUM is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_KMSG_SYSLOG is not set" >> busybox/.config
	echo "CONFIG_SHA3_SMALL=1" >> busybox/.config

ifeq ($(CONFIG_SMP),y)
	echo "CONFIG_TASKSET=y" >> busybox/.config
	echo "CONFIG_FEATURE_TASKSET_FANCY=y" >> busybox/.config
else
	echo "# CONFIG_TASKSET is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_TASKSET_FANCY is not set" >> busybox/.config
endif

	cd busybox && make oldconfig
	
	$(MAKE) -j 4 -C busybox STRIPTOOL=$(STRIP) PREFIX=$(INSTALLDIR)/busybox

busybox-install:
	$(MAKE) -j 4 -C busybox STRIPTOOL=$(STRIP) PREFIX=$(INSTALLDIR)/busybox install
	rm -f $(INSTALLDIR)/busybox/usr/sbin/httpd-busybox
	[ -e $(INSTALLDIR)/busybox/usr/sbin/httpd ] && mv $(INSTALLDIR)/busybox/usr/sbin/httpd $(INSTALLDIR)/busybox/usr/sbin/httpd-busybox || true

busybox-clean: busybox-config
	$(MAKE) -j 4  -C busybox clean PREFIX=$(INSTALLDIR)/busybox 

busybox-distclean: busybox-config
	$(MAKE) -C busybox clean
	$(MAKE) -C busybox/scripts/config clean
	rm -f busybox/.depend busybox/include/config.h busybox.rb500/scripts/mkdep
