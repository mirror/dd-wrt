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
#define TWC_VERSION          7

#define WLAN_REGION          WW_VERSION
#define FW_REGION            WW_VERSION   /* true f/w region */

/*formal version control*/
#define AMBIT_HARDWARE_VERSION     "U12H08100"
#define AMBIT_SOFTWARE_VERSION     "V2.0.8"
#define AMBIT_UI_VERSION           "2.0.8"

#endif /*_AMBITCFG_H*/
