include $(RT28xx_DIR)/os/linux/config.mk

#ifdef CONFIG_AP_SUPPORT
ifeq ($(RT28xx_MODE),AP)
MOD_NAME = rt$(MODULE)ap
DAT_PATH = /etc/Wireless/RT$(CHIPSET_DAT)AP
DAT_FILE_NAME = RT$(CHIPSET_DAT)AP.dat
endif
#endif // CONFIG_AP_SUPPORT //



obj-m := $(MOD_NAME).o

#ifdef CONFIG_AP_SUPPORT
ifeq ($(RT28xx_MODE),AP)
$(MOD_NAME)-objs := \
	../../common/crypt_md5.o\
	../../common/crypt_sha2.o\
	../../common/crypt_hmac.o\
	../../common/crypt_aes.o\
	../../common/crypt_arc4.o\
	../../common/mlme.o\
	../../common/cmm_wep.o\
	../../common/action.o\
	../../common/cmm_data.o\
	../../common/rtmp_init.o\
	../../common/rtmp_init_inf.o\
	../../common/cmm_tkip.o\
	../../common/cmm_aes.o\
	../../common/cmm_sync.o\
	../../common/eeprom.o\
	../../common/cmm_sanity.o\
	../../common/cmm_info.o\
	../../common/cmm_cfg.o\
	../../common/cmm_wpa.o\
	../../common/cmm_radar.o\
	../../common/spectrum.o\
	../../common/rtmp_timer.o\
	../../common/rt_channel.o\
	../../common/cmm_profile.o\
	../../common/cmm_asic.o\
	../../common/cmm_cmd.o\
	../../common/ps.o\
	../../rate_ctrl/ra_ctrl.o\
	../../rate_ctrl/alg_legacy.o\
	../../rate_ctrl/alg_ags.o\
	../../os/linux/rt_profile.o\
	../../ap/ap_mbss.o\
	../../chips/rtmp_chip.o\
	../../ap/ap.o\
	../../ap/ap_assoc.o\
	../../ap/ap_auth.o\
	../../ap/ap_connect.o\
	../../ap/ap_mlme.o\
	../../ap/ap_sanity.o\
	../../ap/ap_sync.o\
	../../ap/ap_wpa.o\
	../../ap/ap_data.o\
	../../common/uapsd.o\
	../../ap/ap_autoChSel.o\
	../../ap/ap_qload.o\
	../../ap/ap_cfg.o

ifeq ($(HAS_ATE),y)
$(MOD_NAME)-objs += ../../ate/common/rt_ate.o
endif

ifeq ($(HAS_QA_SUPPORT),y)
$(MOD_NAME)-objs += ../../ate/common/rt_qa.o
endif

ifeq ($(OSABL),NO)
$(MOD_NAME)-objs += \
	../../ap/ap_mbss_inf.o\
	../../common/rt_os_util.o\
	../../os/linux/ap_ioctl.o\
	../../os/linux/rt_linux.o\
	../../os/linux/rt_main_dev.o
else
$(MOD_NAME)-objs += \
	../../os/linux/rt_symb.o
endif


ifeq ($(HAS_NEW_RATE_ADAPT_SUPPORT),y)
$(MOD_NAME)-objs += ../../rate_ctrl/alg_grp.o
endif

#ifdef DOT11_N_SUPPORT
ifeq ($(HAS_DOT11_N_SUPPORT),y)
$(MOD_NAME)-objs += \
	../../common/ba_action.o

#ifdef TXBF_SUPPORT
ifeq ($(HAS_TXBF_SUPPORT),y)
$(MOD_NAME)-objs += \
	../../common/cmm_txbf.o\
	../../common/cmm_txbf_cal.o
endif
#endif // TXBF_SUPPORT //
endif
#endif // DOT11_N_SUPPORT //

#ifdef BG_FT_SUPPORT
ifeq ($(OSABL),NO)
ifeq ($(HAS_BGFP_SUPPORT),y)
$(MOD_NAME)-objs += \
	../../os/linux/br_ftph.o
endif
endif
#endif // BG_FT_SUPPORT //


#ifdef CRDA_SUPPORT
ifeq ($(OSABL),NO)
ifeq ($(HAS_CFG80211_SUPPORT),y)
$(MOD_NAME)-objs += \
	../../os/linux/cfg80211.o\
	../../os/linux/cfg80211drv.o
endif
endif

ifeq ($(OSABL),YES)
ifeq ($(HAS_CFG80211_SUPPORT),y)
$(MOD_NAME)-objs += \
	../../os/linux/cfg80211drv.o
endif
endif
#endif // CRDA_SUPPORT //

#ifdef LED_CONTROL_SUPPORT
ifeq ($(HAS_LED_CONTROL_SUPPORT),y)
$(MOD_NAME)-objs += \
	../../common/rt_led.o
endif
#endif // LED_CONTROL_SUPPORT //


ifeq ($(HAS_RT2880_RT2860_COEXIST),y)	
RT28XX_AP_OBJ += \
	../../os/linux/rt_pci_rbus.o\
	../../os/linux/rt_rbus_pci_util.o\
	../../os/linux/pci_main_dev.o\
	../../common/dfs.o
endif

#ifdef WSC_INCLUDED
ifeq ($(HAS_WSC),y)
$(MOD_NAME)-objs += \
	../../common/wsc.o\
	../../common/wsc_tlv.o\
	../../common/wsc_ufd.o\
	../../common/crypt_biginteger.o\
	../../common/crypt_dh.o	
endif
ifeq ($(HAS_WSC_V2),y)
$(MOD_NAME)-objs += ../../common/wsc_v2.o
endif
#endif // WSC_INCLUDED //



ifeq ($(HAS_WDS),y)
$(MOD_NAME)-objs += ../../ap/ap_wds.o

ifeq ($(OSABL),NO)
$(MOD_NAME)-objs += \
	../../ap/ap_wds_inf.o
endif
endif

ifeq ($(PLATFORM),BB_SOC)
$(MOD_NAME)-objs += \
	../../os/linux/bb_soc.o
endif

#ifdef APCLI_SUPPORT
ifeq ($(HAS_APCLI),y)
$(MOD_NAME)-objs += \
	../../ap/ap_apcli.o \
	../../ap/apcli_ctrl.o \
	../../ap/apcli_sync.o \
	../../ap/apcli_auth.o \
	../../ap/apcli_assoc.o \
	../../common/cmm_mat.o \
	../../common/cmm_mat_iparp.o \
	../../common/cmm_mat_pppoe.o \
	../../common/cmm_mat_ipv6.o

ifeq ($(OSABL),NO)
$(MOD_NAME)-objs += \
	../../ap/ap_apcli_inf.o
endif
endif
#endif // APCLI_SUPPORT //

ifeq ($(HAS_BLOCK_NET_IF),y)
$(MOD_NAME)-objs += ../../common/netif_block.o
endif

ifeq ($(HAS_IGMP_SNOOP_SUPPORT),y)
$(MOD_NAME)-objs += ../../common/igmp_snoop.o
endif


ifeq ($(HAS_QOS_DLS_SUPPORT),y)
$(MOD_NAME)-objs += ../../ap/ap_dls.o
endif

ifeq ($(HAS_IDS_SUPPORT),y)
$(MOD_NAME)-objs += ../../ap/ap_ids.o
endif

ifeq ($(PLATFORM),IKANOS_V160)
$(MOD_NAME)-objs += ../../os/linux/vr_ikans.o
endif

ifeq ($(PLATFORM),IKANOS_V180)
$(MOD_NAME)-objs += ../../os/linux/vr_ikans.o
endif

ifeq ($(PLATFORM),BL2348)
ifeq ($(OSABL),NO)
$(MOD_NAME)-objs += ../../os/linux/vr_bdlt.o
endif
endif
#ifdef PLATFORM_BL23570
ifeq ($(PLATFORM),BL23570)
rt$(CHIPSET)ap-objs += \
        ../../os/linux/vr_bdlt.o
endif
#endif // PLATFORM_BL23570 //


#ifdef WAPI_SUPPORT
ifeq ($(HAS_WAPI_SUPPORT),y)
$(MOD_NAME)-objs += \
	../../common/wapi.o
endif
#endif // WAPI_SUPPORT //





ifeq ($(HAS_CLIENT_WDS_SUPPORT),y)
$(MOD_NAME)-objs += \
	../../common/client_wds.o
endif

ifeq ($(HAS_EASY_CONFIG_SETUP_SUPPORT),y)
ifeq ($(PLATFORM),RALINK_3052)
$(MOD_NAME)-objs += lib.a
endif
endif

ifeq ($(HAS_DFS_SUPPORT),y)
$(MOD_NAME)-objs += \
        ../../common/cmm_dfs.o
endif

ifeq ($(HAS_CS_SUPPORT),y)
$(MOD_NAME)-objs += \
        ../../common/cmm_cs.o
endif

endif
#endif // CONFIG_AP_SUPPORT //




#chip releated


#ifdef RT3090
ifneq ($(findstring 3090,$(CHIPSET)),)
$(MOD_NAME)-objs += \
	../../common/cmm_mac_pci.o\
	../../common/cmm_data_pci.o\
	../../os/linux/rt_rbus_pci_drv.o\
	../../common/ee_prom.o\
	../../common/ee_efuse.o\
	../../common/rtmp_mcu.o\
../../chips/rt30xx.o\
	../../common/rt_rf.o\
	../../chips/rt3090.o

ifeq ($(HAS_ATE),y)
$(MOD_NAME)-objs += \
	../../ate/chips/rt30xx_ate.o\
	../../ate/common/ate_pci.o
endif

ifeq ($(OSABL),NO)
$(MOD_NAME)-objs += \
	../../os/linux/rt_pci_rbus.o\
	../../os/linux/rt_rbus_pci_util.o\
	../../os/linux/pci_main_dev.o
endif
ifeq ($(HAS_RTMP_FLASH_SUPPORT),y)
$(MOD_NAME)-objs += \
        ../../common/ee_flash.o
endif

endif
#endif // RT3090 //














#ifdef RTMP_MAC_PCI
#ifdef RT3593
ifneq ($(findstring 3593,$(CHIPSET)),)
$(MOD_NAME)-objs += \
	../../common/cmm_mac_pci.o\
	../../common/cmm_data_pci.o\
	../../os/linux/rt_rbus_pci_drv.o\
	../../common/rtmp_mcu.o\
	../../common/ee_prom.o\
	../../common/ee_efuse.o\
	../../common/rt_rf.o\
	../../chips/rt30xx.o\
	../../chips/rt35xx.o\
	../../chips/rt28xx.o\
	../../chips/rt3593.o

ifeq ($(HAS_ATE),y)
$(MOD_NAME)-objs += \
	../../ate/chips/rt30xx_ate.o\
	../../ate/chips/rt35xx_ate.o\
	../../ate/chips/rt3593_ate.o\
	../../ate/common/ate_pci.o
endif

ifeq ($(OSABL),NO)
$(MOD_NAME)-objs += \
	../../os/linux/rt_pci_rbus.o\
	../../os/linux/rt_rbus_pci_util.o\
	../../os/linux/pci_main_dev.o
endif


ifneq ($(findstring $(RT28xx_MODE),STA),)
$(MOD_NAME)-objs += \
	../../common/frq_cal.o
endif

endif
#endif // RT3593 //
#endif // RTMP_MAC_PCI //

#ifdef RT3390
ifneq ($(findstring 3390,$(CHIPSET)),)
$(MOD_NAME)-objs += \
	../../common/cmm_mac_pci.o\
	../../common/cmm_data_pci.o\
	../../os/linux/rt_rbus_pci_drv.o\
	../../common/rtmp_mcu.o\
	../../common/ee_prom.o\
	../../common/ee_efuse.o\
	../../common/rt_rf.o\
	../../chips/rt30xx.o\
	../../chips/rt3090.o\
	../../chips/rt33xx.o\
	../../chips/rt3390.o

ifeq ($(HAS_ATE),y)
$(MOD_NAME)-objs += \
	../../ate/chips/rt33xx_ate.o\
	../../ate/chips/rt30xx_ate.o\
	../../ate/common/ate_pci.o
endif

ifeq ($(OSABL),NO)
$(MOD_NAME)-objs += \
	../../os/linux/rt_pci_rbus.o\
	../../os/linux/rt_rbus_pci_util.o\
	../../os/linux/pci_main_dev.o
endif
endif
#endif // RT3390 //



#ifdef RT5390
ifneq ($(findstring 5390,$(CHIPSET)),)
$(MOD_NAME)-objs += \
        ../../common/cmm_mac_pci.o\
        ../../common/cmm_data_pci.o\
        ../../os/linux/rt_rbus_pci_drv.o\
        ../../common/rtmp_mcu.o\
        ../../common/ee_prom.o\
        ../../common/ee_efuse.o\
        ../../common/rt_rf.o\
	../../chips/rt30xx.o\
        ../../chips/rt3090.o\
        ../../chips/rt33xx.o\
	../../chips/rt3390.o\
        ../../chips/rt5390.o

ifeq ($(HAS_ATE),y)
$(MOD_NAME)-objs += \
	../../ate/chips/rt5390_ate.o\
	../../ate/chips/rt33xx_ate.o\
	../../ate/chips/rt30xx_ate.o\
	../../ate/common/ate_pci.o
endif

ifeq ($(OSABL),NO)
$(MOD_NAME)-objs += \
        ../../os/linux/rt_pci_rbus.o\
        ../../os/linux/rt_rbus_pci_util.o\
        ../../os/linux/pci_main_dev.o
endif

ifneq ($(findstring $(RT28xx_MODE),STA),)
$(MOD_NAME)-objs += \
	../../common/frq_cal.o
endif

endif
#endif // RT5390 //



#ifdef RTMP_MAC_PCI
#ifdef RT5592
ifneq ($(findstring 5592,$(CHIPSET)),)
$(MOD_NAME)-objs += \
		../../common/cmm_mac_pci.o\
		../../common/cmm_data_pci.o\
		../../os/linux/rt_rbus_pci_drv.o\
		../../common/ee_prom.o\
		../../common/ee_efuse.o\
		../../common/rtmp_mcu.o\
		../../common/rt_rf.o\
		../../chips/rt30xx.o\
		../../chips/rt5592.o

ifeq ($(HAS_RTMP_FLASH_SUPPORT),y)
$(MOD_NAME)-objs += \
        ../../common/ee_flash.o
endif

ifeq ($(HAS_ATE),y)
$(MOD_NAME)-objs += \
	../../ate/chips/rt5592_ate.o\
	../../ate/common/ate_pci.o
endif

ifeq ($(OSABL),NO)
$(MOD_NAME)-objs += \
		../../os/linux/rt_pci_rbus.o\
		../../os/linux/rt_rbus_pci_util.o\
		../../os/linux/pci_main_dev.o
endif

ifneq ($(findstring $(RT28xx_MODE),STA),)
$(MOD_NAME)-objs += \
	../../common/frq_cal.o
endif

endif
#endif // RT5592 //
#endif //RTMP_MAC_PCI//


PHONY := clean install uninstall

clean:
	rm -f ../../common/*.o
	rm -f ../../common/.*.{cmd,flags,d}
	rm -f ../../os/linux/*.{o,ko,mod.{o,c}}
	rm -f ../../os/linux/.*.{cmd,flags,d}
	rm -fr ../../os/linux/.tmp_versions
#Must clean Module.symvers; or you will suffer symbol version not match
#when OS_ABL = YES.
	rm -f ../../os/linux/Module.symvers
	rm -f ../../os/linux/Modules.symvers
	rm -f ../../os/linux/Module.markers
	rm -f ../../os/linux/modules.order
	rm -f ../../chips/*.o
	rm -f ../../chips/.*.{cmd,flags,d}
ifeq ($(RT28xx_MODE),AP)
	rm -f ../../ap/*.o
	rm -f ../../ap/.*.{cmd,flags,d}
else
ifeq ($(RT28xx_MODE),STA)
	rm -f ../../sta/*.o
	rm -f ../../sta/.*.{cmd,flags,d}
else
ifeq ($(RT28xx_MODE),APSTA)
	rm -f ../../ap/*.o
	rm -f ../../ap/.*.{cmd,flags,d}
	rm -f ../../sta/*.o
	rm -f ../../sta/.*.{cmd,flags,d}
endif
endif
endif

install:
	rm -rf $(DAT_PATH)
	$(shell [ ! -f /etc/Wireless ] && mkdir /etc/Wireless)
	mkdir $(DAT_PATH)
	cp $(RT28xx_DIR)/$(DAT_FILE_NAME) $(DAT_PATH)/.
	install -d $(LINUX_SRC_MODULE)
	install -m 644 -c $(addsuffix .ko,$(MOD_NAME)) $(LINUX_SRC_MODULE)
	/sbin/depmod -a ${shell uname -r}

uninstall:
#	rm -rf $(DAT_PATH)
	rm -rf $(addprefix $(LINUX_SRC_MODULE),$(addsuffix .ko,$(MOD_NAME)))
	/sbin/depmod -a ${shell uname -r}

# Declare the contents of the .PHONY variable as phony.  We keep that
# information in a variable so we can use it in if_changed and friends.
.PHONY: $(PHONY)
