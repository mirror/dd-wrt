/*
* Copyright c                  Realtek Semiconductor Corporation, 2007  
* All rights reserved.                                                    
* 
* Program : The external header file of high level API
* Abstract :                                                           
* Author : Abel Hsu (abelshie@realtek.com.tw)               
* $Id: rtl8366s_greenethernet.h,v 1.1 2007/08/21 08:30:49 abelshie Exp $
*/

#ifndef __RTL8366S_API_GREENETHERNET_H__
#define __RTL8366S_API_GREENETHERNET_H__

//#include "rtl8366s_api.h"


int32 rtl8366s_setAsicGreenFeature(uint32 txGreen,uint32 rxGreen);
int32 rtl8366s_getAsicGreenFeature(uint32* txGreen,uint32* rxGreen);
int32 rtl8366s_setAsicPowerSaving(uint32 phyNo,uint32 enabled);
int32 rtl8366s_getAsicPowerSaving(uint32 phyNo,uint32* enabled);
/*Green ethernet*/
extern int32 rtl8366s_setGreenEthernet(uint32 greenFeature, uint32 powerSaving);


#endif /* __RTL8366S_API_EXT_H__ */

