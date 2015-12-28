include $(RT28xx_DIR)/os/linux/config.mk

#ifdef CONFIG_AP_SUPPORT
ifeq ($(RT28xx_MODE),AP)
MOD_NAME = rt$(MODULE)ap
DAT_PATH = /etc/Wireless/RT$(CHIPSET_DAT)AP
DAT_FILE_NAME = RT$(CHIPSET_DAT)AP.dat
endif
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
ifeq ($(RT28xx_MODE), STA)
MOD_NAME = rt$(MODULE)sta
DAT_PATH = /etc/Wireless/RT$(CHIPSET_DAT)STA
DAT_FILE_NAME = RT$(CHIPSET_DAT)STA.dat
endif
#endif // CONFIG_STA_SUPPORT //

#ifdef CONFIG_APSTA_SUPPORT
ifeq ($(RT28xx_MODE), APSTA)
MOD_NAME = rt$(MODULE)apsta
endif
#endif // CONFIG_APSTA_SUPPORT //

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

#ifdef CONFIG_STA_SUPPORT
ifeq ($(RT28xx_MODE), STA)
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
	../../common/uapsd.o\
	../../rate_ctrl/ra_ctrl.o\
	../../rate_ctrl/alg_legacy.o\
	../../rate_ctrl/alg_ags.o\
	../../os/linux/rt_profile.o\
	../../chips/rtmp_chip.o\
	../../sta/assoc.o\
	../../sta/auth.o\
	../../sta/auth_rsp.o\
	../../sta/sync.o\
	../../sta/sanity.o\
	../../sta/rtmp_data.o\
	../../sta/connect.o\
	../../sta/wpa.o\
	../../sta/sta_cfg.o

ifeq ($(OSABL),NO)
$(MOD_NAME)-objs += \
	../../common/rt_os_util.o\
	../../os/linux/sta_ioctl.o\
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

#ifdef ETH_CONVERT
ifeq ($(HAS_ETH_CONVERT_SUPPORT), y)
$(MOD_NAME)-objs += \
	../../common/cmm_mat.o \
	../../common/cmm_mat_iparp.o \
	../../common/cmm_mat_pppoe.o \
	../../common/cmm_mat_ipv6.o
endif
#endif // ETH_CONVERT //

#ifdef ADHOC_WPA2PSK_SUPPORT
$(MOD_NAME)-objs += \
	../../common/cmm_wpa_adhoc.o
#endif // ADHOC_WPA2PSK_SUPPORT //

#ifdef WSC_INCLUDED
ifeq ($(HAS_WSC),y)
$(MOD_NAME)-objs += \
	../../common/wsc.o\
	../../common/wsc_tlv.o\
	../../common/crypt_biginteger.o\
	../../common/crypt_dh.o
endif
ifeq ($(HAS_WSC_V2),y)
$(MOD_NAME)-objs += ../../common/wsc_v2.o

ifeq ($(HAS_WIDI_SUPPORT),y)
$(MOD_NAME)-objs += ../../common/l2sd_ta_mod.o
endif

ifeq ($(HAS_IWSC_SUPPORT),y)
$(MOD_NAME)-objs += ../../sta/sta_iwsc.o
endif

endif
#endif // WSC_INCLUDED //



ifeq ($(HAS_BLOCK_NET_IF),y)
$(MOD_NAME)-objs += ../../common/netif_block.o
endif

ifeq ($(HAS_QOS_DLS_SUPPORT),y)
$(MOD_NAME)-objs += ../../sta/dls.o
endif

#ifdef DOT11Z_TDLS_SUPPORT
ifeq ($(HAS_DOT11Z_TDLS_SUPPORT),y)
$(MOD_NAME)-objs += \
	../../sta/tdls.o\
	../../sta/tdls_ctrl.o\
	../../sta/tdls_tlv.o\
	../../sta/tdls_link_mng.o\
	../../sta/tdls_uapsd.o
endif
#endif // DOT11Z_TDLS_SUPPORT //


#ifdef LED_CONTROL_SUPPORT
ifeq ($(HAS_LED_CONTROL_SUPPORT),y)
$(MOD_NAME)-objs += \
	../../common/rt_led.o
endif
#endif // LED_CONTROL_SUPPORT //


ifeq ($(HAS_ATE),y)
$(MOD_NAME)-objs += ../../ate/common/rt_ate.o
endif

ifeq ($(HAS_QA_SUPPORT),y)
$(MOD_NAME)-objs += ../../ate/common/rt_qa.o
endif



#ifdef WAPI_SUPPORT
ifeq ($(HAS_WAPI_SUPPORT),y)
$(MOD_NAME)-objs += \
	../../common/wapi.o
endif
#endif // WAPI_SUPPORT //



#ifdef P2P_SUPPORT
ifeq ($(OSABL),NO)
ifeq ($(HAS_P2P_SUPPORT),y)
$(MOD_NAME)-objs += \
        ../../common/p2p_inf.o
endif
endif

ifeq ($(HAS_P2P_SUPPORT),y)
$(MOD_NAME)-objs += \
	../../ap/ap_mbss.o\
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
	../../sta/p2pcli.o \
	../../sta/p2pcli_ctrl.o \
	../../sta/p2pcli_sync.o \
	../../sta/p2pcli_auth.o \
	../../sta/p2pcli_assoc.o \
	../../common/cmm_mat.o \
	../../common/cmm_mat_iparp.o \
	../../common/cmm_mat_pppoe.o \
	../../common/cmm_mat_ipv6.o \
	../../common/p2p_dbg.o\
	../../common/p2p_cfg.o\
	../../common/p2p.o\
	../../common/p2p_packet.o\
	../../common/p2p_action.o\
	../../common/p2p_table.o\
	../../common/p2p_ctrl.o\
	../../common/p2p_disc_mng.o\
	../../common/p2p_nego_mng.o\
	../../common/p2p_dbg.o\
	../../common/p2p.o\
	../../common/p2p_packet.o\
	../../common/p2p_action.o\
	../../common/p2p_table.o\
	../../common/p2p_ctrl.o\
	../../common/p2p_disc_mng.o\
	../../common/p2p_nego_mng.o\
	../../common/wsc.o\
	../../common/wsc_tlv.o\
	../../common/wsc_ufd.o\
	../../common/crypt_biginteger.o\
	../../common/crypt_dh.o	

ifeq ($(HAS_QOS_DLS_SUPPORT),y)
$(MOD_NAME)-objs += ../../ap/ap_dls.o
endif

endif

ifeq ($(HAS_P2P_SUPPORT),y)
ifeq ($(OSABL),NO)
$(MOD_NAME)-objs += \
        ../../ap/ap_mbss_inf.o\
        ../../common/rt_os_util.o\
        ../../os/linux/ap_ioctl.o\
        ../../os/linux/sta_ioctl.o\
        ../../os/linux/rt_linux.o\
        ../../os/linux/rt_main_dev.o
else
$(MOD_NAME)-objs += \
        ../../os/linux/rt_symb.o
endif
endif
#endif // P2P_SUPPORT //

#ifdef CRDA_SUPPORT
ifeq ($(OSABL),NO)
ifeq ($(HAS_CFG80211_SUPPORT),y)
$(MOD_NAME)-objs += \
	../../os/linux/cfg80211.o \
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


ifeq ($(HAS_DFS_SUPPORT),y)
$(MOD_NAME)-objs += \
        ../../common/cmm_dfs.o
endif

ifeq ($(HAS_CS_SUPPORT),y)
$(MOD_NAME)-objs += \
        ../../common/cmm_cs.o
endif

endif
#endif // CONFIG_STA_SUPPORT //

#ifdef CONFIG_APSTA_SUPPORT
ifeq ($(RT28xx_MODE), APSTA)
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
	../../common/cmm_wpa_adhoc.o\
	../../common/dfs.o\
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
	../../sta/assoc.o\
	../../sta/auth.o\
	../../sta/auth_rsp.o\
	../../sta/sync.o\
	../../sta/sanity.o\
	../../sta/rtmp_data.o\
	../../sta/connect.o\
	../../sta/wpa.o\
	../../ap/ap_autoChSel.o\
	../../ap/ap_qload.o\
	../../ap/ap_cfg.o\
	../../sta/sta_cfg.o

ifeq ($(OSABL),NO)
$(MOD_NAME)-objs += \
	../../ap/ap_mbss_inf.o\
	../../common/rt_os_util.o\
	../../os/linux/ap_ioctl.o\
	../../os/linux/sta_ioctl.o\
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


#ifdef LED_CONTROL_SUPPORT
ifeq ($(HAS_LED_CONTROL_SUPPORT),y)
$(MOD_NAME)-objs += \
	../../common/rt_led.o
endif
#endif // LED_CONTROL_SUPPORT //

#ifdef WSC_INCLUDED
ifeq ($(HAS_WSC),y)
$(MOD_NAME)-objs += \
	../../common/wsc.o\
	../../common/wsc_tlv.o\
	../../common/crypt_biginteger.o\
	../../common/wsc_ufd.o\
	../../common/crypt_dh.o
endif
#endif // WSC_INCLUDED //



ifeq ($(HAS_WDS),y)
$(MOD_NAME)-objs += ../../ap/ap_wds.o

ifeq ($(OSABL),NO)
$(MOD_NAME)-objs += \
	../../ap/ap_wds_inf.o
endif
endif

ifeq ($(HAS_QOS_DLS_SUPPORT),y)
$(MOD_NAME)-objs += ../../ap/ap_dls.o
endif

ifeq ($(HAS_IDS_SUPPORT),y)
$(MOD_NAME)-objs += ../../ap/ap_ids.o
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

#ifdef ETH_CONVERT
ifeq ($(HAS_ETH_CONVERT_SUPPORT), y)
$(MOD_NAME)-objs += \
	../../common/cmm_mat.o \
	../../common/cmm_mat_iparp.o \
	../../common/cmm_mat_pppoe.o \
	../../common/cmm_mat_ipv6.o
endif
#endif // ETH_CONVERT //

ifeq ($(HAS_BLOCK_NET_IF),y)
$(MOD_NAME)-objs += ../../common/netif_block.o
endif

ifeq ($(HAS_IGMP_SNOOP_SUPPORT),y)
$(MOD_NAME)-objs += ../../common/igmp_snoop.o
endif


#ifdef WAPI_SUPPORT
ifeq ($(HAS_WAPI_SUPPORT),y)
$(MOD_NAME)-objs += \
	../../common/libwapi.a\
	../../common/wapi.o
endif
#endif // WAPI_SUPPORT //


ifeq ($(HAS_CLIENT_WDS_SUPPORT),y)
$(MOD_NAME)-objs += \
	../../common/client_wds.o
endif

#ifdef CRDA_SUPPORT
ifeq ($(OSABL),NO)
ifeq ($(HAS_CFG80211_SUPPORT),y)
$(MOD_NAME)-objs += \
	../../os/linux/cfg80211.o
endif
endif
#endif // CRDA_SUPPORT //

#ifdef P2P_SUPPORT
ifeq ($(OSABL),NO)
ifeq ($(HAS_P2P_SUPPORT),y)
$(MOD_NAME)-objs += \
        ../../common/p2p_inf.o
endif
endif

ifeq ($(HAS_P2P_SUPPORT),y)
$(MOD_NAME)-objs += \
	../../sta/p2pcli.o \
	../../sta/p2pcli_ctrl.o \
	../../sta/p2pcli_sync.o \
	../../sta/p2pcli_auth.o \
	../../sta/p2pcli_assoc.o \
	../../common/cmm_mat.o \
	../../common/cmm_mat_iparp.o \
	../../common/cmm_mat_pppoe.o \
	../../common/cmm_mat_ipv6.o \
	../../common/p2p_dbg.o\
	../../common/p2p.o\
	../../common/p2p_packet.o\
	../../common/p2p_action.o\
	../../common/p2p_table.o\
	../../common/p2p_ctrl.o\
	../../common/p2p_disc_mng.o\
	../../common/p2p_nego_mng.o
endif
#endif // P2P_SUPPORT //
endif
#endif // CONFIG_APSTA_SUPPORT //


#chip releated

#ifdef RT2860
ifneq ($(findstring 2860,$(CHIPSET)),)
$(MOD_NAME)-objs += \
	../../common/cmm_mac_pci.o\
	../../common/cmm_data_pci.o\
	../../os/linux/rt_rbus_pci_drv.o\
	../../common/rtmp_mcu.o\
	../../common/ee_prom.o\
	../../chips/rt28xx.o

ifeq ($(HAS_ATE),y)
$(MOD_NAME)-objs += \
	../../ate/chips/rt28xx_ate.o\
	../../ate/common/ate_pci.o
endif

ifeq ($(OSABL),NO)
$(MOD_NAME)-objs += \
	../../os/linux/rt_pci_rbus.o\
	../../os/linux/rt_rbus_pci_util.o\
	../../os/linux/pci_main_dev.o
endif
endif
#endif // RT2860 //




#ifdef RT2880
ifneq ($(findstring 2880,$(CHIPSET)),)
$(MOD_NAME)-objs += \
	../../common/cmm_mac_pci.o\
	../../common/cmm_data_pci.o\
	../../os/linux/rt_rbus_pci_drv.o\
	../../common/ee_flash.o\
	../../common/rtmp_swmcu.o\
	../../chips/rt2880.o

ifeq ($(HAS_ATE),y)
$(MOD_NAME)-objs += \
	../../ate/chips/rt28xx_ate.o\
	../../ate/common/ate_pci.o
endif

ifeq ($(OSABL),NO)
$(MOD_NAME)-objs += \
	../../os/linux/rt_pci_rbus.o\
	../../os/linux/rt_rbus_pci_util.o\
	../../os/linux/pci_main_dev.o
endif

endif
#endif // RT2880 //

#ifdef RT3052
ifneq ($(findstring 3052,$(CHIPSET)),)
$(MOD_NAME)-objs += \
	../../common/cmm_mac_pci.o\
	../../common/cmm_data_pci.o\
	../../common/ee_flash.o\
	../../common/rtmp_swmcu.o\
	../../common/rt_rf.o\
	../../chips/rt305x.o

ifeq ($(HAS_ATE),y)
$(MOD_NAME)-objs += \
	../../ate/chips/rt305x_ate.o\
	../../ate/common/ate_pci.o
endif

ifeq ($(OSABL),NO)
$(MOD_NAME)-objs += \
	../../os/linux/rt_pci_rbus.o\
	../../os/linux/rt_proc.o\
	../../os/linux/rt_rbus_pci_util.o\
	../../os/linux/rt_rbus_pci_drv.o\
	../../os/linux/rbus_main_dev.o
endif

endif
#endif // RT3052 //

#ifdef RT3352
ifneq ($(findstring 3352,$(CHIPSET)),)
$(MOD_NAME)-objs += \
	../../common/cmm_mac_pci.o\
	../../common/cmm_data_pci.o\
	../../common/ee_flash.o\
	../../common/rtmp_swmcu.o\
	../../common/rt_rf.o\
	../../chips/rt305x.o\
	../../chips/rt3352.o

ifeq ($(HAS_ATE),y)
$(MOD_NAME)-objs += \
	../../ate/chips/rt305x_ate.o\
	../../ate/chips/rt3352_ate.o
	../../ate/common/ate_pci.o
endif

ifeq ($(OSABL),NO)
$(MOD_NAME)-objs += \
	../../os/linux/rt_pci_rbus.o\
	../../os/linux/rt_proc.o\
	../../os/linux/rt_rbus_pci_util.o\
	../../os/linux/rt_rbus_pci_drv.o\
	../../os/linux/rbus_main_dev.o
endif

endif
#endif // RT3352 //


#ifdef RT5350
ifneq ($(findstring 5350,$(CHIPSET)),)
$(MOD_NAME)-objs += \
	../../common/cmm_mac_pci.o\
	../../common/cmm_data_pci.o\
	../../common/ee_flash.o\
	../../common/rtmp_swmcu.o\
	../../common/rt_rf.o\
	../../chips/rt305x.o\
	../../chips/rt5350.o

ifeq ($(OSABL),NO)
$(MOD_NAME)-objs += \
	../../os/linux/rt_pci_rbus.o\
	../../os/linux/rt_proc.o\
	../../os/linux/rt_rbus_pci_util.o\
	../../os/linux/rt_rbus_pci_drv.o\
	../../os/linux/rbus_main_dev.o
endif

ifneq ($(findstring $(RT28xx_MODE),STA APSTA),)
$(MOD_NAME)-objs += \
	../../common/frq_cal.o
endif

ifeq ($(HAS_ATE),y)
$(MOD_NAME)-objs += \
#not need to include it yet.
#	../../ate/chips/rt305x_ate.o\
	../../ate/chips/rt5350_ate.o\
	../../ate/common/ate_pci.o
endif

endif
#endif // RT5350 //







#ifdef RTMP_MAC_PCI
#endif // RTMP_MAC_PCI //







#ifdef RTMP_MAC_PCI
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
