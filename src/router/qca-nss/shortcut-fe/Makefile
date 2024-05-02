#
# Makefile for Shortcut FE.
#

#KERNELVERSION := $(word 1, $(subst ., ,$(KERNELVERSION))).$(word 2, $(subst ., ,$(KERNELVERSION)))
SFE_BASE_OBJS := sfe.o sfe_init.o
SFE_IPV4_OBJS := sfe_ipv4.o sfe_ipv4_udp.o sfe_ipv4_tcp.o sfe_ipv4_icmp.o sfe_ipv4_pppoe_br.o sfe_ipv4_multicast.o
SFE_IPV6_OBJS := sfe_ipv6.o sfe_ipv6_udp.o sfe_ipv6_tcp.o sfe_ipv6_icmp.o sfe_ipv6_pppoe_br.o sfe_ipv6_multicast.o sfe_ipv6_etherip.o sfe_ipv6_frag.o
SFE_PPPOE_OBJS := sfe_pppoe.o sfe_pppoe_mgr.o
ifndef SFE_256M_PROFILE
ifndef SFE_16M_PROFILE
SFE_IPV4_OBJS += sfe_ipv4_tun6rd.o sfe_ipv4_esp.o
SFE_IPV6_OBJS += sfe_ipv6_tunipip6.o sfe_ipv6_esp.o
endif
endif

#ifeq ($(findstring 4.4, $(KERNELVERSION)),)
  SFE_IPV4_OBJS += sfe_ipv4_gre.o
  SFE_IPV6_OBJS += sfe_ipv6_gre.o
  ccflags-y += -DSFE_GRE_TUN_ENABLE
#endif

#ifdef SFE_L2TPV3_SUPPORTED
SFE_IPV4_OBJS += sfe_ipv4_l2tpv3.o
SFE_IPV6_OBJS += sfe_ipv6_l2tpv3.o
ccflags-y += -DSFE_L2TPV3_ENABLED
#endif

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

ifdef SFE_RFS_SUPPORTED
ccflags-y += -DSFE_RFS_SUPPORTED
endif

ifdef SFE_PPE_QOS_SUPPORTED
ccflags-y += -DSFE_PPE_QOS_SUPPORTED
endif

#ifdef SFE_PROCESS_LOCAL_OUT
ccflags-y += -DSFE_PROCESS_LOCAL_OUT
#endif

#ifdef SFE_BRIDGE_VLAN_FILTERING_ENABLE
ccflags-y += -DSFE_BRIDGE_VLAN_FILTERING_ENABLE
#endif

ifndef SFE_256M_PROFILE
ifndef SFE_16M_PROFILE
ccflags-y += -DSFE_IPSEC_ENABLE
ccflags-y += -DSFE_6RD_ENABLE
ccflags-y += -DSFE_DSLITE_ENABLE
endif
endif

ccflags-y += -Werror -Wall -Iexports/
