obj-$(CONFIG_IP_NF_MATCH_TOS) += ipt_tos.o
obj-$(CONFIG_IP_NF_MATCH_RPC) += ip_conntrack_rpc_tcp.o ip_conntrack_rpc_udp.o ipt_record_rpc.o
export-objs += ip_conntrack_rpc_tcp.o ip_conntrack_rpc_udp.o
