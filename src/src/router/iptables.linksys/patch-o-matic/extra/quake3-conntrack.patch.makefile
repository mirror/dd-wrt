# connection tracking helpers
obj-$(CONFIG_IP_NF_QUAKE3) += ip_conntrack_quake3.o
ifdef CONFIG_IP_NF_NAT_QUAKE3
	export-objs += ip_conntrack_quake3.o
endif
