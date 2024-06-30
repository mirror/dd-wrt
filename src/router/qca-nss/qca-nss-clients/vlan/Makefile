ccflags-y := -I$(obj)/../exports -I$(obj)/..  -I$(obj)/nss_hal/include
ccflags-y += -DNSS_CLIENT_BUILD_ID="$(BUILD_ID)"

obj-m += qca-nss-vlan-$(SoC).o
qca-nss-vlan-$(SoC)-objs := nss_vlan_mgr.o

ifeq ($(SoC),$(filter $(SoC),ipq807x ipq807x_64 ipq60xx ipq60xx_64))
ccflags-y += -DNSS_VLAN_MGR_PPE_SUPPORT
endif

ccflags-y += -DNSS_VLAN_MGR_DEBUG_LEVEL=4
ccflags-y += -Wall -Werror

ifneq (,$(filter $(CONFIG_BONDING),y m))
ccflags-y += -DBONDING_SUPPORT
endif
