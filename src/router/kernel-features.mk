define kernelfeatures
	if [ "$(CONFIG_LIBMBIM)" = "y" ]; then \
		sed -i 's/\# CONFIG_USB_NET_CDC_NCM is not set/CONFIG_USB_NET_CDC_NCM=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_USB_NET_HUAWEI_CDC_NCM is not set/CONFIG_USB_NET_HUAWEI_CDC_NCM=m/g' $(LINUXDIR)/.config; \
		sed -i 's/\# CONFIG_USB_NET_CDC_MBIM is not set/CONFIG_USB_NET_CDC_MBIM=m/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_WIREGUARD)" = "y" ]; then \
		sed -i 's/\# CONFIG_NET_FOU is not set/CONFIG_NET_FOU=m/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_NTFS3G)" = "y" ]; then \
		sed -i 's/\# CONFIG_FUSE_FS is not set/CONFIG_FUSE_FS=m/g' $(LINUXDIR)/.config; \
		echo "# CONFIG_CUSE is not set" >> $(LINUXDIR)/.config; \
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
	echo "CONFIG_IP_VS_SED=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_NQ=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_SH_TAB_BITS=8" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_FTP=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_NFCT=y" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_PE_SIP=m" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_VSOCKETS)" = "y" ]; then \
		sed -i 's/\# CONFIG_VSOCKETS is not set/CONFIG_VSOCKETS=m/g' $(LINUXDIR)/.config; \
		echo "CONFIG_VMWARE_VMCI_VSOCKETS=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_VIRTIO_VSOCKETS=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_VIRTIO_VSOCKETS_COMMON=m" >> $(LINUXDIR)/.config; \
	fi
endef
