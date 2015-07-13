
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
#define AMBIT_HARDWARE_VERSION     "U12H17200"
#define AMBIT_SOFTWARE_VERSION     "V1.2.0.32"
#define AMBIT_UI_VERSION           "40.0.74"
#define STRING_TBL_VERSION         "1.2.0.30_2.1.22.1"

#define AMBIT_PRODUCT_NAME          "WNR3500Lv2"
#define AMBIT_PRODUCT_DESCRIPTION   "Netgear Wireless Router WNR3500Lv2"
#define UPnP_MODEL_URL              "WNR3500Lv2.aspx"
#define UPnP_MODEL_DESCRIPTION      "NETGEAR WNR3500Lv2 N300 Wireless Gigabit Router"
#define DOCUMENT_URL                "http://documentation.netgear.com/wnr3500lv2/enu/202-10832-01/index.htm"
#define DOCUMENT_URL_GR             "http://documentation.netgear.com/wnr3500lv2/deu/202-10833-01/index.htm"

#define NO_USB_SUPPORT              "N"

#define AMBIT_NVRAM_VERSION  "1" /* digital only */

#ifdef AMBIT_UPNP_SA_ENABLE
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
#define WDS_IF_NAME_NUM             "wds0.1"    /* WDS interface */

#ifdef MULTIPLE_SSID
#define WLAN_BSS1_NAME_NUM          "wl0.1"     /* Multiple BSSID #2 */
#define WLAN_BSS2_NAME_NUM          "wl0.2"     /* Multiple BSSID #3 */
#define WLAN_BSS3_NAME_NUM          "wl0.3"     /* Multiple BSSID #4 */
#endif /* MULTIPLE_SSID */

/* GPIO definitions */
#define GPIO_POWER_LED_GREEN        3
#define GPIO_POWER_LED_GREEN_STR    "3"
#define GPIO_POWER_LED_AMBER        7
#define GPIO_POWER_LED_AMBER_STR    "7"

#define GPIO_WAN_LED                2

#define LANG_TBL_MTD_RD             "/dev/mtdblock"
#define LANG_TBL_MTD_WR             "/dev/mtd"

#define ML_MTD_RD                   "/dev/mtdblock"
#define ML_MTD_WR                   "/dev/mtd"

/* MTD definitions */
#define BOOT_MTD_RD                 "/dev/mtdblock0"
#define BOOT_MTD_WR                 "/dev/mtd0"

#define KERNEL_MTD_RD               "/dev/mtdblock14"
#define KERNEL_MTD_WR               "/dev/mtd14"

#define ROOTFS_MTD_RD               "/dev/mtdblock15"
#define ROOTFS_MTD_WR               "/dev/mtd15"

#define LANG_TBL1_MTD_RD            "/dev/mtdblock7"
#define LANG_TBL1_MTD_WR            "/dev/mtd7"
#define LANG_TBL2_MTD_RD            "/dev/mtdblock8"
#define LANG_TBL2_MTD_WR            "/dev/mtd8"

#define ML1_MTD_RD                  "/dev/mtdblock7"
#define ML1_MTD_WR                  "/dev/mtd7"
#define ML2_MTD_RD                  "/dev/mtdblock8"
#define ML2_MTD_WR                  "/dev/mtd8"

#define ST_SUPPORT_NUM              (7)        /* The maxium value can be 2-10. */
#define LANG_TBL_MTD_START          (7)
#define LANG_TBL_MTD_END            (LANG_TBL_MTD_START + ST_SUPPORT_NUM - 1)
#define FLASH_MTD_ML_SIZE           (0x20000)
#define BUILTIN_LANGUAGE            "English"

#define ML3_MTD_RD                  "/dev/mtdblock9"
#define ML3_MTD_WR                  "/dev/mtd9"
#define ML4_MTD_RD                  "/dev/mtdblock10"
#define ML4_MTD_WR                  "/dev/mtd10"
#define ML5_MTD_RD                  "/dev/mtdblock11"
#define ML5_MTD_WR                  "/dev/mtd11"
#define ML6_MTD_RD                  "/dev/mtdblock12"
#define ML6_MTD_WR                  "/dev/mtd12"
#define ML7_MTD_RD                  "/dev/mtdblock13"
#define ML7_MTD_WR                  "/dev/mtd13"

#define TF1_MTD_RD                  "/dev/mtdblock5"
#define TF1_MTD_WR                  "/dev/mtd5"
#define TF2_MTD_RD                  "/dev/mtdblock6"
#define TF2_MTD_WR                  "/dev/mtd6"

#define POT_MTD_RD                  "/dev/mtdblock3"
#define POT_MTD_WR                  "/dev/mtd3"

#define POT2_MTD_RD                 "/dev/mtdblock4"
#define POT2_MTD_WR                 "/dev/mtd4"

#define BD_MTD_RD                   "/dev/mtdblock2"
#define BD_MTD_WR                   "/dev/mtd2"

#define NVRAM_MTD_RD                "/dev/mtdblock1"
#define NVRAM_MTD_WR                "/dev/mtd1"

#define BRCM_NVRAM          /* use broadcom nvram instead of ours */

#define BACKUP_FILE_KEY         "NtgrBak"

#endif /*_AMBITCFG_H*/
