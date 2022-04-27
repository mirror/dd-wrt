/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2004-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5312/ar5312phy.h#1 $
 */
#ifndef _DEV_ATH_AR5312PHY_H_
#define _DEV_ATH_AR5312PHY_H_

#include "ar5212/ar5212phy.h"

/* PHY registers */
#define AR_PHY_PLL_CTL_44_5312  0x14d6          /* 44 MHz for 11b, 11g */
#define AR_PHY_PLL_CTL_44_5312_HALF  0x15d6          /* 44 MHz for 11b, 11g */
#define AR_PHY_PLL_CTL_44_5312_QUARTER  0x16d6          /* 44 MHz for 11b, 11g */
#define AR_PHY_PLL_CTL_44_5312_SUBQUARTER  0x16d6         /* 44 MHz for 11b, 11g */
//#define AR_PHY_PLL_CTL_44_5312_SUBQUARTER  0x16cb          /* 44 MHz for 11b, 11g */

#define AR_PHY_PLL_CTL_40_5312  0x14d4          /* 40 MHz for 11a, turbos */
#define AR_PHY_PLL_CTL_40_5312_HALF  0x15d4	/* 40 MHz for 11a, turbos (Half)*/
#define AR_PHY_PLL_CTL_40_5312_QUARTER  0x16d4	/* 40 MHz for 11a, turbos (Quarter)*/
#define AR_PHY_PLL_CTL_40_5312_SUBQUARTER  0x16d4	/* 40 MHz for 11a, turbos (Quarter)*/

//#define AR_PHY_PLL_CTL_40_5312_SUBQUARTER  0x16ca	/* 40 MHz for 11a, turbos (Quarter)*/

#endif	/* _DEV_ATH_AR5312PHY_H_ */
