ccflags-y := -I$(obj)/../../exports -I$(obj)/.. -I$(obj)/../../drv/exports
ccflags-y += -DNSS_PPE_BUILD_ID="$(BUILD_ID)"

obj-m += qca-nss-ppe-vlan.o
qca-nss-ppe-vlan-objs := nss_ppe_vlan_mgr.o

ccflags-y += -DNSS_PPE_VLAN_MGR_DEBUG_LEVEL=0
ccflags-y += -Wall -Werror

ifeq ($(SoC),$(filter $(SoC),ipq95xx ipq53xx))
ccflags-y += -DNSS_VLAN_MGR_PPE_VP_TUN_SUPPORT
endif

#
# TODO: Remove the following check when BONDING support is added for PPE
#
ifeq ($(findstring 6.1, $(KERNELVERSION)),)
ifneq (,$(filter $(CONFIG_BONDING),y m))
ccflags-y += -DBONDING_SUPPORT
endif
endif
