ccflags-y := -I$(obj)/../exports -I$(obj)/..  -I$(obj)/nss_hal/include
ccflags-y += -DNSS_CLIENT_BUILD_ID="$(BUILD_ID)"

#
# If you want to build qca-nss-portifmgr with QCA Ref design AP148 support
# set NSS_PORTIFMGR_REF_AP148
#     ccflags-y += -DNSS_PORTIFMGR_REF_AP148
# Otherwise qca-nss-portifmgr will only provide APIs for creating interfaces

obj-m += qca-nss-portifmgr.o
qca-nss-portifmgr-objs := nss_portifmgr.o

ccflags-y += -DNSS_PORTIFMGR_DEBUG_LEVEL=2 #-DNSS_PORTIFMGR_REF_AP148
ccflags-y += -Werror