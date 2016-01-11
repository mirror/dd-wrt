/***************************************************************************
#***
#***    Copyright 2005  Hon Hai Precision Ind. Co. Ltd.
#***    All Rights Reserved.
#***    No portions of this material shall be reproduced in any form without the
#***    written permission of Hon Hai Precision Ind. Co. Ltd.
#***
#***    All information contained in this document is Hon Hai Precision Ind.  
#***    Co. Ltd. company private, proprietary, and trade secret property and 
#***    are protected by international intellectual property laws and treaties.
#***
#****************************************************************************
***
***    Filename: ambitCfg.h
***
***    Description:
***         This file is specific to each project. Every project should have a
***    different copy of this file.
***        Included from ambit.h which is shared by every project.
***
***    History:
***
***	   Modify Reason		                                        Author		         Date		                Search Flag(Option)
***	--------------------------------------------------------------------------------------
***    File Creation                                            Jasmine Yang    11/02/2005
*******************************************************************************/


#ifndef _AMBITCFG_H
#define _AMBITCFG_H

#define WW_VERSION           1 /* WW SKUs */
#define NA_VERSION           2 /* NA SKUs */
#define JP_VERSION           3
#define GR_VERSION           4
#define PR_VERSION           5
#define KO_VERSION           6
#define RU_VERSION           7
#define SS_VERSION           8
#define PT_VERSION           9
#define TWC_VERSION          10
#define BRIC_VERSION         11
#define SK_VERSION           12

#define WLAN_REGION          WW_VERSION
#define FW_REGION            WW_VERSION   /* true f/w region */

/*formal version control*/
#define AMBIT_HARDWARE_VERSION     "U12H315T00"
#define AMBIT_SOFTWARE_VERSION     "V1.0.2.99"
#define AMBIT_UI_VERSION           "1.1.99"
#define STRING_TBL_VERSION         "1.0.2.99_2.1.40.1"

#define AMBIT_PRODUCT_NAME          "R8000"
#define AMBIT_PRODUCT_ALIAS         "Nighthawk X6"
#define AMBIT_PRODUCT_DESCRIPTION   "802.11ac Dual Band Gigabit Wireless Router R8000"
#define UPnP_MODEL_URL              "R8000.aspx"
#define UPnP_MODEL_DESCRIPTION      "802.11ac"

#define AMBIT_NVRAM_VERSION  "1" /* digital only */

#ifdef AMBIT_UPNP_SA_ENABLE /* Jasmine Add, 10/24/2006 */
#define SMART_WIZARD_SPEC_VERSION "0.7"  /* This is specification version of smartwizard 2.0 */
#endif
/****************************************************************************
 * Board-specific defintions
 *
 ****************************************************************************/

/* Interface definitions */
#define WAN_IF_NAME_NUM             "eth0"
#define LAN_IF_NAME_NUM             "vlan1"
#define WLAN_IF_NAME_NUM            "eth1"
#define WLAN_N_IF_NAME_NUM          "eth2"
#if defined(R8000)
#define WLAN_5G_2_IF_NAME_NUM          "eth3"
#endif
#define WDS_IF_NAME_NUM             "wds0.1"    /* WDS interface */

/* Foxconn add start by aspen Bai, 11/13/2008 */
#ifdef MULTIPLE_SSID
#define WLAN_BSS1_NAME_NUM          "wl0.1"     /* Multiple BSSID #2 */
#define WLAN_BSS2_NAME_NUM          "wl0.2"     /* Multiple BSSID #3 */
#define WLAN_BSS3_NAME_NUM          "wl0.3"     /* Multiple BSSID #4 */

/* Foxconn add start, Tony W.Y. Wang, 03/22/2010 @For 5G*/
#define WLAN_5G_BSS1_NAME_NUM       "wl1.1"     /* Multiple BSSID #2 */
#define WLAN_5G_BSS2_NAME_NUM       "wl1.2"     /* Multiple BSSID #3 */
#define WLAN_5G_BSS3_NAME_NUM       "wl1.3"     /* Multiple BSSID #4 */

/* Foxconn add end, Tony W.Y. Wang, 03/22/2010 @For 5G*/
#define WLAN_5G_2_BSS1_NAME_NUM       "wl2.1"     /* Multiple BSSID #2 */
#define WLAN_5G_2_BSS2_NAME_NUM       "wl2.2"     /* Multiple BSSID #3 */
#define WLAN_5G_2_BSS3_NAME_NUM       "wl2.3"     /* Multiple BSSID #4 */
#endif /* MULTIPLE_SSID */
/* Foxconn add end by aspen Bai, 11/13/2008 */

/* GPIO definitions */
/* Foxconn modified start, Wins, 04/11/2011 */

#define GPIO_POWER_LED_GREEN        2
#define GPIO_POWER_LED_GREEN_STR    "2"
#define GPIO_POWER_LED_AMBER        3
#define GPIO_POWER_LED_AMBER_STR    "3"

#define GPIO_LOGO_LED_1             1
#define GPIO_LOGO_LED_1_STR         "1"
#define GPIO_LOGO_LED_2             9
#define GPIO_LOGO_LED_2_STR         "9"

#define GPIO_WAN_LED                8
#define GPIO_WAN_LED_2              9


#define GPIO_WIFI_2G_LED            13
#define GPIO_WIFI_5G_LED            12
#define GPIO_WIFI_5G_2_LED          16
#define GPIO_WIFI_SUMMARY_LED       14   // from 15 to 14 by ken
 
#define GPIO_LED_SWITCH             19

#define LANG_TBL_MTD_RD             "/dev/mtdblock"
#define LANG_TBL_MTD_WR             "/dev/mtd"

#define ML_MTD_RD                   "/dev/mtdblock"
#define ML_MTD_WR                   "/dev/mtd"
/* MTD definitions */
#define ML1_MTD_RD                  "/dev/mtdblock9"
#define ML1_MTD_WR                  "/dev/mtd9"
#define ML2_MTD_RD                  "/dev/mtdblock10"
#define ML2_MTD_WR                  "/dev/mtd10"

#if defined(X_ST_ML)
#define ST_SUPPORT_NUM              (9)        /* The maxium value can be 2-10. */
#define LANG_TBL_MTD_START          (9)
#define LANG_TBL_MTD_END            (LANG_TBL_MTD_START + ST_SUPPORT_NUM - 1)
#define FLASH_MTD_ML_SIZE           0x10000
#define BUILTIN_LANGUAGE            "English"

#define BOOT_MTD_RD                  "/dev/mtdblock0"
#define BOOT_MTD_WR                  "/dev/mtd0"


#define ML3_MTD_RD                  "/dev/mtdblock11"
#define ML3_MTD_WR                  "/dev/mtd11"
#define ML4_MTD_RD                  "/dev/mtdblock12"
#define ML4_MTD_WR                  "/dev/mtd12"
#define ML5_MTD_RD                  "/dev/mtdblock13"
#define ML5_MTD_WR                  "/dev/mtd13"
#define ML6_MTD_RD                  "/dev/mtdblock14"
#define ML6_MTD_WR                  "/dev/mtd14"
#define ML7_MTD_RD                  "/dev/mtdblock15"
#define ML7_MTD_WR                  "/dev/mtd15"

#define TF1_MTD_RD                  "/dev/mtdblock7"
#define TF1_MTD_WR                  "/dev/mtd7"
#define TF2_MTD_RD                  "/dev/mtdblock8"
#define TF2_MTD_WR                  "/dev/mtd8"

#define POT_MTD_RD                  "/dev/mtdblock5"
#define POT_MTD_WR                  "/dev/mtd5"

#define BD_MTD_RD                   "/dev/mtdblock4"
#define BD_MTD_WR                   "/dev/mtd4"

#define NVRAM_MTD_RD                "/dev/mtdblock1"
#define NVRAM_MTD_WR                "/dev/mtd1"
#endif

#define KERNEL_MTD_RD               "/dev/mtdblock2"
#define KERNEL_MTD_WR               "/dev/mtd2"

#define ROOTFS_MTD_RD               "/dev/mtdblock3"
#define ROOTFS_MTD_WR               "/dev/mtd3"

#define LANG_TBL1_MTD_RD            "/dev/mtdblock9"
#define LANG_TBL1_MTD_WR            "/dev/mtd9"
#define LANG_TBL2_MTD_RD            "/dev/mtdblock10"
#define LANG_TBL2_MTD_WR            "/dev/mtd10"

#define POT2_MTD_RD                 "/dev/mtdblock6"
#define POT2_MTD_WR                 "/dev/mtd6"

#define QOS_MTD_RD                 	"/dev/mtdblock16"
#define QOS_MTD_WR                 	"/dev/mtd16"


/* wklin added start, 11/22/2006 */
/* The following definition is used in acosNvramConfig.c and acosNvramConfig.h
 * to distingiush between Foxconn's and Broadcom's implementation.
 */
#define BRCM_NVRAM          /* use broadcom nvram instead of ours */

/* The following definition is to used as the key when doing des
 * encryption/decryption of backup file.
 * Have to be 7 octects.
 */
#define BACKUP_FILE_KEY         "NtgrBak"
/* wklin added end, 11/22/2006 */

/* Foxconn Perry added start, 2011/04/13, for document URL */
#define DOCUMENT_URL		"http://documentation.netgear.com/wndr4500/enu/202-10581-01/index.htm"
/* Foxconn Perry added end, 2011/04/13, for document URL */

/* Foxconn Perry added start, 2011/08/17, for USB Support level */
#define USB_support_level        "13"       /* pling modified 5->13, add bit 4 for Readyshare Vault */
/* Foxconn Perry added end, 2011/08/17, for USB Support level */

#endif /*_AMBITCFG_H*/
