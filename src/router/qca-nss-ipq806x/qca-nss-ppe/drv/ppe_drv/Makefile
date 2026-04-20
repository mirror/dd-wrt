# Makefile for PPE driver

ccflags-y := -I$(obj)/../exports -I$(obj)/..
ccflags-y += -DPPE_DRV_BUILD_ID="$(BUILD_ID)"
ifeq ($(SoC),$(filter $(SoC),ipq53xx))
ccflags-y += -DNSS_PPE_IPQ53XX
endif

ifeq ($(SoC),$(filter $(SoC),ipq95xx))
ccflags-y += -DNSS_PPE_IPQ95XX
endif

ifeq ($(SoC),$(filter $(SoC),ipq95xx ipq53xx))
ccflags-y += -DPPE_DRV_FLOW_IG_MAC_WAR
endif

obj-m += qca-nss-ppe.o
qca-nss-ppe-objs := ppe_drv.o
qca-nss-ppe-objs += ppe_drv_acl.o
qca-nss-ppe-objs += ppe_drv_br.o
qca-nss-ppe-objs += ppe_drv_cc.o
qca-nss-ppe-objs += ppe_drv_dp.o
ifdef PPE_IPSEC_ENABLE
qca-nss-ppe-objs += ppe_drv_eip.o
endif
qca-nss-ppe-objs += ppe_drv_exception.o
qca-nss-ppe-objs += ppe_drv_flow.o
qca-nss-ppe-objs += ppe_drv_host.o
qca-nss-ppe-objs += ppe_drv_iface.o
qca-nss-ppe-objs += ppe_drv_lag.o
qca-nss-ppe-objs += ppe_drv_l3_if.o
qca-nss-ppe-objs += ppe_drv_nexthop.o
qca-nss-ppe-objs += ppe_drv_policer.o
qca-nss-ppe-objs += ppe_drv_port.o
qca-nss-ppe-objs += ppe_drv_pppoe.o
qca-nss-ppe-objs += ppe_drv_pppoe_session.o
qca-nss-ppe-objs += ppe_drv_pub_ip.o
qca-nss-ppe-objs += ppe_drv_qos.o
qca-nss-ppe-objs += ppe_drv_sc.o
qca-nss-ppe-objs += ppe_drv_stats.o
qca-nss-ppe-objs += ppe_drv_vlan.o
qca-nss-ppe-objs += ppe_drv_vp.o
qca-nss-ppe-objs += ppe_drv_vsi.o
qca-nss-ppe-objs += ppe_drv_v4.o
qca-nss-ppe-objs += ppe_drv_v6.o
qca-nss-ppe-objs += ppe_drv_flow_dump.o
qca-nss-ppe-objs += ppe_drv_if_map.o
ifdef PPE_TUN_ENABLE
ccflags-y += -DPPE_TUNNEL_ENABLE
qca-nss-ppe-objs += tun/ppe_drv_tun.o
qca-nss-ppe-objs += tun/ppe_drv_tun_decap.o
qca-nss-ppe-objs += tun/ppe_drv_tun_decap_map.o
qca-nss-ppe-objs += tun/ppe_drv_tun_decap_xlate_rule.o
qca-nss-ppe-objs += tun/ppe_drv_tun_encap.o
qca-nss-ppe-objs += tun/ppe_drv_tun_encap_xlate_rule.o
qca-nss-ppe-objs += tun/ppe_drv_tun_l2tp.o
qca-nss-ppe-objs += tun/ppe_drv_tun_l3_if.o
qca-nss-ppe-objs += tun/ppe_drv_tun_prgm_prsr.o
qca-nss-ppe-objs += tun/ppe_drv_tun_prgm_prsr_gre.o
qca-nss-ppe-objs += tun/ppe_drv_tun_udf.o
qca-nss-ppe-objs += tun/ppe_drv_tun_v4.o
qca-nss-ppe-objs += tun/ppe_drv_tun_v6.o
endif

ccflags-y += -Wall -Werror
ccflags-y += -DPPE_DRV_DEBUG_LEVEL=3
