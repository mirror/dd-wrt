#
# Makefile for Shortcut FE.
#

SFE_BASE_OBJS := sfe.o sfe_init.o
SFE_IPV4_OBJS := sfe_ipv4.o sfe_ipv4_udp.o sfe_ipv4_tcp.o sfe_ipv4_icmp.o sfe_ipv4_tun6rd.o sfe_ipv4_pppoe_br.o sfe_ipv4_esp.o sfe_ipv4_multicast.o
SFE_IPV6_OBJS := sfe_ipv6.o sfe_ipv6_udp.o sfe_ipv6_tcp.o sfe_ipv6_icmp.o sfe_ipv6_tunipip6.o sfe_ipv6_pppoe_br.o sfe_ipv6_esp.o sfe_ipv6_multicast.o sfe_ipv6_etherip.o sfe_ipv6_frag.o
SFE_PPPOE_OBJS := sfe_pppoe.o sfe_pppoe_mgr.o


SFE_IPV4_OBJS += sfe_ipv4_gre.o
SFE_IPV6_OBJS += sfe_ipv6_gre.o
ccflags-y += -DSFE_GRE_TUN_ENABLE


obj-m += qca-nss-sfe.o

#
# Base files
#
qca-nss-sfe-objs := $(SFE_BASE_OBJS)

#
# IPv4 files
#
qca-nss-sfe-objs += $(SFE_IPV4_OBJS)
qca-nss-sfe-objs += $(SFE_PPPOE_OBJS)

#ifdef SFE_SUPPORT_IPV6
qca-nss-sfe-objs += $(SFE_IPV6_OBJS)
ccflags-y += -DSFE_SUPPORT_IPV6
#endif

#ifdef SFE_RFS_SUPPORTED
#ccflags-y += -DSFE_RFS_SUPPORTED
#endif

#ifdef SFE_PPE_QOS_SUPPORTED
ccflags-y += -DSFE_PPE_QOS_SUPPORTED
#endif

#ifdef SFE_PROCESS_LOCAL_OUT
ccflags-y += -DSFE_PROCESS_LOCAL_OUT
#endif

#ifdef SFE_BRIDGE_VLAN_FILTERING_ENABLE
ccflags-y += -DSFE_BRIDGE_VLAN_FILTERING_ENABLE
#endif

ccflags-y += -Werror -Wall -Inet/shortcut-fe_v2/exports/
