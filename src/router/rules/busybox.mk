busybox-config: -
	cd busybox && rm -f Config.h && ln -sf configs/$(CONFIG_BUSYBOX_CONFIG).h Config.h

busybox: busybox-config nvram
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
ifeq ($(CONFIG_MT7621),y)	
	cp busybox/.config_bcmmodern_std busybox/.config
	sed -i 's/\# CONFIG_FEATURE_USE_TERMIOS is not set/CONFIG_FEATURE_USE_TERMIOS=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_TOP_SMP_CPU is not set/CONFIG_FEATURE_TOP_SMP_CPU=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_TOP_SMP_PROCESS is not set/CONFIG_FEATURE_TOP_SMP_PROCESS=y/g' busybox/.config
endif
endif
endif


ifeq ($(ARCH),i386)
	cp busybox/.config_wrap busybox/.config
endif
ifeq ($(ARCH),mips64)
	cp busybox/.config_fonera busybox/.config
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
ifeq ($(ARCH),aarch64)
	cp busybox/.config_laguna busybox/.config
	sed -i 's/\# CONFIG_UBIUPDATEVOL is not set/CONFIG_UBIUPDATEVOL=y/g' busybox/.config
	echo "# CONFIG_MKFS_EXT2 is not set" >> busybox/.config
	sed -i 's/\# CONFIG_FEATURE_TOP_SMP_CPU is not set/CONFIG_FEATURE_TOP_SMP_CPU=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_TOP_SMP_PROCESS is not set/CONFIG_FEATURE_TOP_SMP_PROCESS=y/g' busybox/.config
	echo "CONFIG_FEATURE_TOP_INTERACTIVE=y" >> busybox/.config
	echo "CONFIG_FEATURE_POWERTOP_INTERACTIVE=y" >> busybox/.config
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
ifeq ($(ARCHITECTURE),mvebu)	
	cp busybox/.config_bcmmodern_std busybox/.config
	sed -i 's/\# CONFIG_FEATURE_USE_TERMIOS is not set/CONFIG_FEATURE_USE_TERMIOS=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_TOP_SMP_CPU is not set/CONFIG_FEATURE_TOP_SMP_CPU=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_TOP_SMP_PROCESS is not set/CONFIG_FEATURE_TOP_SMP_PROCESS=y/g' busybox/.config
	echo "CONFIG_FEATURE_TOP_INTERACTIVE=y" >> busybox/.config
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

ifeq ($(CONFIG_BUSYBOX_NTPD),y)
	sed -i 's/\# CONFIG_NTPD is not set/CONFIG_NTPD=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_NTPD_SERVER is not set/CONFIG_FEATURE_NTPD_SERVER=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_NTPD_CONF is not set/CONFIG_FEATURE_NTPD_CONF=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_NTPD_AUTH is not set/CONFIG_FEATURE_NTPD_AUTH=y/g' busybox/.config
	echo "CONFIG_FEATURE_NTPD_CONF=y" >> busybox/.config
	echo "CONFIG_FEATURE_NTPD_AUTH=y" >> busybox/.config
endif

ifeq ($(CONFIG_IPV6),y)
	sed -i 's/\# CONFIG_FEATURE_IPV6 is not set/CONFIG_FEATURE_IPV6=y/g' busybox/.config
	echo "CONFIG_TRACEROUTE6=y" >> busybox/.config
	echo "CONFIG_PING6=y" >> busybox/.config
	echo "CONFIG_FEATURE_IPV6=y" >> busybox/.config
	echo "CONFIG_FEATURE_PREFER_IPV4_ADDRESS=y" >> busybox/.config
	sed -i 's/\# CONFIG_TRACEROUTE& is not set/CONFIG_TRACEROUTE&=y/g' busybox/.config
	sed -i 's/\# CONFIG_PING6 is not set/CONFIG_PING6=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_NETSTAT_PRG is not set/CONFIG_FEATURE_NETSTAT_PRG=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_NETSTAT_WIDE is not set/CONFIG_FEATURE_NETSTAT_WIDE=y/g' busybox/.config
#	sed -i 's/\# CONFIG_UDHCPC6 is not set/CONFIG_UDHCPC6=y/g' busybox/.config
##	echo "CONFIG_FEATURE_UDHCPC6_RFC3646=y" >> busybox/.config
#	echo "CONFIG_FEATURE_UDHCPC6_RFC4704=y" >> busybox/.config
#	echo "CONFIG_FEATURE_UDHCPC6_RFC4833=y" >> busybox/.config
#	echo "# CONFIG_FEATURE_UDHCPC6_RFC5970 is not set" >> busybox/.config
else
	echo "# CONFIG_TRACEROUTE6 is not set" >> busybox/.config
	echo "# CONFIG_PING6 is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_IPV6 is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_PREFER_IPV4_ADDRESS is not set" >> busybox/.config
endif
ifeq ($(CONFIG_SPEEDCHECKER),y)
	sed -i 's/\# CONFIG_FEATURE_TRACEROUTE_USE_ICMP is not set/CONFIG_FEATURE_TRACEROUTE_USE_ICMP=y/g' busybox/.config
endif
	echo "# CONFIG_HDPARM is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_HDPARM_GET_IDENTITY is not set" >> busybox/.config
ifeq ($(CONFIG_USB_ADVANCED),y)
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
	echo "# CONFIG_FEATURE_VOLUMEID_UBIFS is not set" >> busybox/.config
ifeq ($(CONFIG_SWAP),y)
	echo "CONFIG_MKSWAP=y" >> busybox/.config
	echo "CONFIG_SWAPONOFF=y" >> busybox/.config
	echo "CONFIG_SWAPON=y" >> busybox/.config
	echo "CONFIG_SWAPOFF=y" >> busybox/.config
	echo "CONFIG_FEATURE_SWAPON_PRI=y" >> busybox/.config
	echo "# CONFIG_FEATURE_SWAPON_DISCARD is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_SWAPONOFF_LABEL is not set" >> busybox/.config
else
	echo "# CONFIG_SWAPON is not set" >> busybox/.config
	echo "# CONFIG_SWAPOFF is not set" >> busybox/.config
	echo "# CONFIG_MKSWAP is not set" >> busybox/.config
	echo "# CONFIG_SWAPONOFF is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_SWAPON_PRI is not set" >> busybox/.config
endif
	echo "# CONFIG_SHA3SUM is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_KMSG_SYSLOG is not set" >> busybox/.config
	echo "CONFIG_SHA3_SMALL=1" >> busybox/.config
	echo "CONFIG_SHA1_SMALL=3" >> busybox/.config
	echo "CONFIG_SHA1_HWACCEL=y" >> busybox/.config
	echo "CONFIG_SHA256_HWACCEL=y" >> busybox/.config
	echo "# CONFIG_LOOP_CONFIGURE is not set" >> busybox/.config
	echo "CONFIG_NO_LOOP_CONFIGURE=y" >> busybox/.config
	echo "# CONFIG_TRY_LOOP_CONFIGURE is not set" >> busybox/.config

ifeq ($(CONFIG_SMP),y)
	sed -i 's/\# CONFIG_TASKSET is not set//g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_TASKSET_FANCY is not set//g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_TASKSET_CPULIST is not set//g' busybox/.config
	echo "CONFIG_TASKSET=y" >> busybox/.config
	sed -i 's/\# CONFIG_FEATURE_SHOW_THREADS is not set/CONFIG_FEATURE_SHOW_THREADS=y/g' busybox/.config
	echo "CONFIG_FEATURE_TASKSET_FANCY=y" >> busybox/.config
	echo "CONFIG_FEATURE_TASKSET_CPULIST=y" >> busybox/.config
else
ifeq ($(CONFIG_X86),y)
	sed -i 's/\# CONFIG_TASKSET is not set//g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_TASKSET_FANCY is not set//g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_TASKSET_CPULIST is not set//g' busybox/.config
	echo "CONFIG_TASKSET=y" >> busybox/.config
	sed -i 's/\# CONFIG_FEATURE_SHOW_THREADS is not set/CONFIG_FEATURE_SHOW_THREADS=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_VERBOSE is not set/CONFIG_FEATURE_VERBOSE=y/g' busybox/.config
	echo "CONFIG_FEATURE_TASKSET_FANCY=y" >> busybox/.config
	echo "CONFIG_FEATURE_TASKSET_CPULIST=y" >> busybox/.config
else
	echo "# CONFIG_TASKSET is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_TASKSET_FANCY is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_TASKSET_CPULIST is not set" >> busybox/.config
endif
endif
	echo "# CONFIG_FEATURE_GZIP_LEVELS is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_DD_STATUS is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_SYNC_FANCY is not set" >> busybox/.config
	echo "# CONFIG_TRUNCATE is not set" >> busybox/.config
	echo "CONFIG_UNAME_OSNAME=\"DD-WRT\"" >> busybox/.config
	echo "# CONFIG_UEVENT is not set" >> busybox/.config
	echo "# CONFIG_I2CGET is not set" >> busybox/.config
	echo "# CONFIG_I2CSET is not set" >> busybox/.config
	echo "# CONFIG_I2CDUMP is not set" >> busybox/.config
	echo "# CONFIG_I2CDETECT is not set" >> busybox/.config
	echo "# CONFIG_I2CTRANSFER is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_LESS_TRUNCATE is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_WGET_OPENSSL is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_WGET_SSL_HELPER is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_MOUNT_OTHERTAB is not set" >> busybox/.config
	echo "CONFIG_FEATURE_IP_ROUTE_DIR=\"/etc/iproute2\"" >> busybox/.config
	sed -i 's/\# CONFIG_ASH_OPTIMIZE_FOR_SIZE is not set/CONFIG_ASH_OPTIMIZE_FOR_SIZE=y/g' busybox/.config
ifeq ($(CONFIG_TFTP),y)
	sed -i 's/\# CONFIG_TFTP is not set/CONFIG_TFTP=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_TFTP_GET is not set/CONFIG_FEATURE_TFTP_GET=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_TFTP_PUT is not set/CONFIG_FEATURE_TFTP_PUT=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_TFTP_PROGRESS_BAR is not set/CONFIG_FEATURE_TFTP_PROGRESS_BAR=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_TFTP_BLOCKSIZE is not set/CONFIG_FEATURE_TFTP_BLOCKSIZE=y/g' busybox/.config
endif
	echo "# CONFIG_BUSYBOX is not set" >> busybox/.config
	echo "# CONFIG_DEBUG_SANITIZE is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_USE_BSS_TAIL is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_GUNZIP_LONG_OPTIONS is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_CALL_TELINIT is not set" >> busybox/.config
	echo "# CONFIG_LINUXRC is not set" >> busybox/.config
	echo "# CONFIG_MKPASSWD is not set" >> busybox/.config
	echo "# CONFIG_BLKDISCARD is not set" >> busybox/.config
	echo "# CONFIG_NSENTER is not set" >> busybox/.config
	echo "# CONFIG_UNSHARE is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_BCACHE is not set" >> busybox/.config
	echo "# CONFIG_UBIRENAME is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_IP_NEIGH is not set" >> busybox/.config
ifeq ($(CONFIG_BUSYBOX_INETD),y)
	sed -i 's/\# CONFIG_INETD is not set/CONFIG_INETD=y/g' busybox/.config
endif
ifeq ($(CONFIG_HWCLOCK),y)
	sed -i 's/\# CONFIG_HWCLOCK is not set/CONFIG_HWCLOCK=y/g' busybox/.config
	echo "# CONFIG_FEATURE_HWCLOCK_LONG_OPTIONS is not set" >> busybox/.config
	sed -i 's/\# CONFIG_FEATURE_HWCLOCK_ADJTIME_FHS is not set/CONFIG_FEATURE_HWCLOCK_ADJTIME_FHS=y/g' busybox/.config
endif
ifeq ($(CONFIG_EOP_TUNNEL),y)
	sed -i 's/\# CONFIG_SEQ is not set/CONFIG_SEQ=y/g' busybox/.config
endif
ifeq ($(CONFIG_WIREGUARD),y)
	sed -i 's/\# CONFIG_SEQ is not set/CONFIG_SEQ=y/g' busybox/.config
endif
	sed -i 's/\# CONFIG_IP is not set/CONFIG_IP=y/g' busybox/.config
	sed -i 's/\# CONFIG_IPADDR is not set/CONFIG_IPADDR=y/g' busybox/.config
	sed -i 's/\# CONFIG_IPLINK is not set/CONFIG_IPLINK=y/g' busybox/.config
	sed -i 's/\# CONFIG_IPROUTE is not set/CONFIG_IPROUTE=y/g' busybox/.config
	sed -i 's/\# CONFIG_IPTUNNEL is not set/CONFIG_IPTUNNEL=y/g' busybox/.config
	sed -i 's/\# CONFIG_IPRULE is not set/CONFIG_IPRULE=y/g' busybox/.config

	sed -i 's/\# CONFIG_FEATURE_IP_ADDRESS is not set/CONFIG_FEATURE_IP_ADDRESS=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_IP_LINK is not set/CONFIG_FEATURE_IP_LINK=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_IP_ROUTE is not set/CONFIG_FEATURE_IP_ROUTE=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_IP_TUNNEL is not set/CONFIG_FEATURE_IP_TUNNEL=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_IP_RULE is not set/CONFIG_FEATURE_IP_RULE=y/g' busybox/.config
	sed -i 's/\# CONFIG_ASH_EXPAND_PRMT is not set/CONFIG_ASH_EXPAND_PRMT=y/g' busybox/.config
	echo "# CONFIG_ZCAT is not set" >> busybox/.config
	echo "# CONFIG_BZCAT is not set" >> busybox/.config
	echo "# CONFIG_LZCAT is not set" >> busybox/.config
	echo "# CONFIG_XZCAT is not set" >> busybox/.config
	echo "# CONFIG_UNLZOP is not set" >> busybox/.config
	echo "# CONFIG_LZOPCAT is not set" >> busybox/.config
	echo "# CONFIG_TEST1 is not set" >> busybox/.config
	echo "# CONFIG_TEST2 is not set" >> busybox/.config
	echo "# CONFIG_EGREP is not set" >> busybox/.config
	echo "# CONFIG_FGREP is not set" >> busybox/.config
	echo "# CONFIG_MKE2FS is not set" >> busybox/.config
	echo "# CONFIG_MKDOSFS is not set" >> busybox/.config
	echo "# CONFIG_LINUX32 is not set" >> busybox/.config
	echo "# CONFIG_LINUX64 is not set" >> busybox/.config
	echo "# CONFIG_LOCK is not set" >> busybox/.config
	echo "# CONFIG_DNSDOMAINNAME is not set" >> busybox/.config
	echo "# CONFIG_IFUP is not set" >> busybox/.config
	echo "# CONFIG_IFDOWN is not set" >> busybox/.config
	echo "# CONFIG_IPNEIGH is not set" >> busybox/.config
	echo "# CONFIG_NETMSG is not set" >> busybox/.config
	echo "# CONFIG_SVC is not set" >> busybox/.config
	echo "CONFIG_REBOOT=y" >> busybox/.config
ifeq ($(CONFIG_X86),y)
	echo "CONFIG_POWEROFF=y" >> busybox/.config
	sed -i 's/\# CONFIG_FEATURE_USE_TERMIOS is not set/CONFIG_FEATURE_USE_TERMIOS=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_TOP_SMP_CPU is not set/CONFIG_FEATURE_TOP_SMP_CPU=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_TOP_SMP_PROCESS is not set/CONFIG_FEATURE_TOP_SMP_PROCESS=y/g' busybox/.config
	echo "CONFIG_FEATURE_TOP_INTERACTIVE=y" >> busybox/.config
else
	echo "# CONFIG_POWEROFF is not set" >> busybox/.config
endif
ifeq ($(CONFIG_JFFS2),y)
#	sed -i 's/\# CONFIG_LSOF is not set/CONFIG_LSOF=y/g' busybox/.config
endif
	echo "CONFIG_SH_IS_ASH=y" >> busybox/.config
	echo "# CONFIG_SH_IS_HUSH is not set" >> busybox/.config
	echo "# CONFIG_SH_IS_NONE is not set" >> busybox/.config
	echo "# CONFIG_BASH_IS_ASH is not set" >> busybox/.config
	echo "# CONFIG_BASH_IS_HUSH is not set" >> busybox/.config
	echo "CONFIG_BASH_IS_NONE=y" >> busybox/.config
	echo "CONFIG_ASH_INTERNAL_GLOB=y" >> busybox/.config
	echo "CONFIG_FEATURE_SH_MATH=y" >> busybox/.config
	echo "# CONFIG_FEATURE_SH_MATH_64 is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_CATV is not set" >> busybox/.config
	echo "# CONFIG_FACTOR is not set" >> busybox/.config
	echo "# CONFIG_LINK is not set" >> busybox/.config
	echo "CONFIG_FEATURE_LS_WIDTH=y" >> busybox/.config
	echo "# CONFIG_NL is not set" >> busybox/.config
	echo "# CONFIG_NPROC is not set" >> busybox/.config
	echo "# CONFIG_PASTE is not set" >> busybox/.config
	echo "# CONFIG_SHRED is not set" >> busybox/.config
	echo "# CONFIG_FALLOCATE is not set" >> busybox/.config
	echo "# CONFIG_FSFREEZE is not set" >> busybox/.config
	echo "# CONFIG_XXD is not set" >> busybox/.config
	echo "# CONFIG_SETPRIV is not set" >> busybox/.config
	echo "# CONFIG_LSSCSI is not set" >> busybox/.config
	echo "# CONFIG_PARTPROBE is not set" >> busybox/.config
	echo "# CONFIG_SSL_CLIENT is not set" >> busybox/.config
	echo "CONFIG_FEATURE_TELNET_WIDTH=y" >> busybox/.config
	echo "# CONFIG_FEATURE_WGET_HTTPS is not set" >> busybox/.config
	echo "CONFIG_ASH_ECHO=y" >> busybox/.config
	echo "CONFIG_ASH_PRINTF=y" >> busybox/.config
	echo "CONFIG_ASH_TEST=y" >> busybox/.config
	echo "# CONFIG_FEATURE_TOP_INTERACTIVE is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_POWERTOP_INTERACTIVE is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_UNZIP_CDF is not set" >> busybox/.config
	echo "# CONFIG_FEDORA_COMPAT is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_CATN is not set" >> busybox/.config
	echo "# CONFIG_BB_ARCH is not set" >> busybox/.config
	echo "# CONFIG_MINIPS is not set" >> busybox/.config
	echo "# CONFIG_NUKE is not set" >> busybox/.config
	echo "# CONFIG_RESUME is not set" >> busybox/.config
	echo "# CONFIG_RUN_INIT is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_XARGS_SUPPORT_PARALLEL is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_XARGS_SUPPORT_ARGS_FILE is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_MINIX is not set" >> busybox/.config
	echo "# CONFIG_HEXEDIT is not set" >> busybox/.config
	echo "# CONFIG_SETFATTR is not set" >> busybox/.config
	echo "# CONFIG_NETCAT is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_SH_READ_FRAC is not set" >> busybox/.config
	echo "CONFIG_STACK_OPTIMIZATION_386=y" >> busybox/.config
	echo "# CONFIG_FEATURE_ETC_SERVICES is not set" >> busybox/.config
	echo "CONFIG_FEATURE_EDITING_WINCH=y" >> busybox/.config
	echo "CONFIG_FEATURE_SORT_OPTIMIZE_MEMORY=y" >> busybox/.config
	echo "# CONFIG_FEATURE_WAIT_FOR_INIT is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_NSLOOKUP_BIG is not set" >> busybox/.config
	echo "# CONFIG_TC is not set" >> busybox/.config
	echo "# CONFIG_SVOK is not set" >> busybox/.config
	echo "CONFIG_ASH_BASH_SOURCE_CURDIR=y" >> busybox/.config
	echo "# CONFIG_ASH_BASH_NOT_FOUND_HOOK is not set" >> busybox/.config
	echo "CONFIG_BZIP2_SMALL=9" >> busybox/.config
	echo "# CONFIG_FLOAT_DURATION is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_FIND_EXECUTABLE is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_FIND_QUIT is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_FIND_ATIME is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_FIND_CTIME is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_FIND_SAMEFILE is not set" >> busybox/.config
	echo "# CONFIG_NOLOGIN is not set" >> busybox/.config
	echo "# CONFIG_BC is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_DC_BIG is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_DC_LIBM is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_SH_EMBEDDED_SCRIPTS is not set" >> busybox/.config
	echo "# CONFIG_CRC32 is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_CUT_REGEX is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VI_COLON_EXPAND is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VI_VERBOSE_STATUS is not set" >> busybox/.config
	echo "# CONFIG_ASCII is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_WGET_FTP is not set" >> busybox/.config
ifeq ($(CONFIG_RAID),y)
	sed -i 's/\# CONFIG_FEATURE_GETOPT_LONG is not set/CONFIG_FEATURE_GETOPT_LONG=y/g' busybox/.config
	sed -i 's/\# CONFIG_READLINK is not set/CONFIG_READLINK=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_READLINK_FOLLOW is not set/CONFIG_FEATURE_READLINK_FOLLOW=y/g' busybox/.config
	sed -i 's/\# CONFIG_BLOCKDEV is not set/CONFIG_BLOCKDEV=y/g' busybox/.config
endif
ifeq ($(CONFIG_OPENDPI),y)
	sed -i 's/\# CONFIG_MODPROBE is not set/CONFIG_MODPROBE=y/g' busybox/.config
endif
	echo "# CONFIG_FEATURE_SYSLOG_INFO is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_SH_MATH_BASE is not set" >> busybox/.config
	echo "# CONFIG_TS is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_TFTP_HPA_COMPAT is not set" >> busybox/.config
ifeq ($(CONFIG_OPENSSL),y)
	sed -i 's/\# CONFIG_FEATURE_WGET_HTTPS is not set/CONFIG_FEATURE_WGET_HTTPS=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_WGET_OPENSSL is not set/CONFIG_FEATURE_WGET_OPENSSL=y/g' busybox/.config
	sed -i 's/\# CONFIG_SSL_CLIENT is not set/CONFIG_SSL_CLIENT=y/g' busybox/.config
	echo "# CONFIG_FEATURE_TLS_SHA1 is not set" >> busybox/.config
endif

ifeq ($(CONFIG_I2CTOOLS),y)
	sed -i 's/\# CONFIG_I2CGET is not set/CONFIG_I2CGET=y/g' busybox/.config
	sed -i 's/\# CONFIG_I2CSET is not set/CONFIG_I2CSET=y/g' busybox/.config
	sed -i 's/\# CONFIG_I2CDUMP is not set/CONFIG_I2CDUMP=y/g' busybox/.config
	sed -i 's/\# CONFIG_I2CDETECT is not set/CONFIG_I2CDETECT=y/g' busybox/.config
	sed -i 's/\# CONFIG_I2CTRANSFER is not set/CONFIG_I2CTRANSFER=y/g' busybox/.config
endif
	echo "CONFIG_WARN_SIMPLE_MSG=y" >> busybox/.config
	echo "# CONFIG_FEATURE_FIND_EMPTY is not set" >> busybox/.config
	echo "# CONFIG_SHELL_HUSH is not set" >> busybox/.config
	echo "CONFIG_FEATURE_SYSLOGD_PRECISE_TIMESTAMPS=y" >> busybox/.config
ifneq ($(CONFIG_MICRO),y)
	sed -i 's/\# CONFIG_FEATURE_AWK_LIBM is not set/CONFIG_FEATURE_AWK_LIBM=y/g' busybox/.config
	sed -i 's/\# CONFIG_FEATURE_ALLOW_EXEC is not set/CONFIG_FEATURE_ALLOW_EXEC=y/g' busybox/.config
ifeq ($(CONFIG_IPV6),y)
	sed -i 's/\# CONFIG_FEATURE_IP_NEIGH is not set/CONFIG_FEATURE_IP_NEIGH=y/g' busybox/.config
endif
ifeq ($(CONFIG_IPV6),y)
	echo "CONFIG_VXLAN=y" >> busybox/.config
else
	echo "# CONFIG_VXLAN is not set" >> busybox/.config
endif
else
	echo "# CONFIG_VXLAN is not set" >> busybox/.config
endif
	echo "# CONFIG_STATIC_LIBGCC is not set" >> busybox/.config
	echo "# CONFIG_BASE32 is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_VOLUMEID_EROFS is not set" >> busybox/.config
	echo "CONFIG_FEATURE_TELNETD_PORT_DEFAULT=23" >> busybox/.config
	echo "CONFIG_FEATURE_TIMEZONE=y" >> busybox/.config
	echo "# CONFIG_FEATURE_CPIO_IGNORE_DEVNO is not set" >> busybox/.config
	echo "# CONFIG_FEATURE_CPIO_RENUMBER_INODES is not set" >> busybox/.config
	sed -i 's/\CONFIG_LSOF=y/# CONFIG_LSOF is not set/g' busybox/.config
	echo "# CONFIG_TSORT is not set" >> busybox/.config
	echo "# CONFIG_SEEDRNG is not set" >> busybox/.config
	echo "# CONFIG_TREE is not set" >> busybox/.config
	echo "CONFIG_ASH_SLEEP=y" >> busybox/.config
	cd busybox && make oldconfig
	
#	-$(MAKE) -j 4 -C busybox STRIPTOOL=$(STRIP) PREFIX=$(INSTALLDIR)/busybox
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
