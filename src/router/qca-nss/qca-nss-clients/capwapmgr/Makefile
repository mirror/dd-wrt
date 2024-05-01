ccflags-y := -I$(obj)/../exports -I$(obj)/..  -I$(obj)/nss_hal/include
ccflags-y += -DNSS_CLIENT_BUILD_ID="$(BUILD_ID)"

#
# If you want only one netdev for all the CAPWAP tunnels, then
# set NSS_CAPWAPMGR_ONE_NETDEV.
#     ccflags-y += -DNSS_CAPWAPMGR_ONE_NETDEV
#
# If you want one netdevice per-CAPWAP tunnel, then don't set
# NSS_CAPWAPMGR_ONE_NETDEV.
#
obj-m += qca-nss-capwapmgr.o
qca-nss-capwapmgr-objs := nss_capwapmgr.o
ccflags-y += -DNSS_CAPWAPMGR_DEBUG_LEVEL=6

ccflags-y += $(NSS_CCFLAGS) -DNSS_DEBUG_LEVEL=0 -DNSS_PKT_STATS_ENABLED=0
ccflags-y += -Werror
