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
#define AMBIT_HARDWARE_VERSION     "U12H19400"
#define AMBIT_SOFTWARE_VERSION     "V1.0.0.22"
#define AMBIT_UI_VERSION           "1.0.17"
#define STRING_TBL_VERSION         "1.0.0.22_2.1.17.1"

#define AMBIT_PRODUCT_NAME          "WNDR3700v3"
#define AMBIT_PRODUCT_DESCRIPTION   "Netgear Wireless Router WNDR3700v3"
#define UPnP_MODEL_URL              "WNDR3700.aspx"
#define UPnP_MODEL_DESCRIPTION      "NETGEAR WNDR3700v3 N600 Wireless Dual Band Gigabit Router"

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
#endif /* MULTIPLE_SSID */
/* Foxconn add end by aspen Bai, 11/13/2008 */

#define RATE_STR_2G_BG              "54Mbps"
#define RATE_STR_2G_HT20            "145Mbps"
#define RATE_STR_2G_HT40            "300Mbps"
#define RATE_STR_5G_BG              "54Mbps"
#define RATE_STR_5G_HT20            "145Mbps"
#define RATE_STR_5G_HT40            "300Mbps"

#define OPMODE_STR_2G_BG            "wlan_mode_54"
#define OPMODE_STR_2G_HT20          "wlan_mode_145"
#define OPMODE_STR_2G_HT40          "wlan_mode_300"
#define OPMODE_STR_5G_BG            "wlan_mode_54"
#define OPMODE_STR_5G_HT20          "wlan_mode_145"
#define OPMODE_STR_5G_HT40          "wlan_mode_300"

/* GPIO definitions */
/* Ctrl mode mask, bit[11:8] */
#define GPIO_CTRL_MODE_NONE             (0x0000)        /* Direct mode */
#define GPIO_CTRL_MODE_CLK_DATA         (0x0100)        /* Clk/Data mode */
#define GPIO_CTRL_MODE(x)               ((x) & 0x0F00)

/* Active mode mask, bit[12] */
#define GPIO_ACTIVE_MODE_LOW            (0x0000)
#define GPIO_ACTIVE_MODE_HIGH           (0x1000)
#define GPIO_ACTIVE_MODE(x)             ((x) & 0x1000)

/* support max 32 gpios or max 32 extended LED pins */
#define GPIO_MAX_PIN                    (32 - 1)        /* 32 bit */
#define GPIO_PIN(x)                     ((x) & 0x00FF)

/* Direct ctrl mode */
#define GPIO_USB                        (1)
#define GPIO_WIFI                       (2)
#define GPIO_RESET                      (3)
#define GPIO_WPS                        (4)
#define GPIO_ROBO_RESET                 (5)

/* Clk/Data extended ctrl mode */
#define GPIO_EXT_CTRL_DATA              (6)
#define GPIO_EXT_CTRL_CLK               (7)

/* Used for direct output control */
#define EXT_LED_STATUS_ALL_ON           (~0U)
#define EXT_LED_STATUS_ALL_OFF          (0U)

/* Extended LED max shift times */
#define EXT_LED_MAX_SHIFTS              (8 - 1)         /* 8 extended pins */

/* Extended LED shift defines, not gpio pins, all active low */
#define GPIO_LED_PWR_GREEN              (0 | GPIO_CTRL_MODE_CLK_DATA)
#define GPIO_LED_PWR_AMBER              (1 | GPIO_CTRL_MODE_CLK_DATA)
#define GPIO_LED_WAN                    (2 | GPIO_CTRL_MODE_CLK_DATA)
#define GPIO_LED_WLAN_2G                (3 | GPIO_CTRL_MODE_CLK_DATA)
#define GPIO_LED_WLAN_5G                (4 | GPIO_CTRL_MODE_CLK_DATA)
#define GPIO_LED_USB                    (5 | GPIO_CTRL_MODE_CLK_DATA)
#define GPIO_LED_WPS                    (6 | GPIO_CTRL_MODE_CLK_DATA)
#define GPIO_LED_WLAN                   (7 | GPIO_CTRL_MODE_CLK_DATA)

#define GPIO_POWER_LED_GREEN        (GPIO_LED_PWR_GREEN)
#define GPIO_POWER_LED_GREEN_STR    "0x100"
#define GPIO_POWER_LED_AMBER        (GPIO_LED_PWR_AMBER)
#define GPIO_POWER_LED_AMBER_STR    "0x101"

#define GPIO_WAN_LED                (GPIO_LED_WAN)

#define LANG_TBL_MTD_RD             "/dev/mtdblock"
#define LANG_TBL_MTD_WR             "/dev/mtd"

#define ML_MTD_RD                   "/dev/mtdblock"
#define ML_MTD_WR                   "/dev/mtd"

/* MTD definitions */
#define ML1_MTD_RD                  "/dev/mtdblock3"
#define ML1_MTD_WR                  "/dev/mtd3"
#define ML2_MTD_RD                  "/dev/mtdblock4"
#define ML2_MTD_WR                  "/dev/mtd4"

#if defined(X_ST_ML)
#define ST_SUPPORT_NUM              (7)        /* The maxium value can be 2-10. */
#define LANG_TBL_MTD_START          (3)
#define LANG_TBL_MTD_END            (LANG_TBL_MTD_START + ST_SUPPORT_NUM - 1)
#define FLASH_MTD_ML_SIZE           0x10000
#define BUILTIN_LANGUAGE            "English"

#define ML3_MTD_RD                  "/dev/mtdblock5"
#define ML3_MTD_WR                  "/dev/mtd5"
#define ML4_MTD_RD                  "/dev/mtdblock6"
#define ML4_MTD_WR                  "/dev/mtd6"
#define ML5_MTD_RD                  "/dev/mtdblock7"
#define ML5_MTD_WR                  "/dev/mtd7"
#define ML6_MTD_RD                  "/dev/mtdblock8"
#define ML6_MTD_WR                  "/dev/mtd8"
#define ML7_MTD_RD                  "/dev/mtdblock9"
#define ML7_MTD_WR                  "/dev/mtd9"

#define TF1_MTD_RD                  "/dev/mtdblock10"
#define TF1_MTD_WR                  "/dev/mtd10"
#define TF2_MTD_RD                  "/dev/mtdblock11"
#define TF2_MTD_WR                  "/dev/mtd11"

#define POT_MTD_RD                  "/dev/mtdblock12"
#define POT_MTD_WR                  "/dev/mtd12"

#define BD_MTD_RD                   "/dev/mtdblock13"
#define BD_MTD_WR                   "/dev/mtd13"

#define NVRAM_MTD_RD                "/dev/mtdblock14"
#define NVRAM_MTD_WR                "/dev/mtd14"
#else
#define TF1_MTD_RD                  "/dev/mtdblock5"
#define TF1_MTD_WR                  "/dev/mtd5"
#define TF2_MTD_RD                  "/dev/mtdblock6"
#define TF2_MTD_WR                  "/dev/mtd6"

#define POT_MTD_RD                  "/dev/mtdblock7"
#define POT_MTD_WR                  "/dev/mtd7"

#define BD_MTD_RD                   "/dev/mtdblock8"
#define BD_MTD_WR                   "/dev/mtd8"

#define NVRAM_MTD_RD                "/dev/mtdblock9"
#define NVRAM_MTD_WR                "/dev/mtd9"
#endif


/* The following definition is used in acosNvramConfig.c and acosNvramConfig.h
 * to distingiush between Foxconn's and Broadcom's implementation.
 */
#define BRCM_NVRAM          /* use broadcom nvram instead of ours */

/* The following definition is to used as the key when doing des
 * encryption/decryption of backup file.
 * Have to be 7 octects.
 */
#define BACKUP_FILE_KEY         "NtgrBak"

/* Foxconn Perry added start, 2011/04/13, for document URL */
#define DOCUMENT_URL        "http://documentation.netgear.com/wndr3700/enu/202-10845-01/index.htm"
/* Foxconn Perry added end, 2011/04/13, for document URL */

/* Foxconn Perry added start, 2011/08/17, for USB Support level */
#define USB_support_level        "1"    /* pling changed 0->1 for WNDR4000 */
/* Foxconn Perry added end, 2011/08/17, for USB Support level */

#endif /*_AMBITCFG_H*/
