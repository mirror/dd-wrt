/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File under the following licensing terms. 
Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer. 

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution. 

    *   Neither the name of Marvell nor the names of its contributors may be 
        used to endorse or promote products derived from this software without 
        specific prior written permission. 
    
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/


#define DB_88F5182_BOARD_PCI_IF_NUM		0x3
#define DB_88F5182_BOARD_TWSI_DEF_NUM		0x1
#define DB_88F5182_BOARD_MAC_INFO_NUM		0x1
#define DB_88F5182_BOARD_GPP_INFO_NUM		0x4
#define DB_88F5182_BOARD_DEBUG_LED_NUM		0x0
#define DB_88F5182_BOARD_MPP_CONFIG_NUM		0x1
#define DB_88F5182_BOARD_DEVICE_CONFIG_NUM	0x4

MV_BOARD_PCI_IF 	db88f5182ddr2InfoBoardPciIf[DB_88F5182_BOARD_PCI_IF_NUM] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{7, {0, 0, 0, 0}},				/* pciSlot0 */							
	 {8, {1, 1, 1, 1}},				/* pciSlot1 */                      	
	 {9, {1, 1, 1, 1}}};			/* pciSlot2 */                      

MV_BOARD_TWSI_INFO	db88f5182ddr2InfoBoardTwsiDev[DB_88F5182_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO	db88f5182ddr2InfoBoardMacInfo[DB_88F5182_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8}}; 

MV_BOARD_GPP_INFO	db88f5182ddr2InfoBoardGppInfo[DB_88F5182_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RTC, 10},
	{BOARD_DEV_USB_VBUS, 8},
	{BOARD_DEV_USB_VBUS, 9},
	{BOARD_DEV_REF_CLCK, 11}};

MV_BOARD_MPP_INFO	db88f5182ddr2InfoBoardMppConfigValue[DB_88F5182_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{DB_88F5182_DDR2_MPP0_7,			/* mpp0_7 */
	DB_88F5182_DDR2_MPP8_15,			/* mpp8_15 */
	DB_88F5182_DDR2_MPP16_23,			/* mpp16_23 */
	N_A}}						/* mppDev */						
	};

MV_DEV_CS_INFO db88f5182ddr2InfoBoardDeCsInfo[DB_88F5182_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
		{{0, 0x8fcfffff, BOARD_DEV_SEVEN_SEG, N_A}, 	/* devCs0 */                       
		{1, 0x8fdfffff, BOARD_DEV_NOR_FLASH, 16},   	/* devCs1 */                    
		{2, 0x8fcfffff, BOARD_DEV_NAND_FLASH, 8},	/* devCs2/flashCs */            
		{3, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};	/* bootCs */                    

MV_BOARD_INFO db88f5182ddr2Info = {
	"DB-88F5182-DDR2",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F5182_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f5182ddr2InfoBoardMppConfigValue,
	((1<<0)|(1 << 1)|(1<<10)),			/* intsGppMask */
	DB_88F5182_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f5182ddr2InfoBoardDeCsInfo,
	DB_88F5182_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	db88f5182ddr2InfoBoardPciIf,
	DB_88F5182_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f5182ddr2InfoBoardTwsiDev,					
	DB_88F5182_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	db88f5182ddr2InfoBoardMacInfo,
	DB_88F5182_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	db88f5182ddr2InfoBoardGppInfo,
	DB_88F5182_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	NULL,
	N_A,						/* ledsPolarity */		
	DB_88F5182_DDR2_OE,				/* gppOutEnVal */
	DB_88F5182_DDR2_OUT_VAL,			/* gppOutVal */
	0x403,						/* gppPolarityVal */
};

#define DB_88F5182_BOARD_A_PCI_IF_NUM		0x2
#define DB_88F5182_BOARD_A_TWSI_DEF_NUM		0x1
#define DB_88F5182_BOARD_A_MAC_INFO_NUM		0x1
#define DB_88F5182_BOARD_A_GPP_INFO_NUM		0x4
#define DB_88F5182_BOARD_A_DEBUG_LED_NUM	0x0
#define DB_88F5182_BOARD_A_MPP_CONFIG_NUM	0x1
#define DB_88F5182_BOARD_A_DEVICE_CONFIG_NUM	0x4

MV_BOARD_PCI_IF 	db88f5182ddr2AInfoBoardPciIf[DB_88F5182_BOARD_A_PCI_IF_NUM] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	 {{8, {1, 1, 1, 1}},			/* pciSlot1 */                      	
	 {9, {1, 1, 1, 1}}};			/* pciSlot2 */                      

MV_BOARD_TWSI_INFO	db88f5182ddr2AInfoBoardTwsiDev[DB_88F5182_BOARD_A_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO	db88f5182ddr2AInfoBoardMacInfo[DB_88F5182_BOARD_A_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8}}; 

MV_BOARD_GPP_INFO	db88f5182ddr2AInfoBoardGppInfo[DB_88F5182_BOARD_A_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RTC, 10},
	{BOARD_DEV_USB_VBUS, 8},
	{BOARD_DEV_USB_VBUS, 9},
	{BOARD_DEV_REF_CLCK, 11}};

MV_BOARD_MPP_INFO	db88f5182ddr2AInfoBoardMppConfigValue[DB_88F5182_BOARD_A_MPP_CONFIG_NUM] = 
#ifndef  MV_NAND_BOOT
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{DB_88F5182_DDR2_A_MPP0_7,			/* mpp0_7 */
	DB_88F5182_DDR2_A_MPP8_15,			/* mpp8_15 */
	DB_88F5182_DDR2_A_MPP16_23,			/* mpp16_23 */
	N_A}}						/* mppDev */						
	};
#else
	{{{DB_88F5182_DDR2_A_MPP0_7NB,			/* mpp0_7 */
	DB_88F5182_DDR2_A_MPP8_15NB,			/* mpp8_15 */
	DB_88F5182_DDR2_A_MPP16_23NB,			/* mpp16_23 */
	N_A}}						/* mppDev */						
	};
#endif

MV_DEV_CS_INFO db88f5182ddr2AInfoBoardDeCsInfo[DB_88F5182_BOARD_A_DEVICE_CONFIG_NUM] = 
#ifndef  MV_NAND_BOOT
		/*{params, devType, devWidth}*/			   
		{{0, 0x8fcfffff, BOARD_DEV_SEVEN_SEG, N_A}, 	/* devCs0 */                       
		{1, 0x8fdfffff, BOARD_DEV_NOR_FLASH, 16},   	/* devCs1 */                    
		{2, 0x8fcfffff, BOARD_DEV_NAND_FLASH, 8},	/* devCs2/flashCs */            
		{3, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};	/* bootCs */                    
#else
		/*{params, devType, devWidth}*/			   
		{{0, 0x8fcfffff, BOARD_DEV_SEVEN_SEG, N_A}, 	/* devCs0 */                       
		{1, 0x8fdfffff, BOARD_DEV_NOR_FLASH, 16},   	/* devCs1 */                    
		{2, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8},	/* devCs2/flashCs */            
		{3, 0x8fcfffff, BOARD_DEV_NAND_FLASH, 8}};	/* bootCs */                    
#endif

MV_BOARD_INFO db88f5182ddr2AInfo = {
	"DB-88F5182-DDR2-NAND-SUPPORT",			/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F5182_BOARD_A_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f5182ddr2AInfoBoardMppConfigValue,
	((1 << 1)|(1<<10)),				/* intsGppMask */
	DB_88F5182_BOARD_A_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f5182ddr2AInfoBoardDeCsInfo,
	DB_88F5182_BOARD_A_PCI_IF_NUM,			/* numBoardPciIf */
	db88f5182ddr2AInfoBoardPciIf,
	DB_88F5182_BOARD_A_TWSI_DEF_NUM,		/* numBoardTwsiDev */
	db88f5182ddr2AInfoBoardTwsiDev,					
	DB_88F5182_BOARD_A_MAC_INFO_NUM,		/* numBoardMacInfo */
	db88f5182ddr2AInfoBoardMacInfo,
	DB_88F5182_BOARD_A_GPP_INFO_NUM,		/* numBoardGppInfo */
	db88f5182ddr2AInfoBoardGppInfo,
	DB_88F5182_BOARD_A_DEBUG_LED_NUM,		/* activeLedsNumber */              
	NULL,
	N_A,						/* ledsPolarity */		
	DB_88F5182_DDR2_A_OE,				/* gppOutEnVal */
	DB_88F5182_DDR2_A_OUT_VAL,			/* gppOutVal */
	0x403,						/* gppPolarityVal */
};


#define RD_88F5182_2XSATA_BOARD_PCI_IF_NUM		0x1
#define RD_88F5182_2XSATA_BOARD_TWSI_DEF_NUM		0x1
#define RD_88F5182_2XSATA_BOARD_MAC_INFO_NUM		0x1
#define RD_88F5182_2XSATA_BOARD_GPP_INFO_NUM		0x1
#define RD_88F5182_2XSATA_BOARD_DEBUG_LED_NUM		0x1
#define RD_88F5182_2XSATA_BOARD_MPP_CONFIG_NUM		0x1
#define RD_88F5182_2XSATA_BOARD_DEVICE_CONFIG_NUM	0x2

MV_U8			rd88f5182sataX2InfoBoardDebugLedIf[RD_88F5182_2XSATA_BOARD_DEBUG_LED_NUM] =
	{0};

MV_BOARD_PCI_IF 	rd88f5182sataX2InfoBoardPciIf[RD_88F5182_2XSATA_BOARD_PCI_IF_NUM] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{7, {7, 6, N_A, N_A}}};				/* pciSlot0 */                      

MV_BOARD_TWSI_INFO	rd88f5182sataX2InfoBoardTwsiDev[RD_88F5182_2XSATA_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO	rd88f5182sataX2InfoBoardMacInfo[RD_88F5182_2XSATA_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8}}; 

MV_BOARD_GPP_INFO	rd88f5182sataX2InfoBoardGppInfo[RD_88F5182_2XSATA_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RTC, 3}};

MV_BOARD_MPP_INFO	rd88f5182sataX2InfoBoardMppConfigValue[RD_88F5182_2XSATA_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{RD_88F5182_2XSATA_MPP0_7,			/* mpp0_7 */
	RD_88F5182_2XSATA_MPP8_15,			/* mpp8_15 */
	RD_88F5182_2XSATA_MPP16_23,			/* mpp16_23 */
	N_A}}						/* mppDev */						
	};

MV_DEV_CS_INFO rd88f5182sataX2InfoBoardDeCsInfo[RD_88F5182_2XSATA_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
		{{1, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8},   	/* devCs1 */                    
		{3, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};		/* bootCs */                    

MV_BOARD_INFO rd88f5182sataX2Info = {
	"RD-88F5182-NAS-2",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F5182_2XSATA_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88f5182sataX2InfoBoardMppConfigValue,
	((1<<3)|(1 << 6)|(1<<7)),			/* intsGppMask */
	RD_88F5182_2XSATA_BOARD_DEVICE_CONFIG_NUM,	/* numBoardDevIf */
	rd88f5182sataX2InfoBoardDeCsInfo,
	RD_88F5182_2XSATA_BOARD_PCI_IF_NUM,		/* numBoardPciIf */
	rd88f5182sataX2InfoBoardPciIf,
	RD_88F5182_2XSATA_BOARD_TWSI_DEF_NUM,		/* numBoardTwsiDev */
	rd88f5182sataX2InfoBoardTwsiDev,					
	RD_88F5182_2XSATA_BOARD_MAC_INFO_NUM,		/* numBoardMacInfo */
	rd88f5182sataX2InfoBoardMacInfo,
	RD_88F5182_2XSATA_BOARD_GPP_INFO_NUM,		/* numBoardGppInfo */
	rd88f5182sataX2InfoBoardGppInfo,
	RD_88F5182_2XSATA_BOARD_DEBUG_LED_NUM,		/* activeLedsNumber */              
	rd88f5182sataX2InfoBoardDebugLedIf,
	0,						/* ledsPolarity */		
	RD_88F5182_2XSATA_OE,				/* gppOutEnVal */
	RD_88F5182_2XSATA_OUT_VAL,			/* gppOutVal */
	RD_88F5182_2XSATA_POL,				/* gppPolarityVal */
};


#define RD_88F5182_2XSATA3_BOARD_PCI_IF_NUM		0x1
#define RD_88F5182_2XSATA3_BOARD_TWSI_DEF_NUM		0x1
#define RD_88F5182_2XSATA3_BOARD_MAC_INFO_NUM		0x1
#define RD_88F5182_2XSATA3_BOARD_GPP_INFO_NUM		0x4
#define RD_88F5182_2XSATA3_BOARD_DEBUG_LED_NUM		0x2
#define RD_88F5182_2XSATA3_BOARD_MPP_CONFIG_NUM		0x1
#define RD_88F5182_2XSATA3_BOARD_DEVICE_CONFIG_NUM	0x2

MV_U8			rd88f5182sataX23InfoBoardDebugLedIf[RD_88F5182_2XSATA3_BOARD_DEBUG_LED_NUM] =
	{0, 1};

MV_BOARD_PCI_IF 	rd88f5182sataX23InfoBoardPciIf[RD_88F5182_2XSATA3_BOARD_PCI_IF_NUM] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{7, {7, N_A, N_A, N_A}}};

MV_BOARD_TWSI_INFO	rd88f5182sataX23InfoBoardTwsiDev[RD_88F5182_2XSATA3_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO	rd88f5182sataX23InfoBoardMacInfo[RD_88F5182_2XSATA3_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8}}; 

MV_BOARD_GPP_INFO	rd88f5182sataX23InfoBoardGppInfo[RD_88F5182_2XSATA3_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RTC, 3},
	{BOARD_DEV_POWER_BUTTON, 6},
	{BOARD_DEV_HDD0_POWER, 2},
	{BOARD_DEV_HDD1_POWER, 8}
	};

MV_BOARD_MPP_INFO	rd88f5182sataX23InfoBoardMppConfigValue[RD_88F5182_2XSATA3_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{RD_88F5182_2XSATA3_MPP0_7,			/* mpp0_7 */
	RD_88F5182_2XSATA3_MPP8_15,			/* mpp8_15 */
	RD_88F5182_2XSATA3_MPP16_23,			/* mpp16_23 */
	N_A}}						/* mppDev */						
	};

MV_DEV_CS_INFO rd88f5182sataX23InfoBoardDeCsInfo[RD_88F5182_2XSATA3_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
		{{1, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8},   	/* devCs1 */                    
		{3, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};	/* bootCs */                    

MV_BOARD_INFO rd88f5182sataX23Info = {
	"RD-88F5182-NAS-3",					/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F5182_2XSATA3_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88f5182sataX23InfoBoardMppConfigValue,
	((1<<3) | (1<<7)),							/* intsGppMask */
	RD_88F5182_2XSATA3_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	rd88f5182sataX23InfoBoardDeCsInfo,
	RD_88F5182_2XSATA3_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	rd88f5182sataX23InfoBoardPciIf,
	RD_88F5182_2XSATA3_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	rd88f5182sataX23InfoBoardTwsiDev,					
	RD_88F5182_2XSATA3_BOARD_MAC_INFO_NUM,		/* numBoardMacInfo */
	rd88f5182sataX23InfoBoardMacInfo,
	RD_88F5182_2XSATA3_BOARD_GPP_INFO_NUM,		/* numBoardGppInfo */
	rd88f5182sataX23InfoBoardGppInfo,
	RD_88F5182_2XSATA3_BOARD_DEBUG_LED_NUM,		/* activeLedsNumber */              
	rd88f5182sataX23InfoBoardDebugLedIf,
	1,						/* ledsPolarity */		
	RD_88F5182_2XSATA3_OE,				/* gppOutEnVal */
	RD_88F5182_2XSATA3_OUT_VAL,			/* gppOutVal */
	N_A,						/* gppPolarityVal */
};

MV_BOARD_INFO*	boardInfoTbl[]	=	{&db88f5182ddr2Info,
				  	&rd88f5182sataX2Info,
					&rd88f5182sataX23Info,
					&db88f5182ddr2AInfo
					};


#define	BOARD_ID_BASE				BOARD_ID_88F5182_BASE
#define MV_MAX_BOARD_ID				BOARD_ID_88F5182_MAX
