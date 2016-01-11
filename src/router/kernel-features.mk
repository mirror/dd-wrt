define kernelfeatures
	if [ "$(CONFIG_EOP_TUNNEL)" = "y" ]; then \
		echo CONFIG_NET_ETHERIP=m >> $(LINUXDIR)/.config; \
	else \
		echo "# CONFIG_NET_ETHERIP is not set" >> $(LINUXDIR)/.config; \
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
	echo "CONFIG_IP_VS_SED=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_NQ=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_SH_TAB_BITS=8" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_FTP=m" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_NFCT=y" >> $(LINUXDIR)/.config; \
	echo "CONFIG_IP_VS_PE_SIP=m" >> $(LINUXDIR)/.config; \
	fi
endef