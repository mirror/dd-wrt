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

#define WLAN_REGION          WW_VERSION
#define FW_REGION            WW_VERSION   /* true f/w region */

/*formal version control*/
#define AMBIT_HARDWARE_VERSION     "U12H07200"
#define AMBIT_SOFTWARE_VERSION     "V1.1.2"
#define AMBIT_UI_VERSION           "1.0.23"

/* Jasmine modify, from WGR614V8 to WGR614v8, for NGR's Bin requirement, 04/25/2007 */
#define AMBIT_PRODUCT_NAME          "WGR614v8" 
#define AMBIT_PRODUCT_DESCRIPTION   "Netgear Wireless Router WGR614v8"
#define UPnP_MODEL_URL "WGR614.aspx"
#define UPnP_MODEL_DESCRIPTION "54 Mbps"

#define AMBIT_NVRAM_VERSION  "1" /* digital only */

#ifdef AMBIT_UPNP_SA_ENABLE /* Jasmine Add, 10/24/2006 */
#define SMART_WIZARD_SPEC_VERSION "0.7"  /* This is specification version of smartwizard 2.0 */
#endif

/*ambit, add start, william lin, 05/17/2001 */
/****************************************************************************
 ***
 ***        put AMBIT features here!!!
 ***
 ***
 ****************************************************************************/

#define WAN_IF_NAME_NUM     "eth0"
#define LAN_IF_NAME_NUM     "vlan0"
#define WLAN_IF_NAME_NUM    "eth1"

/*definitions: GPIOs, MTD*/
#define NVRAM_MTD_RD        "/dev/mtdblock/7"
#define NVRAM_MTD_WR        "/dev/mtd/7"
#define POT_MTD_RD          "/dev/mtdblock/5"
#define POT_MTD_WR          "/dev/mtd/5"
#define BD_MTD_RD           "/dev/mtdblock/6"
#define BD_MTD_WR           "/dev/mtd/6"



#define GPIO_WAN_LED            4

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

#endif /*_AMBITCFG_H*/
