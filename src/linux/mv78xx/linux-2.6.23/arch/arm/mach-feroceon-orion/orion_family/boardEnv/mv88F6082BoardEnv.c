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

#define DB_88F6082BP_BOARD_PCI_IF_NUM		0x0
#define DB_88F6082BP_BOARD_TWSI_DEF_NUM		0x1
#define DB_88F6082BP_BOARD_MAC_INFO_NUM		0x2
#define DB_88F6082BP_BOARD_GPP_INFO_NUM		0x1
#define DB_88F6082BP_BOARD_DEBUG_LED_NUM	0x1
#define DB_88F6082BP_BOARD_MPP_CONFIG_NUM	0x1
#define DB_88F6082BP_BOARD_DEVICE_CONFIG_NUM	0x1

MV_U8	db88f6082BpInfoBoardDebugLedIf[DB_88F6082BP_BOARD_DEBUG_LED_NUM] =
	{2};

MV_BOARD_TWSI_INFO	db88f6082BpInfoBoardTwsiDev[DB_88F6082BP_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO db88f6082BpInfoBoardMacInfo[DB_88F6082BP_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8},
	{BOARD_MAC_SPEED_AUTO, 0x9},
	}; 

MV_BOARD_GPP_INFO	db88f6082BpInfoBoardGppInfo[DB_88F6082BP_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RESET, 7}};

MV_BOARD_MPP_INFO	db88f6082BpInfoBoardMppConfigValue[DB_88F6082BP_BOARD_MPP_CONFIG_NUM] = 
#if !defined(MV_NAND) && !defined(MV_NAND_BOOT)
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{DB_88F6082_BP_MPP0_7,			/* mpp0_7 */
	DB_88F6082_BP_MPP8_15,				/* mpp8_15 */
	N_A,						/* mpp16_23 */
	N_A}}						/* mppDev */						
	};
#else
	{{{DB_88F6082_BP_MPP0_7_NB,			/* mpp0_7 */
	DB_88F6082_BP_MPP8_15_NB,			/* mpp8_15 */
	N_A,						/* mpp16_23 */
	N_A}}						/* mppDev */						
	};
#endif

#if defined(MV_NAND) || defined(MV_NAND_BOOT)
MV_DEV_CS_INFO db88f6082BpInfoBoardDeCsInfo[DB_88F6082BP_BOARD_DEVICE_CONFIG_NUM] = 
		/*{deviceCS, params, devType, devWidth}*/			   
		{{0, 0x8fcfffff, BOARD_DEV_NAND_FLASH, 8}}; /* NAND DEV */
#else
MV_DEV_CS_INFO db88f6082BpInfoBoardDeCsInfo[DB_88F6082BP_BOARD_DEVICE_CONFIG_NUM] = 
		/*{deviceCS, params, devType, devWidth}*/			   
		 {{2, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* SPI DEV */
#endif

MV_BOARD_INFO db88f6082BpInfo = {
	"DB-88F6082-BP",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F6082BP_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f6082BpInfoBoardMppConfigValue,
	0,						/* intsGppMask */
	DB_88F6082BP_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f6082BpInfoBoardDeCsInfo,
	DB_88F6082BP_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	NULL,
	DB_88F6082BP_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f6082BpInfoBoardTwsiDev,					
	DB_88F6082BP_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	db88f6082BpInfoBoardMacInfo,
	DB_88F6082BP_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	db88f6082BpInfoBoardGppInfo,
	DB_88F6082BP_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	db88f6082BpInfoBoardDebugLedIf,
	N_A,						/* ledsPolarity */		
	DB_88F6082_BP_OE,				/* gppOutEnVal */
	DB_88F6082_BP_OE_VAL,				/* gppOutVal */
	0x1						/* gppPolarityVal */
};


#define DB_88F6082LBP_BOARD_PCI_IF_NUM		0x0
#define DB_88F6082LBP_BOARD_TWSI_DEF_NUM	0x1
#define DB_88F6082LBP_BOARD_MAC_INFO_NUM	0x2
#define DB_88F6082LBP_BOARD_GPP_INFO_NUM	0x1
#define DB_88F6082LBP_BOARD_DEBUG_LED_NUM	0x2
#define DB_88F6082LBP_BOARD_MPP_CONFIG_NUM	0x1
#define DB_88F6082LBP_BOARD_DEVICE_CONFIG_NUM	0x1

MV_U8	db88f6082LBpInfoBoardDebugLedIf[DB_88F6082LBP_BOARD_DEBUG_LED_NUM] =
	{1, 2};

MV_BOARD_TWSI_INFO	db88f6082LBpInfoBoardTwsiDev[DB_88F6082LBP_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO db88f6082LBpInfoBoardMacInfo[DB_88F6082LBP_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8},
	{BOARD_MAC_SPEED_AUTO, 0x9},
	}; 

MV_BOARD_GPP_INFO	db88f6082LBpInfoBoardGppInfo[DB_88F6082LBP_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RESET, 7}};

MV_BOARD_MPP_INFO	db88f6082LBpInfoBoardMppConfigValue[DB_88F6082LBP_BOARD_MPP_CONFIG_NUM] =
#ifndef  MV_NAND_BOOT
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{DB_88F6082L_BP_MPP0_7,			/* mpp0_7 */
	DB_88F6082L_BP_MPP8_15,				/* mpp8_15 */
	N_A,						/* mpp16_23 */
	N_A}}						/* mppDev */						
	};
#else
	{{{DB_88F6082L_BP_MPP0_7_NB,			/* mpp0_7 */
	DB_88F6082L_BP_MPP8_15_NB,			/* mpp8_15 */
	N_A,						/* mpp16_23 */
	N_A}}						/* mppDev */						
	};
#endif

#if defined(MV_NAND) || defined(MV_NAND_BOOT)
MV_DEV_CS_INFO db88f6082LBpInfoBoardDeCsInfo[DB_88F6082LBP_BOARD_DEVICE_CONFIG_NUM] = 
		/*{deviceCS, params, devType, devWidth}*/			   
		{{0, 0x8fcfffff, BOARD_DEV_NAND_FLASH, 8}}; /* NAND DEV */
#else
MV_DEV_CS_INFO db88f6082LBpInfoBoardDeCsInfo[DB_88F6082LBP_BOARD_DEVICE_CONFIG_NUM] = 
		/*{deviceCS, params, devType, devWidth}*/			   
		 {{2, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* SPI DEV */
#endif

MV_BOARD_INFO db88f6082LBpInfo = {
	"DB-88F6082L-BP",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F6082LBP_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f6082LBpInfoBoardMppConfigValue,
	0,						/* intsGppMask */
	DB_88F6082LBP_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f6082LBpInfoBoardDeCsInfo,
	DB_88F6082LBP_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	NULL,
	DB_88F6082LBP_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f6082LBpInfoBoardTwsiDev,					
	DB_88F6082LBP_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	db88f6082LBpInfoBoardMacInfo,
	DB_88F6082LBP_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	db88f6082LBpInfoBoardGppInfo,
	DB_88F6082LBP_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	db88f6082LBpInfoBoardDebugLedIf,
	N_A,						/* ledsPolarity */		
	DB_88F6082L_BP_OE,				/* gppOutEnVal */
	DB_88F6082L_BP_OE_VAL,				/* gppOutVal */
	0x1						/* gppPolarityVal */
};


#define RD_88F6082NAS_BOARD_PCI_IF_NUM		0x0
#define RD_88F6082NAS_BOARD_TWSI_DEF_NUM	0x1
#define RD_88F6082NAS_BOARD_MAC_INFO_NUM	0x2
#define RD_88F6082NAS_BOARD_GPP_INFO_NUM	0x6
#define RD_88F6082NAS_BOARD_DEBUG_LED_NUM	0x0
#define RD_88F6082NAS_BOARD_MPP_CONFIG_NUM	0x1
#define RD_88F6082NAS_BOARD_DEVICE_CONFIG_NUM	0x1

MV_BOARD_TWSI_INFO	rd88f6082NasInfoBoardTwsiDev[RD_88F6082NAS_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO rd88f6082NasInfoBoardMacInfo[RD_88F6082NAS_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8},
	{BOARD_MAC_SPEED_AUTO, 0x9},
	}; 

MV_BOARD_GPP_INFO	rd88f6082NasInfoBoardGppInfo[RD_88F6082NAS_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RESET, 0},
	{BOARD_DEV_POWER_BUTTON, 2},
	{BOARD_DEV_RTC, 7},
	{BOARD_DEV_USB_VBUS, 10},
	{BOARD_DEV_HDD_POWER, 11},
	{BOARD_DEV_HDD0_POWER, 11}
	};

MV_BOARD_MPP_INFO	rd88f6082NasInfoBoardMppConfigValue[RD_88F6082NAS_BOARD_MPP_CONFIG_NUM] = 
#ifndef  MV_NAND_BOOT
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{RD_88F6082_NAS_MPP0_7,			/* mpp0_7 */
	RD_88F6082_NAS_MPP8_15,				/* mpp8_15 */
	N_A,						/* mpp16_23 */
	N_A}},						/* mppDev */						
	};
#else
	{{{RD_88F6082_NAS_MPP0_7_NB,			/* mpp0_7 */
	RD_88F6082_NAS_MPP8_15_NB,			/* mpp8_15 */
	N_A,						/* mpp16_23 */
	N_A}}						/* mppDev */						
	};
#endif

MV_DEV_CS_INFO rd88f6082NasInfoBoardDeCsInfo[RD_88F6082NAS_BOARD_DEVICE_CONFIG_NUM] =
#if defined(MV_NAND) || defined(MV_NAND_BOOT)
		/*{params, devType, devWidth}*/			   
		{{ 0, 0x8fcfffff, BOARD_DEV_NAND_FLASH, 8}}; /* NAND DEV */
#else
		/*{deviceCS, params, devType, devWidth}*/			   
		 {{2, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* SPI DEV */
#endif

MV_BOARD_INFO rd88f6082NasInfo = {
	"RD-88F6082-NAS-B",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F6082NAS_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88f6082NasInfoBoardMppConfigValue,
	0,						/* intsGppMask */
	RD_88F6082NAS_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	rd88f6082NasInfoBoardDeCsInfo,
	RD_88F6082NAS_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	NULL,
	RD_88F6082NAS_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	rd88f6082NasInfoBoardTwsiDev,					
	RD_88F6082NAS_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	rd88f6082NasInfoBoardMacInfo,
	RD_88F6082NAS_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	rd88f6082NasInfoBoardGppInfo,
	RD_88F6082NAS_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	NULL,
	N_A,						/* ledsPolarity */		
	RD_88F6082_NAS_OE,				/* gppOutEnVal */
	RD_88F6082_NAS_OE_VAL,				/* gppOutVal */
	0x0						/* gppPolarityVal */
};


#define RD_88F6082MICRO_DAS_NAS_BOARD_PCI_IF_NUM	0x0
#define RD_88F6082MICRO_DAS_NAS_BOARD_TWSI_DEF_NUM	0x1
#define RD_88F6082MICRO_DAS_NAS_BOARD_MAC_INFO_NUM	0x1
#define RD_88F6082MICRO_DAS_NAS_BOARD_GPP_INFO_NUM	0x8
#define RD_88F6082MICRO_DAS_NAS_BOARD_DEBUG_LED_NUM	0x0
#define RD_88F6082MICRO_DAS_NAS_BOARD_MPP_CONFIG_NUM	0x1
#define RD_88F6082MICRO_DAS_NAS_BOARD_DEVICE_CONFIG_NUM	0x0

MV_BOARD_TWSI_INFO	rd88f6082uDasNasInfoBoardTwsiDev[] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO rd88f6082uDasNasInfoBoardMacInfo[] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8}
	}; 

MV_BOARD_GPP_INFO	rd88f6082uDasNasInfoBoardGppInfo[] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RESET, 0},
	{BOARD_DEV_POWER_BUTTON, 15},
	{BOARD_DEV_RESTOR_BUTTON, 5},
	{BOARD_DEV_RTC, 7},
	{BOARD_DEV_USB_VBUS, 11},
	{BOARD_DEV_HDD0_POWER, 13},
	{BOARD_DEV_FAN_POWER, 12},
	{BOARD_DEV_POWER_ON_LED, 6}
	};

MV_BOARD_MPP_INFO	rd88f6082uDasNasInfoBoardMppConfigValue[] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{RD_88F6082_MICRO_DAS_NAS_MPP0_7,			/* mpp0_7 */
	RD_88F6082_MICRO_DAS_NAS_MPP8_15,			/* mpp8_15 */
	N_A,							/* mpp16_23 */
	N_A}}							/* mppDev */						
	};


MV_BOARD_INFO rd88f6082uDasNasInfo = {
	"RD-88F6082-MICRO-DAS-NAS",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F6082MICRO_DAS_NAS_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88f6082uDasNasInfoBoardMppConfigValue,
	0,							/* intsGppMask */
	RD_88F6082MICRO_DAS_NAS_BOARD_DEVICE_CONFIG_NUM,	/* numBoardDevIf */
	NULL,
	RD_88F6082MICRO_DAS_NAS_BOARD_PCI_IF_NUM,		/* numBoardPciIf */
	NULL,
	RD_88F6082MICRO_DAS_NAS_BOARD_TWSI_DEF_NUM,		/* numBoardTwsiDev */
	rd88f6082uDasNasInfoBoardTwsiDev,					
	RD_88F6082MICRO_DAS_NAS_BOARD_MAC_INFO_NUM,		/* numBoardMacInfo */
	rd88f6082uDasNasInfoBoardMacInfo,
	RD_88F6082MICRO_DAS_NAS_BOARD_GPP_INFO_NUM,		/* numBoardGppInfo */
	rd88f6082uDasNasInfoBoardGppInfo,
	RD_88F6082MICRO_DAS_NAS_BOARD_DEBUG_LED_NUM,		/* activeLedsNumber */              
	NULL,
	N_A,							/* ledsPolarity */		
	RD_88F6082_MICRO_DAS_NAS_OE,				/* gppOutEnVal */
	RD_88F6082_MICRO_DAS_NAS_OE_VAL,			/* gppOutVal */
	BIT15							/* gppPolarityVal */
};

#define RD_88F6082_DX243_BOARD_PCI_IF_NUM		0x0
#define RD_88F6082_DX243_BOARD_TWSI_DEF_NUM		0x1
#define RD_88F6082_DX243_BOARD_MAC_INFO_NUM		0x2
#define RD_88F6082_DX243_BOARD_GPP_INFO_NUM		0x2
#define RD_88F6082_DX243_BOARD_MPP_CONFIG_NUM		0x1
#define RD_88F6082_DX243_BOARD_DEVICE_CONFIG_NUM	0x1
#define RD_88F6082_DX243_BOARD_DEBUG_LED_NUM		0x2

MV_U8	rd88f6082Dx243InfoBoardDebugLedIf[] =
	{1, 2};

MV_BOARD_TWSI_INFO rd88f6082Dx243InfoBoardTwsiDev[] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO rd88f6082Dx243InfoBoardMacInfo[] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8},
	{BOARD_MAC_SPEED_AUTO, 0x9}
	}; 

MV_BOARD_GPP_INFO rd88f6082Dx243InfoBoardGppInfo[] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RESET, 0},
	{BOARD_DEV_RTC, 7}};

MV_BOARD_MPP_INFO	rd88f6082Dx243InfoBoardMppConfigValue[] = 
#ifndef  MV_NAND_BOOT
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{RD_88F6082_DX243_MPP0_7,			/* mpp0_7 */
	RD_88F6082_DX243_MPP8_15,			/* mpp8_15 */
	N_A,						/* mpp16_23 */
	N_A}}						/* mppDev */						
	};
#else
	{{{RD_88F6082_DX243_MPP0_7_NB,			/* mpp0_7 */
	RD_88F6082_DX243_MPP8_15_NB,			/* mpp8_15 */
	N_A,						/* mpp16_23 */
	N_A}}						/* mppDev */						
	};
#endif

#if defined(MV_NAND) || defined(MV_NAND_BOOT)
MV_DEV_CS_INFO rd88f6082Dx243InfoBoardDeCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
		{{0, 0x8fcfffff, BOARD_DEV_NAND_FLASH, 8}}; /* NAND DEV */
#else
MV_DEV_CS_INFO rd88f6082Dx243InfoBoardDeCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
		 {{2, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* SPI DEV */
#endif

MV_BOARD_INFO rd88f6082Dx243Info = {
	"RD-DX243-6082-24G",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F6082_DX243_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88f6082Dx243InfoBoardMppConfigValue,
	((1 << 10) | (1 << 11) | (1 << 7)),		/* intsGppMask */
	RD_88F6082_DX243_BOARD_DEVICE_CONFIG_NUM,	/* numBoardDevIf */
	rd88f6082Dx243InfoBoardDeCsInfo,
	RD_88F6082_DX243_BOARD_PCI_IF_NUM,		/* numBoardPciIf */
	NULL,
	RD_88F6082_DX243_BOARD_TWSI_DEF_NUM,		/* numBoardTwsiDev */
	rd88f6082Dx243InfoBoardTwsiDev,					
	RD_88F6082_DX243_BOARD_MAC_INFO_NUM,		/* numBoardMacInfo */
	rd88f6082Dx243InfoBoardMacInfo,
	RD_88F6082_DX243_BOARD_GPP_INFO_NUM,		/* numBoardGppInfo */
	rd88f6082Dx243InfoBoardGppInfo,
	RD_88F6082_DX243_BOARD_DEBUG_LED_NUM,		/* activeLedsNumber */              
	rd88f6082Dx243InfoBoardDebugLedIf,
	N_A,						/* ledsPolarity */		
	RD_88F6082_DX243_OE,				/* gppOutEnVal */
	RD_88F6082_DX243_OE_VAL,			/* gppOutVal */
	(BIT1 | BIT2)					/* gppPolarityVal */
};

MV_BOARD_INFO*	boardInfoTbl[] = 	{&db88f6082BpInfo, 
					 NULL,
					 &rd88f6082NasInfo,
					 NULL,
					 &db88f6082LBpInfo,
					 NULL,
					 &rd88f6082uDasNasInfo,
					 &rd88f6082Dx243Info
					};



#define	BOARD_ID_BASE				BOARD_ID_88F6082_BASE
#define MV_MAX_BOARD_ID				BOARD_ID_88F6082_MAX
