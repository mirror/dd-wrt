obj-$(CONFIG_IP_NF_CONNTRACK) += ip_conntrack.o

ifdef CONFIG_IP_NF_NAT_NEEDED
	export-objs += ip_conntrack_proto_tcp.o
endif

