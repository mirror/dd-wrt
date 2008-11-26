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
#include "mvCommon.h"
#include "mvBoardEnvLib.h"
#include "mvBoardEnvSpec.h"
#include "twsi/mvTwsi.h"

#define DB_88F6281_BOARD_PCI_IF_NUM		0x0
#define DB_88F6281_BOARD_TWSI_DEF_NUM		0x7
#define DB_88F6281_BOARD_MAC_INFO_NUM		0x2
#define DB_88F6281_BOARD_GPP_INFO_NUM		0x2
#define DB_88F6281_BOARD_MPP_CONFIG_NUM		0x1
#define DB_88F6281_BOARD_MPP_GROUP_TYPE_NUM	0x1
#define DB_88F6281_BOARD_DEVICE_CONFIG_NUM	0x1
#define DB_88F6281_BOARD_DEBUG_LED_NUM		0x1

MV_U8	db88f6281InfoBoardDebugLedIf[] =
	{7};

MV_BOARD_TWSI_INFO	db88f6281InfoBoardTwsiDev[] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{
	{BOARD_DEV_TWSI_EXP, 0x20, ADDR7_BIT},
	{BOARD_DEV_TWSI_EXP, 0x21, ADDR7_BIT},
	{BOARD_DEV_TWSI_EXP, 0x27, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4C, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4D, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4E, ADDR7_BIT},
	{BOARD_TWSI_AUDIO_DEC, 0x4A, ADDR7_BIT}
	};

MV_BOARD_MAC_INFO db88f6281InfoBoardMacInfo[] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{
	{BOARD_MAC_SPEED_AUTO, 0x8},
	{BOARD_MAC_SPEED_AUTO, 0x9}
	}; 

MV_BOARD_MPP_TYPE_INFO db88f6281InfoBoardMppTypeInfo[] = 
	/* {{MV_BOARD_MPP_TYPE_CLASS	boardMppGroup1,
 		MV_BOARD_MPP_TYPE_CLASS	boardMppGroup2}} */
	{{MV_BOARD_AUTO, MV_BOARD_AUTO}
	}; 

MV_BOARD_GPP_INFO db88f6281InfoBoardGppInfo[] = 
	/* {{MV_BOARD_GPP_CLASS	devClass, MV_U8	gppPinNum}} */
	{
	{BOARD_GPP_TSU_DIRCTION, 33},
	{BOARD_GPP_USB_VBUS, 49}
	};

MV_BOARD_MPP_INFO	db88f6281InfoBoardMppConfigValue[] = 
	{{{
	DB_88F6281_MPP0_7,		
	DB_88F6281_MPP8_15,		
	DB_88F6281_MPP16_23,		
	DB_88F6281_MPP24_31,		
	DB_88F6281_MPP32_39,		
	DB_88F6281_MPP40_47,		
	DB_88F6281_MPP48_55		
	}}};

MV_DEV_CS_INFO db88f6281InfoBoardDeCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
#if defined(MV_NAND) || defined(MV_NAND_BOOT)
		 {{0, N_A, BOARD_DEV_NAND_FLASH, 8}};	   /* NAND DEV */
#else
		 {{2, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* NAND DEV */
#endif

MV_BOARD_INFO db88f6281Info = {
	"DB-88F6281-BP",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F6281_BOARD_MPP_GROUP_TYPE_NUM,		/* numBoardMppGroupType */
	db88f6281InfoBoardMppTypeInfo,
	DB_88F6281_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f6281InfoBoardMppConfigValue,
	0,						/* intsGppMaskLow */
	0,						/* intsGppMaskHigh */
	DB_88F6281_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f6281InfoBoardDeCsInfo,
	DB_88F6281_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f6281InfoBoardTwsiDev,					
	DB_88F6281_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	db88f6281InfoBoardMacInfo,
	DB_88F6281_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	db88f6281InfoBoardGppInfo,
	DB_88F6281_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	db88f6281InfoBoardDebugLedIf,
	N_A,						/* ledsPolarity */		
	DB_88F6281_OE_LOW,				/* gppOutEnLow */
	DB_88F6281_OE_HIGH,				/* gppOutEnHigh */
	DB_88F6281_OE_VAL_LOW,				/* gppOutValLow */
	DB_88F6281_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	0, 						/* gppPolarityValHigh */
	NULL						/* pSwitchInfo */
};

#define RD_88F6281_BOARD_PCI_IF_NUM		0x0
#define RD_88F6281_BOARD_TWSI_DEF_NUM		0x1
#define RD_88F6281_BOARD_MAC_INFO_NUM		0x1
#define RD_88F6281_BOARD_GPP_INFO_NUM		0x9
#define RD_88F6281_BOARD_MPP_GROUP_TYPE_NUM	0x1
#define RD_88F6281_BOARD_MPP_CONFIG_NUM		0x1
#define RD_88F6281_BOARD_DEVICE_CONFIG_NUM	0x1
#define RD_88F6281_BOARD_DEBUG_LED_NUM		0x0

MV_BOARD_MAC_INFO rd88f6281InfoBoardMacInfo[] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_1000M, 0x0}
	}; 

MV_BOARD_SWITCH_INFO rd88f6281InfoBoardSwitchInfo[] = 
	/* MV_32 linkStatusIrq, {MV_32 qdPort0, MV_32 qdPort1, MV_32 qdPort2, MV_32 qdPort3, MV_32 qdPort4}, 
		MV_32 qdCpuPort, MV_32 smiScanMode} */
	{{35, {4, 0, 1, 2, 3}, 5, 1}};

MV_BOARD_GPP_INFO rd88f6281InfoBoardGppInfo[] = 
	/* {{MV_BOARD_GPP_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_GPP_USB_VBUS, 22},
	{BOARD_GPP_SDIO_DETECT, 23},
	{BOARD_GPP_SWITCH_PHY_INT, 35},
	{BOARD_GPP_LIFELINE, 38},
	{BOARD_GPP_SDIO_POWER, 45},
	{BOARD_GPP_WPS_BUTTON, 46},
	{BOARD_GPP_RESTOR_BUTTON, 47},
	{BOARD_GPP_HDD_POWER, 48},
	{BOARD_GPP_FAN_POWER, 49}
	};
MV_BOARD_TWSI_INFO	rd88f6281InfoBoardTwsiDev[] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{
	{BOARD_TWSI_AUDIO_DEC, 0x4A, ADDR7_BIT}
	};

MV_BOARD_MPP_TYPE_INFO rd88f6281InfoBoardMppTypeInfo[] = 
	/* {{MV_BOARD_MPP_TYPE_CLASS	boardMppGroup1,
 		MV_BOARD_MPP_TYPE_CLASS	boardMppGroup2}} */
	{{MV_BOARD_TDM, MV_BOARD_AUDIO}
	}; 

MV_BOARD_MPP_INFO	rd88f6281InfoBoardMppConfigValue[] = 
	{{{
	RD_88F6281_MPP0_7,		
	RD_88F6281_MPP8_15,		
	RD_88F6281_MPP16_23,		
	RD_88F6281_MPP24_31,		
	RD_88F6281_MPP32_39,		
	RD_88F6281_MPP40_47,		
	RD_88F6281_MPP48_55		
	}}};

MV_DEV_CS_INFO rd88f6281InfoBoardDeCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
#if defined(MV_NAND) || defined(MV_NAND_BOOT)
		 {{0, N_A, BOARD_DEV_NAND_FLASH, 8}};	   /* NAND DEV */
#else
		 {{2, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* NAND DEV */
#endif

MV_BOARD_INFO rd88f6281Info = {
	"RD-88F6281",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F6281_BOARD_MPP_GROUP_TYPE_NUM,		/* numBoardMppGroupType */
	rd88f6281InfoBoardMppTypeInfo,
	RD_88F6281_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88f6281InfoBoardMppConfigValue,
	0,						/* intsGppMaskLow */
	(1 << 3),					/* intsGppMaskHigh */
	RD_88F6281_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	rd88f6281InfoBoardDeCsInfo,
	RD_88F6281_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	rd88f6281InfoBoardTwsiDev,					
	RD_88F6281_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	rd88f6281InfoBoardMacInfo,
	RD_88F6281_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	rd88f6281InfoBoardGppInfo,
	RD_88F6281_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	NULL,
	N_A,						/* ledsPolarity */		
	RD_88F6281_OE_LOW,				/* gppOutEnLow */
	RD_88F6281_OE_HIGH,				/* gppOutEnHigh */
	RD_88F6281_OE_VAL_LOW,				/* gppOutValLow */
	RD_88F6281_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	BIT3, 						/* gppPolarityValHigh */
	rd88f6281InfoBoardSwitchInfo			/* pSwitchInfo */
};

#define DB_88F6192_BOARD_PCI_IF_NUM		0x0
#define DB_88F6192_BOARD_TWSI_DEF_NUM		0x7
#define DB_88F6192_BOARD_MAC_INFO_NUM		0x2
#define DB_88F6192_BOARD_GPP_INFO_NUM		0x2
#define DB_88F6192_BOARD_MPP_GROUP_TYPE_NUM	0x1
#define DB_88F6192_BOARD_MPP_CONFIG_NUM		0x1
#define DB_88F6192_BOARD_DEVICE_CONFIG_NUM	0x1
#define DB_88F6192_BOARD_DEBUG_LED_NUM		0x1

MV_U8	db88f6192InfoBoardDebugLedIf[] =
	{7};

MV_BOARD_TWSI_INFO	db88f6192InfoBoardTwsiDev[] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{
	{BOARD_DEV_TWSI_EXP, 0x20, ADDR7_BIT},
	{BOARD_DEV_TWSI_EXP, 0x21, ADDR7_BIT},
	{BOARD_DEV_TWSI_EXP, 0x27, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4C, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4D, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4E, ADDR7_BIT},
	{BOARD_TWSI_AUDIO_DEC, 0x4A, ADDR7_BIT}
	};

MV_BOARD_MAC_INFO db88f6192InfoBoardMacInfo[] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{
	{BOARD_MAC_SPEED_AUTO, 0x8},
	{BOARD_MAC_SPEED_AUTO, 0x9}
	}; 

MV_BOARD_GPP_INFO db88f6192InfoBoardGppInfo[] = 
	/* {{MV_BOARD_GPP_CLASS	devClass, MV_U8	gppPinNum}} */
	{
	{BOARD_GPP_USB_VBUS, 22},
	{BOARD_GPP_SDIO_DETECT, 23},
	};

MV_BOARD_MPP_TYPE_INFO db88f6192InfoBoardMppTypeInfo[] = 
	/* {{MV_BOARD_MPP_TYPE_CLASS	boardMppGroup1,
 		MV_BOARD_MPP_TYPE_CLASS	boardMppGroup2}} */
	{{MV_BOARD_AUTO, MV_BOARD_OTHER}
	}; 

MV_BOARD_MPP_INFO	db88f6192InfoBoardMppConfigValue[] = 
	{{{
	DB_88F6192_MPP0_7,		
	DB_88F6192_MPP8_15,		
	DB_88F6192_MPP16_23,		
	DB_88F6192_MPP24_31,		
	DB_88F6192_MPP32_35
	}}};

MV_DEV_CS_INFO db88f6192InfoBoardDeCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
#if defined(MV_NAND) || defined(MV_NAND_BOOT)
		 {{0, N_A, BOARD_DEV_NAND_FLASH, 8}};	   /* NAND DEV */
#else
		 {{2, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* NAND DEV */
#endif

MV_BOARD_INFO db88f6192Info = {
	"DB-88F6192-BP",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F6192_BOARD_MPP_GROUP_TYPE_NUM,		/* numBoardMppGroupType */
	db88f6192InfoBoardMppTypeInfo,
	DB_88F6192_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f6192InfoBoardMppConfigValue,
	0,						/* intsGppMaskLow */
	(1 << 3),					/* intsGppMaskHigh */
	DB_88F6192_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f6192InfoBoardDeCsInfo,
	DB_88F6192_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f6192InfoBoardTwsiDev,					
	DB_88F6192_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	db88f6192InfoBoardMacInfo,
	DB_88F6192_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	db88f6192InfoBoardGppInfo,
	DB_88F6192_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	db88f6192InfoBoardDebugLedIf,
	N_A,						/* ledsPolarity */		
	DB_88F6192_OE_LOW,				/* gppOutEnLow */
	DB_88F6192_OE_HIGH,				/* gppOutEnHigh */
	DB_88F6192_OE_VAL_LOW,				/* gppOutValLow */
	DB_88F6192_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	0						/* gppPolarityValHigh */
};

#define RD_88F6192_BOARD_PCI_IF_NUM		0x0
#define RD_88F6192_BOARD_TWSI_DEF_NUM		0x0
#define RD_88F6192_BOARD_MAC_INFO_NUM		0x1
#define RD_88F6192_BOARD_GPP_INFO_NUM		0xE
#define RD_88F6192_BOARD_MPP_GROUP_TYPE_NUM	0x1
#define RD_88F6192_BOARD_MPP_CONFIG_NUM		0x1
#define RD_88F6192_BOARD_DEVICE_CONFIG_NUM	0x1
#define RD_88F6192_BOARD_DEBUG_LED_NUM		0x3

MV_U8	rd88f6192InfoBoardDebugLedIf[] =
	{17, 28, 29};

MV_BOARD_MAC_INFO rd88f6192InfoBoardMacInfo[] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8}
	}; 

MV_BOARD_GPP_INFO rd88f6192InfoBoardGppInfo[] = 
	/* {{MV_BOARD_GPP_CLASS	devClass, MV_U8	gppPinNum}} */
	{
	{BOARD_GPP_USB_VBUS_EN, 10},
	{BOARD_GPP_USB_HOST_DEVICE, 11},
	{BOARD_GPP_SWITCH_PHY_INT, 14},
	{BOARD_GPP_HDD_POWER, 16},
	{BOARD_GPP_POWER_ON_LED, 19},
	{BOARD_GPP_USB_VBUS, 26},
	{BOARD_GPP_WPS_BUTTON, 24},
	{BOARD_GPP_TS_BUTTON_C, 25},
	{BOARD_GPP_USB_OC, 27},
	{BOARD_GPP_TS_BUTTON_U, 30},
	{BOARD_GPP_TS_BUTTON_R, 31},
	{BOARD_GPP_TS_BUTTON_L, 32},
	{BOARD_GPP_TS_BUTTON_D, 34},
	{BOARD_GPP_FAN_POWER, 35}
	};

MV_BOARD_MPP_TYPE_INFO rd88f6192InfoBoardMppTypeInfo[] = 
	/* {{MV_BOARD_MPP_TYPE_CLASS	boardMppGroup1,
 		MV_BOARD_MPP_TYPE_CLASS	boardMppGroup2}} */
	{{MV_BOARD_OTHER, MV_BOARD_OTHER}
	}; 

MV_BOARD_MPP_INFO	rd88f6192InfoBoardMppConfigValue[] = 
	{{{
	RD_88F6192_MPP0_7,		
	RD_88F6192_MPP8_15,		
	RD_88F6192_MPP16_23,		
	RD_88F6192_MPP24_31,		
	RD_88F6192_MPP32_35
	}}};

MV_DEV_CS_INFO rd88f6192InfoBoardDeCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
		 {{1, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* SPI DEV */

MV_BOARD_INFO rd88f6192Info = {
	"RD-88F6192-NAS",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F6192_BOARD_MPP_GROUP_TYPE_NUM,		/* numBoardMppGroupType */
	rd88f6192InfoBoardMppTypeInfo,
	RD_88F6192_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88f6192InfoBoardMppConfigValue,
	0,						/* intsGppMaskLow */
	(1 << 3),					/* intsGppMaskHigh */
	RD_88F6192_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	rd88f6192InfoBoardDeCsInfo,
	RD_88F6192_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	NULL,					
	RD_88F6192_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	rd88f6192InfoBoardMacInfo,
	RD_88F6192_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	rd88f6192InfoBoardGppInfo,
	RD_88F6192_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	rd88f6192InfoBoardDebugLedIf,
	N_A,						/* ledsPolarity */		
	RD_88F6192_OE_LOW,				/* gppOutEnLow */
	RD_88F6192_OE_HIGH,				/* gppOutEnHigh */
	RD_88F6192_OE_VAL_LOW,				/* gppOutValLow */
	RD_88F6192_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	0						/* gppPolarityValHigh */
};

#define DB_88F6180_BOARD_PCI_IF_NUM		0x0
#define DB_88F6180_BOARD_TWSI_DEF_NUM		0x2
#define DB_88F6180_BOARD_MAC_INFO_NUM		0x1
#define DB_88F6180_BOARD_GPP_INFO_NUM		0x0
#define DB_88F6180_BOARD_MPP_GROUP_TYPE_NUM	0x1
#define DB_88F6180_BOARD_MPP_CONFIG_NUM		0x1
#define DB_88F6180_BOARD_DEVICE_CONFIG_NUM	0x1
#define DB_88F6180_BOARD_DEBUG_LED_NUM		0x2

MV_U8	db88f6180InfoBoardDebugLedIf[] =
	{7, 10};

MV_BOARD_TWSI_INFO	db88f6180InfoBoardTwsiDev[] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{
	{BOARD_DEV_TWSI_SATR, 0x4C, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4E, ADDR7_BIT}
	};

MV_BOARD_MAC_INFO db88f6180InfoBoardMacInfo[] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8}
	}; 

MV_BOARD_GPP_INFO db88f6180InfoBoardGppInfo[] = 
	/* {{MV_BOARD_GPP_CLASS	devClass, MV_U8	gppPinNum}} */
	{
	{BOARD_GPP_USB_VBUS, 6}
	};

MV_BOARD_MPP_TYPE_INFO db88f6180InfoBoardMppTypeInfo[] = 
	/* {{MV_BOARD_MPP_TYPE_CLASS	boardMppGroup1,
 		MV_BOARD_MPP_TYPE_CLASS	boardMppGroup2}} */
	{{MV_BOARD_OTHER, MV_BOARD_OTHER}
	}; 

MV_BOARD_MPP_INFO	db88f6180InfoBoardMppConfigValue[] = 
	{{{
	DB_88F6180_MPP0_7,		
	DB_88F6180_MPP8_10,		
	}}};

MV_DEV_CS_INFO db88f6180InfoBoardDeCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
		 {{2, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* NAND DEV */

MV_BOARD_INFO db88f6180Info = {
	"DB-88F6180-BP",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F6180_BOARD_MPP_GROUP_TYPE_NUM,		/* numBoardMppGroupType */
	db88f6180InfoBoardMppTypeInfo,
	DB_88F6180_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f6180InfoBoardMppConfigValue,
	0,						/* intsGppMaskLow */
	0,					/* intsGppMaskHigh */
	DB_88F6180_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f6180InfoBoardDeCsInfo,
	DB_88F6180_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f6180InfoBoardTwsiDev,					
	DB_88F6180_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	db88f6180InfoBoardMacInfo,
	DB_88F6180_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	NULL,
	DB_88F6180_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	db88f6180InfoBoardDebugLedIf,
	N_A,						/* ledsPolarity */		
	DB_88F6180_OE_LOW,				/* gppOutEnLow */
	DB_88F6180_OE_HIGH,				/* gppOutEnHigh */
	DB_88F6180_OE_VAL_LOW,				/* gppOutValLow */
	DB_88F6180_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	0						/* gppPolarityValHigh */
};

#define RD_88F6180_BOARD_PCI_IF_NUM		0x0
#define RD_88F6180_BOARD_TWSI_DEF_NUM		0x0
#define RD_88F6180_BOARD_MAC_INFO_NUM		0x1
#define RD_88F6180_BOARD_GPP_INFO_NUM		0x2
#define RD_88F6180_BOARD_MPP_GROUP_TYPE_NUM	0x1
#define RD_88F6180_BOARD_MPP_CONFIG_NUM		0x1
#define RD_88F6180_BOARD_DEVICE_CONFIG_NUM	0x1
#define RD_88F6180_BOARD_DEBUG_LED_NUM		0x0

MV_BOARD_MAC_INFO rd88f6180InfoBoardMacInfo[] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_1000M, 0x0}
	}; 

MV_BOARD_SWITCH_INFO rd88f6180InfoBoardSwitchInfo[] = 
	/* MV_32 linkStatusIrq, {MV_32 qdPort0, MV_32 qdPort1, MV_32 qdPort2, MV_32 qdPort3, MV_32 qdPort4}, 
		MV_32 qdCpuPort, MV_32 smiScanMode} */
	{{-1, {4, 0, 1, 2, 3}, 5, 1}};

MV_BOARD_GPP_INFO rd88f6180InfoBoardGppInfo[] = 
	/* {{MV_BOARD_GPP_CLASS	devClass, MV_U8	gppPinNum}} */
	{
	{BOARD_GPP_SWITCH_PHY_INT, 8},
	{BOARD_GPP_WPS_BUTTON, 9}
	};

MV_BOARD_MPP_TYPE_INFO rd88f6180InfoBoardMppTypeInfo[] = 
	/* {{MV_BOARD_MPP_TYPE_CLASS	boardMppGroup1,
 		MV_BOARD_MPP_TYPE_CLASS	boardMppGroup2}} */
	{{MV_BOARD_OTHER, MV_BOARD_OTHER}
	}; 

MV_BOARD_MPP_INFO	rd88f6180InfoBoardMppConfigValue[] = 
	{{{
	RD_88F6180_MPP0_7,		
	RD_88F6180_MPP8_10		
	}}};

MV_DEV_CS_INFO rd88f6180InfoBoardDeCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
		 {{2, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* NAND DEV */

MV_BOARD_INFO rd88f6180Info = {
	"RD-88F6180-AP",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F6180_BOARD_MPP_GROUP_TYPE_NUM,		/* numBoardMppGroupType */
	rd88f6180InfoBoardMppTypeInfo,
	RD_88F6180_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88f6180InfoBoardMppConfigValue,
	(1<<8),						/* intsGppMaskLow */
	0,						/* intsGppMaskHigh */
	RD_88F6180_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	rd88f6180InfoBoardDeCsInfo,
	RD_88F6180_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	NULL,					
	RD_88F6180_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	rd88f6180InfoBoardMacInfo,
	RD_88F6180_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	rd88f6180InfoBoardGppInfo,
	RD_88F6180_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	NULL,
	N_A,						/* ledsPolarity */		
	RD_88F6180_OE_LOW,				/* gppOutEnLow */
	RD_88F6180_OE_HIGH,				/* gppOutEnHigh */
	RD_88F6180_OE_VAL_LOW,				/* gppOutValLow */
	RD_88F6180_OE_VAL_HIGH,				/* gppOutValHigh */
	0,						/* gppPolarityValLow */
	0,						/* gppPolarityValHigh */
	rd88f6180InfoBoardSwitchInfo			/* pSwitchInfo */
};

MV_BOARD_INFO*	boardInfoTbl[] = 	{
					 &db88f6281Info,
					 &rd88f6281Info,
					 &db88f6192Info,
					 &rd88f6192Info,
					 &db88f6180Info,
					 &rd88f6180Info
					};


