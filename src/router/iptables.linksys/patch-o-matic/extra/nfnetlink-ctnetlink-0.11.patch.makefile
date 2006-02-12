ipchains-objs		:= $(ip_nf_compat-objs) ipchains_core.o

# netfilter netlink interface
obj-$(CONFIG_IP_NF_NETLINK) += nfnetlink.o
ifdef CONFIG_IP_NF_NETLINK
	export-objs += nfnetlink.o
endif

# nfnetlink modules
obj-$(CONFIG_IP_NF_NETLINK_CONNTRACK) += nfnetlink_conntrack.o
