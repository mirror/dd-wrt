obj-$(CONFIG_IP_NF_CONNTRACK) += ip_conntrack.o

# Amanda protocol support
obj-$(CONFIG_IP_NF_AMANDA) += ip_conntrack_amanda.o
obj-$(CONFIG_IP_NF_NAT_AMANDA) += ip_nat_amanda.o
ifdef CONFIG_IP_NF_NAT_AMANDA
	export-objs += ip_conntrack_amanda.o
endif

