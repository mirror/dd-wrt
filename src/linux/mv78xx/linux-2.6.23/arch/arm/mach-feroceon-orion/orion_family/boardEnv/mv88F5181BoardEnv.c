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



#define DB_88F5X81_BOARD_PCI_IF_NUM		0x3
#define DB_88F5X81_BOARD_TWSI_DEF_NUM		0x1
#define DB_88F5X81_BOARD_MAC_INFO_NUM		0x1
#define DB_88F5X81_BOARD_GPP_INFO_NUM		0x3
#define DB_88F5X81_BOARD_DEBUG_LED_NUM		0x0
#define DB_88F5X81_BOARD_MPP_CONFIG_NUM		0x1
#define DB_88F5X81_BOARD_DEVICE_CONFIG_NUM	0x4

MV_BOARD_PCI_IF db88f5x81ddr2InfoBoardPciIf[DB_88F5X81_BOARD_PCI_IF_NUM] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{7, {12, 12, 12, 12}},				/* pciSlot0 */							
	 {8, {13, 13, 13, 13}},				/* pciSlot1 */                      	
	 {9, {13, 13, 13, 13}}};			/* pciSlot2 */                      

MV_BOARD_TWSI_INFO db88f5x81ddr2InfoBoardTwsiDev[DB_88F5X81_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO db88f5x81ddr2InfoBoardMacInfo[DB_88F5X81_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8}}; 

MV_BOARD_GPP_INFO db88f5x81ddr2InfoBoardGppInfo[DB_88F5X81_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RTC, 10},
	{BOARD_DEV_USB_VBUS, 1},
	{BOARD_DEV_REF_CLCK,11}};

MV_BOARD_MPP_INFO db88f5x81ddr2InfoBoardMppConfigValue[DB_88F5X81_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
#ifndef MV_NAND_BOOT
	{{{DB_88F5X81_DDRX_MPP0_7,			/* mpp0_7 */
	DB_88F5X81_DDRX_MPP8_15,			/* mpp8_15 */
	DB_88F5X81_DDRX_MPP16_23,			/* mpp16_23 */
	DB_88F5X81_DDRX_MPP_DEV}}			/* mppDev */						
	};
#else
	{{{DB_88F5X81_DDRX_MPP0_7_NB,			/* mpp0_7 */
	DB_88F5X81_DDRX_MPP8_15_NB,			/* mpp8_15 */
	DB_88F5X81_DDRX_MPP16_23_NB,			/* mpp16_23 */
	DB_88F5X81_DDRX_MPP_DEV_NB}}			/* mppDev */
	};
#endif

MV_DEV_CS_INFO db88f5x81ddr2InfoBoardDeCsInfo[DB_88F5X81_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
#ifndef MV_NAND_BOOT
		{{0, 0x8fcfffff, BOARD_DEV_SEVEN_SEG, N_A}, 	/* devCs0 */                       
		{ 1, 0x8fefffff, BOARD_DEV_NOR_FLASH, 16},   	/* devCs1 */                    
		{ 2, 0x8fcfffff, BOARD_DEV_NAND_FLASH, 8},	/* devCs2/flashCs */            
		{ 3, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};	/* bootCs */                    
#else
		{{0, 0x8fcfffff, BOARD_DEV_SEVEN_SEG, N_A}, 	/* devCs0 */                       
		{ 1, 0x8fefffff, BOARD_DEV_NOR_FLASH, 16},   	/* devCs1 */                    
		{ 2, 0x8fcfffff, BOARD_DEV_OTHER, 8},	/* devCs2/flashCs */            
		{ 3, 0x8fdfffff, BOARD_DEV_NAND_FLASH, 16}};	/* bootCs */                    
#endif

MV_BOARD_INFO db88f5x81ddr2Info = {
	"DB-88F5X81-DDR2-A/B",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F5X81_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f5x81ddr2InfoBoardMppConfigValue,
	((1<<10)|(1 << 12)|(1<<13)),			/* intsGppMask */
	DB_88F5X81_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f5x81ddr2InfoBoardDeCsInfo,
	DB_88F5X81_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	db88f5x81ddr2InfoBoardPciIf,
	DB_88F5X81_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f5x81ddr2InfoBoardTwsiDev,					
	DB_88F5X81_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	db88f5x81ddr2InfoBoardMacInfo,
	DB_88F5X81_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	db88f5x81ddr2InfoBoardGppInfo,
	DB_88F5X81_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	NULL,
	N_A,						/* ledsPolarity */		
	DB_88F5X81_DDRX_OE,				/* gppOutEnVal */
	DB_88F5X81_DDRX_OUT_VAL,			/* gppOutVal */
	0x3400,						/* gppPolarityVal */
	NULL						/* pSwitchInfo */
};

MV_BOARD_INFO db88f5x81ddr1Info = {
	"DB-88F5X81-DDR1-A",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F5X81_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f5x81ddr2InfoBoardMppConfigValue,
	((1<<10)|(1 << 12)|(1<<13)),			/* intsGppMask */
	DB_88F5X81_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f5x81ddr2InfoBoardDeCsInfo,
	DB_88F5X81_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	db88f5x81ddr2InfoBoardPciIf,
	DB_88F5X81_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f5x81ddr2InfoBoardTwsiDev,					
	DB_88F5X81_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	db88f5x81ddr2InfoBoardMacInfo,
	DB_88F5X81_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	db88f5x81ddr2InfoBoardGppInfo,
	DB_88F5X81_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	NULL,
	N_A,						/* ledsPolarity */		
	DB_88F5X81_DDRX_OE,				/* gppOutEnVal */
	DB_88F5X81_DDRX_OUT_VAL,			/* gppOutVal */
	0x3400,						/* gppPolarityVal */
	NULL						/* pSwitchInfo */
};

MV_BOARD_INFO db88f1181ddr1Info	= {
	"DB-88F1181-DDR1"				/* boardName[MAX_BOARD_NAME_LEN] */
};

MV_BOARD_INFO db88f1181ddr2Info	= {
	"DB-88F1181-DDR2"				/* boardName[MAX_BOARD_NAME_LEN] */
};

#define DB_PRPMC_BOARD_PCI_IF_NUM		0x1
#define DB_PRPMC_BOARD_TWSI_DEF_NUM		0x1
#define DB_PRPMC_BOARD_MAC_INFO_NUM		0x1
#define DB_PRPMC_BOARD_GPP_INFO_NUM		0x1
#define DB_PRPMC_BOARD_DEBUG_LED_NUM		0x3
#define DB_PRPMC_BOARD_MPP_CONFIG_NUM		0x1
#define DB_PRPMC_BOARD_DEVICE_CONFIG_NUM	0x2

MV_U8			db88f5181prpmcInfoBoardDebugLedIf[DB_PRPMC_BOARD_DEBUG_LED_NUM] =
	{12, 13, 14};
MV_BOARD_PCI_IF 	db88f5181prpmcInfoBoardPciIf[DB_PRPMC_BOARD_PCI_IF_NUM] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{0, {12, 12, 12, 12}}};				/* pciSlot0 */							
MV_BOARD_TWSI_INFO	db88f5181prpmcInfoBoardTwsiDev[DB_PRPMC_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO	db88f5181prpmcInfoBoardMacInfo[DB_PRPMC_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x1F}}; 

MV_BOARD_GPP_INFO	db88f5181prpmcInfoBoardGppInfo[DB_PRPMC_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_REF_CLCK, 0}};

MV_BOARD_MPP_INFO	db88f5181prpmcInfoBoardMppConfigValue[DB_PRPMC_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{DB_88F5181_DDR1_PRPMC_MPP0_7,		/* mpp0_7 */
	DB_88F5181_DDR1_PRPMC_MPP8_15,			/* mpp8_15 */
	DB_88F5181_DDR1_PRPMC_MPP16_23,			/* mpp16_23 */
	DB_88F5181_DDR1_PRPMC_MPP_DEV}}			/* mppDev */						
	};

MV_DEV_CS_INFO db88f5181prpmcInfoBoardDeCsInfo[DB_PRPMC_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
		{{ 1, 0x8fefffff, BOARD_DEV_NOR_FLASH, 16}, /* devCs1 */                    
		{ 3, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};	/* bootCs */                    

MV_BOARD_INFO db88f5181prpmcInfo = {
	"DB-88F5181-DDR1-PRPMC",			/* boardName[MAX_BOARD_NAME_LEN] */
	DB_PRPMC_BOARD_MPP_CONFIG_NUM,			/* numBoardMppConfig */
	db88f5181prpmcInfoBoardMppConfigValue,
	((1 << 4)|(1 << 5)|(1 << 6)|(1 << 7)),		/* intsGppMask */
	DB_PRPMC_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f5181prpmcInfoBoardDeCsInfo,
	DB_PRPMC_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	db88f5181prpmcInfoBoardPciIf,
	DB_PRPMC_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f5181prpmcInfoBoardTwsiDev,					
	DB_PRPMC_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	db88f5181prpmcInfoBoardMacInfo,
	DB_PRPMC_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	db88f5181prpmcInfoBoardGppInfo,
	DB_PRPMC_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	db88f5181prpmcInfoBoardDebugLedIf,
	0,						/* ledsPolarity */		
	DB_88F5181_DDR1_PRPMC_OE,			/* gppOutEnVal */
	DB_88F5181_DDR1_PRPMC_OUT_VAL,			/* gppOutVal */
	N_A,						/* gppPolarityVal */
	NULL						/* pSwitchInfo */
};

#define DB_PEXPCI_BOARD_PCI_IF_NUM		0x1
#define DB_PEXPCI_BOARD_TWSI_DEF_NUM		0x1
#define DB_PEXPCI_BOARD_MAC_INFO_NUM		0x1
#define DB_PEXPCI_BOARD_GPP_INFO_NUM		0x1
#define DB_PEXPCI_BOARD_DEBUG_LED_NUM		0x4
#define DB_PEXPCI_BOARD_MPP_CONFIG_NUM		0x1
#define DB_PEXPCI_BOARD_DEVICE_CONFIG_NUM	0x2

MV_U8			db88f5181pexPciInfoBoardDebugLedIf[DB_PEXPCI_BOARD_DEBUG_LED_NUM] =
	{12, 13, 14, 15};

MV_BOARD_PCI_IF 	db88f5181pexPciInfoBoardPciIf[DB_PEXPCI_BOARD_PCI_IF_NUM] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{0, {3, 6, 7, 7}}};				/* pciSlot0 */							

MV_BOARD_TWSI_INFO	db88f5181pexPciInfoBoardTwsiDev[DB_PEXPCI_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO	db88f5181pexPciInfoBoardMacInfo[DB_PEXPCI_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8}}; 

MV_BOARD_GPP_INFO	db88f5181pexPciInfoBoardGppInfo[DB_PEXPCI_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_REF_CLCK, 0}};

MV_BOARD_MPP_INFO	db88f5181pexPciInfoBoardMppConfigValue[DB_PEXPCI_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{DB_88F5181_DDR1_PRPMC_MPP0_7,		/* mpp0_7 */
	DB_88F5181_DDR1_PRPMC_MPP8_15,			/* mpp8_15 */
	DB_88F5181_DDR1_PRPMC_MPP16_23,			/* mpp16_23 */
	DB_88F5181_DDR1_PRPMC_MPP_DEV}}			/* mppDev */						
	};

MV_DEV_CS_INFO db88f5181pexPciInfoBoardDeCsInfo[DB_PEXPCI_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
		{{1, 0x8fefffff, BOARD_DEV_NOR_FLASH, 16}, /* devCs1 */                    
		{ 3, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};	/* bootCs */                    

MV_BOARD_INFO db88f5181pexPciInfo = {
	"DB-88F5181-DDR1-PEX_PCI",			/* boardName[MAX_BOARD_NAME_LEN] */
	DB_PEXPCI_BOARD_MPP_CONFIG_NUM,			/* numBoardMppConfig */
	db88f5181pexPciInfoBoardMppConfigValue,
	((1 << 4)|(1 << 5)|(1 << 6)|(1 << 7)),		/* intsGppMask */
	DB_PEXPCI_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f5181pexPciInfoBoardDeCsInfo,
	DB_PEXPCI_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	db88f5181pexPciInfoBoardPciIf,
	DB_PEXPCI_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f5181pexPciInfoBoardTwsiDev,					
	DB_PEXPCI_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	db88f5181pexPciInfoBoardMacInfo,
	DB_PEXPCI_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	db88f5181pexPciInfoBoardGppInfo,
	DB_PEXPCI_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	db88f5181pexPciInfoBoardDebugLedIf,
	0,						/* ledsPolarity */		
	DB_88F5181_DDR1_PRPMC_OE,			/* gppOutEnVal */
	DB_88F5181_DDR1_PRPMC_OUT_VAL,			/* gppOutVal */
	N_A,						/* gppPolarityVal */
	NULL						/* pSwitchInfo */
};

#define RD_88F5181_POS_NAS_BOARD_PCI_IF_NUM		0x1
#define RD_88F5181_POS_NAS_BOARD_TWSI_DEF_NUM		0x1
#define RD_88F5181_POS_NAS_BOARD_MAC_INFO_NUM		0x1
#define RD_88F5181_POS_NAS_BOARD_GPP_INFO_NUM		0x3
#define RD_88F5181_POS_NAS_BOARD_DEBUG_LED_NUM		0x4
#define RD_88F5181_POS_NAS_BOARD_MPP_CONFIG_NUM		0x1
#define RD_88F5181_POS_NAS_BOARD_DEVICE_CONFIG_NUM	0x2

MV_U8			rd88f5181posNasInfoBoardDebugLedIf[RD_88F5181_POS_NAS_BOARD_DEBUG_LED_NUM] =
	{12, 13, 14, 15};

MV_BOARD_PCI_IF 	rd88f5181posNasInfoBoardPciIf[RD_88F5181_POS_NAS_BOARD_PCI_IF_NUM] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{7, {6, 4, N_A, N_A}}};				/* pciSlot0 */

MV_BOARD_TWSI_INFO	rd88f5181posNasInfoBoardTwsiDev[RD_88F5181_POS_NAS_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO	rd88f5181posNasInfoBoardMacInfo[RD_88F5181_POS_NAS_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8}}; 

MV_BOARD_GPP_INFO	rd88f5181posNasInfoBoardGppInfo[RD_88F5181_POS_NAS_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RTC, 8},
	{BOARD_DEV_USB_VBUS, 9},
	{BOARD_DEV_REF_CLCK,0}};

MV_BOARD_MPP_INFO	rd88f5181posNasInfoBoardMppConfigValue[RD_88F5181_POS_NAS_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{RD_88F5181_POS_NAS_MPP0_7,				/* mpp0_7 */
	RD_88F5181_POS_NAS_MPP8_15,				/* mpp8_15 */
	RD_88F5181_POS_NAS_MPP16_23,				/* mpp16_23 */
	RD_88F5181_POS_NAS_MPP_DEV}}				/* mppDev */						
	};

MV_DEV_CS_INFO rd88f5181posNasInfoBoardDeCsInfo[RD_88F5181_POS_NAS_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
		{{1, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8},	/* devCs1 */                    
		{3, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};	/* bootCs */                    

MV_BOARD_INFO rd88f5181posNasInfo = {
	"RD-88F5181-88SX7042-2xSATA",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F5181_POS_NAS_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88f5181posNasInfoBoardMppConfigValue,
	((1 << 4)|(1 << 6)|(1 << 8)),				/* intsGppMask */
	RD_88F5181_POS_NAS_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	rd88f5181posNasInfoBoardDeCsInfo,
	RD_88F5181_POS_NAS_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	rd88f5181posNasInfoBoardPciIf,
	RD_88F5181_POS_NAS_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	rd88f5181posNasInfoBoardTwsiDev,					
	RD_88F5181_POS_NAS_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	rd88f5181posNasInfoBoardMacInfo,
	RD_88F5181_POS_NAS_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	rd88f5181posNasInfoBoardGppInfo,
	RD_88F5181_POS_NAS_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	rd88f5181posNasInfoBoardDebugLedIf,
	0,							/* ledsPolarity */		
	RD_88F5181_POS_NAS_OE,					/* gppOutEnVal */
	RD_88F5181_POS_NAS_OUT_VAL,				/* gppOutVal */
	N_A,							/* gppPolarityVal */
	NULL							/* pSwitchInfo */
};


#define RD_88F5181_VOIP_BOARD_PCI_IF_NUM		0x1
#define RD_88F5181_VOIP_BOARD_TWSI_DEF_NUM		0x1
#define RD_88F5181_VOIP_BOARD_MAC_INFO_NUM		0x1
#define RD_88F5181_VOIP_BOARD_GPP_INFO_NUM		0x3
#define RD_88F5181_VOIP_BOARD_DEBUG_LED_NUM		0x3
#define RD_88F5181_VOIP_BOARD_MPP_CONFIG_NUM		0x3
#define RD_88F5181_VOIP_BOARD_DEVICE_CONFIG_NUM		0x4

MV_U8			rd88f5181voipInfoBoardDebugLedIf[RD_88F5181_VOIP_BOARD_DEBUG_LED_NUM] =
	{13, 14, 15};

MV_BOARD_PCI_IF 	rd88f5181voipInfoBoardPciIf[RD_88F5181_VOIP_BOARD_PCI_IF_NUM] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{1, {4, 4, 4, 4}}};				/* pciSlot0 */

MV_BOARD_TWSI_INFO	rd88f5181voipInfoBoardTwsiDev[RD_88F5181_VOIP_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO	rd88f5181voipInfoBoardMacInfo[RD_88F5181_VOIP_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_100M, 0x0}}; 

MV_BOARD_GPP_INFO	rd88f5181voipInfoBoardGppInfo[RD_88F5181_VOIP_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RTC, 7},
	{BOARD_DEV_USB_VBUS, 2},
	{BOARD_DEV_MV_SWITCH,6}};

MV_BOARD_MPP_INFO	rd88f5181voipInfoBoardMppConfigValue[RD_88F5181_VOIP_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{RD_88F5181_VOIP_MPP0_7,			/* mpp0_7 */
	RD_88F5181_VOIP_MPP8_15}}			/* mpp8_15 */
	};

MV_DEV_CS_INFO rd88f5181voipInfoBoardDeCsInfo[RD_88F5181_VOIP_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
		{{ 0, 0x8fdfffff, BOARD_DEV_FPGA, N_A}, 	/* devCs0 */                       
		{ 1, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8},	/* devCs1 */                    
		{ 2, 0x8fdfffff, BOARD_DEV_FPGA, N_A},	/* devCs2/flashCs */            
		{ 3, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};	/* bootCs */                    

MV_BOARD_INFO rd88f5181voipInfo = {
	"RD-88F5181-VOIP-RD1",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F5181_VOIP_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88f5181voipInfoBoardMppConfigValue,
	((1 << 3)|(1 << 4)|(1 << 6)|(1 << 7)),		/* intsGppMask */
	RD_88F5181_VOIP_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	rd88f5181voipInfoBoardDeCsInfo,
	RD_88F5181_VOIP_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	rd88f5181voipInfoBoardPciIf,
	RD_88F5181_VOIP_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	rd88f5181voipInfoBoardTwsiDev,					
	RD_88F5181_VOIP_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	rd88f5181voipInfoBoardMacInfo,
	RD_88F5181_VOIP_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	rd88f5181voipInfoBoardGppInfo,
	RD_88F5181_VOIP_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	rd88f5181voipInfoBoardDebugLedIf,
	0,						/* ledsPolarity */		
	RD_88F5181_VOIP_OE,				/* gppOutEnVal */
	RD_88F5181_VOIP_OUT_VAL,			/* gppOutVal */
	N_A,						/* gppPolarityVal */
	NULL						/* pSwitchInfo */
};

#define RD_88F5181_GTW_FE_BOARD_PCI_IF_NUM		0x1
#define RD_88F5181_GTW_FE_BOARD_TWSI_DEF_NUM		0x1
#define RD_88F5181_GTW_FE_BOARD_MAC_INFO_NUM		0x1
#define RD_88F5181_GTW_FE_BOARD_GPP_INFO_NUM		0x3
#define RD_88F5181_GTW_FE_BOARD_DEBUG_LED_NUM		0x3
#define RD_88F5181_GTW_FE_BOARD_MPP_CONFIG_NUM		0x1
#define RD_88F5181_GTW_FE_BOARD_DEVICE_CONFIG_NUM	0x1

MV_U8			rd88f5181GtwFeInfoBoardDebugLedIf[RD_88F5181_GTW_FE_BOARD_DEBUG_LED_NUM] =
	{13, 14, 15};

MV_BOARD_PCI_IF 	rd88f5181GtwFeInfoBoardPciIf[RD_88F5181_GTW_FE_BOARD_PCI_IF_NUM] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{7, {4, 3, N_A, N_A}}};				/* pciSlot0 */

MV_BOARD_TWSI_INFO	rd88f5181GtwFeInfoBoardTwsiDev[RD_88F5181_GTW_FE_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO	rd88f5181GtwFeInfoBoardMacInfo[RD_88F5181_GTW_FE_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_100M, 0x0}}; 

MV_BOARD_SWITCH_INFO	rd88f5181GtwFeInfoBoardSwitchInfo[] = 
	/* MV_32 linkStatusIrq, {MV_32 qdPort0, MV_32 qdPort1, MV_32 qdPort2, MV_32 qdPort3, MV_32 qdPort4}, 
		MV_32 qdCpuPort, MV_32 smiScanMode} */
	{{-1, {0, 1, 2, 3, 4}, 5, 0}};

MV_BOARD_GPP_INFO	rd88f5181GtwFeInfoBoardGppInfo[RD_88F5181_GTW_FE_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RTC, 11}
	};

MV_BOARD_MPP_INFO	rd88f5181GtwFeInfoBoardMppConfigValue[RD_88F5181_GTW_FE_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{RD_88F5181_GTW_FE_MPP0_7,			/* mpp0_7 */
	RD_88F5181_GTW_FE_MPP8_15}}			/* mpp8_15 */
	};

MV_DEV_CS_INFO rd88f5181GtwFeInfoBoardDeCsInfo[RD_88F5181_GTW_FE_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
		{{ 3, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};	/* bootCs */                    

MV_BOARD_INFO rd88f5181GtwFeInfo = {
	"RD-88F5181-GTW-FE",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F5181_GTW_FE_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88f5181GtwFeInfoBoardMppConfigValue,
	((1 << 2)|(1 << 3)|(1 << 4)|(1 << 5)|(1 << 9)|(1 << 11)),/* intsGppMask */
	RD_88F5181_GTW_FE_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	rd88f5181GtwFeInfoBoardDeCsInfo,
	RD_88F5181_GTW_FE_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	rd88f5181GtwFeInfoBoardPciIf,
	RD_88F5181_GTW_FE_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	rd88f5181GtwFeInfoBoardTwsiDev,					
	RD_88F5181_GTW_FE_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	rd88f5181GtwFeInfoBoardMacInfo,
	RD_88F5181_GTW_FE_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	rd88f5181GtwFeInfoBoardGppInfo,
	RD_88F5181_GTW_FE_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	rd88f5181GtwFeInfoBoardDebugLedIf,
	0,							/* ledsPolarity */		
	RD_88F5181_GTW_FE_OE,					/* gppOutEnVal */
	RD_88F5181_GTW_FE_OUT_VAL,				/* gppOutVal */
    	0xA3C,                              			/* gppPolarity */
    	rd88f5181GtwFeInfoBoardSwitchInfo			/* pSwitchInfo */

};

#define RD_88F5181_GTW_GE_BOARD_PCI_IF_NUM		0x1
#define RD_88F5181_GTW_GE_BOARD_TWSI_DEF_NUM		0x1
#define RD_88F5181_GTW_GE_BOARD_MAC_INFO_NUM		0x1
#define RD_88F5181_GTW_GE_BOARD_GPP_INFO_NUM		0x3
#define RD_88F5181_GTW_GE_BOARD_DEBUG_LED_NUM		0x4
#define RD_88F5181_GTW_GE_BOARD_MPP_CONFIG_NUM		0x1
#define RD_88F5181_GTW_GE_BOARD_DEVICE_CONFIG_NUM	0x1

MV_U8			rd88f5181GtwGeInfoBoardDebugLedIf[RD_88F5181_GTW_GE_BOARD_DEBUG_LED_NUM] =
	{1, 2, 3,0};

MV_BOARD_PCI_IF 	rd88f5181GtwGeInfoBoardPciIf[RD_88F5181_GTW_GE_BOARD_PCI_IF_NUM] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{7, {4, 10, N_A, N_A}}};				/* pciSlot0 */

MV_BOARD_TWSI_INFO	rd88f5181GtwGeInfoBoardTwsiDev[RD_88F5181_GTW_GE_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO	rd88f5181GtwGeInfoBoardMacInfo[RD_88F5181_GTW_GE_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_1000M, 0x0}}; 

MV_BOARD_SWITCH_INFO	rd88f5181GtwGeInfoBoardSwitchInfo[] = 
	/* MV_32 linkStatusIrq, {MV_32 qdPort0, MV_32 qdPort1, MV_32 qdPort2, MV_32 qdPort3, MV_32 qdPort4}, 
		MV_32 qdCpuPort, MV_32 smiScanMode} */
	{{8, {2, 1, 0, 7, 5}, 3, 1}};

MV_BOARD_GPP_INFO	rd88f5181GtwGeInfoBoardGppInfo[RD_88F5181_GTW_GE_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RTC, 7}
	};

MV_BOARD_MPP_INFO	rd88f5181GtwGeInfoBoardMppConfigValue[RD_88F5181_GTW_GE_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{RD_88F5181_GTW_GE_MPP0_7,			/* mpp0_7 */
	RD_88F5181_GTW_GE_MPP8_15,
	RD_88F5181_GTW_GE_MPP16_23}}			/* mpp8_15 */
	};

MV_DEV_CS_INFO rd88f5181GtwGeInfoBoardDeCsInfo[RD_88F5181_GTW_GE_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
		{{ 3, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};	/* bootCs */                    

MV_BOARD_INFO rd88f5181GtwGeInfo = {
	"RD-88F5181-GTW-GE",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F5181_GTW_GE_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88f5181GtwGeInfoBoardMppConfigValue,
	((1 << 4)|(1 << 5)|(1 << 8)),				/* intsGppMask */
	RD_88F5181_GTW_GE_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	rd88f5181GtwGeInfoBoardDeCsInfo,
	RD_88F5181_GTW_GE_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	rd88f5181GtwGeInfoBoardPciIf,
	RD_88F5181_GTW_GE_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	rd88f5181GtwGeInfoBoardTwsiDev,					
	RD_88F5181_GTW_GE_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	rd88f5181GtwGeInfoBoardMacInfo,
	RD_88F5181_GTW_GE_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	rd88f5181GtwGeInfoBoardGppInfo,
	RD_88F5181_GTW_GE_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	rd88f5181GtwGeInfoBoardDebugLedIf,
	0,						/* ledsPolarity */		
	RD_88F5181_GTW_GE_OE,				/* gppOutEnVal */
	RD_88F5181_GTW_GE_OUT_VAL,			/* gppOutVal */
    	0x530,		                              	/* gppPolarity */
    	rd88f5181GtwGeInfoBoardSwitchInfo		/* pSwitchInfo */
};




#define DB_88F5181_5281_DDR1_BOARD_PCI_IF_NUM		0x3
#define DB_88F5181_5281_DDR1_BOARD_TWSI_DEF_NUM		0x1
#define DB_88F5181_5281_DDR1_BOARD_MAC_INFO_NUM		0x1
#define DB_88F5181_5281_DDR1_BOARD_GPP_INFO_NUM		0x1
#define DB_88F5181_5281_DDR1_BOARD_DEBUG_LED_NUM	0x0
#define DB_88F5181_5281_DDR1_BOARD_MPP_CONFIG_NUM	0x1
#define DB_88F5181_5281_DDR1_BOARD_DEVICE_CONFIG_NUM	0x3

MV_BOARD_PCI_IF db88f5181_5281ddr1InfoBoardPciIf[DB_88F5181_5281_DDR1_BOARD_PCI_IF_NUM] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{7, {7, 7, 7, 7}},				/* pciSlot0 */							
	 {8, {6, 6, 6, 6}},				/* pciSlot1 */                      	
	 {9, {6, 6, 6, 6}}};				/* pciSlot2 */                      

MV_BOARD_TWSI_INFO db88f5181_5281ddr1InfoBoardTwsiDev[DB_88F5181_5281_DDR1_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO db88f5181_5281ddr1InfoBoardMacInfo[DB_88F5181_5281_DDR1_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8}}; 

MV_BOARD_GPP_INFO db88f5181_5281ddr1InfoBoardGppInfo[DB_88F5181_5281_DDR1_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_USB_VBUS, 5}};

MV_BOARD_MPP_INFO db88f5181_5281ddr1InfoBoardMppConfigValue[DB_88F5181_5281_DDR1_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{DB_88F5181_5281_DDR1_MPP0_7,			/* mpp0_7 */
	DB_88F5181_5281_DDR1_MPP8_15,			/* mpp8_15 */
	DB_88F5181_5281_DDR1_MPP16_23,			/* mpp16_23 */
	DB_88F5181_5281_DDR1_MPP_DEV}}};		/* mppDev */						

MV_DEV_CS_INFO db88f5181_5281ddr1InfoBoardDeCsInfo[DB_88F5181_5281_DDR1_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
		{{1, 0x8fefffff, BOARD_DEV_NOR_FLASH, 16},   	/* devCs1 */                    
		{2, 0x8fcfffff, BOARD_DEV_SEVEN_SEG, N_A},	/* devCs2 */            
		{3, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};	/* bootCs */                    

MV_BOARD_INFO db88f5181_5281ddr1Info = {
	"DB-88F5181-DDR1",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F5181_5281_DDR1_BOARD_MPP_CONFIG_NUM,	/* numBoardMppConfig */
	db88f5181_5281ddr1InfoBoardMppConfigValue,
	((1 << 6)|(1 << 7)),				/* intsGppMask */
	DB_88F5181_5281_DDR1_BOARD_DEVICE_CONFIG_NUM,	/* numBoardDevIf */
	db88f5181_5281ddr1InfoBoardDeCsInfo,
	DB_88F5181_5281_DDR1_BOARD_PCI_IF_NUM,		/* numBoardPciIf */
	db88f5181_5281ddr1InfoBoardPciIf,
	DB_88F5181_5281_DDR1_BOARD_TWSI_DEF_NUM,	/* numBoardTwsiDev */
	db88f5181_5281ddr1InfoBoardTwsiDev,					
	DB_88F5181_5281_DDR1_BOARD_MAC_INFO_NUM,	/* numBoardMacInfo */
	db88f5181_5281ddr1InfoBoardMacInfo,
	DB_88F5181_5281_DDR1_BOARD_GPP_INFO_NUM,	/* numBoardGppInfo */
	db88f5181_5281ddr1InfoBoardGppInfo,
	DB_88F5181_5281_DDR1_BOARD_DEBUG_LED_NUM,	/* activeLedsNumber */              
	NULL,
	N_A,						/* ledsPolarity */		
	DB_88F5181_5281_DDR1_OE,			/* gppOutEnVal */
	DB_88F5181_5281_DDR1_OUT_VAL,			/* gppOutVal */
	N_A,						/* gppPolarityVal */
	NULL						/* pSwitchInfo */
};



#define DB_88F5181_5281_DDR2_BOARD_PCI_IF_NUM		0x3
#define DB_88F5181_5281_DDR2_BOARD_TWSI_DEF_NUM		0x1
#define DB_88F5181_5281_DDR2_BOARD_MAC_INFO_NUM		0x1
#define DB_88F5181_5281_DDR2_BOARD_GPP_INFO_NUM		0x3
#define DB_88F5181_5281_DDR2_BOARD_DEBUG_LED_NUM	0x4
#define DB_88F5181_5281_DDR2_BOARD_MPP_CONFIG_NUM	0x1
#define DB_88F5181_5281_DDR2_BOARD_DEVICE_CONFIG_NUM	0x2

MV_U8	db88f5181_5281ddr2InfoBoardDebugLedIf[DB_88F5181_5281_DDR2_BOARD_DEBUG_LED_NUM] =
	{14, 15, 6, 7};

MV_BOARD_PCI_IF db88f5181_5281ddr2InfoBoardPciIf[DB_88F5181_5281_DDR2_BOARD_PCI_IF_NUM] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{7, {12, 12, 12, 12}},				/* pciSlot0 */							
	 {8, {13, 13, 13, 13}},				/* pciSlot1 */                      	
	 {9, {13, 13, 13, 13}}};			/* pciSlot2 */                      

MV_BOARD_TWSI_INFO db88f5181_5281ddr2InfoBoardTwsiDev[DB_88F5181_5281_DDR2_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO db88f5181_5281ddr2InfoBoardMacInfo[DB_88F5181_5281_DDR2_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8}}; 

MV_BOARD_GPP_INFO db88f5181_5281ddr2InfoBoardGppInfo[DB_88F5181_5281_DDR2_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RTC, 10},
	{BOARD_DEV_USB_VBUS, 1},
	{BOARD_DEV_REF_CLCK,11}};

MV_BOARD_MPP_INFO db88f5181_5281ddr2InfoBoardMppConfigValue[DB_88F5181_5281_DDR2_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{DB_88F5181_5281_DDR2_MPP0_7,			/* mpp0_7 */
	DB_88F5181_5281_DDR2_MPP8_15,			/* mpp8_15 */
	DB_88F5181_5281_DDR2_MPP16_23,			/* mpp16_23 */
	DB_88F5181_5281_DDR2_MPP_DEV}}};		/* mppDev */						

MV_DEV_CS_INFO db88f5181_5281ddr2InfoBoardDeCsInfo[DB_88F5181_5281_DDR2_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
		{{1, 0x8fefffff, BOARD_DEV_NOR_FLASH, 16},   	/* devCs1 */                    
		{3, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};		/* bootCs */                    

MV_BOARD_INFO db88f5181_5281ddr2Info = {
	"DB-88F5181-DDR2",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F5181_5281_DDR2_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f5181_5281ddr2InfoBoardMppConfigValue,
	((1 << 10)|(1 << 12)|(1 << 13)),			/* intsGppMask */
	DB_88F5181_5281_DDR2_BOARD_DEVICE_CONFIG_NUM,	/* numBoardDevIf */
	db88f5181_5281ddr2InfoBoardDeCsInfo,
	DB_88F5181_5281_DDR2_BOARD_PCI_IF_NUM,		/* numBoardPciIf */
	db88f5181_5281ddr2InfoBoardPciIf,
	DB_88F5181_5281_DDR2_BOARD_TWSI_DEF_NUM,	/* numBoardTwsiDev */
	db88f5181_5281ddr2InfoBoardTwsiDev,					
	DB_88F5181_5281_DDR2_BOARD_MAC_INFO_NUM,	/* numBoardMacInfo */
	db88f5181_5281ddr2InfoBoardMacInfo,
	DB_88F5181_5281_DDR2_BOARD_GPP_INFO_NUM,	/* numBoardGppInfo */
	db88f5181_5281ddr2InfoBoardGppInfo,
	DB_88F5181_5281_DDR2_BOARD_DEBUG_LED_NUM,	/* activeLedsNumber */              
	db88f5181_5281ddr2InfoBoardDebugLedIf,
	1,						/* ledsPolarity */		
	DB_88F5181_5281_DDR2_OE,			/* gppOutEnVal */
	DB_88F5181_5281_DDR2_OUT_VAL,			/* gppOutVal */
	N_A,						/* gppPolarityVal */
	NULL						/* pSwitchInfo */
};


#define DB_88F5181_DDR1_MNG_BOARD_PCI_IF_NUM		0x1
#define DB_88F5181_DDR1_MNG_BOARD_TWSI_DEF_NUM		0x1
#define DB_88F5181_DDR1_MNG_BOARD_MAC_INFO_NUM		0x1
#define DB_88F5181_DDR1_MNG_BOARD_GPP_INFO_NUM		0x1
#define DB_88F5181_DDR1_MNG_BOARD_DEBUG_LED_NUM		0x3
#define DB_88F5181_DDR1_MNG_BOARD_MPP_CONFIG_NUM	0x1
#define DB_88F5181_DDR1_MNG_BOARD_DEVICE_CONFIG_NUM	0x2

MV_BOARD_PCI_IF db88f5181ddr1MngInfoBoardPciIf[] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{2, {7, 7, 7, 7}}};				/* pciSlot0 */							

MV_U8		db88f5181ddr1MngInfoBoardDebugLedIf[DB_88F5181_DDR1_MNG_BOARD_DEBUG_LED_NUM] =
	{12, 13, 14};

MV_BOARD_TWSI_INFO db88f5181ddr1MngInfoBoardTwsiDev[DB_88F5181_DDR1_MNG_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO db88f5181ddr1MngInfoBoardMacInfo[DB_88F5181_DDR1_MNG_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x1F}}; 

MV_BOARD_GPP_INFO db88f5181ddr1MngInfoBoardGppInfo[DB_88F5181_DDR1_MNG_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_REF_CLCK,0}};

MV_BOARD_MPP_INFO db88f5181ddr1MngInfoBoardMppConfigValue[DB_88F5181_DDR1_MNG_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{DB_88F5181_DDR1_MNG_MPP0_7,			/* mpp0_7 */
	DB_88F5181_DDR1_MNG_MPP8_15,			/* mpp8_15 */
	DB_88F5181_DDR1_MNG_MPP16_23,			/* mpp16_23 */
	DB_88F5181_DDR1_MNG_MPP_DEV}}};						/* mppDev */						

MV_DEV_CS_INFO db88f5181ddr1MngInfoBoardDeCsInfo[DB_88F5181_DDR1_MNG_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
		{{1, 0x8fefffff, BOARD_DEV_NOR_FLASH, 16},   	/* devCs1 */                    
		{3, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};	/* bootCs */                    

MV_BOARD_INFO db88f5181ddr1MngInfo = {
	"DB-88F5181-DDR1-MNG",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F5181_DDR1_MNG_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f5181ddr1MngInfoBoardMppConfigValue,
	((1 << 3)|(1 << 6)|(1 << 7)),			/* intsGppMask */
	DB_88F5181_DDR1_MNG_BOARD_DEVICE_CONFIG_NUM,	/* numBoardDevIf */
	db88f5181ddr1MngInfoBoardDeCsInfo,
	DB_88F5181_DDR1_MNG_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	db88f5181ddr1MngInfoBoardPciIf,
	DB_88F5181_DDR1_MNG_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f5181ddr1MngInfoBoardTwsiDev,					
	DB_88F5181_DDR1_MNG_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	db88f5181ddr1MngInfoBoardMacInfo,
	DB_88F5181_DDR1_MNG_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	db88f5181ddr1MngInfoBoardGppInfo,
	DB_88F5181_DDR1_MNG_BOARD_DEBUG_LED_NUM,		/* activeLedsNumber */              
	db88f5181ddr1MngInfoBoardDebugLedIf,
	0,						/* ledsPolarity */		
	DB_88F5181_DDR1_MNG_GPP_OE,			/* gppOutEnVal */
	DB_88F5181_DDR1_MNG_GPP_OUT_VAL,		/* gppOutVal */
	N_A,						/* gppPolarityVal */
	NULL						/* pSwitchInfo */
};

#define DB_88F5X8X_BOARD_PCI_IF_NUM		0x0
#define DB_88F5X8X_BOARD_TWSI_DEF_NUM		0x1
#define DB_88F5X8X_BOARD_MAC_INFO_NUM		0x0
#define DB_88F5X8X_BOARD_GPP_INFO_NUM		0x1
#define DB_88F5X8X_BOARD_DEBUG_LED_NUM		0x4
#define DB_88F5X8X_BOARD_MPP_CONFIG_NUM		0x0
#define DB_88F5X8X_BOARD_DEVICE_CONFIG_NUM	0x2

MV_BOARD_PCI_IF db88f5x8xddr2InfoBoardPciIf[] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{7, {6, 6, 6, 6}},				/* pciSlot0 */							
	 {8, {7, 7, 7, 7}},				/* pciSlot1 */                      	
	 {9, {7, 7, 7, 7}}};			/* pciSlot2 */                      

MV_BOARD_TWSI_INFO db88f5x8xddr2InfoBoardTwsiDev[DB_88F5X8X_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO db88f5x8xddr2InfoBoardMacInfo[] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8}}; 

MV_BOARD_GPP_INFO db88f5x8xddr2InfoBoardGppInfo[] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RTC, 10},
	{BOARD_DEV_USB_VBUS, 1},
	{BOARD_DEV_REF_CLCK,11}};

MV_BOARD_MPP_INFO db88f5x8xddr2InfoBoardMppConfigValue[] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{DB_88F5X8X_DDRX_MPP0_7,			/* mpp0_7 */
	DB_88F5X8X_DDRX_MPP8_15,			/* mpp8_15 */
	DB_88F5X8X_DDRX_MPP16_23,			/* mpp16_23 */
	DB_88F5X8X_DDRX_MPP_DEV}}			/* mppDev */						
	};

MV_DEV_CS_INFO db88f5x8xddr2InfoBoardDeCsInfo[DB_88F5X8X_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
		{{0, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}, 	/* devCs0 */                       
		{ 3, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};	/* bootCs */                    

MV_BOARD_INFO db88f5x8xFpgaddr2Info = {
	"DB-88F5X8X-FPGA-DDR1-A",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F5X8X_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	NULL,
	((1<<6)|(1 << 7)),				/* intsGppMask */
	DB_88F5X8X_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f5x8xddr2InfoBoardDeCsInfo,
	DB_88F5X8X_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	NULL,
	DB_88F5X8X_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f5x8xddr2InfoBoardTwsiDev,					
	DB_88F5X8X_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	NULL,
	DB_88F5X8X_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	db88f5x8xddr2InfoBoardGppInfo,
	DB_88F5X8X_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	NULL,
	N_A,						/* ledsPolarity */		
	DB_88F5X8X_DDRX_OE,				/* gppOutEnVal */
	DB_88F5X8X_DDRX_OUT_VAL,			/* gppOutVal */
	N_A,						/* gppPolarityVal */
	NULL						/* pSwitchInfo */
};

MV_BOARD_INFO*	boardInfoTbl[] = 	{NULL,	 			/* DB_88F1181_DDR1 - OBSOLETE */
					NULL,				/* DB_88F1181_DDR2 - OBSOLETE */
					&db88f5181_5281ddr1Info,	/* DB_88F5181_5281_DDR1 */
					&db88f5181_5281ddr2Info,	/* DB_88F5181_5281_DDR2 */
					&db88f5181prpmcInfo,		/* DB_88F5181_DDR1_PRPMC */
					&db88f5181pexPciInfo,		/* DB_88F5181_DDR1_PEXPCI */
					&rd88f5181posNasInfo,		/* RD_88F5181_POS_NAS */
					&db88f5x81ddr2Info,		/* DB_88F5X81_DDR2 */
					&db88f5x81ddr1Info,		/* DB_88F5X81_DDR1 */
					&rd88f5181voipInfo,		/* RD_88F5181_VOIP - OBSOLETE */
					&db88f5181ddr1MngInfo,		/* DB_88F5181_DDR1_MNG */
					&rd88f5181GtwFeInfo,		/* RD_88F5181_GTW_FE */
					&rd88f5181GtwGeInfo,		/* RD_88F5181_GTW_GE */
					&db88f5x8xFpgaddr2Info		/* DB_88F5X8X_FPGA_DDR2 */
					};

#define	BOARD_ID_BASE				BOARD_ID_88F5181_5281_BASE
#define MV_MAX_BOARD_ID				BOARD_ID_88F5181_5281_MAX

