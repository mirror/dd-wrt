define kernelfeatures
	if [ "$(CONFIG_EOP_TUNNEL)" = "y" ]; then \
		echo CONFIG_NET_ETHERIP=m >> $(LINUXDIR)/.config; \
	else \
		echo "# CONFIG_NET_ETHERIP is not set" >> $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_HIGH_RES_TIMERS)" = "y" ]; then \
		sed -i 's/\# CONFIG_HIGH_RES_TIMERS is not set/CONFIG_HIGH_RES_TIMERS=y/g' $(LINUXDIR)/.config; \
	fi
	if [ "$(CONFIG_IPETH)" = "y" ]; then \
		sed -i 's/\# CONFIG_USB_IPHETH is not set/CONFIG_USB_IPHETH=m/g' $(LINUXDIR)/.config; \
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
	if [ "$(CONFIG_BONDING)" != "y" ]; then \
		sed -i 's/\CONFIG_BONDING=m/# CONFIG_BONDING is not set/g' $(LINUXDIR)/.config; \
	fi	
	if [ "$(CONFIG_USBIP)" = "y" ]; then \
		sed -i 's/\# CONFIG_USBIP_CORE is not set/CONFIG_USBIP_CORE=m/g' $(LINUXDIR)/.config; \
		echo "CONFIG_USBIP_VHCI_HCD=m" >> $(LINUXDIR)/.config; \
		echo "CONFIG_USBIP_HOST=m" >> $(LINUXDIR)/.config; \
		echo "# CONFIG_USBIP_DEBUG is not set" >> $(LINUXDIR)/.config; \
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
endef