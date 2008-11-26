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




#define DB_88F5181L_BOARD_PCI_IF_NUM		0x3
#define DB_88F5181L_BOARD_TWSI_DEF_NUM		0x1
#define DB_88F5181L_BOARD_MAC_INFO_NUM		0x1
#define DB_88F5181L_BOARD_GPP_INFO_NUM		0x3
#define DB_88F5181L_BOARD_DEBUG_LED_NUM		0x0
#define DB_88F5181L_BOARD_MPP_CONFIG_NUM	0x1
#define DB_88F5181L_BOARD_DEVICE_CONFIG_NUM	0x4

MV_BOARD_PCI_IF 	db88f5181Lddr2InfoBoardPciIf[DB_88F5181L_BOARD_PCI_IF_NUM] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{7, {12, 12, 12, 12}},				/* pciSlot0 */							
	 {8, {13, 13, 13, 13}},				/* pciSlot1 */                      	
	 {9, {13, 13, 13, 13}}};			/* pciSlot2 */                      

MV_BOARD_TWSI_INFO	db88f5181Lddr2InfoBoardTwsiDev[DB_88F5181L_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO	db88f5181Lddr2InfoBoardMacInfo[DB_88F5181L_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8}}; 

MV_BOARD_GPP_INFO	db88f5181Lddr2InfoBoardGppInfo[DB_88F5181L_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RTC, 10},
	{BOARD_DEV_USB_VBUS, 1},
	{BOARD_DEV_REF_CLCK, 11}};

MV_BOARD_MPP_INFO	db88f5181Lddr2InfoBoardMppConfigValue[DB_88F5181L_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{DB_88F5181L_DDR2_2XTDM_MPP0_7,			/* mpp0_7 */
	DB_88F5181L_DDR2_2XTDM_MPP8_15,				/* mpp8_15 */
	N_A,							/* mpp16_23 */
	N_A}}							/* mppDev */						
	};

MV_DEV_CS_INFO db88f5181Lddr2InfoBoardDeCsInfo[DB_88F5181L_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
		{{0, 0x8fcfffff, BOARD_DEV_SEVEN_SEG, N_A}, 	/* devCs0 */                       
		{1, 0x8fdfffff, BOARD_DEV_NOR_FLASH, 16},   	/* devCs1 */                    
		{2, 0x8fcfffff, BOARD_DEV_NAND_FLASH, 8},		/* devCs2/flashCs */            
		{3, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};		/* bootCs */                    

MV_BOARD_INFO db88f5181Lddr2Info = {
	"DB-88F5181L-DDR2-2xTDM",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F5181L_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f5181Lddr2InfoBoardMppConfigValue,
	((1<<8)|(1 << 9)|(1<<10)|(1<<12)|(1<<13)),	/* intsGppMask */
	DB_88F5181L_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f5181Lddr2InfoBoardDeCsInfo,
	DB_88F5181L_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	db88f5181Lddr2InfoBoardPciIf,
	DB_88F5181L_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f5181Lddr2InfoBoardTwsiDev,					
	DB_88F5181L_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	db88f5181Lddr2InfoBoardMacInfo,
	DB_88F5181L_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	db88f5181Lddr2InfoBoardGppInfo,
	DB_88F5181L_BOARD_DEBUG_LED_NUM,		/* activeLedsNumber */              
	NULL,
	N_A,						/* ledsPolarity */		
	DB_88F5181L_DDR2_2XTDM_OE,			/* gppOutEnVal */
	DB_88F5181L_DDR2_2XTDM_OUT_VAL,			/* gppOutVal */
	0x3700,						/* gppPolarityVal */
	NULL						/* pSwitchInfo */
};



#define RD_88F5181L_VOIP_FE_BOARD_PCI_IF_NUM		0x1
#define RD_88F5181L_VOIP_FE_BOARD_TWSI_DEF_NUM		0x1
#define RD_88F5181L_VOIP_FE_BOARD_MAC_INFO_NUM		0x1
#define RD_88F5181L_VOIP_FE_BOARD_GPP_INFO_NUM		0x3
#define RD_88F5181L_VOIP_FE_BOARD_DEBUG_LED_NUM		0x3
#define RD_88F5181L_VOIP_FE_BOARD_MPP_CONFIG_NUM	0x1
#define RD_88F5181L_VOIP_FE_BOARD_DEVICE_CONFIG_NUM	0x1

MV_U8			rd88f5181LvoipFeInfoBoardDebugLedIf[RD_88F5181L_VOIP_FE_BOARD_DEBUG_LED_NUM] =
	{13, 14, 15};

MV_BOARD_PCI_IF 	rd88f5181LvoipFeInfoBoardPciIf[RD_88F5181L_VOIP_FE_BOARD_PCI_IF_NUM] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{7, {4, 3, N_A, N_A}}};			/* pciSlot0 */							

MV_BOARD_TWSI_INFO	rd88f5181LvoipFeInfoBoardTwsiDev[RD_88F5181L_VOIP_FE_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO	rd88f5181LvoipFeInfoBoardMacInfo[RD_88F5181L_VOIP_FE_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_100M, 0x0}}; 

MV_BOARD_SWITCH_INFO	rd88f5181LvoipFeInfoBoardSwitchInfo[] = 
	/* MV_32 linkStatusIrq, {MV_32 qdPort0, MV_32 qdPort1, MV_32 qdPort2, MV_32 qdPort3, MV_32 qdPort4}, 
		MV_32 qdCpuPort, MV_32 smiScanMode} */
	{{-1, {0, 1, 2, 3, 4}, 5, 0}};

MV_BOARD_GPP_INFO	rd88f5181LvoipFeInfoBoardGppInfo[RD_88F5181L_VOIP_FE_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RTC, 11}};

MV_BOARD_MPP_INFO	rd88f5181LvoipFeInfoBoardMppConfigValue[RD_88F5181L_VOIP_FE_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{RD_88F5181L_VOIP_FE_MPP0_7,				/* mpp0_7 */
	RD_88F5181L_VOIP_FE_MPP8_15,				/* mpp8_15 */
	N_A,							/* mpp16_23 */
	N_A}}							/* mppDev */						
	};

MV_DEV_CS_INFO rd88f5181LvoipFeInfoBoardDeCsInfo[RD_88F5181L_VOIP_FE_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
		{{3, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};	/* bootCs */                    

MV_BOARD_INFO rd88f5181LvoipFeInfo = {
	"RD-88F5181L-VOIP-FE",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F5181L_VOIP_FE_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88f5181LvoipFeInfoBoardMppConfigValue,
	((1<<2)|(1 << 3)|(1<<4)|(1<<5)|(1<<9)|(1<<11)),	/* intsGppMask */
	RD_88F5181L_VOIP_FE_BOARD_DEVICE_CONFIG_NUM,	/* numBoardDevIf */
	rd88f5181LvoipFeInfoBoardDeCsInfo,
	RD_88F5181L_VOIP_FE_BOARD_PCI_IF_NUM,		/* numBoardPciIf */
	rd88f5181LvoipFeInfoBoardPciIf,
	RD_88F5181L_VOIP_FE_BOARD_TWSI_DEF_NUM,		/* numBoardTwsiDev */
	rd88f5181LvoipFeInfoBoardTwsiDev,					
	RD_88F5181L_VOIP_FE_BOARD_MAC_INFO_NUM,		/* numBoardMacInfo */
	rd88f5181LvoipFeInfoBoardMacInfo,
	RD_88F5181L_VOIP_FE_BOARD_GPP_INFO_NUM,		/* numBoardGppInfo */
	rd88f5181LvoipFeInfoBoardGppInfo,
	RD_88F5181L_VOIP_FE_BOARD_DEBUG_LED_NUM,	/* activeLedsNumber */              
	rd88f5181LvoipFeInfoBoardDebugLedIf,
	0,						/* ledsPolarity */		
	RD_88F5181L_VOIP_FE_GPP_OE,			/* gppOutEnVal */
	RD_88F5181L_VOIP_FE_GPP_IO,			/* gppOutVal */
	N_A,						/* gppPolarityVal */
	rd88f5181LvoipFeInfoBoardSwitchInfo		/* pSwitchInfo */
};



#define RD_88F5181L_VOIP_GE_BOARD_PCI_IF_NUM		0x1
#define RD_88F5181L_VOIP_GE_BOARD_TWSI_DEF_NUM		0x1
#define RD_88F5181L_VOIP_GE_BOARD_MAC_INFO_NUM		0x1
#define RD_88F5181L_VOIP_GE_BOARD_GPP_INFO_NUM		0x3
#define RD_88F5181L_VOIP_GE_BOARD_DEBUG_LED_NUM		0x4
#define RD_88F5181L_VOIP_GE_BOARD_MPP_CONFIG_NUM	0x1
#define RD_88F5181L_VOIP_GE_BOARD_DEVICE_CONFIG_NUM	0x1

MV_U8			rd88f5181LvoipGeInfoBoardDebugLedIf[RD_88F5181L_VOIP_GE_BOARD_DEBUG_LED_NUM] =
	{1, 2, 3, 0};

MV_BOARD_PCI_IF 	rd88f5181LvoipGeInfoBoardPciIf[RD_88F5181L_VOIP_GE_BOARD_PCI_IF_NUM] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{7, {4, 10, N_A, N_A}}};			/* pciSlot0 */							

MV_BOARD_TWSI_INFO	rd88f5181LvoipGeInfoBoardTwsiDev[RD_88F5181L_VOIP_GE_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO	rd88f5181LvoipGeInfoBoardMacInfo[RD_88F5181L_VOIP_GE_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_1000M, 0x0}}; 

MV_BOARD_SWITCH_INFO	rd88f5181LvoipGeInfoBoardSwitchInfo[] = 
	/* MV_32 linkStatusIrq, {MV_32 qdPort0, MV_32 qdPort1, MV_32 qdPort2, MV_32 qdPort3, MV_32 qdPort4}, 
		MV_32 qdCpuPort, MV_32 smiScanMode} */
	{{8, {2, 1, 0, 7, 5}, 3, 1}};

MV_BOARD_GPP_INFO	rd88f5181LvoipGeInfoBoardGppInfo[RD_88F5181L_VOIP_GE_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RTC, 5}};

MV_BOARD_MPP_INFO	rd88f5181LvoipGeInfoBoardMppConfigValue[RD_88F5181L_VOIP_GE_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{RD_88F5181L_VOIP_GE_MPP0_7,				/* mpp0_7 */
	RD_88F5181L_VOIP_GE_MPP8_15,				/* mpp8_15 */
	RD_88F5181L_VOIP_GE_MPP16_23,				/* mpp16_23 */
	N_A}}							/* mppDev */						
	};

MV_DEV_CS_INFO rd88f5181LvoipGeInfoBoardDeCsInfo[RD_88F5181L_VOIP_GE_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
		{{3, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};	/* bootCs */                    

MV_BOARD_INFO rd88f5181LvoipGeInfo = {
	"RD-88F5181L-VOIP-GE",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F5181L_VOIP_GE_BOARD_MPP_CONFIG_NUM,	/* numBoardMppConfig */
	rd88f5181LvoipGeInfoBoardMppConfigValue,
	((1<<4)|(1 << 5)|(1<<8)),			/* intsGppMask */
	RD_88F5181L_VOIP_GE_BOARD_DEVICE_CONFIG_NUM,	/* numBoardDevIf */
	rd88f5181LvoipGeInfoBoardDeCsInfo,
	RD_88F5181L_VOIP_GE_BOARD_PCI_IF_NUM,		/* numBoardPciIf */
	rd88f5181LvoipGeInfoBoardPciIf,
	RD_88F5181L_VOIP_GE_BOARD_TWSI_DEF_NUM,		/* numBoardTwsiDev */
	rd88f5181LvoipGeInfoBoardTwsiDev,					
	RD_88F5181L_VOIP_GE_BOARD_MAC_INFO_NUM,		/* numBoardMacInfo */
	rd88f5181LvoipGeInfoBoardMacInfo,
	RD_88F5181L_VOIP_GE_BOARD_GPP_INFO_NUM,		/* numBoardGppInfo */
	rd88f5181LvoipGeInfoBoardGppInfo,
	RD_88F5181L_VOIP_GE_BOARD_DEBUG_LED_NUM,	/* activeLedsNumber */              
	rd88f5181LvoipGeInfoBoardDebugLedIf,
	0,						/* ledsPolarity */		
	RD_88F5181L_VOIP_GE_GPP_OE,			/* gppOutEnVal */
	RD_88F5181L_VOIP_GE_GPP_IO,			/* gppOutVal */
	N_A,						/* gppPolarityVal */
	rd88f5181LvoipGeInfoBoardSwitchInfo		/* pSwitchInfo */
};


#define RD_88F5181L_FXO_GE_BOARD_PCI_IF_NUM		0x1
#define RD_88F5181L_FXO_GE_BOARD_TWSI_DEF_NUM		0x1
#define RD_88F5181L_FXO_GE_BOARD_MAC_INFO_NUM		0x1
#define RD_88F5181L_FXO_GE_BOARD_GPP_INFO_NUM		0x3
#define RD_88F5181L_FXO_GE_BOARD_DEBUG_LED_NUM		0x1
#define RD_88F5181L_FXO_GE_BOARD_MPP_CONFIG_NUM		0x3
#define RD_88F5181L_FXO_GE_BOARD_DEVICE_CONFIG_NUM	0x1

/* Note the last LED's will be use for init and Linux heartbeat */
MV_U8			rd88f5181LFXOGeInfoBoardDebugLedIf[RD_88F5181L_FXO_GE_BOARD_DEBUG_LED_NUM] =
	{0};

MV_BOARD_PCI_IF 	rd88f5181LFXOGeInfoBoardPciIf[RD_88F5181L_FXO_GE_BOARD_PCI_IF_NUM] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{7, {1, 1, N_A, N_A}}};			/* pciSlot0 */							

MV_BOARD_TWSI_INFO	rd88f5181LFXOGeInfoBoardTwsiDev[RD_88F5181L_FXO_GE_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO	rd88f5181LFXOGeInfoBoardMacInfo[RD_88F5181L_FXO_GE_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_1000M, 0x0}}; 

MV_BOARD_SWITCH_INFO rd88f5181LFXOGeInfoBoardSwitchInfo[] = 
	/* MV_32 linkStatusIrq, {MV_32 qdPort0, MV_32 qdPort1, MV_32 qdPort2, MV_32 qdPort3, MV_32 qdPort4}, 
		MV_32 qdCpuPort, MV_32 smiScanMode} */
	{{-1, {2, 1, 0, 7, 5}, 3, 1}};

MV_BOARD_GPP_INFO	rd88f5181LFXOGeInfoBoardGppInfo[RD_88F5181L_FXO_GE_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RTC, 5}};

MV_BOARD_MPP_INFO	rd88f5181LFXOGeInfoBoardMppConfigValue[RD_88F5181L_FXO_GE_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{RD_88F5181L_FXO_GE_MPP0_7,				/* mpp0_7 */
	   RD_88F5181L_FXO_GE_MPP8_15,				/* mpp8_15 */
	   RD_88F5181L_FXO_GE_MPP16_23,				/* mpp16_23 */
	   N_A}}							/* mppDev */					
	};

MV_DEV_CS_INFO rd88f5181LFXOGeInfoBoardDeCsInfo[RD_88F5181L_FXO_GE_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
		{{3,0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};	/* bootCs */                    

MV_BOARD_INFO rd88f5181LFXOGeInfo = {
	"RD-88F5181L-VOIP-FXO-GE",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F5181L_FXO_GE_BOARD_MPP_CONFIG_NUM,	/* numBoardMppConfig */
	rd88f5181LFXOGeInfoBoardMppConfigValue,
	((1 << 1)|(1 << 3)|(1 << 8)|(1 << 11)),				/* intsGppMask */
	RD_88F5181L_FXO_GE_BOARD_DEVICE_CONFIG_NUM,	/* numBoardDevIf */
	rd88f5181LFXOGeInfoBoardDeCsInfo,
	RD_88F5181L_FXO_GE_BOARD_PCI_IF_NUM,		/* numBoardPciIf */
	rd88f5181LFXOGeInfoBoardPciIf,
	RD_88F5181L_FXO_GE_BOARD_TWSI_DEF_NUM,		/* numBoardTwsiDev */
	rd88f5181LFXOGeInfoBoardTwsiDev,					
	RD_88F5181L_FXO_GE_BOARD_MAC_INFO_NUM,		/* numBoardMacInfo */
	rd88f5181LFXOGeInfoBoardMacInfo,
	RD_88F5181L_FXO_GE_BOARD_GPP_INFO_NUM,		/* numBoardGppInfo */
	rd88f5181LFXOGeInfoBoardGppInfo,		/* pBoardGppInfo */
	RD_88F5181L_FXO_GE_BOARD_DEBUG_LED_NUM,	/* activeLedsNumber */              
	rd88f5181LFXOGeInfoBoardDebugLedIf,		/* ledsPolarity */
	0,						/* ledsPolarity */		
	RD_88F5181L_FXO_GE_GPP_OE,			/* gppOutEnVal */
	RD_88F5181L_FXO_GE_GPP_IO,			/* gppOutVal */
	0x2,						/* gppPolarityVal */
	rd88f5181LFXOGeInfoBoardSwitchInfo		/* pSwitchInfo */
};

MV_BOARD_INFO*	boardInfoTbl[4] =	{&db88f5181Lddr2Info,
					&rd88f5181LvoipFeInfo,
					&rd88f5181LvoipGeInfo,
					&rd88f5181LFXOGeInfo
					};

#define	BOARD_ID_BASE				BOARD_ID_88F5181L_BASE
#define MV_MAX_BOARD_ID				BOARD_ID_88F5181L_MAX


