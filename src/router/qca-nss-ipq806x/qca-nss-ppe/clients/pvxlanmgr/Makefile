ccflags-y := -I$(obj)/../../exports -I$(obj)/../../drv/exports
ccflags-y += -DNSS_PPE_BUILD_ID="$(BUILD_ID)"
ccflags-y += -DNSS_PVXLANMGR_DEBUG_LEVEL=0 -Wall -Werror

obj-m += qca-nss-pvxlanmgr.o
qca-nss-pvxlanmgr-objs := nss_pvxlanmgr.o
qca-nss-pvxlanmgr-objs += nss_pvxlanmgr_tunnel.o
