ccflags-y := -I$(obj)/../../exports -I$(obj)/.. -I$(obj)/../../drv/exports
ccflags-y += -DNSS_PPE_BUILD_ID="$(BUILD_ID)"

#Remove this once the file is exported
ccflags-y += -I$(obj)/../../drv/ppe_drv/tun/

obj-m += qca-nss-ppe-vxlanmgr.o
qca-nss-ppe-vxlanmgr-objs := nss_ppe_vxlanmgr.o
qca-nss-ppe-vxlanmgr-objs += nss_ppe_vxlanmgr_tunnel.o
qca-nss-ppe-vxlanmgr-objs += nss_ppe_vxlanmgr_tun_stats.o

ccflags-y += -DNSS_PPE_VXLAN_MGR_DEBUG_LEVEL=3
ccflags-y += -Wall -Werror
