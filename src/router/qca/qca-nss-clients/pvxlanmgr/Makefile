ccflags-y := -I$(obj)/../exports -I$(obj)/..  -I$(obj)/nss_hal/include \
	-DNSS_CLIENT_BUILD_ID="$(BUILD_ID)" -DNSS_PVXLANMGR_DEBUG_LEVEL=2 -Wall -Werror

obj-m += qca-nss-pvxlanmgr.o
qca-nss-pvxlanmgr-objs := nss_pvxlanmgr.o
qca-nss-pvxlanmgr-objs += nss_pvxlanmgr_tunnel.o
