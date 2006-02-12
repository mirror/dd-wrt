obj-$(CONFIG_IP_NF_CONNTRACK) += ip_conntrack.o

# talk protocol support
obj-$(CONFIG_IP_NF_TALK) += ip_conntrack_talk.o
obj-$(CONFIG_IP_NF_NAT_TALK) += ip_nat_talk.o
ifdef CONFIG_IP_NF_NAT_TALK
	export-objs += ip_conntrack_talk.o
endif

