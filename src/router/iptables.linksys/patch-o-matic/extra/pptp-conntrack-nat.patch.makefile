# connection tracking helpers
obj-$(CONFIG_IP_NF_PPTP) += ip_conntrack_pptp.o
ifdef CONFIG_IP_NF_NAT_PPTP
	export-objs += ip_conntrack_pptp.o
endif
