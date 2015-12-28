EXTRA_CFLAGS = -Idrivers/net/wireless/rt5592/include

obj-$(CONFIG_RTPCI_AP) += RTPCI_ap.o
RTPCI_ap-objs += common/crypt_md5.o\
				 common/crypt_sha2.o\
				 common/crypt_hmac.o\
				 common/crypt_aes.o\
				 common/crypt_arc4.o\
				 common/mlme.o\
				 common/cmm_wep.o\
				 common/action.o\
				 common/cmm_data.o\
				 common/rtmp_init.o\
				 common/rtmp_init_inf.o\
				 common/cmm_tkip.o\
				 common/cmm_aes.o\
				 common/cmm_sync.o\
				 common/eeprom.o\
				 common/cmm_sanity.o\
				 common/cmm_info.o\
				 common/cmm_cfg.o\
				 common/cmm_wpa.o\
				 common/cmm_radar.o\
				 common/spectrum.o\
				 common/rtmp_timer.o\
				 common/rt_channel.o\
				 common/cmm_profile.o\
				 common/cmm_asic.o\
				 common/cmm_cmd.o\
				 common/ps.o\
				 rate_ctrl/ra_ctrl.o\
				 rate_ctrl/alg_legacy.o\
				 rate_ctrl/alg_ags.o\
				 os/linux/rt_profile.o\
				 ap/ap_mbss.o\
				 chips/rtmp_chip.o\
				 ap/ap.o\
				 ap/ap_assoc.o\
				 ap/ap_auth.o\
				 ap/ap_connect.o\
				 ap/ap_mlme.o\
				 ap/ap_sanity.o\
				 ap/ap_sync.o\
				 ap/ap_wpa.o\
				 ap/ap_data.o\
				 common/uapsd.o\
				 ap/ap_autoChSel.o\
				 ap/ap_qload.o\
				 ap/ap_cfg.o

ifeq ($(CONFIG_RTPCI_AP_ATE),y)
RTPCI_ap-objs += ate/common/rt_ate.o
endif

ifeq ($(CONFIG_RTPCI_AP_QA),y)
RTPCI_ap-objs += ate/common/rt_qa.o
endif

#(OSABL),NO
RTPCI_ap-objs += \
		ap/ap_mbss_inf.o\
		common/rt_os_util.o\
		os/linux/ap_ioctl.o\
		os/linux/rt_linux.o\
		os/linux/rt_main_dev.o

#DOT11_N_SUPPORT
RTPCI_ap-objs += common/ba_action.o

ifeq ($(CONFIG_RTPCI_TXBF_SUPPORT),y)
RTPCI_ap-objs += \
		common/cmm_txbf.o\
		common/cmm_txbf_cal.o
endif

ifeq ($(CONFIG_RTPCI_AP_LED),y)
RTPCI_ap-objs += common/rt_led.o
endif

ifeq ($(CONFIG_RTPCI_AP_WSC),y)
RTPCI_ap-objs +=  \
          common/wsc.o\
          common/wsc_tlv.o\
          common/wsc_ufd.o\
          common/crypt_biginteger.o\
          common/crypt_dh.o
endif
ifeq ($(CONFIG_RTPCI_AP_WSC_V2),y)
RTPCI_ap-objs += common/wsc_v2.o
endif

ifeq ($(CONFIG_RTPCI_AP_WDS),y)
RTPCI_ap-objs += ap/ap_wds.o
#(OSABL),NO)
RTPCI_ap-objs += ap/ap_wds_inf.o
endif

ifeq ($(CONFIG_RTPCI_AP_NINTENDO),y)
RTPCI_ap-objs += ap/ap_nintendo.o
endif

ifeq ($(CONFIG_RTPCI_AP_APCLI),y)
RTPCI_ap-objs += \
          ap/ap_apcli.o \
          ap/apcli_ctrl.o \
          ap/apcli_sync.o \
          ap/apcli_auth.o \
          ap/apcli_assoc.o \
          common/cmm_mat.o \
          common/cmm_mat_iparp.o \
          common/cmm_mat_pppoe.o \
          common/cmm_mat_ipv6.o

#(OSABL),NO)
RTPCI_ap-objs += ap/ap_apcli_inf.o
endif

ifeq ($(CONFIG_RTPCI_AP_NETIF_BLOCK),y)
RTPCI_ap-objs += common/netif_block.o
endif

ifeq ($(CONFIG_RTPCI_AP_IGMP_SNOOP),y)
RTPCI_ap-objs += common/igmp_snoop.o
endif

ifeq ($(CONFIG_RTPCI_AP_DLS),y)
RTPCI_ap-objs += ap/ap_dls.o
endif

ifeq ($(CONFIG_RTPCI_AP_IDS),y)
RTPCI_ap-objs += ap/ap_ids.o
endif

ifeq ($(CONFIG_RTPCI_AP_WAPI),y)
RTPCI_ap-objs += common/wapi.o \
				 common/wapi_crypt.o \
				 common/wapi_sms4.o
endif

ifeq ($(CONFIG_RTPCI_AP_DFS),y)
RTPCI_ap-objs += common/cmm_dfs.o
endif

ifeq ($(CONFIG_RTPCI_AP_CARRIER),y)
RTPCI_ap-objs += common/cmm_cs.o
endif

ifeq ($(CONFIG_RTPCI_AP_FLASH),y)
RTPCI_ap-objs += common/ee_flash.o
else
RTPCI_ap-objs += common/ee_efuse.o
endif

#chip releated
#RT3090
ifeq ($(CONFIG_RT3090_AP_INCLUDE),y)
RTPCI_ap-objs += \
	common/cmm_mac_pci.o\
	common/cmm_data_pci.o\
	os/linux/rt_rbus_pci_drv.o\
	common/ee_prom.o\
	common/rtmp_mcu.o\
	chips/rt30xx.o\
	common/rt_rf.o\
	chips/rt3090.o

ifeq ($(CONFIG_RT3090_AP_ATE),y)
RTPCI_ap-objs += \
	ate/chips/rt30xx_ate.o\
	ate/common/ate_pci.o
endif

#ifeq ($(OSABL),NO)
RTPCI_ap-objs += \
	os/linux/rt_pci_rbus.o\
	os/linux/rt_rbus_pci_util.o\
	os/linux/pci_main_dev.o
#endif
endif
#end of RT3090

#RT3593
ifeq ($(CONFIG_RT3593_AP_INCLUDE),y)
RTPCI_ap-objs += \
    	common/cmm_mac_pci.o\
    	common/cmm_data_pci.o\
    	os/linux/rt_rbus_pci_drv.o\
    	common/rtmp_mcu.o\
    	common/ee_prom.o\
    	common/rt_rf.o\
    	chips/rt30xx.o\
    	chips/rt35xx.o\
    	chips/rt28xx.o\
    	chips/rt3593.o\
		rate_ctrl/alg_grp.o

ifeq ($(CONFIG_RTPCI_AP_VIDEO_TURBINE),y)
RTPCI_ap-objs += \
		common/cmm_video.o
endif

ifeq ($(CONFIG_RT3593_AP_ATE),y)
RTPCI_ap-objs += \
	    ate/chips/rt30xx_ate.o\
        ate/chips/rt35xx_ate.o\
        ate/chips/rt3593_ate.o\
        ate/common/ate_pci.o
endif

#(OSABL),NO)
RTPCI_ap-objs += \
        os/linux/rt_pci_rbus.o\
        os/linux/rt_rbus_pci_util.o\
        os/linux/pci_main_dev.o
endif
#end of RT3593

#RT5392
ifeq ($(CONFIG_RT5392_AP_INCLUDE),y)
RTPCI_ap-objs += \
        common/cmm_mac_pci.o\
        common/cmm_data_pci.o\
        os/linux/rt_rbus_pci_drv.o\
        common/rtmp_mcu.o\
        common/ee_prom.o\
        common/rt_rf.o\
        chips/rt30xx.o\
        chips/rt3090.o\
        chips/rt33xx.o\
        chips/rt3390.o\
        chips/rt5390.o

ifeq ($(CONFIG_RT5392_AP_ATE),y)
RTPCI_ap-objs += \
        ate/chips/rt5390_ate.o\
        ate/chips/rt33xx_ate.o\
        ate/chips/rt30xx_ate.o\
        ate/common/ate_pci.o
endif

#(OSABL),NO)
RTPCI_ap-objs += \
        os/linux/rt_pci_rbus.o\
        os/linux/rt_rbus_pci_util.o\
        os/linux/pci_main_dev.o
endif
#end of RT5390

#RT5592
ifeq ($(CONFIG_RT5592_AP_INCLUDE),y)
RTPCI_ap-objs += \
        common/cmm_mac_pci.o\
        common/cmm_data_pci.o\
        os/linux/rt_rbus_pci_drv.o\
        common/ee_prom.o\
        common/rtmp_mcu.o\
        common/rt_rf.o\
        chips/rt30xx.o\
        chips/rt5592.o

ifeq ($(CONFIG_RT5592_EP_SUPPORT),y)
EXTRA_CFLAGS += -DRT5592EP_SUPPORT
endif

ifeq ($(CONFIG_RT5592_AP_ATE),y)
RTPCI_ap-objs += \
	    ate/chips/rt30xx_ate.o\
        ate/chips/rt5592_ate.o\
        ate/common/ate_pci.o
endif

#(OSABL),NO)
RTPCI_ap-objs += \
        os/linux/rt_pci_rbus.o\
        os/linux/rt_rbus_pci_util.o\
        os/linux/pci_main_dev.o
endif
#end of RT5592

####################
##  CFLAGS
####################
EXTRA_CFLAGS += -DAGGREGATION_SUPPORT \
				-DPIGGYBACK_SUPPORT \
				-DWMM_SUPPORT \
				-DLINUX -Wall \
				-Wstrict-prototypes \
				-Wno-trigraphs \
				-DSYSTEM_LOG_SUPPORT \
				-DRESOURCE_PRE_ALLOC \
				-DRT_SOC_SUPPORT

ifeq ($(CONFIG_RTPCI_AP_FLASH),y)
EXTRA_CFLAGS += -DRTMP_FLASH_SUPPORT
else
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT
endif

#ifeq ($(CONFIG_STREAM_MODE_SUPPORT),y)
#EXTRA_CFLAGS += -DSTREAM_MODE_SUPPORT
#endif

#CONFIG_AP_SUPPORT
EXTRA_CFLAGS += -DCONFIG_AP_SUPPORT  \
				-DUAPSD_SUPPORT \
				-DMBSS_SUPPORT \
				-DIAPP_SUPPORT \
				-DDOT1X_SUPPORT \
				-DAP_SCAN_SUPPORT

ifeq ($(CONFIG_RTPCI_AP_ATE),y)
EXTRA_CFLAGS += -DRALINK_ATE \
				-DCONFIG_RT2880_ATE_CMD_NEW \
				-Idrivers/net/wireless/RTPCI_ap/ate/include

ifeq ($(CONFIG_RTPCI_AP_QA),y)
EXTRA_CFLAGS += -DRALINK_QA
endif
endif 

#ifdef WSC_INCLUDED
ifeq ($(CONFIG_RTPCI_AP_WSC),y)
EXTRA_CFLAGS += -DWSC_AP_SUPPORT

ifeq ($(CONFIG_RTPCI_AP_WSC_V2),y)
EXTRA_CFLAGS += -DWSC_V2_SUPPORT
endif

ifeq ($(CONFIG_RTPCI_AP_LED),y)
EXTRA_CFLAGS += -DWSC_LED_SUPPORT
endif
endif
#end of WSC_INCLUDED

ifeq ($(CONFIG_RTPCI_AP_WDS),y)
EXTRA_CFLAGS += -DWDS_SUPPORT
endif

ifeq ($(CONFIG_RTPCI_AP_APCLI),y)
EXTRA_CFLAGS += -DAPCLI_SUPPORT \
				-DMAT_SUPPORT \
				-DAP_SCAN_SUPPORT
endif

ifeq ($(CONFIG_RTPCI_AP_IGMP_SNOOP),y)
EXTRA_CFLAGS += -DIGMP_SNOOP_SUPPORT
endif

ifeq ($(CONFIG_RTPCI_AP_NETIF_BLOCK),y)
EXTRA_CFLAGS += -DBLOCK_NET_IF
endif

ifeq ($(CONFIG_RTPCI_AP_DFS),y)
EXTRA_CFLAGS += -DDFS_SUPPORT
endif

ifeq ($(CONFIG_RTPCI_AP_CARRIER),y)
EXTRA_CFLAGS += -DCARRIER_DETECTION_SUPPORT
endif

ifeq ($(CONFIG_RTPCI_AP_DLS),y)
EXTRA_CFLAGS += -DQOS_DLS_SUPPORT
endif

#DOT11_N_SUPPORT
EXTRA_CFLAGS += -DDOT11_N_SUPPORT

ifeq ($(CONFIG_RTPCI_80211N_DRAFT3),y)
EXTRA_CFLAGS += -DDOT11N_DRAFT3
endif

#ifeq ($(CONFIG_RTPCI_GREENAP),y)
#EXTRA_CFLAGS += -DGREENAP_SUPPORT
#endif
#end of DOT11_N_SUPPORT

ifeq ($(CONFIG_RTPCI_AP_LLTD),y)
EXTRA_CFLAGS += -DLLTD_SUPPORT
endif

#STATS_COUNT
EXTRA_CFLAGS += -DSTATS_COUNT_SUPPORT

ifeq ($(CONFIG_RTPCI_EXT_CHANNEL_LIST),y)
EXTRA_CFLAGS += -DEXT_BUILD_CHANNEL_LIST
endif

ifeq ($(CONFIG_RTPCI_AP_IDS),y)
EXTRA_CFLAGS += -DIDS_SUPPORT
endif

ifeq ($(CONFIG_RTPCI_AP_WAPI),y)
EXTRA_CFLAGS += -DWAPI_SUPPORT -DEXPORT_SYMTAB
endif

ifeq ($(CONFIG_RTPCI_AP_LED),y)
EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
endif

ifeq ($(CONFIG_RTPCI_MC_SUPPORT),y)
EXTRA_CFLAGS += -DMULTIPLE_CARD_SUPPORT
endif

#################################################
# ChipSet specific definitions.
#
ifeq ($(CONFIG_RT3090_AP_INCLUDE),y)
EXTRA_CFLAGS += -DRTMP_MAC_PCI \
				-DRT30xx \
				-DRT3090 \
				-DRTMP_PCI_SUPPORT \
				-DRTMP_RF_RW_SUPPORT \
				-DVCORECAL_SUPPORT
endif

ifeq ($(CONFIG_RT3593_AP_INCLUDE),y)
EXTRA_CFLAGS +=	-DRTMP_MAC_PCI \
				-DDOT11N_SS3_SUPPORT \
				-DRT3593 \
				-DRT28xx \
				-DRT30xx \
				-DRT35xx \
				-DRTMP_PCI_SUPPORT \
				-DRTMP_RF_RW_SUPPORT \
				-DA_BAND_SUPPORT \
				-DNEW_MBSSID_MODE \
				-DVCORECAL_SUPPORT \
				-DSPECIFIC_BCN_BUF_SUPPORT \
				-DNEW_RATE_ADAPT_SUPPORT

ifeq ($(CONFIG_RTPCI_AP_HIGH_POWER),y)
EXTRA_CFLAGS += -DHIGH_POWER_SUPPORT
endif

ifeq ($(CONFIG_RTPCI_AP_VIDEO_TURBINE),y)
EXTRA_CFLAGS += -DVIDEO_TURBINE_SUPPORT
endif

ifeq ($(CONFIG_RTPCI_TXBF_SUPPORT),y)
EXTRA_CFLAGS += -DTXBF_SUPPORT \
				-DSTREAM_MODE_SUPPORT \
				-DPRE_ANT_SWITCH \
				-DCFO_TRACK \
				-DRANGE_EXTEND
endif
endif 

ifeq ($(CONFIG_RT5392_AP_INCLUDE),y)
EXTRA_CFLAGS +=	-DRTMP_MAC_PCI \
				-DRT30xx \
				-DRT33xx \
				-DRT3090 \
				-DRT3390 \
				-DRT5390 \
				-DRTMP_PCI_SUPPORT \
				-DRTMP_RF_RW_SUPPORT \
				-DRTMP_FREQ_CALIBRATION_SUPPORT \
				-DRTMP_INTERNAL_TX_ALC \
				-DVCORECAL_SUPPORT \
				-DIQ_CAL_SUPPORT \
				-DNEW_MBSSID_MODE \
				-DRTMP_TEMPERATURE_COMPENSATION \
				-DSPECIFIC_BCN_BUF_SUPPORT
endif

ifeq ($(CONFIG_RT5592_AP_INCLUDE),y)
EXTRA_CFLAGS += -DRTMP_MAC_PCI \
				-DRTMP_PCI_SUPPORT \
				-DRT30xx \
				-DRT5592 \
				-DRTMP_RF_RW_SUPPORT \
				-DA_BAND_SUPPORT \
				-DIQ_CAL_SUPPORT \
				-DRX_DMA_SCATTER \
				-DVCORECAL_SUPPORT \
				-DNEW_MBSSID_MODE \
				-DRTMP_TEMPERATURE_COMPENSATION \
				-DSPECIFIC_BCN_BUF_SUPPORT

ifeq ($(CONFIG_RT5592_EP_SUPPORT),y)
EXTRA_CFLAGS += -DRT5592EP_SUPPORT
endif

#ifeq ($(CONFIG_TSO_SUPPORT), y)
#EXTRA_CFLAGS += -DCONFIG_TSO_SUPPORT
#endif
endif

#APsoc Specific
EXTRA_CFLAGS += -DCONFIG_RA_NAT_NONE -DLINUX -DRA_MTD_RW_BY_NUM
ifeq ($(CONFIG_RA_NETWORK_WORKQUEUE_BH),y)
EXTRA_CFLAGS   += -DWORKQUEUE_BH
endif

clean:
	@rm -f `find ./ -name "*.cmd"`
	@rm -f `find ./ -name "*.ko"`
	@rm -f `find ./ -name "*.o"`
	@rm -f *.c
