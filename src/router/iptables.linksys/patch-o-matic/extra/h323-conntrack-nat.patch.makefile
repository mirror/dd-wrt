obj-$(CONFIG_IP_NF_CONNTRACK) += ip_conntrack.o
 
# H.323 support
obj-$(CONFIG_IP_NF_H323) += ip_conntrack_h323.o
obj-$(CONFIG_IP_NF_NAT_H323) += ip_nat_h323.o
ifdef CONFIG_IP_NF_NAT_H323
	export-objs += ip_conntrack_h323.o
endif

