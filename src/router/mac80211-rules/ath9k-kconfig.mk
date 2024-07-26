include $(TOP)/.config

CONFIG_CFG80211_INTERNAL_REGDB=y
ifeq ($(CONFIG_MAC80211_MESH),y)
  CPTCFG_MAC80211_MESH=y
endif

MODPROBE:= modprobe

REGPATH=crda
REGBIN=regulatory.bin
REGTXTORIG=db.txt
REGTXT=db-temp.txt

ifeq ($(CONFIG_RAIEXTRA),y)
REGTXTEXTRA=db-rai.txt
endif
ifeq ($(CONFIG_TESTEM),y)
REGTXTEXTRA=db-testem.txt
endif

ifeq ($(CONFIG_TMK),y)
REGTXTEXTRA=db-tmk.txt
endif

ifeq ($(CONFIG_IDEXX),y)
ifeq ($(CONFIG_WZRHPAG300NH),y)
REGTXTORIG=regulatory/wzrhpag300h_idexx/db.txt
endif
endif

ifeq ($(CONFIG_ENEO),y)
ifeq ($(CONFIG_UBNTM),y)
REGTXTORIG=regulatory/ubntm_eneo/db.txt
endif
endif

ifeq ($(CONFIG_BUFFALO),y)
ifeq ($(CONFIG_WZRHPAG300NH),y)
REGTXTORIG=regulatory/wzrhpag300h/db.txt
endif
ifeq ($(CONFIG_WZR450HP2),y)
REGTXTORIG=regulatory/wzr450hp2/db.txt
endif
endif

MAKE_OPTS:=

ifeq ($(ARCHITECTURE),ap83)
  BUILDFLAGS += -DCONFIG_ATHEROS_AR71XX
  CPTCFG_ATH9K_AHB=y
else
ifeq ($(CONFIG_UBNTXW),y)
  CPTCFG_ATH9K_PCI=y
  CPTCFG_ATH9K_AHB=y
else
ifeq ($(ARCHITECTURE),wasp)
  CPTCFG_ATH9K_PCI=y
ifneq ($(CONFIG_WR941NV6_CN),y)
  CPTCFG_ATH9K_AHB=y
endif
else
ifeq ($(CONFIG_DIR615I),y)
  CPTCFG_ATH9K_AHB=y
else
ifeq ($(ARCHITECTURE),hornet)
  CPTCFG_ATH9K_AHB=y
else
  CPTCFG_ATH9K_PCI=y
endif
endif
endif
endif
endif
  CPTCFG_ATH9K_DYNACK=y
ifeq ($(CONFIG_SUPERCHANNEL),y)
  BUILDFLAGS += -DHAVE_SUPERCHANNEL
ifeq ($(CONFIG_ATH5K),y)
  BUILDFLAGS += -DCPTCFG_ATH5K_TEST_CHANNELS
endif
endif

ifeq ($(CONFIG_ATH5K_AHB),y)
  BUILDFLAGS += -DCONFIG_NOPRINTK
endif
ifeq ($(CONFIG_ONNET),y)
  BUILDFLAGS += -DHAVE_ONNET
ifeq ($(CONFIG_XD3200),y)
  BUILDFLAGS += -DHAVE_XD3200
endif
ifeq ($(CONFIG_MMS344),y)
  BUILDFLAGS += -DHAVE_MMS344
endif
endif
ifeq ($(CONFIG_NDTRADE),y)
  BUILDFLAGS += -DHAVE_NDTRADE
ifeq ($(CONFIG_NDTRADE_CHANSHIFT),y)
  BUILDFLAGS += -DHAVE_NDTRADE_CHANSHIFT
endif
endif
ifeq ($(CONFIG_NOMESSAGE),y)
ifneq ($(CONFIG_ATH10K),y)
  BUILDFLAGS += -DCONFIG_NOPRINTK
endif
endif
ifeq ($(ARCHITECTURE),ap83)
  BUILDFLAGS += -DCONFIG_ATHEROS_AR71XX
endif
ifeq ($(CONFIG_ATH10K),y)
  BUILDFLAGS += -DCONFIG_ATH10K=y -DCONFIG_ATH10K_PCI=y
  CPTCFG_ATH10K=y
  CPTCFG_ATH10K_PCI=y
  CPTCFG_ATH10K_DEBUGFS=y
endif
ifeq ($(CONFIG_ATH11K),y)
  CPTCFG_ATH11K=y
  CPTCFG_ATH11K_DEBUGFS=y
  CPTCFG_ATH11K_THERMAL=y
  CPTCFG_ATH11K_SPECTRAL=y
  CPTCFG_ATH11K_PCI=y
  CPTCFG_MHI_BUS=y
  CPTCFG_MHI_QRTR=y
  CPTCFG_QRTR_MHI=y
endif
ifeq ($(CONFIG_IPQ6018),y)
  CPTCFG_ATH11K=y
  CPTCFG_ATH11K_DEBUGFS=y
  CPTCFG_ATH11K_THERMAL=y
  CPTCFG_ATH11K_SPECTRAL=y
  CPTCFG_ATH11K_NSS_MESH_SUPPORT=y
  CPTCFG_ATH11K_PCI=y
  CPTCFG_ATH11K_AHB=y
  CPTCFG_MHI_BUS=y
  CPTCFG_MHI_QRTR=y
  CPTCFG_QRTR_MHI=y
  CPTCFG_ATH11K_NSS_SUPPORT=y
  CPTCFG_MAC80211_NSS_SUPPORT=y
  CPTCFG_MAC80211_MESH_NSS_SUPPORT=y
endif
ifeq ($(CONFIG_MCPHERSON),y)
  BUILDFLAGS += -DHAVE_MCPHERSON
endif
ifeq ($(CONFIG_MVEBU),y)
  CPTCFG_MWIFIEX=y
  CPTCFG_MWIFIEX_SDIO=y
  CPTCFG_MWLWIFI=y
endif
ifeq ($(CONFIG_MAC80211_RTLWIFI),y)
  CPTCFG_WLAN_VENDOR_REALTEK=y
endif
ifeq ($(CONFIG_BRCMFMAC),y)
  CPTCFG_BRCMUTIL=m
  CPTCFG_BRCMFMAC=m
  CPTCFG_BRCMSMAC=m
  CPTCFG_BRCMFMAC_PCIE=y
  CPTCFG_BRCMFMAC_PROTO_MSGBUF=y
  CPTCFG_BRCMFMAC_USB=y
  CPTCFG_BRCMFMAC_PROTO_BCDC=y
  CPTCFG_WLAN_VENDOR_BROADCOM=y
endif
ifeq ($(CONFIG_TDMA),y)
  CPTCFG_MAC80211_TDMA=y
endif
ifeq ($(CONFIG_MAC80211_COMPRESS),y)
  CPTCFG_MAC80211_COMPRESS=y
endif

KERNEL_ARCH=$(strip $(subst aarch64,arm64,$(subst i386,x86,$(subst armeb,arm,$(subst mipsel,mips,$(subst mips64,mips,$(subst mips64el,mips,$(subst sh2,sh,$(subst sh3,sh,$(subst sh4,sh,$(ARCH)))))))))))

MAKE_OPTS += \
	CROSS_COMPILE="ccache $(ARCH)-linux-" \
	ARCH="$(KERNEL_ARCH)" \
	EXTRA_CFLAGS="$(BUILDFLAGS) -I$(TOP)/qca-nss/qca-nss-drv/exports -I$(TOP)/qca-nss/qca-nss-clients/exports" \
	MADWIFI= \
	OLD_IWL= \
	KLIB_BUILD="$(LINUXDIR)" \
	MODPROBE=true \
	KBUILD_MODPOST_WARN=1 \
	KLIB=$(INSTALLDIR)/ath9k \
	KERNEL_SUBLEVEL=$(word 3,$(subst ., ,$(KERNELRELEASE))) \
	KBUILD_LDFLAGS_MODULE_PREREQ=

ifeq ($(ARCHITECTURE),ap83)
  MAKE_OPTS += CONFIG_ATHEROS_AR71XX=y
endif

IW_CFLAGS=-I$(TOP)/libnl-tiny/include \
	-DCONFIG_LIBNL20 \
	-DCONFIG_TDMA \
	-I$(TOP)/shared \
	-D_GNU_SOURCE
IW_LDFLAGS=-L$(TOP)/libnl-tiny/



ath9k-checkconfig:
	cp $(REGPATH)/$(REGTXTORIG) $(REGPATH)/$(REGTXT)
ifdef REGTXTEXTRA
	cat $(REGPATH)/$(REGTXTEXTRA) >> $(REGPATH)/$(REGTXT)
	echo >> $(REGPATH)/$(REGTXT)
endif
ifeq ($(CONFIG_CFG80211_INTERNAL_REGDB),y)
	cmp $(REGPATH)/$(REGTXT) $(TOP)/$(MAC80211_PATH)/net/wireless/db.txt || \
		cp $(REGPATH)/$(REGTXT) $(TOP)/$(MAC80211_PATH)/net/wireless/db.txt
endif
	test -f $(MAC80211_PATH)/.config_temp || touch $(MAC80211_PATH)/.config_temp
	mv $(MAC80211_PATH)/.config_temp $(MAC80211_PATH)/.config_temp_old
	test -f $(MAC80211_PATH)/.kernel_config || touch $(MAC80211_PATH)/.kernel_config
	cat $(TOP)/mac80211-rules/configs/mac80211.config > $(MAC80211_PATH)/.config_temp
ifneq ($(CONFIG_MAC80211_NOATH9K),y)
	cat $(TOP)/mac80211-rules/configs/ath9k.config >> $(MAC80211_PATH)/.config_temp
ifeq ($(CPTCFG_ATH9K_PCI),y)
	echo "CPTCFG_ATH9K_PCI=y" >>$(MAC80211_PATH)/.config_temp
endif
ifeq ($(CPTCFG_ATH9K_AHB),y)
	echo "CPTCFG_ATH9K_AHB=y" >>$(MAC80211_PATH)/.config_temp
endif
ifeq ($(CPTCFG_ATH9K_DYNACK),y)
	echo "CPTCFG_ATH9K_DYNACK=y" >>$(MAC80211_PATH)/.config_temp
#	echo "CPTCFG_ATH_DEBUG=y" >>$(MAC80211_PATH)/.config_temp
endif
endif
ifeq ($(CONFIG_MAC80211_ATH9K_HTC),y)
	cat $(TOP)/mac80211-rules/configs/ath9k-htc.config >> $(MAC80211_PATH)/.config_temp
endif

ifeq ($(CONFIG_ATH10K),y)
	cat $(TOP)/mac80211-rules/configs/ath10k.config >> $(MAC80211_PATH)/.config_temp
ifeq ($(CPTCFG_ATH10K_PCI),y)
	echo "CPTCFG_ATH10K_PCI=y" >>$(MAC80211_PATH)/.config_temp
endif
endif
ifeq ($(CONFIG_ATH11K),y)
	echo "CPTCFG_ATH11K=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_ATH11K_DEBUGFS=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_ATH11K_THERMAL=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_ATH11K_SPECTRAL=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_ATH11K_PCI=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_MHI_BUS=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_MHI_QRTR=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_QRTR_MHI=y" >>$(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_IPQ6018),y)
	echo "CPTCFG_ATH11K=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_ATH11K_DEBUGFS=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_ATH11K_THERMAL=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_ATH11K_SPECTRAL=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_ATH11K_PCI=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_ATH11K_AHB=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_MHI_BUS=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_MHI_QRTR=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_QRTR_MHI=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_ATH11K_NSS_SUPPORT=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_MAC80211_NSS_SUPPORT=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_MAC80211_NSS_MESH_SUPPORT=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_ATH11K_NSS_MESH_SUPPORT=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_ATH11K_MEM_PROFILE_512M=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_ATH11K_DEBUGFS=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_ATH11K_DEBUGFS_STA=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_ATH11K_DEBUGFS_HTT_STATS=y" >>$(MAC80211_PATH)/.config_temp
	echo "CPTCFG_ATH11K_THERMAL=y" >>$(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_IWLWIFI),y)
	cat $(TOP)/mac80211-rules/configs/iwlwifi.config >> $(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_BRCMFMAC),y)
	cat $(TOP)/mac80211-rules/configs/brcmfmac.config >> $(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_ATH11K),y)
	cat $(TOP)/mac80211-rules/configs/ath11k.config >> $(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_B43),y)
	cat $(TOP)/mac80211-rules/configs/b43.config >> $(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_B43LEGACY),y)
	cat $(TOP)/mac80211-rules/configs/b43legacy.config >> $(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_BRCMSMAC),y)
	cat $(TOP)/mac80211-rules/configs/brcmsmac.config >> $(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_MVEBU),y)
	cat $(TOP)/mac80211-rules/configs/mwlwifi.config >> $(MAC80211_PATH)/.config_temp
else
ifeq ($(CONFIG_MWLWIFI),y)
	cat $(TOP)/mac80211-rules/configs/mwlwifi.config >> $(MAC80211_PATH)/.config_temp
endif
endif
ifeq ($(CONFIG_MT7620),y)
	cat $(TOP)/mac80211-rules/configs/mt76xx.config >> $(MAC80211_PATH)/.config_temp
ifeq ($(CONFIG_MT7915),y)
	echo "CPTCFG_MT7915E=m" >> $(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_MT7921),y)
	echo "CPTCFG_MT7921E=m" >> $(MAC80211_PATH)/.config_temp
	echo "CPTCFG_MT7921S=m" >> $(MAC80211_PATH)/.config_temp
	echo "CPTCFG_MT7921U=m" >> $(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_MT7996),y)
	echo "CPTCFG_MT7996E=m" >> $(MAC80211_PATH)/.config_temp
else
	echo "# CPTCFG_MT7996E is not set" >> $(MAC80211_PATH)/.config_temp
endif
else
ifeq ($(CONFIG_MT7615),y)
	cat $(TOP)/mac80211-rules/configs/mt76xx.config >> $(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_MT7915),y)
	echo "CPTCFG_MT7915E=m" >> $(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_MT7921),y)
	echo "CPTCFG_MT7921E=m" >> $(MAC80211_PATH)/.config_temp
	echo "CPTCFG_MT7921S=m" >> $(MAC80211_PATH)/.config_temp
	echo "CPTCFG_MT7921U=m" >> $(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_MT7996),y)
	echo "CPTCFG_MT7996E=m" >> $(MAC80211_PATH)/.config_temp
else
	echo "# CPTCFG_MT7996E is not set" >> $(MAC80211_PATH)/.config_temp
endif
endif
ifeq ($(CONFIG_X86),y)
	cat $(TOP)/mac80211-rules/configs/mt76full.config >> $(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_WIL6210),y)
	cat $(TOP)/mac80211-rules/configs/wil6210.config >> $(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_ATH5K),y)
	cat $(TOP)/mac80211-rules/configs/ath5k.config >> $(MAC80211_PATH)/.config_temp
ifeq ($(CONFIG_ATH5K_PCI),y)
	echo "CPTCFG_ATH5K_PCI=y" >>$(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_ATH5K_AHB),y)
	echo "CPTCFG_ATH5K_AHB=y" >>$(MAC80211_PATH)/.config_temp
endif
endif
ifeq ($(CONFIG_ATH10KUSB),y)
	echo "CPTCFG_ATH10K_SDIO=m" >> $(MAC80211_PATH)/.config_temp
	echo "CPTCFG_ATH10K_USB=m" >> $(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_IPQ806X),y)
	echo "CPTCFG_ATH10K_AHB=y" >> $(MAC80211_PATH)/.config_temp
ifeq ($(CONFIG_QCA_NSS),y)
	echo "CPTCFG_MAC80211_NSS_SUPPORT=y" >> $(MAC80211_PATH)/.config_temp
else
	echo "# CPTCFG_MAC80211_NSS_SUPPORT is not set" >> $(MAC80211_PATH)/.config_temp
endif
else
#ifeq ($(CONFIG_QCA_NSS),y)
#	echo "CPTCFG_MAC80211_NSS_SUPPORT=y" >> $(MAC80211_PATH)/.config_temp
#else
#	echo "# CPTCFG_MAC80211_NSS_SUPPORT is not set" >> $(MAC80211_PATH)/.config_temp
#endif
	echo "# CPTCFG_MAC80211_NSS_SUPPORT is not set" >> $(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_MAC80211_RTL8192CU),y)
	cat $(TOP)/mac80211-rules/configs/rtl8192cu.config >> $(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_MAC80211_RTLWIFI),y)
	cat $(TOP)/mac80211-rules/configs/rtlwifi.config >> $(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_MAC80211_RT2800USB),y)
	cat $(TOP)/mac80211-rules/configs/rt2800.config>> $(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_MAC80211_RT2800),y)
	cat $(TOP)/mac80211-rules/configs/rt2800_soc.config>> $(MAC80211_PATH)/.config_temp
endif
ifeq ($(CPTCFG_MAC80211_MESH),y)
	echo "CPTCFG_MAC80211_MESH=y" >>$(MAC80211_PATH)/.config_temp
endif
ifeq ($(CPTCFG_MAC80211_COMPRESS),y)
	echo "CPTCFG_MAC80211_COMPRESS=y" >>$(MAC80211_PATH)/.config_temp
endif
ifneq ($(CONFIG_MAC80211_NOLEDS),y)
	echo "CPTCFG_MAC80211_LEDS=y" >>$(MAC80211_PATH)/.config_temp
endif
ifeq ($(CONFIG_TDMA),y)
	echo "CPTCFG_MAC80211_TDMA=y" >>$(MAC80211_PATH)/.config_temp
ifeq ($(CPTCFG_MAC80211_MESH),y)
	echo "CPTCFG_MAC80211_TDMA_MESH=y" >>$(MAC80211_PATH)/.config_temp
endif
endif

	rm -f $(TOP)/$(MAC80211_PATH)/include/linux/ath9k_platform.h
	test -f $(MAC80211_PATH)/.config || rm -f $(MAC80211_PATH)/.configured
	cmp $(MAC80211_PATH)/.config_temp $(MAC80211_PATH)/.config_temp_old || rm -f $(MAC80211_PATH)/.configured
	cmp $(LINUXDIR)/.config $(MAC80211_PATH)/.kernel_config || (rm -f $(MAC80211_PATH)/.configured ; printf "\n\nKERNEL CONFIG CHANGED ; DOING RECONFIGURE\n\n\n")
	test -f $(MAC80211_PATH)/.configured || printf "\n\nDOING RECONFIGURE\n\n\n"
	test -f $(MAC80211_PATH)/.configured || ( MAKEFLAGS= KERNELRELEASE= CFLAGS= $(MAKE) -C $(MAC80211_PATH) $(MAKE_OPTS) clean ; true)
	test -f $(MAC80211_PATH)/.configured || rm -f $(MAC80211_PATH)/.config $(MAC80211_PATH)/.compat_autoconf_*
	test -f $(MAC80211_PATH)/.configured || cat $(MAC80211_PATH)/.config_temp | sort | uniq > $(MAC80211_PATH)/.config 
	test -f $(MAC80211_PATH)/.configured || CC=gcc MAKEFLAGS= KERNELRELEASE= CFLAGS= $(MAKE) -C $(MAC80211_PATH) $(MAKE_OPTS) allnoconfig
	-cp $(LINUXDIR)/.config $(MAC80211_PATH)/.kernel_config
	touch $(MAC80211_PATH)/.configured

ath9k-configure:
	rm -f $(MAC80211_PATH)/.configured

ath9k: ath9k-checkconfig
	MAKEFLAGS= KERNELRELEASE= CFLAGS= $(MAKE) -C $(MAC80211_PATH) $(MAKE_OPTS) modules

ath9k-install: ath9k
	rm -rf $(INSTALLDIR)/ath9k/
ifneq ($(CONFIG_NOWIFI),y)
	cd $(MAC80211_PATH) ; mkdir -p $(INSTALLDIR)/ath9k/lib/modules/$(KERNELRELEASE) ; cp `find -name \*.ko` $(INSTALLDIR)/ath9k/lib/modules/$(KERNELRELEASE)
ifeq ($(CONFIG_MAC80211_RT2800USB),y)
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware
	-cp $(MAC80211_PATH)/ath10k-firmware-*/rt*.bin $(INSTALLDIR)/ath9k/lib/firmware
endif
ifeq ($(CONFIG_MAC80211_RT2800),y)
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware
	-cp $(MAC80211_PATH)/ath10k-firmware-*/rt*.bin $(INSTALLDIR)/ath9k/lib/firmware
endif
ifeq ($(CONFIG_MAC80211_ATH9K_HTC),y)
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware
	-cp $(MAC80211_PATH)/ath10k-firmware-*/htc_*.fw $(INSTALLDIR)/ath9k/lib/firmware
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/ath9k_htc
	-cp $(MAC80211_PATH)/ath10k-firmware-*/ath9k_htc/* $(INSTALLDIR)/ath9k/lib/firmware/ath9k_htc
endif
ifeq ($(CONFIG_MAC80211_RTL8192CU),y)
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/rtlwifi
	-cp -av $(MAC80211_PATH)/ath10k-firmware-*/rtlwifi/* $(INSTALLDIR)/ath9k/lib/firmware/rtlwifi
endif
ifeq ($(CONFIG_MAC80211_RTLWIFI),y)
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/rtlwifi
	-cp -av $(MAC80211_PATH)/ath10k-firmware-*/rtlwifi/* $(INSTALLDIR)/ath9k/lib/firmware/rtlwifi
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/rtw88
	-cp -av $(MAC80211_PATH)/ath10k-firmware-*/rtw88/* $(INSTALLDIR)/ath9k/lib/firmware/rtw88
endif
ifeq ($(CONFIG_WIL6210),y)	
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware
	-cp -av $(MAC80211_PATH)/ath10k-firmware*/wil6210/* $(INSTALLDIR)/ath9k/lib/firmware/
endif
ifeq ($(CONFIG_B43),y)	
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/b43/
	-cp -av $(MAC80211_PATH)/ath10k-firmware*/b43/* $(INSTALLDIR)/ath9k/lib/firmware/b43
endif
ifeq ($(CONFIG_B43LEGACY),y)	
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/b43legacy/
	-cp -av $(MAC80211_PATH)/ath10k-firmware*/b43legacy/* $(INSTALLDIR)/ath9k/lib/firmware/b43legacy
endif
ifeq ($(CONFIG_BRCMSMAC),y)	
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/brcm/
	-cp -av $(MAC80211_PATH)/ath10k-firmware*/brcm/brcm43* $(INSTALLDIR)/ath9k/lib/firmware/brcm
endif
ifeq ($(CONFIG_IWLWIFI),y)	
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware
	-cp -av $(MAC80211_PATH)/ath10k-firmware*/iwl* $(INSTALLDIR)/ath9k/lib/firmware/
	-cp -av $(MAC80211_PATH)/ath10k-firmware*/ipw* $(INSTALLDIR)/ath9k/lib/firmware/
endif
ifeq ($(CONFIG_BRCMFMAC),y)	
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/brcm/
	-cp -av $(MAC80211_PATH)/ath10k-firmware*/brcm/brcmfmac43602-pcie.bin $(INSTALLDIR)/ath9k/lib/firmware/brcm
	-cp -av $(MAC80211_PATH)/ath10k-firmware*/brcm/brcmfmac4366b-pcie.bin $(INSTALLDIR)/ath9k/lib/firmware/brcm
	-cp -av $(MAC80211_PATH)/ath10k-firmware*/brcm/brcmfmac4366c-pcie.bin $(INSTALLDIR)/ath9k/lib/firmware/brcm
ifeq ($(CONFIG_X86),y)
	-cp -av $(MAC80211_PATH)/ath10k-firmware*/brcm/* $(INSTALLDIR)/ath9k/lib/firmware/brcm
endif	
#	-cp -av $(MAC80211_PATH)/ath10k-firmware*/b43/* $(INSTALLDIR)/ath9k/lib/firmware/b43
#	-cp -av $(MAC80211_PATH)/ath10k-firmware*/b43legacy/* $(INSTALLDIR)/ath9k/lib/firmware/b43legacy
endif
ifeq ($(CONFIG_ATH11K),y)
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/ath11k
	-cp -av $(MAC80211_PATH)/ath10k-firmware*/ath11k/* $(INSTALLDIR)/ath9k/lib/firmware/ath11k
endif
ifeq ($(CONFIG_IPQ6018),y)
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/ath11k/IPQ6018
	-cp -av $(MAC80211_PATH)/ath10k-firmware*/ath11k/IPQ6018/* $(INSTALLDIR)/ath9k/lib/firmware/ath11k/IPQ6018/
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/ath11k/IPQ8074
	-cp -av $(MAC80211_PATH)/ath10k-firmware*/ath11k/IPQ8074/* $(INSTALLDIR)/ath9k/lib/firmware/ath11k/IPQ8074/
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/ath11k/IPQ5018
	-cp -av $(MAC80211_PATH)/ath10k-firmware*/ath11k/IPQ5018/* $(INSTALLDIR)/ath9k/lib/firmware/ath11k/IPQ5018/
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/ath11k/QCN9074
	-cp -av $(MAC80211_PATH)/ath10k-firmware*/ath11k/QCN9074/* $(INSTALLDIR)/ath9k/lib/firmware/ath11k/QCN9074/
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath11k/IPQ6018/hw1.0 && rm -f cal-ahb-c000000.wifi.bin && ln -s /tmp/board.bin cal-ahb-c000000.wifi.bin 
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath11k/IPQ6018/hw1.0 && rm -f caldata.bin && ln -s /tmp/caldata.bin caldata.bin 
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath11k/IPQ6018/hw1.0 && rm -f board.bin && ln -s /tmp/board.bin board.bin 

	cd $(INSTALLDIR)/ath9k/lib/firmware/ath11k/IPQ8074/hw2.0 && rm -f cal-ahb-c000000.wifi.bin && ln -s /tmp/board.bin cal-ahb-c000000.wifi.bin 
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath11k/IPQ8074/hw2.0 && rm -f caldata.bin && ln -s /tmp/caldata.bin caldata.bin 
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath11k/IPQ8074/hw2.0 && rm -f board.bin && ln -s /tmp/board.bin board.bin 

	cd $(INSTALLDIR)/ath9k/lib/firmware/ath11k/IPQ5018/hw1.0 && rm -f cal-ahb-c000000.wifi.bin && ln -s /tmp/board.bin cal-ahb-c000000.wifi.bin 
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath11k/IPQ5018/hw1.0 && rm -f caldata.bin && ln -s /tmp/caldata.bin caldata.bin 
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath11k/IPQ5018/hw1.0 && rm -f board.bin && ln -s /tmp/board.bin board.bin 

	cd $(INSTALLDIR)/ath9k/lib/firmware/ath11k/QCN9074/hw1.0 && rm -f cal-pci-0001:01:00.0.bin && ln -s /tmp/cal-pci-0001:01:00.0.bin cal-pci-0001:01:00.0.bin 
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath11k/QCN9074/hw1.0 && rm -f caldata2.bin && ln -s /tmp/caldata2.bin caldata.bin 
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath11k/QCN9074/hw1.0 && rm -f board2.bin && ln -s /tmp/board2.bin board.bin 
endif

ifeq ($(CONFIG_ATH10K),y)
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware
	-mkdir -p $(INSTALLDIR)/ath9k/lib/ath10k
	-cp -av $(MAC80211_PATH)/ath10k-firmware*/ath10k $(INSTALLDIR)/ath9k/lib/firmware/
	rm -f $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA988X/hw2.0/firmware-2.bin
	rm -f $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA988X/hw2.0/firmware-4.bin
	rm -rf $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA988X/hw1.0
ifneq ($(CONFIG_QCA99X0),y)
	rm -rf $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA99X0
ifneq ($(CONFIG_QCA9984),y)
	rm -rf $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA9984
endif
else
ifeq ($(CONFIG_G10),y)
	cp $(MAC80211_PATH)/ath10k-firmware*/g10-boarddata/* $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA99X0/hw2.0
endif
	mv $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA9984/hw1.0/board.bin $(INSTALLDIR)/ath9k/lib/ath10k/board_9984.bin
	mv $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA99X0/hw2.0/board.bin $(INSTALLDIR)/ath9k/lib/ath10k/board_99X0.bin
	-cd $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA99X0/hw2.0 && ln -s /tmp/board1.bin board.bin 
	-cd $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA99X0/hw2.0 && ln -s /tmp/board2.bin board_2.bin 
	-cd $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA9984/hw1.0 && ln -s /tmp/board1.bin board.bin 
	-cd $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA9984/hw1.0 && ln -s /tmp/board2.bin board_2.bin 
	-cd $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA4019/hw1.0 && ln -s /tmp/board1.bin board.bin 
	-cd $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA4019/hw1.0 && ln -s /tmp/board2.bin board_2.bin 
endif
ifneq ($(CONFIG_QCA9887),y)
	rm -rf $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA9887
endif
ifneq ($(CONFIG_QCA9888),y)
	rm -rf $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA9888
endif
ifneq ($(CONFIG_QCA9984),y)
	rm -rf $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA9984
endif
	mv $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA988X/hw2.0/board.bin $(INSTALLDIR)/ath9k/lib/ath10k
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA988X/hw2.0 && ln -s /tmp/ath10k-board.bin board.bin 
ifeq ($(CONFIG_R9000),y)
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath10k && rm -f pre-cal-pci-0000:01:00.0.bin && ln -s /tmp/board1.bin pre-cal-pci-0001:03:00.0.bin 
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath10k && rm -f pre-cal-pci-0001:01:00.0.bin && ln -s /tmp/board2.bin pre-cal-pci-0001:04:00.0.bin 
else
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath10k && rm -f pre-cal-pci-0000:00:00.0.bin && ln -s /tmp/board1.bin pre-cal-pci-0000:00:00.0.bin 
ifeq ($(CONFIG_HABANERO),y)
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath10k && rm -f pre-cal-pci-0000:01:00.0.bin && ln -s /tmp/board3.bin pre-cal-pci-0000:01:00.0.bin 
else
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath10k && rm -f pre-cal-pci-0000:01:00.0.bin && ln -s /tmp/board1.bin pre-cal-pci-0000:01:00.0.bin 
endif
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath10k && rm -f pre-cal-pci-0001:01:00.0.bin && ln -s /tmp/board2.bin pre-cal-pci-0001:01:00.0.bin 
endif
ifeq ($(CONFIG_QCA9887),y)
	rm -f $(INSTALLDIR)/ath9k/lib/ath10k/board.bin
ifeq ($(CONFIG_ARCHERC25),y)
	rm -rf $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA988X
endif
	mv $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA9887/hw1.0/board.bin $(INSTALLDIR)/ath9k/lib/ath10k
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA9887/hw1.0 && ln -s /tmp/ath10k-board.bin board.bin 
endif
ifneq ($(CONFIG_QCA4019),y)
	rm -rf $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA4019
else
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath10k && rm -f pre-cal-ahb-a000000.wifi.bin && ln -s /tmp/board1.bin pre-cal-ahb-a000000.wifi.bin
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath10k && rm -f pre-cal-ahb-a800000.wifi.bin && ln -s /tmp/board2.bin pre-cal-ahb-a800000.wifi.bin
ifeq ($(CONFIG_HABANERO),y)
	-cp $(MAC80211_PATH)/ipq-wifi/board-8dev_habanero-dvk.qca4019 $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA4019/hw1.0/board-2.bin
endif
#	rm -rf $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA9888
	rm -rf $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA988X
	rm -rf $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA9887
#	rm -rf $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA9984
	rm -rf $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA99X0
ifneq ($(CONFIG_EA8300),y)
	rm -rf $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA9888/hw2.0/ea8300
	rm -rf $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA4019/hw1.0/ea8300
	rm -rf $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA9984/hw1.0/mr9000
else
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA4019/hw1.0/ && rm -f board-2.bin && ln -s /tmp/ipq4019.bin board-2.bin
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA9888/hw2.0/ && rm -f board-2.bin && ln -s /tmp/qca9888.bin board-2.bin
	cd $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA9984/hw1.0/ && rm -f board-2.bin && ln -s /tmp/qca9984.bin board-2.bin
endif
endif
endif
ifeq ($(CONFIG_MVEBU),y)
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/mwlwifi
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/mrvl
	-mkdir -p $(INSTALLDIR)/ath9k/lib/mvebu
	-cp -av $(MAC80211_PATH)/drivers/net/wireless/mwlwifi/bin/firmware/*.bin $(INSTALLDIR)/ath9k/lib/firmware/mwlwifi
	-cp -av $(MAC80211_PATH)/drivers/net/wireless/marvell/mwlwifi/bin/firmware/*.bin $(INSTALLDIR)/ath9k/lib/firmware/mwlwifi
	-cp -av $(MAC80211_PATH)/drivers/bluetooth/firmware/*.bin $(INSTALLDIR)/ath9k/lib/firmware/mrvl
	rm -f $(INSTALLDIR)/ath9k/lib/modules/$(KERNELRELEASE)/ath*
	rm -rf $(INSTALLDIR)/ath9k/lib/firmare/ath10k
	rm -rf $(INSTALLDIR)/ath9k/lib/ath10k
endif

ifneq ($(CONFIG_ATH10KUSB),y)
	rm -f $(INSTALLDIR)/ath9k/lib/modules/$(KERNELRELEASE)/ath10k_usb.ko
	rm -f $(INSTALLDIR)/ath9k/lib/modules/$(KERNELRELEASE)/ath10k_sdio.ko
	rm -rf $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA6174
	rm -rf $(INSTALLDIR)/ath9k/lib/firmware/ath10k/QCA9377
endif
ifeq ($(CONFIG_NORTHSTAR),y)
	rm -f $(INSTALLDIR)/ath9k/lib/modules/$(KERNELRELEASE)/ath*
endif
ifeq ($(CONFIG_ATH5KONLY),y)
	rm -f $(INSTALLDIR)/ath9k/lib/modules/$(KERNELRELEASE)/ath9*
endif
ifeq ($(CONFIG_MT7620),y)
	rm -f $(INSTALLDIR)/ath9k/lib/modules/$(KERNELRELEASE)/ath*
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/mediatek
ifeq ($(CONFIG_MT7615),y)
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/mediatek
	-cp -av $(MAC80211_PATH)/drivers/net/wireless/mediatek/mt76/firmware/mt7615* $(INSTALLDIR)/ath9k/lib/firmware/mediatek
else
	rm -f $(INSTALLDIR)/ath9k/lib/modules/$(KERNELRELEASE)/mt7615*
endif
ifeq ($(CONFIG_MT7662),y)
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/mediatek
	-cp -av $(MAC80211_PATH)/drivers/net/wireless/mediatek/mt76/firmware/mt7662_patch* $(INSTALLDIR)/ath9k/lib/firmware/mediatek
	-cp -av $(MAC80211_PATH)/drivers/net/wireless/mediatek/mt76/firmware/mt7662_firmware*v2.3* $(INSTALLDIR)/ath9k/lib/firmware/mediatek
	-cp -av $(MAC80211_PATH)/drivers/net/wireless/mediatek/mt76/firmware/mt7662_rom* $(INSTALLDIR)/ath9k/lib/firmware/mediatek
	-cp -av $(MAC80211_PATH)/drivers/net/wireless/mediatek/mt76/firmware/mt7662.bin $(INSTALLDIR)/ath9k/lib/firmware/mediatek
else
ifeq ($(CONFIG_MT7612),y)
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/mediatek
	-cp -av $(MAC80211_PATH)/drivers/net/wireless/mediatek/mt76/firmware/mt7662_patch* $(INSTALLDIR)/ath9k/lib/firmware/mediatek
	-cp -av $(MAC80211_PATH)/drivers/net/wireless/mediatek/mt76/firmware/mt7662_firmware*v2.3* $(INSTALLDIR)/ath9k/lib/firmware/mediatek
	-cp -av $(MAC80211_PATH)/drivers/net/wireless/mediatek/mt76/firmware/mt7662_rom* $(INSTALLDIR)/ath9k/lib/firmware/mediatek
	-cp -av $(MAC80211_PATH)/drivers/net/wireless/mediatek/mt76/firmware/mt7662.bin $(INSTALLDIR)/ath9k/lib/firmware/mediatek
	-cp -av $(MAC80211_PATH)/drivers/net/wireless/mediatek/mt76/firmware/mt7610e.bin $(INSTALLDIR)/ath9k/lib/firmware/mediatek
else
	rm -f $(INSTALLDIR)/ath9k/lib/modules/$(KERNELRELEASE)/mt76x*
endif
endif
endif
ifeq ($(CONFIG_MT7603),y)
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/mediatek
	-cp -av $(MAC80211_PATH)/drivers/net/wireless/mediatek/mt76/firmware/mt7603* $(INSTALLDIR)/ath9k/lib/firmware/mediatek
	-cp -av $(MAC80211_PATH)/drivers/net/wireless/mediatek/mt76/firmware/mt7628* $(INSTALLDIR)/ath9k/lib/firmware/mediatek
endif
ifeq ($(CONFIG_MT7663),y)
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/mediatek
	-cp -av $(MAC80211_PATH)/drivers/net/wireless/mediatek/mt76/firmware/mt7663* $(INSTALLDIR)/ath9k/lib/firmware/mediatek
endif
ifeq ($(CONFIG_MT7915),y)
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/mediatek
	-cp -av $(MAC80211_PATH)/drivers/net/wireless/mediatek/mt76/firmware/mt7915* $(INSTALLDIR)/ath9k/lib/firmware/mediatek
	-cp -av $(MAC80211_PATH)/drivers/net/wireless/mediatek/mt76/firmware/mt7916* $(INSTALLDIR)/ath9k/lib/firmware/mediatek
endif
ifeq ($(CONFIG_MT7921),y)
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/mediatek
	-cp -av $(MAC80211_PATH)/drivers/net/wireless/mediatek/mt76/firmware/*MT7961* $(INSTALLDIR)/ath9k/lib/firmware/mediatek
endif
ifeq ($(CONFIG_MT7996),y)
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/mediatek
	-cp -av $(MAC80211_PATH)/drivers/net/wireless/mediatek/mt76/firmware/mt7996* $(INSTALLDIR)/ath9k/lib/firmware/mediatek
endif
ifeq ($(CONFIG_X86),y)
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/mediatek
	-cp -av $(MAC80211_PATH)/drivers/net/wireless/mediatek/mt76/firmware/* $(INSTALLDIR)/ath9k/lib/firmware/mediatek
endif
ifeq ($(CONFIG_NEWPORT),y)
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware/mediatek
	-cp -av $(MAC80211_PATH)/drivers/net/wireless/mediatek/mt76/firmware/* $(INSTALLDIR)/ath9k/lib/firmware/mediatek
endif
	-mkdir -p $(INSTALLDIR)/ath9k/lib/firmware
	-cp $(REGPATH)/regulatory.db $(INSTALLDIR)/ath9k/lib/firmware
else
	@true
endif

ath9k-clean:
	rm -f $(MAC80211_PATH)/.config $(MAC80211_PATH)/.compat_autoconf_*
	MAKEFLAGS= KERNELRELEASE= CFLAGS= $(MAKE) -C $(MAC80211_PATH) $(MAKE_OPTS) clean

include $(TOP)/mac80211-rules/ath9k-userspace.mk
