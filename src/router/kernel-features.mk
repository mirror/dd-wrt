define kernelfeatures
	sed -i 's/\# CONFIG_PRINTK_TIME is not set/CONFIG_PRINTK_TIME=y/g' $(LINUXDIR)/.config
	echo "CONFIG_INOTIFY_USER=y" >> $(LINUXDIR)/.config
	echo "# CONFIG_RANDOM_TRUST_BOOTLOADER is not set" >> $(LINUXDIR)/.config
	echo "CONFIG_RANDOM_TRUST_CPU=y" >> $(LINUXDIR)/.config
	echo "CONFIG_INET_TABLE_PERTURB_ORDER=16" >> $(LINUXDIR)/.config
	echo "# CONFIG_WARN_ALL_UNSEEDED_RANDOM is not set" >> $(LINUXDIR)/.config
	if [ "$(CONFIG_IPSET)" = "y" ]; then \
		sed -i 's/\# CONFIG_NETFILTER_XT_SET is not set/CONFIG_NETFILTER_XT_SET=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_IP_SET is not set/CONFIG_IP_SET=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_IP_SET_HASH_IP is not set/CONFIG_IP_SET_HASH_IP=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_IP_SET_HASH_NET is not set/CONFIG_IP_SET_HASH_NET=y/g' $(LINUXDIR)/.config; \
		echo "CONFIG_NETFILTER_XT_SET=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP_SET_MAX=256" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP_SET_BITMAP_IP=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP_SET_BITMAP_IPMAC=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP_SET_BITMAP_PORT=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP_SET_HASH_IP=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP_SET_HASH_IPMARK=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP_SET_HASH_IPPORT=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP_SET_HASH_IPPORTIP=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP_SET_HASH_IPPORTNET=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP_SET_HASH_MAC=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP_SET_HASH_NETPORTNET=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP_SET_HASH_NET=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP_SET_HASH_NETNET=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP_SET_HASH_NETPORT=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP_SET_HASH_NETIFACE=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP_SET_HASH_IPMAC=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP_SET_LIST_NET=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP_SET_LIST_SET=y" >> $(LINUXDIR)/.config; \
	else \
		sed -i 's/\CONFIG_IP_SET=y/# CONFIG_IP_SET is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_IP_SET=m/# CONFIG_IP_SET is not set/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_NOEXTIRQ)" = "y" ]; then \
		sed -i 's/noinitrd/noextirq noinitrd/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_LIBMBIM)" = "y" ]; then \
		sed -i 's/\# CONFIG_USB_NET_CDC_NCM is not set/CONFIG_USB_NET_CDC_NCM=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_USB_NET_HUAWEI_CDC_NCM is not set/CONFIG_USB_NET_HUAWEI_CDC_NCM=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_USB_NET_CDC_MBIM is not set/CONFIG_USB_NET_CDC_MBIM=m/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_ANTAIRA)" = "y" ]; then \
		sed -i 's/\# CONFIG_GPIO_ANTAIRA is not set/CONFIG_GPIO_ANTAIRA=m/g' $(LINUXDIR)/.config; \
	else \
		echo "# CONFIG_GPIO_ANTAIRA is not set" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_WIREGUARD)" = "y" ]; then \
		sed -i 's/\# CONFIG_NET_FOU is not set/CONFIG_NET_FOU=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_NETFILTER_XT_MATCH_ADDRTYPE is not set/CONFIG_NETFILTER_XT_MATCH_ADDRTYPE=m/g' $(LINUXDIR)/.config; \
		echo "CONFIG_XFRM=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_XFRM_ALGO=m" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_XFRM_USER is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_XFRM_SUB_POLICY is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_XFRM_MIGRATE is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_XFRM_STATISTICS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_NETFILTER_XT_MATCH_POLICY is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_XFRM_IPCOMP=m" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_SCREEN)" = "y" ]; then \
		sed -i 's/\# CONFIG_USB_SERIAL_FTDI_SIO is not set/CONFIG_USB_SERIAL_FTDI_SIO=y/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_HTOP)" = "y" ]; then \
		sed -i 's/\# CONFIG_TASKSTATS is not set/CONFIG_TASKSTATS=y/g' $(LINUXDIR)/.config; \
		echo "CONFIG_TASK_DELAY_ACCT=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_TASK_XACCT=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_TASK_IO_ACCOUNTING=y" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_FRR)" = "y" ]; then \
		sed -i 's/\# CONFIG_NAMESPACES is not set/CONFIG_NAMESPACES=y/g' $(LINUXDIR)/.config; \
		echo "# CONFIG_UTS_NS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IPC_NS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_PID_NS is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NET_NS=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_USER_NS is not set" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_LEGACY_KERNEL)" = "y" ]; then \
		if [ "$(CONFIG_NTFS3G)" = "y" ]; then \
			sed -i 's/\# CONFIG_FUSE_FS is not set/CONFIG_FUSE_FS=m/g' $(LINUXDIR)/.config; \
			echo "# CONFIG_CUSE is not set" >> $(LINUXDIR)/.config; \
		fi \
	fi
	if [ "$(CONFIG_SAMBA)" != "y" ]; then \
		sed -i 's/\CONFIG_CIFS=m/# CONFIG_CIFS is not set/g' $(LINUXDIR)/.config; \
	else \
		sed -i 's/\# CONFIG_CIFS_SMB2 is not set/CONFIG_CIFS_SMB2=y/g' $(LINUXDIR)/.config; \
		echo "CONFIG_DNS_RESOLVER=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_FSCACHE=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_CIFS_SMB311=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_FSCACHE_STATS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_FSCACHE_HISTOGRAM is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_FSCACHE_DEBUG is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_FSCACHE_OBJECT_LIST is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_CACHEFILES is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_CIFS_UPCALL is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_CIFS_ACL is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_CIFS_DFS_UPCALL is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_CIFS_FSCACHE is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_ECRYPT_FS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_ENCRYPTED_KEYS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_KEY_DH_OPERATIONS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_KEYS_DEBUG_PROC_KEYS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_ASYMMETRIC_KEY_TYPE is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_SYSTEM_TRUSTED_KEYRING is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_PERSISTENT_KEYRINGS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_BIG_KEYS is not set" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_ATH9K)" = "y" ]; then \
		sed -i 's/\# CONFIG_CRYPTO_CCM is not set/CONFIG_CRYPTO_CCM=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CTR is not set/CONFIG_CRYPTO_CTR=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CMAC is not set/CONFIG_CRYPTO_CMAC=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_CRYPTO_CCM=m/CONFIG_CRYPTO_CCM=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_CRYPTO_CTR=m/CONFIG_CRYPTO_CTR=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_CRYPTO_CMAC=m/CONFIG_CRYPTO_CMAC=y/g' $(LINUXDIR)/.config; \
		echo "CONFIG_CRYPTO_DRBG_CTR=y" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_WPA3)" = "y" ]; then \
		sed -i 's/\# CONFIG_CRYPTO_GCM is not set/CONFIG_CRYPTO_GCM=m/g' $(LINUXDIR)/.config; \
		echo "CONFIG_CRYPTO_DRBG_CTR=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_CRYPTO_DRBG_HASH=y" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_MPTCP)" = "y" ]; then \
		sed -i 's/\# CONFIG_MPTCP is not set/CONFIG_MPTCP=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_IPV6 is not set/CONFIG_IPV6=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_IPV6=m/CONFIG_IPV6=y/g' $(LINUXDIR)/.config; \
		echo "CONFIG_MPTCP=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_TCP_CONG_LIA=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_TCP_CONG_OLIA=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_TCP_CONG_WVEGAS=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_TCP_CONG_BALIA=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEFAULT_LIA is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEFAULT_OLIA is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEFAULT_WVEGAS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEFAULT_BALIA is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_MPTCP_PM_ADVANCED=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_MPTCP_FULLMESH=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_MPTCP_NDIFFPORTS=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_MPTCP_BINDER=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DEFAULT_FULLMESH=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEFAULT_NDIFFPORTS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEFAULT_BINDER is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEFAULT_DUMMY is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_MPTCP_SCHED_ADVANCED=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_MPTCP_ROUNDROBIN=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_MPTCP_REDUNDANT=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DEFAULT_SCHEDULER=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEFAULT_ROUNDROBIN is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEFAULT_REDUNDANT is not set" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_EOP_TUNNEL)" = "y" ]; then \
		echo CONFIG_NET_ETHERIP=m >> $(LINUXDIR)/.config; \
		echo CONFIG_NET_EOIP=m >> $(LINUXDIR)/.config; \
		echo CONFIG_PPTP=m >> $(LINUXDIR)/.config; \
		echo "# CONFIG_NET_IPGRE is not set" >> $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_NET_IPGRE_DEMUX is not set/CONFIG_NET_IPGRE_DEMUX=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_NET_EOIP is not set/CONFIG_NET_EOIP=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_NET_ETHERIP is not set/CONFIG_NET_ETHERIP=m/g' $(LINUXDIR)/.config; \
	else \
		echo "# CONFIG_NET_ETHERIP is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_NET_EOIP is not set" >> $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_NET_EOIP=m/# CONFIG_NET_EOIP is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_NET_ETHERIP=m/# CONFIG_NET_ETHERIP is not set/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_WIREGUARD)" = "y" ]; then \
		echo CONFIG_NET_ETHERIP=m >> $(LINUXDIR)/.config; \
		echo CONFIG_NET_EOIP=m >> $(LINUXDIR)/.config; \
		echo CONFIG_PPTP=m >> $(LINUXDIR)/.config; \
		echo "# CONFIG_NET_IPGRE is not set" >> $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_NET_IPGRE_DEMUX is not set/CONFIG_NET_IPGRE_DEMUX=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_NET_EOIP is not set/CONFIG_NET_EOIP=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_NET_ETHERIP is not set/CONFIG_NET_ETHERIP=m/g' $(LINUXDIR)/.config; \
	else \
		echo "# CONFIG_NET_ETHERIP is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_NET_EOIP is not set" >> $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_NET_EOIP=m/# CONFIG_NET_EOIP is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_NET_ETHERIP=m/# CONFIG_NET_ETHERIP is not set/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_HIGH_RES_TIMERS)" = "y" ]; then \
		sed -i 's/\# CONFIG_HIGH_RES_TIMERS is not set/CONFIG_HIGH_RES_TIMERS=y/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_OPTIMIZE_O3)" = "y" ]; then \
		sed -i 's/\CONFIG_CC_OPTIMIZE_FOR_SIZE=y/# CONFIG_CC_OPTIMIZE_FOR_SIZE is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_CC_OPTIMIZE_FOR_PERFORMANCE=y/# CONFIG_CC_OPTIMIZE_FOR_PERFORMANCE is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CC_OPTIMIZE_FOR_PERFORMANCE_O3 is not set/CONFIG_CC_OPTIMIZE_FOR_PERFORMANCE_O3=y/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_OPTIMIZE_OS)" = "y" ]; then \
		sed -i 's/\# CONFIG_CC_OPTIMIZE_FOR_SIZE is not set/CONFIG_CC_OPTIMIZE_FOR_SIZE=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_CC_OPTIMIZE_FOR_PERFORMANCE=y/# CONFIG_CC_OPTIMIZE_FOR_PERFORMANCE is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_CC_OPTIMIZE_FOR_PERFORMANCE_O3=y/# CONFIG_CC_OPTIMIZE_FOR_PERFORMANCE_O3 is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_KALLSYMS=y/# CONFIG_KALLSYMS is not set/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_KERNELLTO)" = "y" ]; then \
		echo "CONFIG_LTO_MENU=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_LTO_GCC=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_LTO_DISABLE is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_LTO=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_LTO_DEBUG is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_LTO_CP_CLONE is not set" >> $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_KALLSYMS=y/# CONFIG_KALLSYMS is not set/g' $(LINUXDIR)/.config; \
	else \
		echo "# CONFIG_LTO_MENU is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_LTO_GCC is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_LTO_NONE=y" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_KERNELLTO_CP_CLONE)" = "y" ]; then \
		sed -i 's/\# CONFIG_LTO_CP_CLONE is not set/CONFIG_LTO_CP_CLONE=y/g' $(LINUXDIR)/.config; \
		echo "CONFIG_LTO_CP_CLONE=y" >> $(LINUXDIR)/.config; \
	else \
		echo "# CONFIG_LTO_CP_CLONE is not set" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_IPETH)" = "y" ]; then \
		sed -i 's/\# CONFIG_USB_IPHETH is not set/CONFIG_USB_IPHETH=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_USB_NET_CDCETHER is not set/CONFIG_USB_NET_CDCETHER=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_USB_USBNET is not set/CONFIG_USB_USBNET=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_USB_NET_CDC_NCM is not set/CONFIG_USB_NET_CDC_NCM=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_USB_NET_RNDIS_HOST is not set/CONFIG_USB_NET_RNDIS_HOST=m/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_NOCAT)" = "y" ]; then \
		sed -i 's/\# CONFIG_IFB is not set/CONFIG_IFB=m/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_MRP)" = "y" ]; then \
		sed -i 's/\# CONFIG_BRIDGE_MRP is not set/CONFIG_BRIDGE_MRP=y/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_CFM)" = "y" ]; then \
		sed -i 's/\# CONFIG_BRIDGE_CFM is not set/CONFIG_BRIDGE_CFM=y/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_OPENVPN)" = "y" && [ "$(KERNELVERSION)" = "6.1"]; then \
		sed -i 's/\# CONFIG_TUN is not set/CONFIG_TUN=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CMAC is not set/CONFIG_CRYPTO_CMAC=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CHACHA20 is not set/CONFIG_CRYPTO_CHACHA20=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_POLY1305 is not set/CONFIG_CRYPTO_POLY1305=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CTS is not set/CONFIG_CRYPTO_CTS=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CTR is not set/CONFIG_CRYPTO_CTR=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_SEQIV is not set/CONFIG_CRYPTO_SEQIV=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_GHASH is not set/CONFIG_CRYPTO_GHASH=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_GCM is not set/CONFIG_CRYPTO_GCM=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CCM is not set/CONFIG_CRYPTO_CCM=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CHACHA20POLY1305 is not set/CONFIG_CRYPTO_CHACHA20POLY1305=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CMAC=m/CONFIG_CRYPTO_CMAC=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CHACHA20=m/CONFIG_CRYPTO_CHACHA20=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_POLY1305=m/CONFIG_CRYPTO_POLY1305=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CTS=m/CONFIG_CRYPTO_CTS=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CTR=m/CONFIG_CRYPTO_CTR=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_SEQIV=m/CONFIG_CRYPTO_SEQIV=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_GHASH=m/CONFIG_CRYPTO_GHASH=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_GCM=m/CONFIG_CRYPTO_GCM=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CCM=m/CONFIG_CRYPTO_CCM=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CHACHA20POLY1305=m/CONFIG_CRYPTO_CHACHA20POLY1305=y/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_OPENVPN)" = "y" && [ "$(KERNELVERSION)" = "4.14"]; then \
		sed -i 's/\# CONFIG_TUN is not set/CONFIG_TUN=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CMAC is not set/CONFIG_CRYPTO_CMAC=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CHACHA20 is not set/CONFIG_CRYPTO_CHACHA20=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_POLY1305 is not set/CONFIG_CRYPTO_POLY1305=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CTS is not set/CONFIG_CRYPTO_CTS=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CTR is not set/CONFIG_CRYPTO_CTR=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_SEQIV is not set/CONFIG_CRYPTO_SEQIV=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_GHASH is not set/CONFIG_CRYPTO_GHASH=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_GCM is not set/CONFIG_CRYPTO_GCM=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CCM is not set/CONFIG_CRYPTO_CCM=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CHACHA20POLY1305 is not set/CONFIG_CRYPTO_CHACHA20POLY1305=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CMAC=m/CONFIG_CRYPTO_CMAC=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CHACHA20=m/CONFIG_CRYPTO_CHACHA20=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_POLY1305=m/CONFIG_CRYPTO_POLY1305=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CTS=m/CONFIG_CRYPTO_CTS=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CTR=m/CONFIG_CRYPTO_CTR=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_SEQIV=m/CONFIG_CRYPTO_SEQIV=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_GHASH=m/CONFIG_CRYPTO_GHASH=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_GCM=m/CONFIG_CRYPTO_GCM=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CCM=m/CONFIG_CRYPTO_CCM=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CHACHA20POLY1305=m/CONFIG_CRYPTO_CHACHA20POLY1305=y/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_OPENVPN)" = "y" && [ "$(KERNELVERSION)" = "4.9"]; then \
		sed -i 's/\# CONFIG_TUN is not set/CONFIG_TUN=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CMAC is not set/CONFIG_CRYPTO_CMAC=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CHACHA20 is not set/CONFIG_CRYPTO_CHACHA20=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_POLY1305 is not set/CONFIG_CRYPTO_POLY1305=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CTS is not set/CONFIG_CRYPTO_CTS=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CTR is not set/CONFIG_CRYPTO_CTR=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_SEQIV is not set/CONFIG_CRYPTO_SEQIV=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_GHASH is not set/CONFIG_CRYPTO_GHASH=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_GCM is not set/CONFIG_CRYPTO_GCM=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CCM is not set/CONFIG_CRYPTO_CCM=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CHACHA20POLY1305 is not set/CONFIG_CRYPTO_CHACHA20POLY1305=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CMAC=m/CONFIG_CRYPTO_CMAC=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CHACHA20=m/CONFIG_CRYPTO_CHACHA20=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_POLY1305=m/CONFIG_CRYPTO_POLY1305=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CTS=m/CONFIG_CRYPTO_CTS=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CTR=m/CONFIG_CRYPTO_CTR=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_SEQIV=m/CONFIG_CRYPTO_SEQIV=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_GHASH=m/CONFIG_CRYPTO_GHASH=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_GCM=m/CONFIG_CRYPTO_GCM=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CCM=m/CONFIG_CRYPTO_CCM=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CHACHA20POLY1305=m/CONFIG_CRYPTO_CHACHA20POLY1305=y/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_OPENVPN)" = "y" && [ "$(KERNELVERSION)" = "4.4"]; then \
		sed -i 's/\# CONFIG_TUN is not set/CONFIG_TUN=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CMAC is not set/CONFIG_CRYPTO_CMAC=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CHACHA20 is not set/CONFIG_CRYPTO_CHACHA20=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_POLY1305 is not set/CONFIG_CRYPTO_POLY1305=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CTS is not set/CONFIG_CRYPTO_CTS=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CTR is not set/CONFIG_CRYPTO_CTR=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_SEQIV is not set/CONFIG_CRYPTO_SEQIV=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_GHASH is not set/CONFIG_CRYPTO_GHASH=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_GCM is not set/CONFIG_CRYPTO_GCM=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CCM is not set/CONFIG_CRYPTO_CCM=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CHACHA20POLY1305 is not set/CONFIG_CRYPTO_CHACHA20POLY1305=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CMAC=m/CONFIG_CRYPTO_CMAC=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CHACHA20=m/CONFIG_CRYPTO_CHACHA20=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_POLY1305=m/CONFIG_CRYPTO_POLY1305=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CTS=m/CONFIG_CRYPTO_CTS=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CTR=m/CONFIG_CRYPTO_CTR=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_SEQIV=m/CONFIG_CRYPTO_SEQIV=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_GHASH=m/CONFIG_CRYPTO_GHASH=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_GCM=m/CONFIG_CRYPTO_GCM=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CCM=m/CONFIG_CRYPTO_CCM=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CHACHA20POLY1305=m/CONFIG_CRYPTO_CHACHA20POLY1305=y/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_ATH9K)" = "y" ]; then \
		sed -i 's/\# CONFIG_RELAY is not set/CONFIG_RELAY=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_DEBUG_FS is not set/CONFIG_DEBUG_FS=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_FW_LOADER is not set/CONFIG_FW_LOADER=y/g' $(LINUXDIR)/.config; \
		echo "# CONFIG_GCOV_KERNEL is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DYNAMIC_DEBUG is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IRQ_DOMAIN_DEBUG is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_L2TP_DEBUGFS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_LKDTM is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_FIRMWARE_IN_KERNEL is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_FW_LOADER_USER_HELPER is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_FW_LOADER_USER_HELPER_FALLBACK is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_TEST_FIRMWARE is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_EXTRA_FIRMWARE=\"\"" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_IPV6)" != "y" ]; then \
		sed -i 's/\CONFIG_IPV6=m/# CONFIG_IPV6 is not set/g' $(LINUXDIR)/.config; \
	else \
		sed -i 's/\# CONFIG_NF_DEFRAG_IPV6 is not set/CONFIG_NF_DEFRAG_IPV6=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_NF_CONNTRACK_IPV6 is not set/CONFIG_NF_CONNTRACK_IPV6=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_NF_REJECT_IPV6 is not set/CONFIG_NF_REJECT_IPV6=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_IP6_NF_IPTABLES is not set/CONFIG_IP6_NF_IPTABLES=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_IP6_NF_MATCH_AH is not set/CONFIG_IP6_NF_MATCH_AH=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_IP6_NF_MATCH_FRAG is not set/CONFIG_IP6_NF_MATCH_FRAG=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_IP6_NF_MATCH_IPV6HEADER is not set/CONFIG_IP6_NF_MATCH_IPV6HEADER=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_IP6_NF_MATCH_RPFILTER is not set/CONFIG_IP6_NF_MATCH_RPFILTER=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_IP6_NF_MATCH_RT is not set/CONFIG_IP6_NF_MATCH_RT=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_IP6_NF_FILTER is not set/CONFIG_IP6_NF_FILTER=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_IP6_NF_TARGET_REJECT is not set/CONFIG_IP6_NF_TARGET_REJECT=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_IP6_NF_MANGLE is not set/CONFIG_IP6_NF_MANGLE=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_IP6_NF_NAT is not set/CONFIG_IP6_NF_NAT=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_NF_NAT_MASQUERADE_IPV6 is not set/CONFIG_NF_NAT_MASQUERADE_IPV6=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_IP6_NF_TARGET_MASQUERADE is not set/CONFIG_IP6_NF_TARGET_MASQUERADE=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_IP6_NF_TARGET_NPT is not set/CONFIG_IP6_NF_TARGET_NPT=m/g' $(LINUXDIR)/.config; \
		echo "CONFIG_NF_REJECT_IPV6=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NF_LOG_IPV6=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NF_NAT_IPV6=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NF_NAT_MASQUERADE_IPV6=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP6_NF_IPTABLES=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP6_NF_MATCH_AH=m" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_MATCH_EUI64 is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP6_NF_MATCH_FRAG=m" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_MATCH_OWNER is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_MATCH_LIMIT is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_MATCH_MAC is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_MATCH_MULTIPORT is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_MATCH_MARK is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_MATCH_AHESP is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_MATCH_LENGTH is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_MATCH_OPTS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_MATCH_HL is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_MATCH_SRH is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP6_NF_MATCH_IPV6HEADER=m" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_MATCH_MH is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP6_NF_MATCH_RPFILTER=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP6_NF_MATCH_RT=m" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_TARGET_HL is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP6_NF_FILTER=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP6_NF_TARGET_REJECT=m" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_TARGET_SYNPROXY is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_TARGET_IMQ is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_TARGET_LOG is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_TARGET_MARK is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP6_NF_TARGET_MASQUERADE=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP6_NF_TARGET_NPT=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP6_NF_MANGLE=m" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_RAW is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP6_NF_NAT=m" >> $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_VXLAN is not set/CONFIG_VXLAN=m/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_JFFS2)" != "y" ]; then \
		sed -i 's/\CONFIG_JFFS2=m/# CONFIG_JFFS2 is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_JFFS2_FS=m/# CONFIG_JFFS2_FS is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_JFFS2_FS=y/# CONFIG_JFFS2_FS is not set/g' $(LINUXDIR)/.config; \
	fi	
	if [ "$(CONFIG_SAMBA)" != "y" ]; then \
		sed -i 's/\CONFIG_CIFS=m/# CONFIG_CIFS is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_NETWORK_FILESYSTEMS=y/# CONFIG_NETWORK_FILESYSTEMS is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_NLS=y/# CONFIG_NLS is not set/g' $(LINUXDIR)/.config; \
	fi	
	if [ "$(CONFIG_USB)" != "y" ]; then \
		sed -i 's/\CONFIG_USB=m/# CONFIG_USB is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_USB=y/# CONFIG_USB is not set/g' $(LINUXDIR)/.config; \
	fi	

	if [ "$(CONFIG_USB_ADVANCED)" != "y" ]; then \
		sed -i 's/\CONFIG_JBD=m/# CONFIG_JBD is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_JBD=y/# CONFIG_JBD is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_JBD2=m/# CONFIG_JBD2 is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_JBD2=y/# CONFIG_JBD2 is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_EXT4_FS=m/# CONFIG_EXT4_FS is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_EXT4_FS=y/# CONFIG_EXT4_FS is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_EXT3_FS=m/# CONFIG_EXT3_FS is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_EXT3_FS=y/# CONFIG_EXT3_FS is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_XFS_FS=m/# CONFIG_XFS_FS is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_XFS_FS=y/# CONFIG_XFS_FS is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_BTRFS_FS=m/# CONFIG_BTRFS_FS is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_BTRFS_FS=y/# CONFIG_BTRFS_FS is not set/g' $(LINUXDIR)/.config; \
		echo "# CONFIG_USER_NS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_UIDGID_STRICT_TYPE_CHECKS is not set" >> $(LINUXDIR)/.config; \
	fi	
	if [ "$(CONFIG_USB)" != "y" ]; then \
		sed -i 's/\CONFIG_ISO9660_FS=m/# CONFIG_ISO9660_FS is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_UDF_FS=m/# CONFIG_UDF_FS is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_FAT_FS=m/# CONFIG_FAT_FS is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_MSDOS_FS=m/# CONFIG_MSDOS_FS is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_VFAT_FS=m/# CONFIG_VFAT_FS is not set/g' $(LINUXDIR)/.config; \
	fi	
	if [ "$(CONFIG_EXFAT)" != "y" ]; then \
		sed -i 's/\CONFIG_EXFAT_FS=m/# CONFIG_EXFAT_FS is not set/g' $(LINUXDIR)/.config; \
		echo "# CONFIG_EXFAT_FS is not set" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_PROFILING)" = "y" ]; then \
		sed -i 's/\# CONFIG_PERF_EVENTS is not set/CONFIG_PERF_EVENTS=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_VM_EVENT_COUNTERS is not set/CONFIG_VM_EVENT_COUNTERS=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_PROFILING is not set/CONFIG_PROFILING=y/g' $(LINUXDIR)/.config; \
		echo "# CONFIG_OPROFILE is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_ARM_CCN is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_ARM_CCI400_PMU is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_ARM_CCI500_PMU is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_BRCMSTB_GISB_ARB is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_ARM_PMU=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_PERF_USE_VMALLOC is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_CGROUP_PERF is not set is not set" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_EXFAT)" = "y" ]; then \
		sed -i 's/\# CONFIG_EXFAT_FS is not set/CONFIG_EXFAT_FS=m/g' $(LINUXDIR)/.config; \
		echo "CONFIG_EXFAT_FS=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_EXFAT_DISCARD=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_EXFAT_DEFAULT_CODEPAGE=437" >> $(LINUXDIR)/.config; \
		echo "CONFIG_EXFAT_DEFAULT_IOCHARSET=\"utf8\"" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_EXFAT_VIRTUAL_XATTR is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_EXFAT_DEBUG is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_EXFAT_UEVENT=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_EXFAT_DELAYED_SYNC is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_EXFAT_KERNEL_DEBUG is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_EXFAT_DEBUG_MSG is not set" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_F2FS)" = "y" ]; then \
		sed -i 's/\# CONFIG_F2FS_FS is not set/CONFIG_F2FS_FS=m/g' $(LINUXDIR)/.config; \
		echo "# CONFIG_F2FS_STAT_FS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_F2FS_FS_XATTR is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_F2FS_CHECK_FS=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_F2FS_FAULT_INJECTION=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_CRYPTO_DRBG_HASH is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_F2FS_FS_COMPRESSION is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_F2FS_FS_IOSTAT=y" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_F2FS)" != "y" ]; then \
		sed -i 's/\CONFIG_F2FS_FS=m/# CONFIG_F2FS_FS is not set/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_BONDING)" != "y" ]; then \
		sed -i 's/\CONFIG_BONDING=m/# CONFIG_BONDING is not set/g' $(LINUXDIR)/.config; \
	fi	
	if [ "$(CONFIG_BONDING)" = "y" ]; then \
		sed -i 's/\# CONFIG_BONDING is not set/CONFIG_BONDING=m/g' $(LINUXDIR)/.config; \
	fi	
	if [ "$(CONFIG_SWAP)" != "y" ]; then \
		sed -i 's/\CONFIG_SWAP=y/# CONFIG_SWAP is not set/g' $(LINUXDIR)/.config; \
	else \
		sed -i 's/\# CONFIG_SWAP is not set/CONFIG_SWAP=y/g' $(LINUXDIR)/.config; \
		echo "# CONFIG_MTD_SWAP is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_FRONTSWAP is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_HIBERNATION is not set" >> $(LINUXDIR)/.config; \
	fi	
	if [ "$(CONFIG_USBIP)" = "y" ]; then \
		sed -i 's/\# CONFIG_USBIP_CORE is not set/CONFIG_USBIP_CORE=m/g' $(LINUXDIR)/.config; \
		echo "CONFIG_USBIP_VHCI_HCD=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_USBIP_HOST=m" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_USBIP_DEBUG is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_USBIP_VHCI_HC_PORTS=8" >> $(LINUXDIR)/.config; \
		echo "CONFIG_USBIP_VHCI_NR_HCS=1" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_USBIP_VUDC is not set" >> $(LINUXDIR)/.config; \
	else \
		sed -i 's/\CONFIG_USBIP_CORE=m/# CONFIG_USBIP_CORE is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_USBIP_CORE=y/# CONFIG_USBIP_CORE is not set/g' $(LINUXDIR)/.config; \
	fi	
	if [ "$(CONFIG_IPVS)" = "y" ]; then \
	sed -i 's/\# CONFIG_IP_VS is not set/CONFIG_IP_VS=m/g' $(LINUXDIR)/.config; \
	echo "CONFIG_NETFILTER_XT_MATCH_IPVS=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_IPV6=y" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP6_NF_MATCH_AH=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP6_NF_MATCH_EUI64=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP6_NF_MATCH_FRAG=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP6_NF_MATCH_OPTS=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP6_NF_MATCH_HL=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP6_NF_MATCH_IPV6HEADER=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP6_NF_MATCH_MH=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP6_NF_MATCH_RT=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP6_NF_MATCH_RPFILTER=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP6_NF_FILTER=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP6_NF_TARGET_REJECT=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP6_NF_TARGET_SYNPROXY=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP6_NF_TARGET_HL=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP6_NF_MANGLE=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP6_NF_RAW=m" >> $(LINUXDIR)/.config; \
	echo "# CONFIG_IP_VS_DEBUG is not set" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_TAB_BITS=12" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_PROTO_TCP=y" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_PROTO_UDP=y" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_PROTO_ESP=y" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_PROTO_AH=y" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_PROTO_SCTP=y" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_RR=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_WRR=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_LC=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_WLC=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_FO=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_LBLC=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_LBLCR=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_DH=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_SH=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_OVF=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_MH=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_MH_TAB_INDEX=12" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_SED=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_NQ=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_SH_TAB_BITS=8" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_FTP=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_NFCT=y" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_PE_SIP=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_TWOS=m" >> $(LINUXDIR)/.config; \
	fi
	echo "# CONFIG_ASN1 is not set" >> $(LINUXDIR)/.config; \
	if [ "$(CONFIG_SMBD)" = "y" ]; then \
		sed -i 's/\# CONFIG_CRYPTO_MD5 is not set/CONFIG_CRYPTO_MD5=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_DES is not set/CONFIG_CRYPTO_DES=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CBC is not set/CONFIG_CRYPTO_CBC=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CTS is not set/CONFIG_CRYPTO_CTS=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_ECB is not set/CONFIG_CRYPTO_ECB=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_HMAC is not set/CONFIG_CRYPTO_HMAC=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_SHA1 is not set/CONFIG_CRYPTO_SHA1=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_AES is not set/CONFIG_CRYPTO_AES=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CMAC is not set/CONFIG_CRYPTO_CMAC=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_SHA256 is not set/CONFIG_CRYPTO_SHA256=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_SHA512 is not set/CONFIG_CRYPTO_SHA512=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_GCM is not set/CONFIG_CRYPTO_GCM=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CCM is not set/CONFIG_CRYPTO_CCM=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_AEAD is not set/CONFIG_CRYPTO_AEAD=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_AEAD2 is not set/CONFIG_CRYPTO_AEAD2=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_LIBCRC32C is not set/CONFIG_LIBCRC32C=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_ASN1 is not set/CONFIG_ASN1=m/g' $(LINUXDIR)/.config; \
		echo "CONFIG_CRYPTO_DRBG_CTR=y" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_NFS)" = "y" ]; then \
		sed -i 's/\# CONFIG_NFS_FS is not set/CONFIG_NFS_FS=m/g' $(LINUXDIR)/.config; \
		echo "CONFIG_NFS_V2=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NFS_V3=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_NFS_V3_ACL is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NFS_V4=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_NFS_SWAP is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NFS_V4_1=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NFS_V4_2=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NFS_V4_2_READ_PLUS=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NFSD_V4_2_INTER_SSC=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NFS_V4_1_IMPLEMENTATION_ID_DOMAIN=\"kernel.org\"" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NFS_V4_1_MIGRATION=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NFS_FSCACHE=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NFS_USE_LEGACY_DNS=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_SUNRPC_DISABLE_INSECURE_ENCTYPES is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_SUNRPC_DEBUG is not set" >> $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_NFSD is not set/CONFIG_NFSD=m/g' $(LINUXDIR)/.config; \
		echo "CONFIG_NFSD_V3=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NFSD_PNFS=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_NFSD_V3_ACL is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_NFSD_V2_ACL is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_NFSD_FAULT_INJECTION is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_NFS_DISABLE_UDP_SUPPORT is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NFSD_V4=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NFSD_V2=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NFSD_BLOCKLAYOUT=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_RPCSEC_GSS_KRB5=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NFSD_SCSILAYOUT=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NFSD_FLEXFILELAYOUT=y" >> $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_MD5 is not set/CONFIG_CRYPTO_MD5=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_DES is not set/CONFIG_CRYPTO_DES=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CBC is not set/CONFIG_CRYPTO_CBC=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_CTS is not set/CONFIG_CRYPTO_CTS=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_ECB is not set/CONFIG_CRYPTO_ECB=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_HMAC is not set/CONFIG_CRYPTO_HMAC=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_SHA1 is not set/CONFIG_CRYPTO_SHA1=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_AES is not set/CONFIG_CRYPTO_AES=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_CRYPTO_ARC4 is not set/CONFIG_CRYPTO_ARC4=m/g' $(LINUXDIR)/.config; \
		echo "# CONFIG_SYSTEM_BLACKLIST_KEYRING is not set" >> $(LINUXDIR)/.config; \
	else \
		echo "# CONFIG_RPCSEC_GSS_KRB5 is not set" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_VSOCKETS)" = "y" ]; then \
		sed -i 's/\# CONFIG_VSOCKETS is not set/CONFIG_VSOCKETS=m/g' $(LINUXDIR)/.config; \
		echo "CONFIG_VMWARE_VMCI_VSOCKETS=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_VIRTIO_VSOCKETS=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_VIRTIO_VSOCKETS_COMMON=m" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_BTRFSPROGS)" = "y" ]; then \
		sed -i 's/\# CONFIG_BTRFS_FS is not set/CONFIG_BTRFS_FS=m/g' $(LINUXDIR)/.config; \
		echo "# CONFIG_BTRFS_FS_POSIX_ACL is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_BTRFS_FS_CHECK_INTEGRITY is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_BTRFS_FS_RUN_SANITY_TESTS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_BTRFS_DEBUG is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_BTRFS_ASSERT is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_BTRFS_FSREF_VERIFY is not set" >> $(LINUXDIR)/.config; \
	else \
		sed -i 's/\CONFIG_BTRFS_FS=m/# CONFIG_BTRFS_FS is not set/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_XFSPROGS)" = "y" ]; then \
		sed -i 's/\# CONFIG_XFS_FS is not set/CONFIG_XFS_FS=m/g' $(LINUXDIR)/.config; \
		echo "# CONFIG_XFS_QUOTA is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_XFS_POSIX_ACL is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_XFS_RT is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_XFS_WARN is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_XFS_DEBUG is not set" >> $(LINUXDIR)/.config; \
	else \
		sed -i 's/\CONFIG_XFS_FS=m/# CONFIG_XFS_FS is not set/g' $(LINUXDIR)/.config; \
		echo "# CONFIG_USER_NS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_UIDGID_STRICT_TYPE_CHECKS is not set" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_LOCKDEBUG)" = "y" ]; then \
		sed -i 's/\# CONFIG_TASKSTATS is not set/CONFIG_TASKSTATS=y/g' $(LINUXDIR)/.config; \
		echo "CONFIG_TASK_DELAY_ACCT=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_TASK_XACCT=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_TASK_IO_ACCOUNTING=y" >> $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_DEBUG_KERNEL is not set/CONFIG_DEBUG_KERNEL=y/g' $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_PAGEALLOC is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_OBJECTS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_KMEMLEAK is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_STACK_USAGE is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_VM is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_PER_CPU_MAPS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_HIGHMEM is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_STACKOVERFLOW is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_SHIRQ is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_LOCKUP_DETECTOR=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_SOFTLOCKUP_DETECTOR=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_BOOTPARAM_SOFTLOCKUP_PANIC is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_BOOTPARAM_SOFTLOCKUP_PANIC_VALUE=0" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DETECT_HUNG_TASK=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DEFAULT_HUNG_TASK_TIMEOUT=120" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_BOOTPARAM_HUNG_TASK_PANIC is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_BOOTPARAM_HUNG_TASK_PANIC_VALUE=0" >> $(LINUXDIR)/.config; \
		echo "CONFIG_WQ_WATCHDOG=y" >> $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_SCHED_INFO is not set/CONFIG_SCHED_INFO=y/g' $(LINUXDIR)/.config; \
		echo "CONFIG_SCHED_DEBUG=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_SCHED_INFO=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_SCHEDSTATS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_SCHED_STACK_END_CHECK is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DEBUG_RT_MUTEXES=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DEBUG_SPINLOCK=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DEBUG_MUTEXES=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_WW_MUTEX_SLOWPATH is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DEBUG_LOCK_ALLOC=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_PROVE_LOCKING=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_LOCKDEP=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_LOCK_STAT is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_LOCKDEP is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DEBUG_ATOMIC_SLEEP=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_LOCKING_API_SELFTESTS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_LOCK_TORTURE_TEST is not set" >> $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_STACKTRACE is not set/CONFIG_STACKTRACE=y/g' $(LINUXDIR)/.config; \
		echo "CONFIG_TRACE_IRQFLAGS=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_KOBJECT is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_LIST is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_PI_LIST is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_SG is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_NOTIFIERS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_CREDENTIALS is not set" >> $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_PROVE_RCU is not set/CONFIG_PROVE_RCU=y/g' $(LINUXDIR)/.config; \
		echo "# CONFIG_RCU_PERF_TEST is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_RCU_TORTURE_TEST is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_RCU_TRACE=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_RCU_EQS_DEBUG is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_WQ_FORCE_RR_CPU is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_BLOCK_EXT_DEVT is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_NOTIFIER_ERROR_INJECTION is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_FAULT_INJECTION is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_LATENCYTOP is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_BACKTRACE_SELF_TEST is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_RBTREE_TEST is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_INTERVAL_TREE_TEST is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_PERCPU_TEST is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_ASYNC_RAID6_TEST is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_KGDB is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_PCI_DEBUG is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_DRIVER is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_DEVRES is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_TEST_DRIVER_REMOVE is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_SPI_DEBUG is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_PINCTRL is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_GPIO is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_BOOT_PRINTK_DELAY is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_INFO is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_READABLE_ASM is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_PAGE_OWNER is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_FORCE_WEAK_PER_CPU is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_PERF_USE_VMALLOC is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_MAXSMP=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_HWPOISON_INJECT is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_VIRTUAL is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_KMEMCHECK is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_BOOTPARAM_HARDLOCKUP_PANIC is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_TIMER_STATS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_PROVE_RCU_REPEATEDLY is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_CPU_HOTPLUG_STATE_CONTROL is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_TEST_LIST_SORT is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_X86_PTDUMP is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_NX_TEST is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_TLBFLUSH is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IOMMU_DEBUG is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_X86_DECODER_SELFTEST is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_BOOT_PARAMS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_CPA_DEBUG is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_ENTRY is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DEBUG_NMI_SELFTEST is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_X86_DEBUG_FPU is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_CMA_DEBUG is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_ARM64_PTDUMP_DEBUGFS is not set" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_RAID)" = "y" ]; then \
		sed -i 's/\# CONFIG_MD is not set/CONFIG_MD=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_RAID6_PQ is not set/CONFIG_RAID6_PQ=y/g' $(LINUXDIR)/.config; \
		echo "CONFIG_MD=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_BLK_DEV_MD=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_MD_AUTODETECT=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_MD_LINEAR=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_MD_RAID0=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_MD_RAID1=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_MD_RAID10=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_MD_RAID456=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_MD_MULTIPATH=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_MD_FAULTY=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_MD_CLUSTER=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_BCACHE=m" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_BCACHE_DEBUG is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_BCACHE_ASYNC_REGISTRATION is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_BCACHE_CLOSURES_DEBUG is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_BLK_DEV_DM_BUILTIN=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_BLK_DEV_DM=m" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DM_MQ_DEFAULT is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DM_DEBUG is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DM_DEBUG_BLOCK_STACK_TRACING is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_BUFIO=m" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DM_DEBUG_BLOCK_MANAGER_LOCKING is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_BIO_PRISON=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_PERSISTENT_DATA=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_CRYPT=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_SNAPSHOT=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_THIN_PROVISIONING=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_CACHE=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_CACHE_CLEANER=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_CACHE_SMQ=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_ERA=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_MIRROR=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_LOG_USERSPACE=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_RAID=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_ZERO=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_MULTIPATH=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_MULTIPATH_QL=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_MULTIPATH_ST=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_MULTIPATH_HST=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_MULTIPATH_IOA=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_DELAY=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_UEVENT=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_FLAKEY=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_VERITY=m" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DM_VERITY_FEC is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DM_UNSTRIPED is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DM_EBS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DM_CLONE is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DM_DUST is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DM_VERITY_VERIFY_ROOTHASH_SIG is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_SWITCH=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_WRITECACHE=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_LOG_WRITES=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_INTEGRITY=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_ZONED=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DEV_DAX=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NR_DEV_DAX=32768" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_CACHE_MQ=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_CRYPTO_CRCT10DIF_ARM64_CE=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_CRYPTO_CRCT10DIF_ARM_CE=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_ASYNC_RAID6_TEST is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_BCACHE_EDEBUG is not set" >> $(LINUXDIR)/.config; \
	else \
		sed -i 's/\CONFIG_MD=y/# CONFIG_MD is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_RAID6_PQ=m/# CONFIG_RAID6_PQ is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_RAID6_PQ=y/# CONFIG_RAID6_PQ is not set/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_WIREGUARD)" = "y" ] && [ "$(KERNELVERSION)" = "6.1" ]; then \
		sed -i 's/\# CONFIG_WIREGUARD is not set/CONFIG_WIREGUARD=m/g' $(LINUXDIR)/.config; \
		echo "# CONFIG_WIREGUARD_DEBUG is not set" >> $(LINUXDIR)/.config; \
	fi
endef
