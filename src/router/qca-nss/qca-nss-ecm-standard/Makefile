##########################################################################
# Copyright (c) 2014-2016, 2018-2019, The Linux Foundation. All rights reserved.
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
# OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
##########################################################################

# ###################################################
# Makefile for the QCA NSS ECM
# ###################################################

obj-m += ecm.o

# #####################################################
# Example builds.
# Enable example build by using the menuconfig options.
# #####################################################
ifeq ($(EXAMPLES_BUILD_PCC),y)
obj-m += examples/ecm_pcc_test.o
endif
ifeq ($(EXAMPLES_BUILD_MARK),y)
obj-m += examples/ecm_mark_test.o
endif

ecm-y := \
	 ecm_tracker_udp.o \
	 ecm_tracker_tcp.o \
	 ecm_tracker_datagram.o \
	 ecm_tracker.o \
	 frontends/ecm_front_end_ipv4.o \
	 frontends/ecm_front_end_ipv6.o \
	 frontends/ecm_front_end_common.o \
	 ecm_db/ecm_db.o \
	 ecm_db/ecm_db_connection.o \
	 ecm_db/ecm_db_mapping.o \
	 ecm_db/ecm_db_host.o \
	 ecm_db/ecm_db_node.o \
	 ecm_db/ecm_db_iface.o \
	 ecm_db/ecm_db_listener.o \
	 ecm_db/ecm_db_timer.o \
	 ecm_classifier.o \
	 ecm_classifier_default.o \
	 ecm_interface.o \
	 ecm_conntrack_notifier.o \
	 ecm_init.o

# #############################################################################
# Define ECM_FRONT_END_NSS_ENABLE=y in order to select
# nss as ECM's front end.
# #############################################################################
ifeq ($(SoC),$(filter $(SoC),ipq806x ipq807x ipq807x_64 ipq60xx ipq60xx_64))
ECM_FRONT_END_NSS_ENABLE=y
ecm-$(ECM_FRONT_END_NSS_ENABLE) += frontends/nss/ecm_nss_ipv4.o
ecm-$(ECM_FRONT_END_NSS_ENABLE) += frontends/nss/ecm_nss_ported_ipv4.o
ccflags-$(ECM_FRONT_END_NSS_ENABLE) += -DECM_FRONT_END_NSS_ENABLE
endif

# #############################################################################
# Define ECM_FRONT_END_SFE_ENABLE=y in order to select
# sfe as ECM's front end.
# #############################################################################
ifeq ($(ECM_FRONT_END_SFE_ENABLE), y)
ecm-$(ECM_FRONT_END_SFE_ENABLE) += frontends/sfe/ecm_sfe_ipv4.o
ecm-$(ECM_FRONT_END_SFE_ENABLE) += frontends/sfe/ecm_sfe_ported_ipv4.o
ccflags-$(ECM_FRONT_END_SFE_ENABLE) += -DECM_FRONT_END_SFE_ENABLE
endif

# #############################################################################
# Define ECM_INTERFACE_BOND_ENABLE=y in order to enable
# Bonding / Link Aggregation support.
# #############################################################################
ecm-$(ECM_INTERFACE_BOND_ENABLE) += frontends/nss/ecm_nss_bond_notifier.o
ccflags-$(ECM_INTERFACE_BOND_ENABLE) += -DECM_INTERFACE_BOND_ENABLE

# #############################################################################
# Define ECM_INTERFACE_PPPOE_ENABLE=y in order
# to enable support for PPPoE acceleration.
# #############################################################################
ECM_INTERFACE_PPPOE_ENABLE=y
ccflags-$(ECM_INTERFACE_PPPOE_ENABLE) += -DECM_INTERFACE_PPPOE_ENABLE

# #############################################################################
# Define ECM_INTERFACE_L2TPV2_ENABLE=y in order
# to enable support for l2tpv2 acceleration.
# #############################################################################
ccflags-$(ECM_INTERFACE_L2TPV2_ENABLE) += -DECM_INTERFACE_L2TPV2_ENABLE

# #############################################################################
# Define ECM_INTERFACE_PPTP_ENABLE=y in order
# to enable support for pptp acceleration.
# #############################################################################
ccflags-$(ECM_INTERFACE_PPTP_ENABLE) += -DECM_INTERFACE_PPTP_ENABLE

# #############################################################################
# if pppoe, l2tpv2, pptp acceleration is enabled, ppp should
# be enabled automatically
# #############################################################################
ECM_INTERFACE_PPP_ENABLE=y
ifeq "$(ECM_INTERFACE_PPPOE_ENABLE)" "n"
ifeq "$(ECM_INTERFACE_L2TPV2_ENABLE)" "n"
ifeq "$(ECM_INTERFACE_PPTP_ENABLE)" "n"
ECM_INTERFACE_PPP_ENABLE=n
endif
endif
endif
ccflags-$(ECM_INTERFACE_PPP_ENABLE) += -DECM_INTERFACE_PPP_ENABLE

# #############################################################################
# Define ECM_INTERFACE_MAP_T_ENABLE=y in order
# to enable support for MAP-T interface.
# #############################################################################
ccflags-$(ECM_INTERFACE_MAP_T_ENABLE) += -DECM_INTERFACE_MAP_T_ENABLE

# #############################################################################
# Define ECM_INTERFACE_GRE_TAP_ENABLE=y in order
# to enable support for GRE TAP interface.
# #############################################################################
ccflags-$(ECM_INTERFACE_GRE_TAP_ENABLE) += -DECM_INTERFACE_GRE_TAP_ENABLE

# #############################################################################
# Define ECM_INTERFACE_GRE_TUN_ENABLE=y in order
# to enable support for GRE TUN interface.
# #############################################################################
ccflags-$(ECM_INTERFACE_GRE_TUN_ENABLE) += -DECM_INTERFACE_GRE_TUN_ENABLE

# #############################################################################
# Define ECM_INTERFACE_SIT_ENABLE=y in order
# to enable support for SIT interface.
# #############################################################################
ccflags-$(ECM_INTERFACE_SIT_ENABLE) += -DECM_INTERFACE_SIT_ENABLE

# #############################################################################
# Define ECM_INTERFACE_TUNIPIP6_ENABLE=y in order
# to enable support for TUNIPIP6 interface.
# #############################################################################
ccflags-$(ECM_INTERFACE_TUNIPIP6_ENABLE) += -DECM_INTERFACE_TUNIPIP6_ENABLE

# #############################################################################
# Define ECM_INTERFACE_RAWIP_ENABLE=y in order
# to enable support for RAWIP interface.
# #############################################################################
ccflags-$(ECM_INTERFACE_RAWIP_ENABLE) += -DECM_INTERFACE_RAWIP_ENABLE

# #############################################################################
# Define ECM_MULTICAST_ENABLE=y in order to enable support for ECM Multicast
# #############################################################################
ifeq ($(ECM_FRONT_END_NSS_ENABLE), y)
#
# TODO: This is a workaround for external builds in which the qca-mcs source
# code is not available. This will be fixed later by breaking the dependency from ECM
# to qca-mcs
#
MCS_CONFIG:=$(shell  grep "CONFIG_PACKAGE_kmod-qca-mcs=y" $(TOPDIR)/.config)
MCS_ENABLED:=CONFIG_PACKAGE_kmod-qca-mcs=y
ifeq ($(MCS_CONFIG),$(MCS_ENABLED))
ECM_MULTICAST_ENABLE=y
ecm-$(ECM_MULTICAST_ENABLE) += frontends/nss/ecm_nss_multicast_ipv4.o
ecm-$(ECM_MULTICAST_ENABLE) += frontends/nss/ecm_nss_multicast_ipv6.o
ecm-$(ECM_MULTICAST_ENABLE) += ecm_db/ecm_db_multicast.o
ccflags-$(ECM_MULTICAST_ENABLE) += -DECM_MULTICAST_ENABLE
endif
endif

# #############################################################################
# Define ECM_INTERFACE_VLAN_ENABLE=y in order to enable support for VLAN
# #############################################################################
ECM_INTERFACE_VLAN_ENABLE=y
ccflags-$(ECM_INTERFACE_VLAN_ENABLE) += -DECM_INTERFACE_VLAN_ENABLE

# #############################################################################
# Define ECM_INTERFACE_IPSEC_ENABLE=y in order to enable support for IPSEC
# #############################################################################
ccflags-$(ECM_INTERFACE_IPSEC_ENABLE) += -DECM_INTERFACE_IPSEC_ENABLE

ECM_INTERFACE_IPSEC_GLUE_LAYER_SUPPORT_ENABLE=n
ifeq ($(SoC),$(filter $(SoC), ipq807x ipq807x_64 ipq60xx ipq60xx_64))
ECM_INTERFACE_IPSEC_GLUE_LAYER_SUPPORT_ENABLE=$(ECM_INTERFACE_IPSEC_ENABLE)
ccflags-$(ECM_INTERFACE_IPSEC_GLUE_LAYER_SUPPORT_ENABLE) += -DECM_INTERFACE_IPSEC_GLUE_LAYER_SUPPORT_ENABLE
endif

# #############################################################################
# Define ECM_IPV6_ENABLE=y in order to enable IPv6 support in the ECM.
# #############################################################################
ECM_IPV6_ENABLE=y
ifeq ($(ECM_FRONT_END_NSS_ENABLE), y)
ecm-$(ECM_IPV6_ENABLE) += frontends/nss/ecm_nss_ipv6.o
ecm-$(ECM_IPV6_ENABLE) += frontends/nss/ecm_nss_ported_ipv6.o
endif
ifeq ($(ECM_FRONT_END_SFE_ENABLE), y)
ecm-$(ECM_IPV6_ENABLE) += frontends/sfe/ecm_sfe_ipv6.o
ecm-$(ECM_IPV6_ENABLE) += frontends/sfe/ecm_sfe_ported_ipv6.o
endif
ccflags-$(ECM_IPV6_ENABLE) += -DECM_IPV6_ENABLE

# #############################################################################
# Define ECM_CLASSIFIER_MARK_ENABLE=y in order to enable mark classifier.
# #############################################################################
ECM_CLASSIFIER_MARK_ENABLE=y
ecm-$(ECM_CLASSIFIER_MARK_ENABLE) += ecm_classifier_mark.o
ccflags-$(ECM_CLASSIFIER_MARK_ENABLE) += -DECM_CLASSIFIER_MARK_ENABLE

# #############################################################################
# Define ECM_CLASSIFIER_NL_ENABLE=y in order to enable NL classifier.
# #############################################################################
ifeq ($(findstring 4.4., $(KERNELVERSION)),)
ECM_CLASSIFIER_NL_ENABLE=y
endif
ecm-$(ECM_CLASSIFIER_NL_ENABLE) += ecm_classifier_nl.o
ccflags-$(ECM_CLASSIFIER_NL_ENABLE) += -DECM_CLASSIFIER_NL_ENABLE

# #############################################################################
# Define ECM_CLASSIFIER_DSCP_ENABLE=y in order to enable DSCP classifier.
# #############################################################################
ECM_CLASSIFIER_DSCP_ENABLE=n
ecm-$(ECM_CLASSIFIER_DSCP_ENABLE) += ecm_classifier_dscp.o
ccflags-$(ECM_CLASSIFIER_DSCP_ENABLE) += -DECM_CLASSIFIER_DSCP_ENABLE

# #############################################################################
# Define ECM_CLASSIFIER_HYFI_ENABLE=y in order to enable
# the Hy-Fi classifier in ECM. Currently disabled until the integration
# with Hy-Fi is completed.
# #############################################################################
#
# TODO: This is a workaround for external builds in which the qca-hyfi-bridge source
# code is not available. This will be fixed later by breaking the dependency from ECM
# to qca-hyfi-bridge
#
HYFI_BRIDGE_CONFIG:=$(shell  grep "CONFIG_PACKAGE_kmod-qca-hyfi-bridge=y" $(TOPDIR)/.config)
HYFI_BRIDGE_ENABLED:=CONFIG_PACKAGE_kmod-qca-hyfi-bridge=y
ifeq ($(HYFI_BRIDGE_CONFIG),$(HYFI_BRIDGE_ENABLED))
ecm-$(ECM_CLASSIFIER_HYFI_ENABLE) += ecm_classifier_hyfi.o
ccflags-$(ECM_CLASSIFIER_HYFI_ENABLE) += -DECM_CLASSIFIER_HYFI_ENABLE
endif

# #############################################################################
# Define ECM_CLASSIFIER_PCC_ENABLE=y in order to enable
# the Parental Controls subsystem classifier in ECM. Currently disabled until
# customers require it / if they need to integrate their Parental Controls with it.
# #############################################################################
ECM_CLASSIFIER_PCC_ENABLE=y
ecm-$(ECM_CLASSIFIER_PCC_ENABLE) += ecm_classifier_pcc.o
ccflags-$(ECM_CLASSIFIER_PCC_ENABLE) += -DECM_CLASSIFIER_PCC_ENABLE

# #############################################################################
# Define ECM_NON_PORTED_SUPPORT_ENABLE=y in order to enable non-ported protocol.
# #############################################################################
ECM_NON_PORTED_SUPPORT_ENABLE=y
ifeq ($(ECM_FRONT_END_NSS_ENABLE), y)
ecm-$(ECM_NON_PORTED_SUPPORT_ENABLE) += frontends/nss/ecm_nss_non_ported_ipv4.o
ecm-$(ECM_NON_PORTED_SUPPORT_ENABLE) += frontends/nss/ecm_nss_non_ported_ipv6.o
endif
ifeq ($(ECM_FRONT_END_SFE_ENABLE), y)
ecm-$(ECM_NON_PORTED_SUPPORT_ENABLE) += frontends/sfe/ecm_sfe_non_ported_ipv4.o
ecm-$(ECM_NON_PORTED_SUPPORT_ENABLE) += frontends/sfe/ecm_sfe_non_ported_ipv6.o
endif
ccflags-$(ECM_NON_PORTED_SUPPORT_ENABLE) += -DECM_NON_PORTED_SUPPORT_ENABLE

# #############################################################################
# Define ECM_STATE_OUTPUT_ENABLE=y to support XML state output
# #############################################################################
ECM_STATE_OUTPUT_ENABLE=y
ecm-$(ECM_STATE_OUTPUT_ENABLE) += ecm_state.o
ccflags-$(ECM_STATE_OUTPUT_ENABLE) += -DECM_STATE_OUTPUT_ENABLE

# #############################################################################
# Define ECM_DB_ADVANCED_STATS_ENABLE to support XML state output
# #############################################################################
ECM_DB_ADVANCED_STATS_ENABLE=y
ccflags-$(ECM_DB_ADVANCED_STATS_ENABLE) += -DECM_DB_ADVANCED_STATS_ENABLE

# #############################################################################
# Define ECM_DB_CONNECTION_CROSS_REFERENCING_ENABLE=y in order to enable
# the database to track relationships between objects.
# #############################################################################
ECM_DB_CONNECTION_CROSS_REFERENCING_ENABLE=y
ccflags-$(ECM_DB_CONNECTION_CROSS_REFERENCING_ENABLE) += -DECM_DB_XREF_ENABLE

# #############################################################################
# Define ECM_TRACKER_DPI_SUPPORT_ENABLE=y in order to enable support for
# deep packet inspection and tracking of data with the trackers.
# #############################################################################
ECM_TRACKER_DPI_SUPPORT_ENABLE=y
ccflags-$(ECM_TRACKER_DPI_SUPPORT_ENABLE) += -DECM_TRACKER_DPI_SUPPORT_ENABLE

# #############################################################################
# Define ECM_DB_CLASSIFIER_TYPE_ASSIGNMENTS_TRACK_ENABLE=y in order to enable
# support for the database keeping lists of connections that are assigned
# on a per TYPE of classifier basis.
# #############################################################################
ECM_DB_CLASSIFIER_TYPE_ASSIGNMENTS_TRACK_ENABLE=y
ccflags-$(ECM_DB_CLASSIFIER_TYPE_ASSIGNMENTS_TRACK_ENABLE) += -DECM_DB_CTA_TRACK_ENABLE

# #############################################################################
# Define ECM_BAND_STEERING_ENABLE=y in order to enable
# band steering feature.
# #############################################################################
ECM_BAND_STEERING_ENABLE=y
ccflags-$(ECM_BAND_STEERING_ENABLE) += -DECM_BAND_STEERING_ENABLE

# #############################################################################
# Define ECM_INTERFACE_OVPN_ENABLE=y in order
# to enable support for OVPN acceleration.
# #############################################################################
ccflags-$(ECM_INTERFACE_OVPN_ENABLE) += -DECM_INTERFACE_OVPN_ENABLE

# #############################################################################
# Debug flags, set these to = 0 if you want to disable all debugging for that
# file.
# By turning off debugs you gain maximum ECM performance.
# #############################################################################
ccflags-y += -DECM_CLASSIFIER_DEBUG_LEVEL=1
ccflags-y += -DECM_CLASSIFIER_MARK_DEBUG_LEVEL=1
ccflags-y += -DECM_CLASSIFIER_DSCP_DEBUG_LEVEL=1
ccflags-y += -DECM_CLASSIFIER_HYFI_DEBUG_LEVEL=1
ccflags-y += -DECM_CLASSIFIER_PCC_DEBUG_LEVEL=1
ccflags-y += -DECM_CLASSIFIER_NL_DEBUG_LEVEL=1
ccflags-y += -DECM_CLASSIFIER_DEFAULT_DEBUG_LEVEL=1
ccflags-y += -DECM_DB_DEBUG_LEVEL=1
ccflags-y += -DECM_INIT_DEBUG_LEVEL=3
ccflags-y += -DECM_FRONT_END_IPV4_DEBUG_LEVEL=1
ccflags-y += -DECM_FRONT_END_IPV6_DEBUG_LEVEL=1
ccflags-y += -DECM_FRONT_END_COMMON_DEBUG_LEVEL=1
ccflags-y += -DECM_NSS_IPV4_DEBUG_LEVEL=1
ccflags-y += -DECM_NSS_PORTED_IPV4_DEBUG_LEVEL=1
ccflags-y += -DECM_NSS_NON_PORTED_IPV4_DEBUG_LEVEL=1
ccflags-y += -DECM_NSS_MULTICAST_IPV4_DEBUG_LEVEL=1
ccflags-y += -DECM_NSS_IPV6_DEBUG_LEVEL=1
ccflags-y += -DECM_NSS_PORTED_IPV6_DEBUG_LEVEL=1
ccflags-y += -DECM_NSS_NON_PORTED_IPV6_DEBUG_LEVEL=1
ccflags-y += -DECM_NSS_MULTICAST_IPV6_DEBUG_LEVEL=1
ccflags-y += -DECM_SFE_IPV4_DEBUG_LEVEL=1
ccflags-y += -DECM_SFE_PORTED_IPV4_DEBUG_LEVEL=1
ccflags-y += -DECM_SFE_NON_PORTED_IPV4_DEBUG_LEVEL=1
ccflags-y += -DECM_SFE_IPV6_DEBUG_LEVEL=1
ccflags-y += -DECM_SFE_PORTED_IPV6_DEBUG_LEVEL=1
ccflags-y += -DECM_SFE_NON_PORTED_IPV6_DEBUG_LEVEL=1
ccflags-y += -DECM_CONNTRACK_NOTIFIER_DEBUG_LEVEL=1
ccflags-y += -DECM_TRACKER_DEBUG_LEVEL=1
ccflags-y += -DECM_TRACKER_DATAGRAM_DEBUG_LEVEL=1
ccflags-y += -DECM_TRACKER_TCP_DEBUG_LEVEL=1
ccflags-y += -DECM_TRACKER_UDP_DEBUG_LEVEL=1
ccflags-y += -DECM_BOND_NOTIFIER_DEBUG_LEVEL=1
ccflags-y += -DECM_INTERFACE_DEBUG_LEVEL=1
ccflags-y += -DECM_STATE_DEBUG_LEVEL=1
ccflags-y += -DECM_OPENWRT_SUPPORT=1

ccflags-y += -I$(obj)/ -I$(obj)/ecm_db -I$(obj)/frontends/include -I$(obj)/frontends/nss -I$(obj)/frontends/sfe -I$(obj)/exports
ccflags-y += -Werror

obj ?= .
