
#ifndef _AMBITCFG_H
#define _AMBITCFG_H

#define WW_VERSION           1 /* WW SKUs */
#define NA_VERSION           2 /* NA SKUs */
#define JP_VERSION           3
#define GR_VERSION           4
#define PR_VERSION           5
#define KO_VERSION           6
#define TWC_VERSION          7

#define WLAN_REGION          NA_VERSION
#define FW_REGION            NA_VERSION   /* true f/w region */

/*formal version control*/
#if (defined WNR3500v2)
#define AMBIT_HARDWARE_VERSION     "U12H12700"
#define AMBIT_SOFTWARE_VERSION     "V1.0.0.64"
#define AMBIT_UI_VERSION           "11.0.51NA"
#define AMBIT_PRODUCT_NAME          "WNR3500v2"
#define AMBIT_PRODUCT_DESCRIPTION   "Netgear Wireless Router WNR3500v2"
#define UPnP_MODEL_URL              "WNR3500v2.aspx"
#define UPnP_MODEL_DESCRIPTION      "RangeMax NEXT"
#endif

#if (defined WNR3500U)
#define AMBIT_HARDWARE_VERSION     "U12H13600"
#define AMBIT_SOFTWARE_VERSION     "V1.0.0.64"
#define AMBIT_UI_VERSION           "11.0.51NA"
#define AMBIT_PRODUCT_NAME          "WNR3500U"
#define AMBIT_PRODUCT_DESCRIPTION   "Netgear Wireless Router WNR3500U"
#define UPnP_MODEL_URL              "WNR3500U.aspx"
#define UPnP_MODEL_DESCRIPTION      "RangeMax NEXT"
#endif

#if (defined WNR3500v2VCNA)
#define AMBIT_HARDWARE_VERSION     "U12H12700"
#define AMBIT_SOFTWARE_VERSION     "V1.0.0.158"
#define AMBIT_UI_VERSION           "11.0.48NA"
#define AMBIT_PRODUCT_NAME          "WNR3500v2"
#define AMBIT_PRODUCT_DESCRIPTION   "Netgear Wireless Router WNR3500v2"
#define UPnP_MODEL_URL              "WNR3500v2.aspx"
#define UPnP_MODEL_DESCRIPTION      "RangeMax NEXT"
#endif

#define AMBIT_NVRAM_VERSION  "1" /* digital only */

#ifdef AMBIT_UPNP_SA_ENABLE 
#define SMART_WIZARD_SPEC_VERSION "0.7"  /* This is specification version of smartwizard 2.0 */
#endif

#endif /*_AMBITCFG_H*/
