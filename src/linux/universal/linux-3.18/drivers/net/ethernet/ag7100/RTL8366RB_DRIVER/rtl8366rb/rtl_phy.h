/***************************************************************
 *
 * Copyright(c) 2005-2007 Shenzhen TP-Link Technologies Co. Ltd.
 * All right reserved.
 *
 * Filename		:	rtl_phy.h
 * Version		:	1.0
 * Abstract		:	
 * Author		:	
 * Created Date	:	03/01/2008
 *
 * Modified History:
 ***************************************************************/

#ifndef _RTL_PHY_H_
#define _RTL_PHY_H_

#include <linux/types.h>
//#include "ar7100.h"

#define DELAY						10000
#define CLK_DURATION(clk)			{ int i; for(i=0; i<clk; i++); }
#define _SMI_ACK_RESPONSE(ok)		{ /*if (!(ok)) return FAILED; */}

#define PHY_NUM_MAX	5

#if 0
#define smi_SCK		5		/* GPIO used for SMI Clock Generation */
#define smi_SDA		4		/* GPIO used for SMI Data signal */
//#define smi_RST     /* GPIO used for reset swtich */

#define ack_timer					5
#define max_register				0x018A 
#endif

#define	GPIO_DIR_OUT		0
#define GPIO_DIR_IN			1
#define GPIO_INT_DISABLE	0
#define GPIO_INT_ENABLE		1

#if 0
/*****************/
/* PHY Registers */
/*****************/
#define PHY_CONTROL_REG						0
#define PHY_STATUS_REG						1
#define PHY_AN_ADVERTISEMENT_REG			4
#define PHY_AN_LINKPARTNER_REG				5
#define PHY_1000_BASET_CONTROL_REG			9
#define PHY_1000_BASET_STATUS_REG			10

/* PHY_CONTROL fields */
#define PHY_CTRL_SOFTWARE_RESET                    0x8000
#define PHY_CTRL_SPEED_100                         0x2000
#define PHY_CTRL_AUTONEGOTIATION_ENABLE            0x1000
#define PHY_CTRL_START_AUTONEGOTIATION             0x0200
#define PHY_CTRL_SPEED_FULL_DUPLEX                 0x0100

/* Phy status fields */
#define PHY_STATUS_AUTO_NEG_DONE                   0x0020
#define PHY_STATUS_LINK_PASS                       0x0004

#define PHY_AUTONEG_DONE(ip_phy_status)                   \
    (((ip_phy_status) &                                  \
        (PHY_STATUS_AUTO_NEG_DONE)) ==                    \
        (PHY_STATUS_AUTO_NEG_DONE))

/* Link Partner ability */
#define PHY_LINK_100BASETX_FULL_DUPLEX       0x0100
#define PHY_LINK_100BASETX                   0x0080
#define PHY_LINK_10BASETX_FULL_DUPLEX        0x0040
#define PHY_LINK_10BASETX                    0x0020

/* Advertisement register. */
#define PHY_ADVERTISE_100FULL                0x0100
#define PHY_ADVERTISE_100HALF                0x0080  
#define PHY_ADVERTISE_10FULL                 0x0040  
#define PHY_ADVERTISE_10HALF                 0x0020  

#define PHY_ADVERTISE_ALL (PHY_ADVERTISE_10HALF | PHY_ADVERTISE_10FULL | \
                       PHY_ADVERTISE_100HALF | PHY_ADVERTISE_100FULL)
#endif

#define BOOL    uint32_t
#define FALSE	0
#define TRUE	1



#define SUCCESS	0
#define FAILED	1

BOOL rtl_phySetup(int ethUnit);
int rtl_phyIsUp(int ethUnit);
int rtl_phyIsFullDuplex(int ethUnit);
BOOL rtl_phySpeed(int ethUnit);


#endif

