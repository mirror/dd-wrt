include $(TOP)/.config

KERNEL_VERSION := $(shell cat $(LINUXDIR)/include/config/kernel.release 2> /dev/null)
IDIR := $(INSTALLDIR)/lib/modules/$(KERNEL_VERSION)/net/sched
KDIR := $(LINUXDIR)
PWD := $(shell pwd)

export CROSS_COMPILE := ccache $(CROSS_COMPILE)


ifeq ($(CONFIG_IPQ6018),y)
CFI_CRYPTOAPI_DIR:=cryptoapi/v2.0
CFI_OCF_DIR:=ocf/v2.0
CFI_IPSEC_DIR:=ipsec/v2.0
KERNEL_FLAGS=-I$(TOP)/qca-nss/qca-nss-drv/exports -I$(TOP)/qca-nss/qca-ssdk/include -I$(TOP)/qca-nss/qca-ssdk/include/common -I$(TOP)/qca-nss/qca-ssdk/include/sal/os  -I$(TOP)/qca-nss/qca-ssdk/include/sal/os/linux -I$(TOP)/qca-nss/qca-nss-gmac/ipq806x/exports -I$(TOP)/qca-nss/qca-nss-crypto/v1.0/include -I$(TOP)/qca-nss/shortcut-fe/exports -I$(TOP)/qca-nss/qca-nss-mcs -I$(TOP)/qca-nss/qca-ovsmgr/exports -I$(LINUXDIR)/net/openvswitch -I$(TOP)/qca-nss/qca-nss-hyfi-bridge/hyfi-netfilter -DNSS_DEBUG_LEVEL=0 -DNDEBUG=1 -UCONFIG_NOPRINTK -DWEAK=__weak -DNSS_FIRMWARE_VERSION_12_5
KERNEL_FLAGS_IPQ6018=-I$(TOP)/qca-nss/ipq6018/qca-nss-ppe/drv/exports -I$(TOP)/qca-nss/ipq6018/qca-nss-drv/exports -I$(TOP)/qca-nss/ipq6018/qca-nss-cfi/cryptoapi/exports -I$(TOP)/qca-nss/ipq6018/qca-nss-clients/exports -I$(TOP)/qca-nss/ipq6018/qca-nss-dp/exports  -I$(TOP)/qca-nss/ipq6018/qca-ssdk/include -I$(TOP)/qca-nss/ipq6018/qca-ssdk/include/init -I$(TOP)/qca-nss/ipq6018/qca-ssdk/include/fal -I$(TOP)/qca-nss/ipq6018/qca-ssdk/include/common -I$(TOP)/qca-nss/ipq6018/qca-ssdk/include/sal/os  -I$(TOP)/qca-nss/ipq6018/qca-ssdk/include/sal/os/linux -I$(TOP)/qca-nss/ipq6018/qca-nss-gmac/ipq806x/exports -I$(TOP)/qca-nss/ipq6018/qca-nss-crypto/v2.0/include -I$(TOP)/qca-nss/ipq6018/shortcut-fe/exports -I$(TOP)/qca-nss/ipq6018/qca-nss-mcs -I$(TOP)/qca-nss/ipq6018/qca-ovsmgr/exports -I$(LINUXDIR)/net/openvswitch  -I$(TOP)/qca-nss/ipq6018/qca-nss-hyfi-bridge/hyfi-netfilter -DNSS_DEBUG_LEVEL=0 -DNDEBUG=1 -UCONFIG_NOPRINTK -DWEAK= -DNSS_FIRMWARE_VERSION_12_5
KERNEL_FLAGS_IPQ807x=-I$(TOP)/qca-nss/ipq807x/qca-nss-ppe/drv/exports -I$(TOP)/qca-nss/ipq807x/qca-nss-drv/exports -I$(TOP)/qca-nss/ipq807x/qca-nss-cfi/cryptoapi/exports -I$(TOP)/qca-nss/ipq807x/qca-nss-clients/exports -I$(TOP)/qca-nss/ipq807x/qca-nss-dp/exports  -I$(TOP)/qca-nss/ipq807x/qca-ssdk/include -I$(TOP)/qca-nss/ipq807x/qca-ssdk/include/init -I$(TOP)/qca-nss/ipq807x/qca-ssdk/include/fal -I$(TOP)/qca-nss/ipq807x/qca-ssdk/include/common -I$(TOP)/qca-nss/ipq807x/qca-ssdk/include/sal/os  -I$(TOP)/qca-nss/ipq807x/qca-ssdk/include/sal/os/linux -I$(TOP)/qca-nss/ipq807x/qca-nss-gmac/ipq806x/exports -I$(TOP)/qca-nss/ipq807x/qca-nss-crypto/v2.0/include -I$(TOP)/qca-nss/ipq807x/shortcut-fe/exports -I$(TOP)/qca-nss/ipq807x/qca-nss-mcs -I$(TOP)/qca-nss/ipq807x/qca-ovsmgr/exports -I$(LINUXDIR)/net/openvswitch  -I$(TOP)/qca-nss/ipq807x/qca-nss-hyfi-bridge/hyfi-netfilter -DNSS_DEBUG_LEVEL=0 -DNDEBUG=1 -UCONFIG_NOPRINTK -DWEAK= -DNSS_FIRMWARE_VERSION_12_5
KERNEL_FLAGS_IPQ5018=-I$(TOP)/qca-nss/ipq5018/qca-nss-ppe/drv/exports -I$(TOP)/qca-nss/ipq5018/qca-nss-drv/exports -I$(TOP)/qca-nss/ipq5018/qca-nss-cfi/cryptoapi/exports -I$(TOP)/qca-nss/ipq5018/qca-nss-clients/exports -I$(TOP)/qca-nss/ipq5018/qca-nss-dp/exports  -I$(TOP)/qca-nss/ipq5018/qca-ssdk/include -I$(TOP)/qca-nss/ipq5018/qca-ssdk/include/init -I$(TOP)/qca-nss/ipq5018/qca-ssdk/include/fal -I$(TOP)/qca-nss/ipq5018/qca-ssdk/include/common -I$(TOP)/qca-nss/ipq5018/qca-ssdk/include/sal/os  -I$(TOP)/qca-nss/ipq5018/qca-ssdk/include/sal/os/linux -I$(TOP)/qca-nss/ipq5018/qca-nss-gmac/ipq806x/exports -I$(TOP)/qca-nss/ipq5018/qca-nss-crypto/v2.0/include -I$(TOP)/qca-nss/ipq5018/shortcut-fe/exports -I$(TOP)/qca-nss/ipq5018/qca-nss-mcs -I$(TOP)/qca-nss/ipq5018/qca-ovsmgr/exports -I$(LINUXDIR)/net/openvswitch  -I$(TOP)/qca-nss/ipq5018/qca-nss-hyfi-bridge/hyfi-netfilter -DNSS_DEBUG_LEVEL=0 -DNDEBUG=1 -UCONFIG_NOPRINTK -DWEAK= -DNSS_FIRMWARE_VERSION_12_5
KERNEL_FLAGS_IPQ95xx=-I$(TOP)/qca-nss/ipq95xx/qca-nss-ppe/drv/exports -I$(TOP)/qca-nss/ipq95xx/qca-nss-drv/exports -I$(TOP)/qca-nss/ipq95xx/qca-nss-cfi/cryptoapi/exports -I$(TOP)/qca-nss/ipq95xx/qca-nss-clients/exports -I$(TOP)/qca-nss/ipq95xx/qca-nss-dp/exports  -I$(TOP)/qca-nss/ipq95xx/qca-ssdk/include -I$(TOP)/qca-nss/ipq95xx/qca-ssdk/include/init -I$(TOP)/qca-nss/ipq95xx/qca-ssdk/include/fal -I$(TOP)/qca-nss/ipq95xx/qca-ssdk/include/common -I$(TOP)/qca-nss/ipq95xx/qca-ssdk/include/sal/os  -I$(TOP)/qca-nss/ipq95xx/qca-ssdk/include/sal/os/linux -I$(TOP)/qca-nss/ipq95xx/qca-nss-gmac/ipq806x/exports -I$(TOP)/qca-nss/ipq95xx/qca-nss-crypto/v2.0/include -I$(TOP)/qca-nss/ipq95xx/shortcut-fe/exports -I$(TOP)/qca-nss/ipq95xx/qca-nss-mcs -I$(TOP)/qca-nss/ipq95xx/qca-ovsmgr/exports -I$(LINUXDIR)/net/openvswitch  -I$(TOP)/qca-nss/ipq95xx/qca-nss-hyfi-bridge/hyfi-netfilter -DNSS_DEBUG_LEVEL=0 -DNDEBUG=1 -UCONFIG_NOPRINTK -DWEAK= -DNSS_FIRMWARE_VERSION_12_5
NSS_CRYPTO_DIR:=v2.0
DTLSMGR_DIR:=v2.0
IPSECMGR_KLIPS:=

IPSECMGR_DIR:=v2.0
# KLIPS plugin not needed
IPSECMGR_KLIPS:=
else
CFI_CRYPTOAPI_DIR:=cryptoapi/v1.1
CFI_OCF_DIR:=ocf/v1.0
CFI_IPSEC_DIR:=ipsec/v1.0
KERNEL_FLAGS=-I$(TOP)/qca-nss/qca-nss-drv/exports -I$(TOP)/qca-nss/qca-ssdk/include -I$(TOP)/qca-nss/qca-ssdk/include/common -I$(TOP)/qca-nss/qca-ssdk/include/sal/os  -I$(TOP)/qca-nss/qca-ssdk/include/sal/os/linux -I$(TOP)/qca-nss/qca-nss-gmac/ipq806x/exports -I$(TOP)/qca-nss/qca-nss-crypto/v1.0/include -I$(TOP)/qca-nss/shortcut-fe/exports -I$(TOP)/qca-nss/qca-nss-mcs -I$(TOP)/qca-nss/qca-ovsmgr/exports -I$(LINUXDIR)/net/openvswitch -I$(TOP)/qca-nss/qca-nss-hyfi-bridge/hyfi-netfilter -DNSS_DEBUG_LEVEL=0 -DNDEBUG=1 -UCONFIG_NOPRINTK -DWEAK=__weak -DNSS_FIRMWARE_VERSION_12_5
NSS_CRYPTO_DIR:=v1.0
# DTLS Manager v1.0 for Akronite.
DTLSMGR_DIR:=v1.0
# IPsec Manager v1.0 for Akronite.
IPSECMGR_DIR:=v1.0
# KLIPS plugin not needed
IPSECMGR_KLIPS:=
endif


LNX_CONFIG_OPTS = LNX_MAKEOPTS='' PRJ_PATH=$(TOP)/qca-nss/qca-ssdk MODULE_TYPE=KSLIB modules
LNX_CONFIG_OPTS_IPQ6018 = LNX_MAKEOPTS='' PRJ_PATH=$(TOP)/qca-nss/ipq6018/qca-ssdk MODULE_TYPE=KSLIB modules
LNX_CONFIG_OPTS_IPQ807x = LNX_MAKEOPTS='' PRJ_PATH=$(TOP)/qca-nss/ipq807x/qca-ssdk MODULE_TYPE=KSLIB modules
LNX_CONFIG_OPTS_IPQ5018 = LNX_MAKEOPTS='' PRJ_PATH=$(TOP)/qca-nss/ipq5018/qca-ssdk MODULE_TYPE=KSLIB modules
LNX_CONFIG_OPTS_IPQ95xx = LNX_MAKEOPTS='' PRJ_PATH=$(TOP)/qca-nss/ipq95xx/qca-ssdk MODULE_TYPE=KSLIB modules

SSDK_MAKE_FLAGS_IPQ6018+= \
	TARGET_NAME=$(ARCH)-linux- \
	TOOL_PATH=${shell dirname ${shell which $(ARCH)-openwrt-linux-gcc}} \
	SYS_PATH=$(LINUXDIR) \
	TOOLPREFIX=$(ARCH)-linux- \
	KVER=$(KERNEL_VERSION) \
	ARCH=arm64 \
	TARGET_SUFFIX= \
	GCC_VERSION=16.1.0 \
	EXTRA_CFLAGS="-fno-stack-protector -Wno-enum-conversion" \
	SoC=ipq60xx \
	SHELL="/usr/bin/bash" \
	PTP_FEATURE=disable SWCONFIG_FEATURE=enable \
	ISISC_ENABLE=disable MHT_ENABLE=disable \
	IN_QCA803X_PHY=FALSE IN_QCA808X_PHY=FALSE \
	IN_MALIBU_PHY=FALSE IN_AQUANTIA_PHY=TRUE

SSDK_MAKE_FLAGS_IPQ5018+= \
	TARGET_NAME=$(ARCH)-linux- \
	TOOL_PATH=${shell dirname ${shell which $(ARCH)-openwrt-linux-gcc}} \
	SYS_PATH=$(LINUXDIR) \
	TOOLPREFIX=$(ARCH)-linux- \
	KVER=$(KERNEL_VERSION) \
	ARCH=arm64 \
	TARGET_SUFFIX= \
	GCC_VERSION=16.1.0 \
	EXTRA_CFLAGS="-fno-stack-protector -Wno-enum-conversion" \
	SoC=ipq50xx \
	SHELL="/usr/bin/bash" \
	PTP_FEATURE=disable SWCONFIG_FEATURE=enable \
	ISISC_ENABLE=disable MHT_ENABLE=disable \
	IN_QCA803X_PHY=FALSE IN_QCA808X_PHY=FALSE \
	IN_MALIBU_PHY=FALSE

SSDK_MAKE_FLAGS_IPQ807X+= \
	TARGET_NAME=$(ARCH)-linux- \
	TOOL_PATH=${shell dirname ${shell which $(ARCH)-openwrt-linux-gcc}} \
	SYS_PATH=$(LINUXDIR) \
	TOOLPREFIX=$(ARCH)-linux- \
	KVER=$(KERNEL_VERSION) \
	ARCH=arm64 \
	TARGET_SUFFIX= \
	GCC_VERSION=16.1.0 \
	EXTRA_CFLAGS="-fno-stack-protector -Wno-enum-conversion" \
	SoC=ipq807x \
	SHELL="/usr/bin/bash" \
	PTP_FEATURE=disable SWCONFIG_FEATURE=enable \
	ISISC_ENABLE=disable MHT_ENABLE=disable \
	IN_QCA803X_PHY=FALSE IN_QCA808X_PHY=FALSE \
	IN_MALIBU_PHY=FALSE

SSDK_MAKE_FLAGS_IPQ95XX+= \
	TARGET_NAME=$(ARCH)-linux- \
	TOOL_PATH=${shell dirname ${shell which $(ARCH)-openwrt-linux-gcc}} \
	SYS_PATH=$(LINUXDIR) \
	TOOLPREFIX=$(ARCH)-linux- \
	KVER=$(KERNEL_VERSION) \
	ARCH=arm64 \
	TARGET_SUFFIX= \
	GCC_VERSION=16.1.0 \
	EXTRA_CFLAGS="-fno-stack-protector -Wno-enum-conversion"\
	SoC=ipq95xx \
	SHELL="/usr/bin/bash" \
	PTP_FEATURE=enable SWCONFIG_FEATURE=enable \
	ISISC_ENABLE=disable MHT_ENABLE=enable \
	IN_QCA803X_PHY=TRUE IN_QCA808X_PHY=TRUE IN_AQUANTIA_PHY=TRUE \
	IN_SFP_PHY=TRUE IN_SFP=TRUE IN_MALIBU_PHY=TRUE IN_QCA803X_PHY=TRUE

SSDK_MAKE_FLAGS_IPQ807X+= CHIP_TYPE=HPPE
SSDK_MAKE_FLAGS_IPQ6018+= CHIP_TYPE=CPPE
SSDK_MAKE_FLAGS_IPQ5018+= CHIP_TYPE=MP ISISC_ENABLE=enable IN_QCA808X_PHY=TRUE
SSDK_MAKE_FLAGS_IPQ95XX+= CHIP_TYPE=APPE IN_QCA808X_PHY=TRUE

IPQ95XX_NSS_FLAGS= NSS_DRV_POINT_OFFLOAD=y \
	NSS_DRV_BRIDGE_ENABLE=n \
	NSS_DRV_ETH_RX_ENABLE=n \
	NSS_DRV_IPV4_ENABLE=n \
	NSS_DRV_IPV6_ENABLE=n \
	NSS_DRV_LSO_RX_ENABLE=n \
	NSS_DRV_PPPOE_ENABLE=n \
	NSS_DRV_UDP_ST_ENABLE=n \
	NSS_DRV_VLAN_ENABLE=n \
	NSS_DRV_WIFIOFFLOAD_ENABLE=n \
	NSS_DRV_PORTID_ENABLE=n \
	NSS_DRV_RMNET_ENABLE=n \
	PPE_RFS_ENABLED=y \
	PPE_ACL_ENABLED=y \
	PPE_POLICER_ENABLED=y \
	PPE_MIRROR_ENABLED=y \
	PPE_PRIORITY_ENABLED=y


QCASSDK_CONFIG_OPTS+= TOOL_PATH=${shell dirname ${shell which $(ARCH)-openwrt-linux-gcc}} \
                SYS_PATH=$(LINUXDIR) \
		TOOLPREFIX=$(ARCH)-linux- \
                KVER=$(KERNEL_VERSION) \
                CFLAGS="$(COPTS) $(LTO) -DNEED_PRINTF" \
                LDFLAGS="$(LDLTO)" \
                ARCH=arm64


all:
ifeq ($(CONFIG_IPQ6018),y)
	mkdir -p ipq6018
	mkdir -p ipq807x
	mkdir -p ipq5018
	mkdir -p ipq95xx
	cp -urv qca-nss-dp ipq6018
	cp -urv qca-nss-dp ipq807x
	cp -urv qca-nss-dp ipq5018
	cp -urv qca-nss-dp ipq95xx

	cp -urv qca-nss-crypto ipq6018
	cp -urv qca-nss-crypto ipq807x
	cp -urv qca-nss-crypto ipq5018
	cp -urv qca-nss-crypto ipq95xx

	cp -urv qca-nss-drv ipq6018
	cp -urv qca-nss-drv ipq807x
	cp -urv qca-nss-drv ipq5018
	cp -urv qca-nss-drv ipq95xx

	cp -urv qca-nss-cfi ipq6018
	cp -urv qca-nss-cfi ipq807x
	cp -urv qca-nss-cfi ipq5018
	cp -urv qca-nss-cfi ipq95xx

	cp -urv qca-nss-ecm-standard ipq6018
	cp -urv qca-nss-ecm-standard ipq807x
	cp -urv qca-nss-ecm-standard ipq5018
	cp -urv qca-nss-ecm-standard ipq95xx

	cp -urv qca-nss-mcs ipq6018
	cp -urv qca-nss-mcs ipq807x
	cp -urv qca-nss-mcs ipq5018
	cp -urv qca-nss-mcs ipq95xx

	cp -urv qca-nss-eip ipq6018
	cp -urv qca-nss-eip ipq807x
	cp -urv qca-nss-eip ipq5018
	cp -urv qca-nss-eip ipq95xx

	cp -urv qca-ovsmgr ipq6018
	cp -urv qca-ovsmgr ipq807x
	cp -urv qca-ovsmgr ipq5018
	cp -urv qca-ovsmgr ipq95xx

	cp -urv qca-nss-macsec ipq6018
	cp -urv qca-nss-macsec ipq807x
	cp -urv qca-nss-macsec ipq5018
	cp -urv qca-nss-macsec ipq95xx


	cp -urv qca-nss-ppe ipq6018
	cp -urv qca-nss-ppe ipq807x
	cp -urv qca-nss-ppe ipq5018
	cp -urv qca-nss-ppe ipq95xx

	cp -urv shortcut-fe ipq6018
	cp -urv shortcut-fe ipq807x
	cp -urv shortcut-fe ipq5018
	cp -urv shortcut-fe ipq95xx

	cp -urv qca-nss-clients ipq6018
	cp -urv qca-nss-clients ipq807x
	cp -urv qca-nss-clients ipq5018
	cp -urv qca-nss-clients ipq95xx

	cp -urv qca-nss-hyfi-bridge ipq6018
	cp -urv qca-nss-hyfi-bridge ipq807x
	cp -urv qca-nss-hyfi-bridge ipq5018
	cp -urv qca-nss-hyfi-bridge ipq95xx


	cp -urv nss-ifb ipq6018
	cp -urv nss-ifb ipq807x
	cp -urv nss-ifb ipq5018
	cp -urv nss-ifb ipq95xx

	cp -urv qca-ssdk ipq6018
	cp -urv qca-ssdk ipq807x
	cp -urv qca-ssdk ipq5018
	cp -urv qca-ssdk ipq95xx



	make -C lib KERNEL_FLAGS="$(KERNEL_FLAGS)"
	make -C nssinfo KERNEL_FLAGS="$(KERNEL_FLAGS)"

	make $(QCASSDK_CONFIG_OPTS) -C ssdk-shell uslib_o
	make $(QCASSDK_CONFIG_OPTS) -C ssdk-shell

	cp 	-v qca-nss-dp/hal/soc_ops/ipq60xx/nss_ipq60xx.h ipq6018/qca-nss-dp/exports/nss_dp_arch.h
	cp	-v qca-nss-dp/hal/soc_ops/ipq807x/nss_ipq807x.h ipq807x/qca-nss-dp/exports/nss_dp_arch.h
	cp	-v qca-nss-dp/hal/soc_ops/ipq50xx/nss_ipq50xx.h ipq5018/qca-nss-dp/exports/nss_dp_arch.h
	cp	-v qca-nss-dp/hal/soc_ops/ipq95xx/nss_ipq95xx.h ipq95xx/qca-nss-dp/exports/nss_dp_arch.h

ifeq ($(CONFIG_IPQ95XX),y)
	make $(SSDK_MAKE_FLAGS_IPQ95XX) $(LNX_CONFIG_OPTS_IPQ95xx) -C ipq95xx/qca-ssdk $(LNX_CONFIG_OPTS_IPQ95xx)
	ln -sf arch/nss_ipq95xx.h ipq95xx/qca-nss-drv/exports/nss_arch.h
	make -C $(KDIR) M=$(PWD)/ipq95xx/qca-nss-dp modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ95xx) -DNSS_DP_POINT_OFFLOAD" dp-ppe-ds=y dp-loopback=y SoC="ipq95xx" INSTALL_MOD_PATH=$(INSTALLDIR)
	ln -sf arch/nss_ipq95xx.h ipq95xx/qca-nss-drv/exports/nss_arch.h
	make -C $(KDIR) M=$(PWD)/ipq95xx/qca-nss-drv modules MODPROBE=true DEPMOD=/bin/true $(IPQ95XX_NSS_FLAGS) KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ95xx)" SoC="ipq95xx" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq95xx/qca-nss-macsec modules MODPROBE=true DEPMOD=/bin/true $(IPQ95XX_NSS_FLAGS) KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ95xx)" SoC="ipq95xx" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq95xx/qca-nss-ppe modules MODPROBE=true DEPMOD=/bin/true $(IPQ95XX_NSS_FLAGS) KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ95xx)" SoC="ipq95xx" INSTALL_MOD_PATH=$(INSTALLDIR) PPE_TUN_ENABLE=y ppe-drv=y vlan-mgr=y netlink=y pppoe-mgr=y lag-mgr=y bridge-mgr=y ppe-tun=y ppe-ds=y vxlanmgr=y l2tp-mgr=y ppe-vp=y ppe-rule=y
	make -C $(KDIR) M=$(PWD)/ipq95xx/qca-nss-mcs modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 CONFIG_SUPPORT_MLD=y EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ95xx)" SoC="ipq95xx" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq95xx/shortcut-fe modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ95xx)" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq95xx/qca-nss-eip modules MODPROBE=true DEPMOD=/bin/true $(IPQ95XX_NSS_FLAGS) KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ95xx)" SoC="ipq95xx" INSTALL_MOD_PATH=$(INSTALLDIR) NSS_EIP_DIR=eip197 eip_crypto=m
	make -C $(KDIR) M=$(PWD)/ipq95xx/qca-nss-ecm-standard modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 ECM_CLASSIFIER_PCC_ENABLE=y ECM_INTERFACE_PPTP_ENABLE=y ECM_INTERFACE_L2TPV2_ENABLE=y ECM_INTERFACE_L2TPV3_ENABLE=y ECM_FRONT_END_PPE_ENABLE=y ECM_INTERFACE_RAWIP_ENABLE=y ECM_NON_PORTED_SUPPORT_ENABLE=y ECM_INTERFACE_VLAN_ENABLE=y ECM_FRONT_END_SFE_ENABLE=y ECM_IPV6_ENABLE=y ECM_CLASSIFIER_MARK_ENABLE=y  ECM_CLASSIFIER_DSCP_ENABLE=y ECM_CLASSIFIER_HYFI_ENABLE=y EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ95xx)"  SoC="ipq95xx" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq95xx/qca-nss-clients modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ95xx) -include $(TOP)/qca-nss/qca-nss-clients/compat.h" SoC="ipq95xx" tunipip6=n tlsmgr=n  pppoe=n vxlanmgr=n pptp=n l2tpv2=n  qdisc=n vlan-mgr=n netlink=n bridge-mgr=y lag-mgr=n ovsmgr=y wifi-meshmgr=n \
		DTLSMGR_DIR="$(DTLSMGR_DIR)" \
		IPSECMGR_DIR="$(IPSECMGR_DIR)" INSTALL_MOD_PATH=$(INSTALLDIR)

	mkdir -p $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)
	cp ipq95xx/qca-ssdk/qca-ssdk.ko $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/qca-ssdk-ipq95xx.ko
else

	make $(SSDK_MAKE_FLAGS_IPQ5018) $(LNX_CONFIG_OPTS_IPQ5018) -C ipq5018/qca-ssdk $(LNX_CONFIG_OPTS_IPQ5018)
	ln -sf arch/nss_ipq50xx.h ipq5018/qca-nss-drv/exports/nss_arch.h
	make -C $(KDIR) M=$(PWD)/ipq5018/qca-nss-crypto modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ5018)" SoC="ipq50xx" \
		NSS_CRYPTO_DIR=$(NSS_CRYPTO_DIR)  INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq5018/qca-nss-cfi modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ5018)" SoC="ipq50xx" \
		CFI_CRYPTOAPI_DIR=$(CFI_CRYPTOAPI_DIR) \
		CFI_OCF_DIR=$(CFI_OCF_DIR) \
		CFI_IPSEC_DIR=$(CFI_IPSEC_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq5018/qca-nss-dp modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ5018)" SoC="ipq50xx" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq5018/qca-nss-cfi modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ5018)" SoC="ipq50xx" \
		CFI_CRYPTOAPI_DIR=$(CFI_CRYPTOAPI_DIR) \
		CFI_OCF_DIR=$(CFI_OCF_DIR) \
		CFI_IPSEC_DIR=$(CFI_IPSEC_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq5018/qca-nss-crypto modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ5018)" SoC="ipq50xx" \
		NSS_CRYPTO_DIR=$(NSS_CRYPTO_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)

	ln -sf arch/nss_ipq50xx.h ipq5018/qca-nss-drv/exports/nss_arch.h
	make -C $(KDIR) M=$(PWD)/ipq5018/qca-nss-drv modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ5018)" SoC="ipq50xx" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq5018/qca-nss-clients modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ5018) -include $(TOP)/qca-nss/qca-nss-clients/compat.h" SoC="ipq50xx" tunipip6=y tlsmgr=n  pppoe=y vxlanmgr=y pptp=y l2tpv2=y  qdisc=y vlan-mgr=n netlink=y bridge-mgr=y lag-mgr=y ovsmgr=y wifi-meshmgr=y \
		DTLSMGR_DIR="$(DTLSMGR_DIR)" \
		IPSECMGR_DIR="$(IPSECMGR_DIR)" INSTALL_MOD_PATH=$(INSTALLDIR)

	mkdir -p $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)
	cp ipq5018/qca-ssdk/qca-ssdk.ko $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/qca-ssdk-ipq50xx.ko


#6018
	cp	qca-nss-dp/hal/soc_ops/ipq60xx/nss_ipq60xx.h ipq6018/qca-nss-dp/exports/nss_dp_arch.h
	make $(SSDK_MAKE_FLAGS_IPQ6018) $(LNX_CONFIG_OPTS_IPQ6018) -C ipq6018/qca-ssdk $(LNX_CONFIG_OPTS_IPQ6018)
	ln -sf arch/nss_ipq60xx.h ipq6018/qca-nss-drv/exports/nss_arch.h
	make -C $(KDIR) M=$(PWD)/ipq6018/qca-nss-crypto modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ6018)" SoC="ipq60xx" \
		NSS_CRYPTO_DIR=$(NSS_CRYPTO_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq6018/qca-nss-dp modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ6018)" SoC="ipq60xx" INSTALL_MOD_PATH=$(INSTALLDIR)


	make -C $(KDIR) M=$(PWD)/ipq6018/qca-nss-cfi modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ6018)" SoC="ipq60xx" \
		CFI_CRYPTOAPI_DIR=$(CFI_CRYPTOAPI_DIR) \
		CFI_OCF_DIR=$(CFI_OCF_DIR) \
		CFI_IPSEC_DIR=$(CFI_IPSEC_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)

	make -C $(KDIR) M=$(PWD)/ipq6018/qca-nss-crypto modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ6018)" SoC="ipq60xx" \
		NSS_CRYPTO_DIR=$(NSS_CRYPTO_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)

	ln -sf arch/nss_ipq60xx.h qca-nss-drv/exports/nss_arch.h
	make -C $(KDIR) M=$(PWD)/ipq6018/qca-nss-drv modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1  EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ6018)" SoC="ipq60xx" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq6018/qca-nss-clients modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ6018) -include $(TOP)/qca-nss/qca-nss-clients/compat.h" SoC="ipq60xx" tunipip6=y tlsmgr=n  pppoe=y vxlanmgr=y pptp=y l2tpv2=y  qdisc=y vlan-mgr=y netlink=y bridge-mgr=y lag-mgr=y ovsmgr=y wifi-meshmgr=y \
		DTLSMGR_DIR="$(DTLSMGR_DIR)" \
		IPSECMGR_DIR="$(IPSECMGR_DIR)" INSTALL_MOD_PATH=$(INSTALLDIR)

	mkdir -p $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)
	cp ipq6018/qca-ssdk/qca-ssdk.ko $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/qca-ssdk-ipq60xx.ko

#ipq807x
	cp	qca-nss-dp/hal/soc_ops/ipq807x/nss_ipq807x.h ipq807x/qca-nss-dp/exports/nss_dp_arch.h
	make $(SSDK_MAKE_FLAGS_IPQ807X) $(LNX_CONFIG_OPTS_IPQ807x) -C ipq807x/qca-ssdk $(LNX_CONFIG_OPTS_IPQ807x)
	ln -sf arch/nss_ipq807x.h qca-nss-drv/exports/nss_arch.h
	make -C $(KDIR) M=$(PWD)/ipq807x/qca-nss-dp modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ807x)" SoC="ipq807x" INSTALL_MOD_PATH=$(INSTALLDIR)
	mkdir -p $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)
	cp qca-ssdk/qca-ssdk.ko $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/qca-ssdk-ipq807x.ko

	make -C $(KDIR) M=$(PWD)/ipq807x/qca-nss-cfi modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ807x)" SoC="ipq807x" \
		CFI_CRYPTOAPI_DIR=$(CFI_CRYPTOAPI_DIR) \
		CFI_OCF_DIR=$(CFI_OCF_DIR) \
		CFI_IPSEC_DIR=$(CFI_IPSEC_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)

	make -C $(KDIR) M=$(PWD)/ipq807x/qca-nss-crypto modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ807x)" SoC="ipq807x" \
		NSS_CRYPTO_DIR=$(NSS_CRYPTO_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)

	ln -sf arch/nss_ipq807x.h ipq807x/qca-nss-drv/exports/nss_arch.h
	make -C $(KDIR) M=$(PWD)/ipq807x/qca-nss-drv modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ807x)" SoC="ipq807x" INSTALL_MOD_PATH=$(INSTALLDIR)
#	make -C $(KDIR) M=$(PWD)/ipq807x/qca-ovsmgr MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ807x)" CONFIG_SUPPORT_MLD=y INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq807x/qca-nss-mcs modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 CONFIG_SUPPORT_MLD=y EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ807x)" SoC="ipq807x" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq807x/shortcut-fe modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ807x)" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq807x/qca-nss-ecm-standard modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 ECM_INTERFACE_PPTP_ENABLE=y ECM_INTERFACE_L2TPV2_ENABLE=y ECM_INTERFACE_L2TPV3_ENABLE=y ECM_FRONT_END_NSS_ENABLE=y ECM_INTERFACE_RAWIP_ENABLE=y ECM_NON_PORTED_SUPPORT_ENABLE=y ECM_INTERFACE_VLAN_ENABLE=y ECM_FRONT_END_SFE_ENABLE=y ECM_IPV6_ENABLE=y ECM_CLASSIFIER_MARK_ENABLE=y  ECM_CLASSIFIER_DSCP_ENABLE=y ECM_CLASSIFIER_HYFI_ENABLE=y EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ807x)"  SoC="ipq807x" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq807x/qca-nss-clients modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ807x) -include $(TOP)/qca-nss/qca-nss-clients/compat.h" SoC="ipq807x" tunipip6=y tlsmgr=y  pppoe=y vxlanmgr=y pptp=y l2tpv2=y  qdisc=y vlan-mgr=y netlink=y bridge-mgr=y lag-mgr=y ovsmgr=y wifi-meshmgr=y \
		DTLSMGR_DIR="$(DTLSMGR_DIR)" \
		IPSECMGR_DIR="$(IPSECMGR_DIR)" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq807x/nss-ifb modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ807x)" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq807x/qca-nss-hyfi-bridge modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS_IPQ807x)" SoC="ipq807x" INSTALL_MOD_PATH=$(INSTALLDIR)
endif
else
	ln -sf arch/nss_ipq806x.h qca-nss-drv/exports/nss_arch.h
	make -C lib KERNEL_FLAGS="$(KERNEL_FLAGS)"
	make -C nssinfo KERNEL_FLAGS="$(KERNEL_FLAGS)"
	make -C $(KDIR) M=$(PWD)/qca-ovsmgr modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)"
	make -C $(KDIR) M=$(PWD)/qca-nss-mcs modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" CONFIG_SUPPORT_MLD=y CONFIG_SUPPORT_OVS=y
	make -C $(KDIR) M=$(PWD)/shortcut-fe modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)"
	make -C $(KDIR) M=$(PWD)/nss-ifb modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)"
	make -C $(KDIR) M=$(PWD)/qca-nss-cfi modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" \
		CFI_CRYPTOAPI_DIR=$(CFI_CRYPTOAPI_DIR) \
		CFI_OCF_DIR=$(CFI_OCF_DIR) \
		CFI_IPSEC_DIR=$(CFI_IPSEC_DIR)
	make -C $(KDIR) M=$(PWD)/qca-nss-clients modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -include $(TOP)/qca-nss/qca-nss-clients/compat.h" SoC="ipq806x" pppoe=y tunipip6=m l2tpv2=y qdisc=y vxlanmgr=y pptp=y vlan-mgr=y netlink=y bridge-mgr=y \
		DTLSMGR_DIR="$(DTLSMGR_DIR)" \
		IPSECMGR_DIR="$(IPSECMGR_DIR)"
	make -C $(KDIR) M=$(PWD)/qca-nss-crypto modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" \
		NSS_CRYPTO_DIR=$(NSS_CRYPTO_DIR)
	make -C $(KDIR) M=$(PWD)/qca-nss-drv modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x"
	make -C $(KDIR) M=$(PWD)/qca-nss-ecm-standard modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 ECM_INTERFACE_PPTP_ENABLE=y ECM_FRONT_END_NSS_ENABLE=y ECM_FRONT_END_SFE_ENABLE=y ECM_IPV6_ENABLE=y ECM_CLASSIFIER_HYFI_ENABLE=y EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x"
	make -C $(KDIR) M=$(PWD)/qca-nss-hyfi-bridge modules MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" INSTALL_MOD_PATH=$(INSTALLDIR)
endif

install:
ifeq ($(CONFIG_IPQ6018),y)
	cp 	-v qca-nss-dp/hal/soc_ops/ipq60xx/nss_ipq60xx.h ipq6018/qca-nss-dp/exports/nss_dp_arch.h
	cp	-v qca-nss-dp/hal/soc_ops/ipq807x/nss_ipq807x.h ipq807x/qca-nss-dp/exports/nss_dp_arch.h
	cp	-v qca-nss-dp/hal/soc_ops/ipq50xx/nss_ipq50xx.h ipq5018/qca-nss-dp/exports/nss_dp_arch.h
	cp	-v qca-nss-dp/hal/soc_ops/ipq95xx/nss_ipq95xx.h ipq95xx/qca-nss-dp/exports/nss_dp_arch.h

# 95xx
ifeq ($(CONFIG_IPQ95XX),y)
	ln -sf arch/nss_ipq95xx.h ipq95xx/qca-nss-drv/exports/nss_arch.h
	make -C $(KDIR) M=$(PWD)/ipq95xx/qca-nss-dp modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -DNSS_DP_POINT_OFFLOAD" dp-ppe-ds=y dp-loopback=y SoC="ipq95xx" INSTALL_MOD_PATH=$(INSTALLDIR)
	ln -sf arch/nss_ipq95xx.h ipq95xx/qca-nss-drv/exports/nss_arch.h
	make -C $(KDIR) M=$(PWD)/ipq95xx/qca-nss-drv modules_install MODPROBE=true DEPMOD=/bin/true $(IPQ95XX_NSS_FLAGS) KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq95xx" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq95xx/qca-nss-macsec modules_install MODPROBE=true DEPMOD=/bin/true $(IPQ95XX_NSS_FLAGS) KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq95xx" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq95xx/qca-nss-ppe modules_install MODPROBE=true DEPMOD=/bin/true $(IPQ95XX_NSS_FLAGS) KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq95xx" INSTALL_MOD_PATH=$(INSTALLDIR) PPE_TUN_ENABLE=y ppe-drv=y vlan-mgr=y netlink=y pppoe-mgr=y lag-mgr=y bridge-mgr=y ppe-tun=y ppe-ds=y vxlanmgr=y l2tp-mgr=y ppe-vp=y ppe-rule=y
	make -C $(KDIR) M=$(PWD)/ipq95xx/qca-nss-mcs modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 CONFIG_SUPPORT_MLD=y EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq95xx" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq95xx/shortcut-fe modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq95xx/qca-nss-eip modules_install MODPROBE=true DEPMOD=/bin/true $(IPQ95XX_NSS_FLAGS) KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq95xx" INSTALL_MOD_PATH=$(INSTALLDIR) NSS_EIP_DIR=eip197 eip_crypto=m
	make -C $(KDIR) M=$(PWD)/ipq95xx/qca-nss-ecm-standard modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 ECM_CLASSIFIER_PCC_ENABLE=y ECM_INTERFACE_PPTP_ENABLE=y ECM_INTERFACE_L2TPV2_ENABLE=y ECM_INTERFACE_L2TPV3_ENABLE=y ECM_FRONT_END_PPE_ENABLE=y ECM_INTERFACE_RAWIP_ENABLE=y ECM_NON_PORTED_SUPPORT_ENABLE=y ECM_INTERFACE_VLAN_ENABLE=y ECM_FRONT_END_SFE_ENABLE=y ECM_IPV6_ENABLE=y ECM_CLASSIFIER_MARK_ENABLE=y  ECM_CLASSIFIER_DSCP_ENABLE=y ECM_CLASSIFIER_HYFI_ENABLE=y EXTRA_CFLAGS="$(KERNEL_FLAGS)"  SoC="ipq95xx" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq95xx/qca-nss-clients modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -include $(TOP)/qca-nss/qca-nss-clients/compat.h" SoC="ipq95xx" tunipip6=n tlsmgr=n  pppoe=n vxlanmgr=n pptp=n l2tpv2=n  qdisc=n vlan-mgr=n netlink=n bridge-mgr=y lag-mgr=n ovsmgr=y wifi-meshmgr=n \
		DTLSMGR_DIR="$(DTLSMGR_DIR)" \
		IPSECMGR_DIR="$(IPSECMGR_DIR)" INSTALL_MOD_PATH=$(INSTALLDIR)

	mkdir -p $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)
	cp ipq95xx/qca-ssdk/qca-ssdk.ko $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/qca-ssdk-ipq95xx.ko
else
#50xx

	ln -sf arch/nss_ipq50xx.h ipq5018/qca-nss-drv/exports/nss_arch.h
	make -C $(KDIR) M=$(PWD)/ipq5018/qca-nss-crypto modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq50xx" \
		NSS_CRYPTO_DIR=$(NSS_CRYPTO_DIR)  INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq5018/qca-nss-cfi modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq50xx" \
		CFI_CRYPTOAPI_DIR=$(CFI_CRYPTOAPI_DIR) \
		CFI_OCF_DIR=$(CFI_OCF_DIR) \
		CFI_IPSEC_DIR=$(CFI_IPSEC_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq5018/qca-nss-dp modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq50xx" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq5018/qca-nss-cfi modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq50xx" \
		CFI_CRYPTOAPI_DIR=$(CFI_CRYPTOAPI_DIR) \
		CFI_OCF_DIR=$(CFI_OCF_DIR) \
		CFI_IPSEC_DIR=$(CFI_IPSEC_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq5018/qca-nss-crypto modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq50xx" \
		NSS_CRYPTO_DIR=$(NSS_CRYPTO_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)

	ln -sf arch/nss_ipq50xx.h ipq5018/qca-nss-drv/exports/nss_arch.h
	make -C $(KDIR) M=$(PWD)/ipq5018/qca-nss-drv modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq50xx" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq5018/qca-nss-clients modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -include $(TOP)/qca-nss/qca-nss-clients/compat.h" SoC="ipq50xx" tunipip6=y tlsmgr=n  pppoe=y vxlanmgr=y pptp=y l2tpv2=y  qdisc=y vlan-mgr=n netlink=y bridge-mgr=y lag-mgr=y ovsmgr=y wifi-meshmgr=y \
		DTLSMGR_DIR="$(DTLSMGR_DIR)" \
		IPSECMGR_DIR="$(IPSECMGR_DIR)" INSTALL_MOD_PATH=$(INSTALLDIR)

	mkdir -p $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)
	cp ipq5018/qca-ssdk/qca-ssdk.ko $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/qca-ssdk-ipq50xx.ko


#6018
	cp	qca-nss-dp/hal/soc_ops/ipq60xx/nss_ipq60xx.h ipq6018/qca-nss-dp/exports/nss_dp_arch.h
	ln -sf arch/nss_ipq60xx.h ipq6018/qca-nss-drv/exports/nss_arch.h
	make -C $(KDIR) M=$(PWD)/ipq6018/qca-nss-crypto modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq60xx" \
		NSS_CRYPTO_DIR=$(NSS_CRYPTO_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq6018/qca-nss-dp modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq60xx" INSTALL_MOD_PATH=$(INSTALLDIR)


	make -C $(KDIR) M=$(PWD)/ipq6018/qca-nss-cfi modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq60xx" \
		CFI_CRYPTOAPI_DIR=$(CFI_CRYPTOAPI_DIR) \
		CFI_OCF_DIR=$(CFI_OCF_DIR) \
		CFI_IPSEC_DIR=$(CFI_IPSEC_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)

	make -C $(KDIR) M=$(PWD)/ipq6018/qca-nss-crypto modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq60xx" \
		NSS_CRYPTO_DIR=$(NSS_CRYPTO_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)

	ln -sf arch/nss_ipq60xx.h qca-nss-drv/exports/nss_arch.h
	make -C $(KDIR) M=$(PWD)/ipq6018/qca-nss-drv modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1  EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq60xx" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq6018/qca-nss-clients modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -include $(TOP)/qca-nss/qca-nss-clients/compat.h" SoC="ipq60xx" tunipip6=y tlsmgr=n  pppoe=y vxlanmgr=y pptp=y l2tpv2=y  qdisc=y vlan-mgr=y netlink=y bridge-mgr=y lag-mgr=y ovsmgr=y wifi-meshmgr=y \
		DTLSMGR_DIR="$(DTLSMGR_DIR)" \
		IPSECMGR_DIR="$(IPSECMGR_DIR)" INSTALL_MOD_PATH=$(INSTALLDIR)

	mkdir -p $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)
	cp ipq6018/qca-ssdk/qca-ssdk.ko $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/qca-ssdk-ipq60xx.ko

#ipq807x
	cp	qca-nss-dp/hal/soc_ops/ipq807x/nss_ipq807x.h ipq807x/qca-nss-dp/exports/nss_dp_arch.h
	ln -sf arch/nss_ipq807x.h qca-nss-drv/exports/nss_arch.h
	make -C $(KDIR) M=$(PWD)/ipq807x/qca-nss-dp modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq807x" INSTALL_MOD_PATH=$(INSTALLDIR)
	mkdir -p $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)
	cp qca-ssdk/qca-ssdk.ko $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/qca-ssdk-ipq807x.ko

	make -C $(KDIR) M=$(PWD)/ipq807x/qca-nss-cfi modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq807x" \
		CFI_CRYPTOAPI_DIR=$(CFI_CRYPTOAPI_DIR) \
		CFI_OCF_DIR=$(CFI_OCF_DIR) \
		CFI_IPSEC_DIR=$(CFI_IPSEC_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)

	make -C $(KDIR) M=$(PWD)/ipq807x/qca-nss-crypto modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq807x" \
		NSS_CRYPTO_DIR=$(NSS_CRYPTO_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)

	ln -sf arch/nss_ipq807x.h ipq807x/qca-nss-drv/exports/nss_arch.h
	make -C $(KDIR) M=$(PWD)/ipq807x/qca-nss-drv modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq807x" INSTALL_MOD_PATH=$(INSTALLDIR)
#	make -C $(KDIR) M=$(PWD)/ipq807x/qca-ovsmgr modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" CONFIG_SUPPORT_MLD=y INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq807x/qca-nss-mcs modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 CONFIG_SUPPORT_MLD=y EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq807x" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq807x/shortcut-fe modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq807x/qca-nss-ecm-standard modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 ECM_INTERFACE_PPTP_ENABLE=y ECM_INTERFACE_L2TPV2_ENABLE=y ECM_INTERFACE_L2TPV3_ENABLE=y ECM_FRONT_END_NSS_ENABLE=y ECM_INTERFACE_RAWIP_ENABLE=y ECM_NON_PORTED_SUPPORT_ENABLE=y ECM_INTERFACE_VLAN_ENABLE=y ECM_FRONT_END_SFE_ENABLE=y ECM_IPV6_ENABLE=y ECM_CLASSIFIER_MARK_ENABLE=y  ECM_CLASSIFIER_DSCP_ENABLE=y ECM_CLASSIFIER_HYFI_ENABLE=y EXTRA_CFLAGS="$(KERNEL_FLAGS)"  SoC="ipq807x" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq807x/qca-nss-clients modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -include $(TOP)/qca-nss/qca-nss-clients/compat.h" SoC="ipq807x" tunipip6=y tlsmgr=y  pppoe=y vxlanmgr=y pptp=y l2tpv2=y  qdisc=y vlan-mgr=y netlink=y bridge-mgr=y lag-mgr=y ovsmgr=y wifi-meshmgr=y \
		DTLSMGR_DIR="$(DTLSMGR_DIR)" \
		IPSECMGR_DIR="$(IPSECMGR_DIR)" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq807x/nss-ifb modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/ipq807x/qca-nss-hyfi-bridge modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq807x" INSTALL_MOD_PATH=$(INSTALLDIR)
endif

	-find $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/extra -name *.ko -exec mv {} $(INSTALLDIR)/lib/modules/$(KERNELRELEASE) \;
	-rm -rf $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/extra 
	-find $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/updates -name *.ko -exec mv {} $(INSTALLDIR)/lib/modules/$(KERNELRELEASE) \;
	-rm -rf $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/updates
	-find $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/update -name *.ko -exec mv {} $(INSTALLDIR)/lib/modules/$(KERNELRELEASE) \;
	-rm -rf $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/update
	rm -f $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/modules*
	mkdir -p $(INSTALLDIR)/usr/sbin
	cp ssdk-shell/build/bin/ssdk_sh $(INSTALLDIR)/usr/sbin

	mkdir -p $(INSTALLDIR)/lib/firmware
ifeq ($(CONFIG_IPQ95XX),y)
	cp firmware-11.4/crypto-ipq95xx/* $(INSTALLDIR)/lib/firmware
else
	cp firmware-11.4/hawkeye/* $(INSTALLDIR)/lib/firmware
	cp firmware-11.4/cypress/* $(INSTALLDIR)/lib/firmware
	cp firmware-11.4/maple/* $(INSTALLDIR)/lib/firmware
	cp firmware-11.4/crypto/* $(INSTALLDIR)/lib/firmware
endif
	mkdir -p $(INSTALLDIR)/usr/sbin
	cp nssinfo/obj/nssinfo $(INSTALLDIR)/usr/sbin
	cp tools/qca-nss-drv.debug $(INSTALLDIR)/usr/sbin/nss_stats
	make -C hyctl hyctl
	make -C hyctl install
else
	ln -sf arch/nss_ipq806x.h qca-nss-drv/exports/nss_arch.h
	make -C $(KDIR) M=$(PWD)/qca-ovsmgr modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" CONFIG_SUPPORT_MLD=y INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/qca-nss-mcs modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" INSTALL_MOD_PATH=$(INSTALLDIR) CONFIG_SUPPORT_MLD=y CONFIG_SUPPORT_OVS=y
	make -C $(KDIR) M=$(PWD)/shortcut-fe modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/nss-ifb modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/qca-nss-cfi modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" \
		CFI_CRYPTOAPI_DIR=$(CFI_CRYPTOAPI_DIR) \
		CFI_OCF_DIR=$(CFI_OCF_DIR) \
		CFI_IPSEC_DIR=$(CFI_IPSEC_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/qca-nss-clients modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -include $(TOP)/qca-nss/qca-nss-clients/compat.h" SoC="ipq806x" pppoe=y tunipip6=m l2tpv2=y qdisc=y vxlanmgr=y pptp=y vlan-mgr=y bridge-mgr=y \
		DTLSMGR_DIR="$(DTLSMGR_DIR)" \
		IPSECMGR_DIR="$(IPSECMGR_DIR)" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/qca-nss-crypto modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" \
		NSS_CRYPTO_DIR=$(NSS_CRYPTO_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/qca-nss-drv modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" INSTALL_MOD_PATH=$(INSTALLDIR)

	make -C $(KDIR) M=$(PWD)/qca-nss-ecm-standard modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 ECM_INTERFACE_PPTP_ENABLE=y ECM_FRONT_END_NSS_ENABLE=y ECM_FRONT_END_SFE_ENABLE=y ECM_IPV6_ENABLE=y ECM_CLASSIFIER_OVS_ENABLE=y ECM_INTERFACE_OVS_BRIDGE_ENABLE=y ECM_CLASSIFIER_OVS_ENABLE=y  ECM_CLASSIFIER_HYFI_ENABLE=y EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/qca-nss-hyfi-bridge modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/qca-nss-gmac modules_install MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -DCONFIG_NSS_DEBUG_LEVEL=0 -I$(PKG_BUILD_DIR)/nss_hal/include -I$(PKG_BUILD_DIR)/nss_hal/$(BOARD)" SoC="ipq806x" INSTALL_MOD_PATH=$(INSTALLDIR)

	-find $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/extra -name *.ko -exec mv {} $(INSTALLDIR)/lib/modules/$(KERNELRELEASE) \;
	-rm -rf $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/extra 
	-find $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/updates -name *.ko -exec mv {} $(INSTALLDIR)/lib/modules/$(KERNELRELEASE) \;
	-rm -rf $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/updates
	-find $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/update -name *.ko -exec mv {} $(INSTALLDIR)/lib/modules/$(KERNELRELEASE) \;
	-rm -rf $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/update
	rm -f $(INSTALLDIR)/lib/modules/$(KERNELRELEASE)/modules*
	mkdir -p $(INSTALLDIR)/lib/firmware
	cp firmware-11.4/akronite/* $(INSTALLDIR)/lib/firmware
	mkdir -p $(INSTALLDIR)/usr/sbin
	cp nssinfo/obj/nssinfo $(INSTALLDIR)/usr/sbin
	cp tools/qca-nss-drv.debug $(INSTALLDIR)/usr/sbin/nss_stats
	make -C hyctl hyctl
	make -C hyctl install
endif

clean:
ifeq ($(CONFIG_IPQ6018),y)
	rm -rf ipq6018
	rm -rf ipq807x
	rm -rf ipq5018
	rm -rf ipq95xx
	make $(QCASSDK_CONFIG_OPTS) -C ssdk-shell clean
	make -C hyctl clean
	make -C $(KDIR) M=$(PWD)/qca-nss-ecm-standard clean MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 ECM_INTERFACE_PPTP_ENABLE=y ECM_FRONT_END_NSS_ENABLE=y ECM_FRONT_END_SFE_ENABLE=y ECM_IPV6_ENABLE=y ECM_CLASSIFIER_OVS_ENABLE=y ECM_INTERFACE_OVS_BRIDGE_ENABLE=y ECM_CLASSIFIER_OVS_ENABLE=y  ECM_CLASSIFIER_HYFI_ENABLE=y EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" INSTALL_MOD_PATH=$(INSTALLDIR)
else

	-make -C lib clean KERNEL_FLAGS="$(KERNEL_FLAGS)"
	-make -C nssinfo clean KERNEL_FLAGS="$(KERNEL_FLAGS)"
	make -C $(KDIR) M=$(PWD)/qca-ovsmgr clean MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" CONFIG_SUPPORT_MLD=y INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/qca-nss-mcs clean MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" INSTALL_MOD_PATH=$(INSTALLDIR) CONFIG_SUPPORT_MLD=y CONFIG_SUPPORT_OVS=y
	make -C $(KDIR) M=$(PWD)/shortcut-fe clean MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/nss-ifb clean MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/qca-nss-cfi clean MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" \
		CFI_CRYPTOAPI_DIR=$(CFI_CRYPTOAPI_DIR) \
		CFI_OCF_DIR=$(CFI_OCF_DIR) \
		CFI_IPSEC_DIR=$(CFI_IPSEC_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)

	make -C $(KDIR) M=$(PWD)/qca-nss-clients clean MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -include $(TOP)/qca-nss/qca-nss-clients/compat.h" SoC="ipq806x" pppoe=y tunipip6=m l2tpv2=y qdisc=y vxlanmgr=y pptp=y vlan-mgr=y bridge-mgr=y \
		DTLSMGR_DIR="$(DTLSMGR_DIR)" \
		IPSECMGR_DIR="$(IPSECMGR_DIR)" INSTALL_MOD_PATH=$(INSTALLDIR)

	make -C $(KDIR) M=$(PWD)/qca-nss-crypto clean MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" \
		NSS_CRYPTO_DIR=$(NSS_CRYPTO_DIR) INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/qca-nss-drv clean MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/qca-nss-ecm-standard clean MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 ECM_INTERFACE_PPTP_ENABLE=y ECM_FRONT_END_NSS_ENABLE=y ECM_FRONT_END_SFE_ENABLE=y ECM_IPV6_ENABLE=y ECM_CLASSIFIER_OVS_ENABLE=y ECM_INTERFACE_OVS_BRIDGE_ENABLE=y ECM_CLASSIFIER_OVS_ENABLE=y EXTRA_CFLAGS="$(KERNEL_FLAGS)" SoC="ipq806x" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C $(KDIR) M=$(PWD)/qca-nss-gmac clean MODPROBE=true DEPMOD=/bin/true KBUILD_MODPOST_WARN=1 EXTRA_CFLAGS="$(KERNEL_FLAGS) -DCONFIG_NSS_DEBUG_LEVEL=0 -I$(PKG_BUILD_DIR)/nss_hal/include -I$(PKG_BUILD_DIR)/nss_hal/$(BOARD)" SoC="ipq806x" INSTALL_MOD_PATH=$(INSTALLDIR)
	make -C hyctl clean
endif
