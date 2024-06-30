ccflags-y += -I$(obj)/../exports -I$(obj)/.. -I$(obj)/nss_hal/include
ccflags-y += -DNSS_CLIENT_BUILD_ID="$(BUILD_ID)"
ccflags-y += -DNSS_VXLANMGR_DEBUG_LEVEL=2
ccflags-y += -Wall -Werror

obj-m += qca-nss-vxlanmgr-$(SoC).o
qca-nss-vxlanmgr-$(SoC)-objs := nss_vxlanmgr.o
qca-nss-vxlanmgr-$(SoC)-objs += nss_vxlanmgr_tunnel.o
qca-nss-vxlanmgr-$(SoC)-objs += nss_vxlanmgr_tun_stats.o
