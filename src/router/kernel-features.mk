define kernelfeatures
	if [ "$(CONFIG_LIBMBIM)" = "y" ]; then \
		sed -i 's/\# CONFIG_USB_NET_CDC_NCM is not set/CONFIG_USB_NET_CDC_NCM=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_USB_NET_HUAWEI_CDC_NCM is not set/CONFIG_USB_NET_HUAWEI_CDC_NCM=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_USB_NET_CDC_MBIM is not set/CONFIG_USB_NET_CDC_MBIM=m/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_WIREGUARD)" = "y" ]; then \
		sed -i 's/\# CONFIG_NET_FOU is not set/CONFIG_NET_FOU=m/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_FRR)" = "y" ]; then \
		sed -i 's/\# CONFIG_NAMESPACES is not set/CONFIG_NAMESPACES=y/g' $(LINUXDIR)/.config; \
		echo "# CONFIG_UTS_NS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IPC_NS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_PID_NS is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NET_NS=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_USER_NS is not set" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_NTFS3G)" = "y" ]; then \
		sed -i 's/\# CONFIG_FUSE_FS is not set/CONFIG_FUSE_FS=m/g' $(LINUXDIR)/.config; \
		echo "# CONFIG_CUSE is not set" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_SAMBA)" != "y" ]; then \
		sed -i 's/\CONFIG_CIFS=m/# CONFIG_CIFS is not set/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_WPA3)" = "y" ]; then \
		sed -i 's/\# CONFIG_CRYPTO_GCM is not set/CONFIG_CRYPTO_GCM=m/g' $(LINUXDIR)/.config; \
		echo "# CONFIG_CRYPTO_DRBG_CTR is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_CRYPTO_DRBG_HASH is not set" >> $(LINUXDIR)/.config; \
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
	if [ "$(CONFIG_HIGH_RES_TIMERS)" = "y" ]; then \
		sed -i 's/\# CONFIG_HIGH_RES_TIMERS is not set/CONFIG_HIGH_RES_TIMERS=y/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_IPETH)" = "y" ]; then \
		sed -i 's/\# CONFIG_USB_IPHETH is not set/CONFIG_USB_IPHETH=m/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_OPENVPN)" = "y" ]; then \
		sed -i 's/\# CONFIG_TUN is not set/CONFIG_TUN=m/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_ATH9K)" = "y" ]; then \
		sed -i 's/\# CONFIG_RELAY is not set/CONFIG_RELAY=y/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_DEBUG_FS is not set/CONFIG_DEBUG_FS=y/g' $(LINUXDIR)/.config; \
		echo "# CONFIG_GCOV_KERNEL is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DYNAMIC_DEBUG is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IRQ_DOMAIN_DEBUG is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_L2TP_DEBUGFS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_LKDTM is not set" >> $(LINUXDIR)/.config; \
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
		echo "CONFIG_NF_REJECT_IPV6=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NF_LOG_IPV6=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NF_NAT_IPV6=m" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_NF_NAT_MASQUERADE_IPV6 is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP6_NF_IPTABLES=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP6_NF_MATCH_AH=m" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_MATCH_EUI64 is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_IP6_NF_MATCH_FRAG=m" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_MATCH_OWNER is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_MATCH_OPTS is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_MATCH_HL is not set" >> $(LINUXDIR)/.config; \
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
		echo "CONFIG_IP6_NF_MANGLE=m" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_RAW is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_IP6_NF_NAT is not set" >> $(LINUXDIR)/.config; \
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
	fi
	if [ "$(CONFIG_F2FS)" != "y" ]; then \
		sed -i 's/\CONFIG_F2FS_FS=m/# CONFIG_F2FS_FS is not set/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_BONDING)" != "y" ]; then \
		sed -i 's/\CONFIG_BONDING=m/# CONFIG_BONDING is not set/g' $(LINUXDIR)/.config; \
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
		echo "CONFIG_NFS_V4_1_IMPLEMENTATION_ID_DOMAIN=\"kernel.org\"" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NFS_V4_1_MIGRATION=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NFS_FSCACHE=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NFS_USE_LEGACY_DNS=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_SUNRPC_DEBUG is not set" >> $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_NFSD is not set/CONFIG_NFSD=m/g' $(LINUXDIR)/.config; \
		echo "CONFIG_NFSD_V3=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NFSD_PNFS=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_NFSD_V3_ACL is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_NFSD_FAULT_INJECTION is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NFSD_V4=y" >> $(LINUXDIR)/.config; \
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
		echo "CONFIG_DM_DELAY=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_UEVENT=y" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_FLAKEY=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_VERITY=m" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_DM_VERITY_FEC is not set" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_SWITCH=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_LOG_WRITES=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_INTEGRITY=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_ZONED=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DEV_DAX=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_NR_DEV_DAX=32768" >> $(LINUXDIR)/.config; \
		echo "CONFIG_DM_CACHE_MQ=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_CRYPTO_CRCT10DIF_ARM64_CE=y" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_ASYNC_RAID6_TEST is not set" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_BCACHE_EDEBUG is not set" >> $(LINUXDIR)/.config; \
	else \
		sed -i 's/\CONFIG_MD=y/# CONFIG_MD is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_RAID6_PQ=m/# CONFIG_RAID6_PQ is not set/g' $(LINUXDIR)/.config; \
		sed -i 's/\CONFIG_RAID6_PQ=y/# CONFIG_RAID6_PQ is not set/g' $(LINUXDIR)/.config; \
	fi
endef
