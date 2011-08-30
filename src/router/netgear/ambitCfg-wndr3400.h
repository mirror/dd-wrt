

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

#define WLAN_REGION          WW_VERSION
#define FW_REGION            WW_VERSION   /* true f/w region */

/*formal version control*/
#define AMBIT_HARDWARE_VERSION     "U12H15500"
#define AMBIT_SOFTWARE_VERSION     "V1.0.0.38"
#define AMBIT_UI_VERSION           "16.0.48"
#define STRING_TBL_VERSION         "1.0.0.38_2.1.9.1"

#define AMBIT_PRODUCT_NAME          "WNDR3400"
#define AMBIT_PRODUCT_DESCRIPTION   "Netgear Wireless Router WNDR3400"
#define UPnP_MODEL_URL              "WNDR3400.aspx"
#define UPnP_MODEL_DESCRIPTION      "N600"

#define AMBIT_NVRAM_VERSION  "1" /* digital only */

#ifdef AMBIT_UPNP_SA_ENABLE /* Jasmine Add, 10/24/2006 */
#define SMART_WIZARD_SPEC_VERSION "0.7"  /* This is specification version of smartwizard 2.0 */
#endif

#endif /*_AMBITCFG_H*/
