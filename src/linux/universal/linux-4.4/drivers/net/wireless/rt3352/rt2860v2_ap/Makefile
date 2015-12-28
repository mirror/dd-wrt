EXTRA_CFLAGS = -Idrivers/net/wireless/rt3352/rt2860common/include

obj-$(CONFIG_RT2860V2_AP) += rt2860v2_ap.o

rt2860v2_ap-objs += ../rt2860common/common/crypt_md5.o
rt2860v2_ap-objs += ../rt2860common/common/crypt_sha2.o
rt2860v2_ap-objs += ../rt2860common/common/crypt_hmac.o
rt2860v2_ap-objs += ../rt2860common/common/crypt_aes.o
rt2860v2_ap-objs += ../rt2860common/common/crypt_arc4.o
rt2860v2_ap-objs += ../rt2860common/common/mlme.o
rt2860v2_ap-objs += ../rt2860common/common/cmm_wep.o
rt2860v2_ap-objs += ../rt2860common/common/action.o
rt2860v2_ap-objs += ../rt2860common/common/ba_action.o
rt2860v2_ap-objs += ../rt2860common/common/cmm_data.o
rt2860v2_ap-objs += ../rt2860common/common/rtmp_init.o
rt2860v2_ap-objs += ../rt2860common/common/rtmp_init_inf.o
rt2860v2_ap-objs += ../rt2860common/common/cmm_tkip.o
rt2860v2_ap-objs += ../rt2860common/common/cmm_aes.o
rt2860v2_ap-objs += ../rt2860common/common/cmm_sync.o
rt2860v2_ap-objs += ../rt2860common/common/cmm_sanity.o
rt2860v2_ap-objs += ../rt2860common/common/cmm_info.o
rt2860v2_ap-objs += ../rt2860common/common/cmm_cfg.o
rt2860v2_ap-objs += ../rt2860common/common/cmm_wpa.o
rt2860v2_ap-objs += ../rt2860common/common/dfs.o
rt2860v2_ap-objs += ../rt2860common/common/dfs_mcu.o
rt2860v2_ap-objs += ../rt2860common/common/spectrum.o

rt2860v2_ap-objs += ../rt2860common/common/rtmp_timer.o
rt2860v2_ap-objs += ../rt2860common/common/rt_channel.o
rt2860v2_ap-objs += ../rt2860common/common/cmm_profile.o
rt2860v2_ap-objs += ../rt2860common/common/cmm_asic.o
rt2860v2_ap-objs += ../rt2860common/common/cmm_cmd.o
rt2860v2_ap-objs += ../rt2860common/common/rtmp_swmcu.o
rt2860v2_ap-objs += ../rt2860common/common/rt_os_util.o

rt2860v2_ap-objs += ../rt2860common/common/eeprom.o
rt2860v2_ap-objs += ../rt2860common/common/ee_flash.o

rt2860v2_ap-objs += ../rt2860common/common/cmm_mac_pci.o
rt2860v2_ap-objs += ../rt2860common/common/cmm_data_pci.o

rt2860v2_ap-objs += ../rt2860common/chips/rtmp_chip.o

rt2860v2_ap-objs += ../rt2860common/ap/ap.o
rt2860v2_ap-objs += ../rt2860common/ap/ap_assoc.o
rt2860v2_ap-objs += ../rt2860common/ap/ap_auth.o
rt2860v2_ap-objs += ../rt2860common/ap/ap_connect.o
rt2860v2_ap-objs += ../rt2860common/ap/ap_mlme.o
rt2860v2_ap-objs += ../rt2860common/ap/ap_sanity.o
rt2860v2_ap-objs += ../rt2860common/ap/ap_sync.o
rt2860v2_ap-objs += ../rt2860common/ap/ap_wpa.o
rt2860v2_ap-objs += ../rt2860common/ap/ap_data.o
rt2860v2_ap-objs += ../rt2860common/ap/ap_uapsd.o
rt2860v2_ap-objs += ../rt2860common/ap/ap_autoChSel.o
rt2860v2_ap-objs += ../rt2860common/ap/ap_qload.o
rt2860v2_ap-objs += ../rt2860common/ap/ap_cfg.o

#rt2860v2_ap-objs += ../rt2860common/os/linux/rt_proc.o
rt2860v2_ap-objs += ../rt2860common/os/linux/rt_linux.o
rt2860v2_ap-objs += ../rt2860common/os/linux/rt_profile.o
rt2860v2_ap-objs += ../rt2860common/os/linux/rt_main_dev.o
rt2860v2_ap-objs += ../rt2860common/os/linux/ap_ioctl.o
rt2860v2_ap-objs += ../rt2860common/os/linux/rt_pci_rbus.o
rt2860v2_ap-objs += ../rt2860common/os/linux/rt_rbus_pci_util.o
rt2860v2_ap-objs += ../rt2860common/os/linux/rt_rbus_pci_drv.o
rt2860v2_ap-objs += ../rt2860common/os/linux/rbus_main_dev.o
#rt2860v2_ap-objs += ../rt2860common/common/rt_ate.o

ifeq ($(CONFIG_RT2860V2_AP_WMM_ACM),y)
rt2860v2_ap-objs += ../rt2860common/common/acm_edca.o
rt2860v2_ap-objs += ../rt2860common/common/acm_comm.o
rt2860v2_ap-objs += ../rt2860common/common/acm_iocl.o
endif

ifeq ($(CONFIG_RT2860V2_AP_LED),y)
rt2860v2_ap-objs += ../rt2860common/common/rt_led.o
endif

ifeq ($(CONFIG_RT2860V2_AP_WSC),y)
rt2860v2_ap-objs += ../rt2860common/common/wsc.o 
rt2860v2_ap-objs += ../rt2860common/common/wsc_tlv.o
rt2860v2_ap-objs += ../rt2860common/common/crypt_dh.o
rt2860v2_ap-objs += ../rt2860common/common/crypt_biginteger.o
rt2860v2_ap-objs += ../rt2860common/common/wsc_ufd.o
endif

ifeq ($(CONFIG_RT2860V2_AP_WSC_V2),y)
rt2860v2_ap-objs += ../rt2860common/common/wsc_v2.o
endif

ifeq ($(CONFIG_RT2860V2_AP_WDS),y)
rt2860v2_ap-objs += ../rt2860common/ap/ap_wds.o
rt2860v2_ap-objs += ../rt2860common/ap/ap_wds_inf.o
rt2860v2_ap-objs += ../rt2860common/common/client_wds.o
endif

ifeq ($(CONFIG_RT2860V2_AP_MBSS),y)
rt2860v2_ap-objs += ../rt2860common/ap/ap_mbss.o
rt2860v2_ap-objs += ../rt2860common/ap/ap_mbss_inf.o
endif

ifeq ($(CONFIG_RT2860V2_AP_APCLI),y)
rt2860v2_ap-objs += ../rt2860common/ap/ap_apcli.o
rt2860v2_ap-objs += ../rt2860common/ap/ap_apcli_inf.o
rt2860v2_ap-objs += ../rt2860common/ap/apcli_assoc.o
rt2860v2_ap-objs += ../rt2860common/ap/apcli_auth.o
rt2860v2_ap-objs += ../rt2860common/ap/apcli_ctrl.o
rt2860v2_ap-objs += ../rt2860common/ap/apcli_sync.o
rt2860v2_ap-objs += ../rt2860common/common/cmm_mat.o
rt2860v2_ap-objs += ../rt2860common/common/cmm_mat_iparp.o
rt2860v2_ap-objs += ../rt2860common/common/cmm_mat_pppoe.o
rt2860v2_ap-objs += ../rt2860common/common/cmm_mat_ipv6.o
endif

#ifeq ($(CONFIG_RT2860V2_AP_WAPI),y)
#rt2860v2_ap-objs += ../rt2860common/common/wapi.o
#rt2860v2_ap-objs += wapi_crypt.obj
#rt2860v2_ap-objs += wapi_sms4.obj
#endif

ifeq ($(CONFIG_RT2860V2_AP_IGMP_SNOOP),y)
rt2860v2_ap-objs += ../rt2860common/common/igmp_snoop.o
endif

ifeq ($(CONFIG_RT2860V2_AP_NETIF_BLOCK),y)
rt2860v2_ap-objs += ../rt2860common/common/netif_block.o
endif

ifeq ($(CONFIG_RT2860V2_AP_DLS),y)
rt2860v2_ap-objs += ../rt2860common/ap/ap_dls.o
endif

ifeq ($(CONFIG_RT2860V2_AP_IDS),y)
rt2860v2_ap-objs += ../rt2860common/ap/ap_ids.o
endif

#ifeq ($(CONFIG_RT2860V2_RT3XXX_AP_ANTENNA_DIVERSITY),y)
#rt2860v2_ap-objs += ../rt2860common/os/linux/ap_diversity.o
#endif

#ifeq ($(CONFIG_RT2860V2_80211R_FT),y)
#rt2860v2_ap-objs += ../rt2860common/common/ft.o
#rt2860v2_ap-objs += ../rt2860common/common/ft_tlv.o
#rt2860v2_ap-objs += ../rt2860common/common/ft_ioctl.o
#rt2860v2_ap-objs += ../rt2860common/common/ft_rc.o
#rt2860v2_ap-objs += ../rt2860common/ap/ap_ftkd.o
#endif

#ifeq ($(CONFIG_RT2860V2_80211K_RR),y)
#rt2860v2_ap-objs += ../rt2860common/common/rrm_tlv.o
#rt2860v2_ap-objs += ../rt2860common/common/rrm_sanity.o
#rt2860v2_ap-objs += ../rt2860common/common/rrm.o
#endif

ifeq ($(CONFIG_RT2860V2_AP_VIDEO_TURBINE),y)
rt2860v2_ap-objs += ../rt2860common/common/cmm_video.o
endif

ifeq ($(CONFIG_RALINK_RT2880),y)
rt2860v2_ap-objs +=  ../rt2860common/chips/rt2880.o
endif

ifeq ($(CONFIG_RALINK_RT3052),y)
rt2860v2_ap-objs +=  ../rt2860common/common/rt_rf.o
rt2860v2_ap-objs +=  ../rt2860common/chips/rt305x.o
endif

ifeq ($(CONFIG_RALINK_RT3352),y)
rt2860v2_ap-objs +=  ../rt2860common/common/rt_rf.o
rt2860v2_ap-objs +=  ../rt2860common/chips/rt305x.o
rt2860v2_ap-objs +=  ../rt2860common/chips/rt3352.o
endif

ifeq ($(CONFIG_RT3x52),y)
rt2860v2_ap-objs +=  ../rt2860common/common/rt_rf.o
rt2860v2_ap-objs +=  ../rt2860common/chips/rt305x.o
rt2860v2_ap-objs +=  ../rt2860common/chips/rt3352.o
endif

ifeq ($(CONFIG_RALINK_RT5350),y)
rt2860v2_ap-objs +=  ../rt2860common/common/rt_rf.o
rt2860v2_ap-objs +=  ../rt2860common/chips/rt305x.o
rt2860v2_ap-objs +=  ../rt2860common/chips/rt5350.o
endif

###################
#  CFLAGS
##################
EXTRA_CFLAGS += -DAGGREGATION_SUPPORT -DPIGGYBACK_SUPPORT -DWMM_SUPPORT  -DLINUX \
               -Wall -Wstrict-prototypes -Wno-trigraphs

EXTRA_CFLAGS += -DCONFIG_AP_SUPPORT -DAP_SCAN_SUPPORT -DUAPSD_AP_SUPPORT -DRTMP_RBUS_SUPPORT -DRTMP_MAC_PCI
EXTRA_CFLAGS += -DDOT11_N_SUPPORT -DSTATS_COUNT_SUPPORT -DRELASE_EXCLUDE -DIAPP_SUPPORT -DDOT1X_SUPPORT
EXTRA_CFLAGS += -DNEW_TXCONT -DNEW_TXCARRSUPP
EXTRA_CFLAGS += -DCONFIG_RA_NAT_NONE
EXTRA_CFLAGS += -DRTMP_FLASH_SUPPORT -DCONFIG_SWMCU_SUPPORT
EXTRA_CFLAGS += -DRA_MTD_RW_BY_NUM
#EXTRA_CFLAGS += -DBW_FALLBACK_SUPPORT

#provide busy time statistics for every TBTT */
#EXTRA_CFLAGS += -DQLOAD_FUNC_BUSY_TIME_STATS 

# provide busy time alarm mechanism 
# use the function to avoid to locate in some noise environments 
#EXTRA_CFLAGS += -DQLOAD_FUNC_BUSY_TIME_ALARM

ifeq ($(CONFIG_RALINK_RT2880),y)
EXTRA_CFLAGS += -DRT2880
endif

ifeq ($(CONFIG_RALINK_RT3052),y)
EXTRA_CFLAGS += -DSPECIFIC_VCORECAL_SUPPORT
ifeq ($(CONFIG_RALINK_RT3350),y)
EXTRA_CFLAGS += -DRT3350 -DRT305x -DRTMP_RF_RW_SUPPORT
else
EXTRA_CFLAGS += -DRT3052 -DRT305x -DRTMP_RF_RW_SUPPORT
endif
endif

ifeq ($(CONFIG_RALINK_RT3352),y)
EXTRA_CFLAGS += -DRT3352 -DRT305x -DRTMP_RF_RW_SUPPORT -DSPECIFIC_BCN_BUF_SUPPORT -DVCORECAL_SUPPORT
endif

ifeq ($(CONFIG_RT3x52),y)
EXTRA_CFLAGS += -DRT3052 -DRT3352 -DRT305x -DRTMP_RF_RW_SUPPORT -DSPECIFIC_BCN_BUF_SUPPORT -DVCORECAL_SUPPORT
endif

ifeq ($(CONFIG_RALINK_RT5350),y)
EXTRA_CFLAGS += -DRT5350 -DRT305x -DRTMP_RF_RW_SUPPORT -DSPECIFIC_BCN_BUF_SUPPORT -DVCORECAL_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_AP_DLS),y)
EXTRA_CFLAGS += -DQOS_DLS_SUPPORT
EXTRA_CFLAGS += -DAP_DLS_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_AP_IDS),y)
EXTRA_CFLAGS += -DIDS_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_AP_DFS),y)
EXTRA_CFLAGS += -DDFS_SUPPORT
EXTRA_CFLAGS += -DNEW_DFS
endif

ifeq ($(CONFIG_RT2860V2_AP_CARRIER),y)
EXTRA_CFLAGS += -DCARRIER_DETECTION_SUPPORT -DTONE_RADAR_DETECT_SUPPORT -DTONE_RADAR_DETECT_V2
ifeq ($(CONFIG_RALINK_RT3052),y)
EXTRA_CFLAGS += -DTONE_RADAR_DETECT_V1
endif
endif

ifeq ($(CONFIG_RT2860V2_AUTO_CH_SELECT_ENCANCE),y)
EXTRA_CFLAGS += -DAUTO_CH_SELECT_ENHANCE
endif

ifeq ($(CONFIG_RT2860V2_AP_80211N_DRAFT3),y)
EXTRA_CFLAGS += -DDOT11N_DRAFT3
endif

ifeq ($(CONFIG_SINGLE_SKU),y)
EXTRA_CFLAGS += -DSINGLE_SKU
endif

ifeq ($(CONFIG_RT2860V2_SNMP),y)
EXTRA_CFLAGS += -DSNMP_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_MCAST_RATE_SPECIFIC),y)
EXTRA_CFLAGS += -DMCAST_RATE_SPECIFIC
endif

ifeq ($(CONFIG_RT2860V2_AP_WMM_ACM),y)
EXTRA_CFLAGS += -DWMM_ACM_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_AP_LED),y)
EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT -DCONFIG_SWMCU_SUPPORT
ifeq ($(CONFIG_RT2860V2_AP_WSC),y)
EXTRA_CFLAGS += -DWSC_LED_SUPPORT
endif
endif

ifeq ($(CONFIG_RT2860V2_AP_WSC),y)
EXTRA_CFLAGS += -DWSC_AP_SUPPORT
ifeq ($(CONFIG_RT2860V2_AP_WSC_V2),y)
EXTRA_CFLAGS += -DWSC_V2_SUPPORT
endif
endif

ifeq ($(CONFIG_RT2860V2_AP_LLTD),y)
EXTRA_CFLAGS += -DLLTD_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_AP_WDS),y)
EXTRA_CFLAGS += -DWDS_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_AP_MBSS),y)
EXTRA_CFLAGS += -DMBSS_SUPPORT
endif

ifeq ($(CONFIG_NEW_MBSSID_MODE),y)
EXTRA_CFLAGS += -DNEW_MBSSID_MODE
endif

ifeq ($(CONFIG_RT2860V2_AP_APCLI),y)
EXTRA_CFLAGS += -DAPCLI_SUPPORT
EXTRA_CFLAGS += -DMAT_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_AP_IGMP_SNOOP),y)
EXTRA_CFLAGS += -DIGMP_SNOOP_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_AP_NETIF_BLOCK),y)
EXTRA_CFLAGS += -DBLOCK_NET_IF
endif

#ifeq ($(CONFIG_RT2860V2_RT3XXX_AP_ANTENNA_DIVERSITY),y)
#EXTRA_CFLAGS   += -DRT3XXX_ANTENNA_DIVERSITY_SUPPORT
#endif

#ifeq ($(CONFIG_RT2860V2_AP_WAPI),y)
#EXTRA_CFLAGS   += -DWAPI_SUPPORT
#ifeq ($(CONFIG_RALINK_RT3052),y)
#EXTRA_CFLAGS += -DSOFT_ENCRYPT
#endif
#endif

ifeq ($(CONFIG_RT2860V2_GREEN_AP),y)
EXTRA_CFLAGS   += -DGREENAP_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_EXT_CHANNEL_LIST),y)
EXTRA_CFLAGS   += -DEXT_BUILD_CHANNEL_LIST
endif

ifeq ($(CONFIG_KTHREAD),y)
EXTRA_CFLAGS   += -DKTHREAD_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_AP_MEMORY_OPTIMIZATION),y)
EXTRA_CFLAGS   += -DMEMORY_OPTIMIZATION
else
#EXTRA_CFLAGS   += -DDBG
endif

ifeq ($(CONFIG_RT2860V2_AP_VIDEO_TURBINE),y)
EXTRA_CFLAGS   += -DVIDEO_TURBINE_SUPPORT
endif

ifeq ($(CONFIG_RA_NETWORK_WORKQUEUE_BH),y)
EXTRA_CFLAGS   += -DWORKQUEUE_BH
endif

ifeq ($(CONFIG_RT2860V2_AP_RTMP_INTERNAL_TX_ALC),y)
EXTRA_CFLAGS   += -DRTMP_INTERNAL_TX_ALC
endif

