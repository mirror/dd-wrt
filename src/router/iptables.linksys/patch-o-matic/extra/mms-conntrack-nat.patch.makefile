# connection tracking helpers
obj-$(CONFIG_IP_NF_MMS) += ip_conntrack_mms.o
ifdef CONFIG_IP_NF_NAT_MMS
	export-objs += ip_conntrack_mms.o
endif
