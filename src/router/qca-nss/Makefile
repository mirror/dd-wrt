include $(TOP)/.config

KERNEL_VERSION := $(shell cat $(LINUXDIR)/include/config/kernel.release 2> /dev/null)
IDIR := $(INSTALLDIR)/lib/modules/$(KERNEL_VERSION)/net/sched
KDIR := $(LINUXDIR)
PWD := $(shell pwd)



#KERNEL_FLAGS = "-UCONFIG_NOPRINTK -I$(TOP)/qca-nss/qca-nss-drv/exports -I$(TOP)/qca-nss/qca-nss-gmac/ipq806x/exports -I$(TOP)/qca-nss/qca-nss-crypto/v1.0/include -I$(TOP)/qca-nss/shortcut-fe/exports -I$(TOP)/qca-nss/qca-nss-mcs -I$(TOP)/qca-nss/qca-ovsmgr/exports -I$(LINUXDIR)/net/openvswitch" 
ifeq ($(CONFIG_IPQ6018),y)
CFI_CRYPTOAPI_DIR:=cryptoapi/v2.0
CFI_OCF_DIR:=ocf/v2.0
CFI_IPSEC_DIR:=ipsec/v2.0
KERNEL_FLAGS=-I$(TOP)/qca-nss/qca-nss-drv/exports -I$(TOP)/qca-nss/qca-ssdk/include -I$(TOP)/qca-nss/qca-ssdk/include/common -I$(TOP)/qca-nss/qca-ssdk/include/sal/os  -I$(TOP)/qca-nss/qca-ssdk/include/sal/os/linux -I$(TOP)/qca-nss/qca-nss-gmac/ipq806x/exports -I$(TOP)/qca-nss/qca-nss-crypto/v2.0/include -I$(TOP)/qca-nss/shortcut-fe/exports -I$(TOP)/qca-nss/qca-nss-mcs -I$(TOP)/qca-nss/qca-ovsmgr/exports -I$(LINUXDIR)/net/openvswitch -DNSS_DEBUG_LEVEL=0 -DNDEBUG=1 -UCONFIG_NOPRINTK
NSS_CRYPTO_DIR:=v2.0
DTLSMGR_DIR:=v2.0
IPSECMGR_KLIPS:=
else
CFI_CRYPTOAPI_DIR:=cryptoapi/v1.1
CFI_OCF_DIR:=ocf/v1.0
CFI_IPSEC_DIR:=ipsec/v1.0
KERNEL_FLAGS=-I$(TOP)/qca-nss/qca-nss-drv/exports -I$(TOP)/qca-nss/qca-ssdk/include -I$(TOP)/qca-nss/qca-ssdk/include/common -I$(TOP)/qca-nss/qca-ssdk/include/sal/os  -I$(TOP)/qca-nss/qca-ssdk/include/sal/os/linux -I$(TOP)/qca-nss/qca-nss-gmac/ipq806x/exports -I$(TOP)/qca-nss/qca-nss-crypto/v1.0/include -I$(TOP)/qca-nss/shortcut-fe/exports -I$(TOP)/qca-nss/qca-nss-mcs -I$(TOP)/qca-nss/qca-ovsmgr/exports -I$(LINUXDIR)/net/openvswitch -DNSS_DEBUG_LEVEL=0 -DNDEBUG=1 -UCONFIG_NOPRINTK
NSS_CRYPTO_DIR:=v1.0
# DTLS Manager v1.0 for Akronite.
DTLSMGR_DIR:=v1.0
# IPsec Manager v1.0 for Akronite.
IPSECMGR_DIR:=v1.0
# KLIPS plugin not needed
IPSECMGR_KLIPS:=
endif

LNX_CONFIG_OPTS = LNX_MAKEOPTS='' PRJ_PATH=$(TOP)/qca-nss/qca-ssdk MODULE_TYPE=KSLIB modules

SSDK_MAKE_FLAGS+= \
	TARGET_NAME=$(ARCH)-linux- \
	TOOL_PATH=${shell dirname ${shell which $(ARCH)-openwrt-linux-gcc}} \
	SYS_PATH=$(LINUXDIR) \
	TOOLPREFIX=$(ARCH)-linux- \
	KVER=$(KERNEL_VERSION) \
	ARCH=arm64 \
	TARGET_SUFFIX= \
	GCC_VERSION=14.1.0 \
	EXTRA_CFLAGS="-fno-stack-protector" \
	SoC=ipq60xx \
	SHELL="/usr/bin/bash" \
	PTP_FEATURE=disable SWCONFIG_FEATURE=enable \
	ISISC_ENABLE=disable IN_QCA803X_PHY=FALSE \
	IN_QCA808X_PHY=FALSE IN_MALIBU_PHY=FALSE

ifeq ($(CONFIG_IPQ807x),y)
    SSDK_MAKE_FLAGS+= CHIP_TYPE=HPPE
endif

ifeq ($(CONFIG_IPQ6018),y)
    SSDK_MAKE_FLAGS+= CHIP_TYPE=CPPE
endif


all:
ifeq ($(CONFIG_IPQ6018),y)
	cp	qca-nss-dp/hal/soc_ops/ipq60xx/nss_ipq60xx.h qca-nss-dp/exports/nss_dp_arch.h
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-dp modules MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq60xx"
	$(MAKE) $(SSDK_MAKE_FLAGS) $(LNX_CONFIG_OPTS) -C qca-ssdk $(LNX_CONFIG_OPTS)
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-crypto modules MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq60xx" \
		NSS_CRYPTO_DIR=$(NSS_CRYPTO_DIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-cfi modules MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq60xx" \
		CFI_CRYPTOAPI_DIR=$(CFI_CRYPTOAPI_DIR) \
		CFI_OCF_DIR=$(CFI_OCF_DIR) \
		CFI_IPSEC_DIR=$(CFI_IPSEC_DIR)
#	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-drv modulesMODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq60xx"
else

	$(MAKE) -C lib KERNEL_FLAGS="$(KERNEL_FLAGS)"
	$(MAKE) -C nssinfo KERNEL_FLAGS="$(KERNEL_FLAGS)"
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-ovsmgr modules MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)"
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-mcs modules MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" CONFIG_SUPPORT_MLD=y CONFIG_SUPPORT_OVS=y
	$(MAKE) -C $(KDIR) M=$(PWD)/shortcut-fe modules MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)"
	$(MAKE) -C $(KDIR) M=$(PWD)/nss-ifb modules MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)"
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-cfi modules MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" \
		CFI_CRYPTOAPI_DIR=$(CFI_CRYPTOAPI_DIR) \
		CFI_OCF_DIR=$(CFI_OCF_DIR) \
		CFI_IPSEC_DIR=$(CFI_IPSEC_DIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-clients modules MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" pppoe=y tunipip6=m l2tpv2=y qdisc=y vxlanmgr=y pptp=y vlan-mgr=y netlink=y bridge-mgr=y \
		DTLSMGR_DIR="$(DTLSMGR_DIR)" \
		IPSECMGR_DIR="$(IPSECMGR_DIR)"
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-crypto modules MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" \
		NSS_CRYPTO_DIR=$(NSS_CRYPTO_DIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-drv modules MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x"
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-ecm-standard modules MODPROBE=true KBUILD_MODPOST_WARN=1 ECM_FRONT_END_NSS_ENABLE=y ECM_FRONT_END_SFE_ENABLE=y ECM_IPV6_ENABLE=y ECM_CLASSIFIER_OVS_ENABLE=y ECM_INTERFACE_OVS_BRIDGE_ENABLE=y ECM_CLASSIFIER_OVS_ENABLE=y EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x"
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-gmac modules MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -DCONFIG_NSS_DEBUG_LEVEL=4 -I$(PKG_BUILD_DIR)/nss_hal/include -I$(PKG_BUILD_DIR)/nss_hal/$(BOARD)" SoC="ipq806x"
endif

install:
ifeq ($(CONFIG_IPQ6018),y)
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-dp modules_install MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq60xx" INSTALL_MOD_PATH=$(INSTALLDIR)
	mkdir -p $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)
	cp qca-ssdk/*.ko $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)
	-find $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/extra -name *.ko -exec mv {} $(INSTALLDIR)/lib/modules/$(KERNELRELEASE) \;
	-rm -rf $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/extra 
	-find $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/updates -name *.ko -exec mv {} $(INSTALLDIR)/lib/modules/$(KERNELRELEASE) \;
	-rm -rf $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/updates
	-find $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/update -name *.ko -exec mv {} $(INSTALLDIR)/lib/modules/$(KERNELRELEASE) \;
	-rm -rf $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/update
	rm -f $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/modules*
else
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-ovsmgr modules_install MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" CONFIG_SUPPORT_MLD=y INSTALL_MOD_PATH=$(INSTALLDIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-mcs modules_install MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" INSTALL_MOD_PATH=$(INSTALLDIR) CONFIG_SUPPORT_MLD=y CONFIG_SUPPORT_OVS=y
	$(MAKE) -C $(KDIR) M=$(PWD)/shortcut-fe modules_install MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" INSTALL_MOD_PATH=$(INSTALLDIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/nss-ifb modules_install MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" INSTALL_MOD_PATH=$(INSTALLDIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-cfi modules_install MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" \
		CFI_CRYPTOAPI_DIR=$(CFI_CRYPTOAPI_DIR) \
		CFI_OCF_DIR=$(CFI_OCF_DIR) \
		CFI_IPSEC_DIR=$(CFI_IPSEC_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-clients modules_install MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" pppoe=y tunipip6=m l2tpv2=y qdisc=y vxlanmgr=y pptp=y vlan-mgr=y bridge-mgr=y \
		DTLSMGR_DIR="$(DTLSMGR_DIR)" \
		IPSECMGR_DIR="$(IPSECMGR_DIR)" INSTALL_MOD_PATH=$(INSTALLDIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-crypto modules_install MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" \
		NSS_CRYPTO_DIR=$(NSS_CRYPTO_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-drv modules_install MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" INSTALL_MOD_PATH=$(INSTALLDIR)

	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-ecm-standard modules_install MODPROBE=true KBUILD_MODPOST_WARN=1 ECM_FRONT_END_NSS_ENABLE=y ECM_FRONT_END_SFE_ENABLE=y ECM_IPV6_ENABLE=y ECM_CLASSIFIER_OVS_ENABLE=y ECM_INTERFACE_OVS_BRIDGE_ENABLE=y ECM_CLASSIFIER_OVS_ENABLE=y EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" INSTALL_MOD_PATH=$(INSTALLDIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-gmac modules_install MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -DCONFIG_NSS_DEBUG_LEVEL=4 -I$(PKG_BUILD_DIR)/nss_hal/include -I$(PKG_BUILD_DIR)/nss_hal/$(BOARD)" SoC="ipq806x" INSTALL_MOD_PATH=$(INSTALLDIR)

	-find $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/extra -name *.ko -exec mv {} $(INSTALLDIR)/lib/modules/$(KERNELRELEASE) \;
	-rm -rf $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/extra 
	-find $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/updates -name *.ko -exec mv {} $(INSTALLDIR)/lib/modules/$(KERNELRELEASE) \;
	-rm -rf $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/updates
	-find $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/update -name *.ko -exec mv {} $(INSTALLDIR)/lib/modules/$(KERNELRELEASE) \;
	-rm -rf $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/update
	rm -f $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/modules*
	mkdir -p $(INSTALLDIR)/lib/firmware
	cp firmware/akronite/* $(INSTALLDIR)/lib/firmware
	mkdir -p $(INSTALLDIR)/usr/sbin
	cp nssinfo/obj/nssinfo $(INSTALLDIR)/usr/sbin
endif

clean:
ifeq ($(CONFIG_IPQ6018),y)
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-dp clean MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq60xx"
	$(MAKE) $(SSDK_MAKE_FLAGS) LNX_MAKEOPTS='' PRJ_PATH=$(TOP)/qca-nss/qca-ssdk MODULE_TYPE=KSLIB clean -C qca-ssdk clean
	rm -f qca-ssdk/qca-ssdk.*
	rm -f qca-ssdk/*.dep
	rm -f qca-ssdk/modules.order
	rm -f qca-ssdk/Module.symvers
	rm -f qca-ssdk/.*.cmd
else

	-$(MAKE) -C lib clean KERNEL_FLAGS="$(KERNEL_FLAGS)"
	-$(MAKE) -C nssinfo clean KERNEL_FLAGS="$(KERNEL_FLAGS)"
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-ovsmgr clean MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" CONFIG_SUPPORT_MLD=y INSTALL_MOD_PATH=$(INSTALLDIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-mcs clean MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" INSTALL_MOD_PATH=$(INSTALLDIR) CONFIG_SUPPORT_MLD=y CONFIG_SUPPORT_OVS=y
	$(MAKE) -C $(KDIR) M=$(PWD)/shortcut-fe clean MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" INSTALL_MOD_PATH=$(INSTALLDIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/nss-ifb clean MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" INSTALL_MOD_PATH=$(INSTALLDIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-cfi clean MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" \
		CFI_CRYPTOAPI_DIR=$(CFI_CRYPTOAPI_DIR) \
		CFI_OCF_DIR=$(CFI_OCF_DIR) \
		CFI_IPSEC_DIR=$(CFI_IPSEC_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-clients clean MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" pppoe=y tunipip6=m l2tpv2=y qdisc=y vxlanmgr=y pptp=y vlan-mgr=y bridge-mgr=y \
		DTLSMGR_DIR="$(DTLSMGR_DIR)" \
		IPSECMGR_DIR="$(IPSECMGR_DIR)" INSTALL_MOD_PATH=$(INSTALLDIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-crypto clean MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" \
		NSS_CRYPTO_DIR=$(NSS_CRYPTO_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-drv clean MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" INSTALL_MOD_PATH=$(INSTALLDIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-ecm-standard clean MODPROBE=true KBUILD_MODPOST_WARN=1 ECM_FRONT_END_NSS_ENABLE=y ECM_FRONT_END_SFE_ENABLE=y ECM_IPV6_ENABLE=y ECM_CLASSIFIER_OVS_ENABLE=y ECM_INTERFACE_OVS_BRIDGE_ENABLE=y ECM_CLASSIFIER_OVS_ENABLE=y EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" INSTALL_MOD_PATH=$(INSTALLDIR)
	$(MAKE) -C $(KDIR) M=$(PWD)/qca-nss-gmac clean MODPROBE=true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -DCONFIG_NSS_DEBUG_LEVEL=4 -I$(PKG_BUILD_DIR)/nss_hal/include -I$(PKG_BUILD_DIR)/nss_hal/$(BOARD)" SoC="ipq806x" INSTALL_MOD_PATH=$(INSTALLDIR)
endif
