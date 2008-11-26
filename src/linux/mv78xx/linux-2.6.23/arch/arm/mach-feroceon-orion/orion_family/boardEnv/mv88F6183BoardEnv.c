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

#define DB_88F6183BP_BOARD_PCI_IF_NUM		0x0
#define DB_88F6183BP_BOARD_TWSI_DEF_NUM		0x2
#define DB_88F6183BP_BOARD_MAC_INFO_NUM		0x1
#define DB_88F6183BP_BOARD_GPP_INFO_NUM		0x3
#define DB_88F6183BP_BOARD_DEBUG_LED_NUM	0x4
#define DB_88F6183BP_BOARD_MPP_CONFIG_NUM	0x1
#define DB_88F6183BP_BOARD_DEVICE_CONFIG_NUM	0x1

MV_U8	db88f6183BpInfoBoardDebugLedIf[DB_88F6183BP_BOARD_DEBUG_LED_NUM] =
	{24,25,26,27};

MV_BOARD_TWSI_INFO	db88f6183BpInfoBoardTwsiDev[DB_88F6183BP_BOARD_TWSI_DEF_NUM] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT},
     {BOARD_DEV_AUDIO_DEC, 0x4B, ADDR7_BIT}
    };

MV_BOARD_MAC_INFO db88f6183BpInfoBoardMacInfo[DB_88F6183BP_BOARD_MAC_INFO_NUM] =
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8},
	};

MV_BOARD_GPP_INFO	db88f6183BpInfoBoardGppInfo[DB_88F6183BP_BOARD_GPP_INFO_NUM] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_USB_VBUS, 14},
	 {BOARD_DEV_RESET, 20},
	{BOARD_DEV_SDIO_DETECT, 15}
    };

MV_BOARD_MPP_INFO	db88f6183BpInfoBoardMppConfigValue[DB_88F6183BP_BOARD_MPP_CONFIG_NUM] =
#ifndef  MV_NAND_BOOT
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{DB_88F6183_BP_MPP0_7,			/* mpp0_7 */
	DB_88F6183_BP_MPP8_15,				/* mpp8_15 */
	DB_88F6183_BP_MPP16_23,				/* mpp16_23 */
	DB_88F6183_BP_MPP24_31}}			/* mpp24_31 */
	};
#else
	{{{DB_88F6183_BP_MPP0_7_NB,			/* mpp0_7 */
	DB_88F6183_BP_MPP8_15_NB,			/* mpp8_15 */
	DB_88F6183_BP_MPP16_23_NB,			/* mpp16_23 */
	DB_88F6183_BP_MPP24_31_NB}}			/* mpp24_31 */
	};
#endif

#if defined(MV_NAND) || defined(MV_NAND_BOOT)
MV_DEV_CS_INFO db88f6183BpInfoBoardDeCsInfo[DB_88F6183BP_BOARD_DEVICE_CONFIG_NUM] =
		/*{deviceCS, params, devType, devWidth}*/
		{{0, 0x8fcfffff, BOARD_DEV_NAND_FLASH, 8}}; /* NAND DEV */
#else
MV_DEV_CS_INFO db88f6183BpInfoBoardDeCsInfo[DB_88F6183BP_BOARD_DEVICE_CONFIG_NUM] =
		/*{deviceCS, params, devType, devWidth}*/
		 {{2, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* SPI DEV */
#endif

MV_BOARD_INFO db88f6183BpInfo = {
	"DB-88F6183-BP",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F6183BP_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f6183BpInfoBoardMppConfigValue,
	0,						/* intsGppMask */
	DB_88F6183BP_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f6183BpInfoBoardDeCsInfo,
	DB_88F6183BP_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	NULL,
	DB_88F6183BP_BOARD_TWSI_DEF_NUM,		/* numBoardTwsiDev */
	db88f6183BpInfoBoardTwsiDev,
	DB_88F6183BP_BOARD_MAC_INFO_NUM,		/* numBoardMacInfo */
	db88f6183BpInfoBoardMacInfo,
	DB_88F6183BP_BOARD_GPP_INFO_NUM,		/* numBoardGppInfo */
	db88f6183BpInfoBoardGppInfo,
	DB_88F6183BP_BOARD_DEBUG_LED_NUM,		/* activeLedsNumber */
	db88f6183BpInfoBoardDebugLedIf,
	N_A,						/* ledsPolarity */
	DB_88F6183_BP_OE,				/* gppOutEnVal */
	DB_88F6183_BP_OE_VAL,				/* gppOutVal */
	0x1,						/* gppPolarityVal */
	NULL						/* pSwitchInfo */
};

#define RD_88F6183GP_BOARD_PCI_IF_NUM		0x0
#define RD_88F6183GP_BOARD_TWSI_DEF_NUM		0x1
#define RD_88F6183GP_BOARD_MAC_INFO_NUM		0x1
#define RD_88F6183GP_BOARD_GPP_INFO_NUM		0x3
#define RD_88F6183GP_BOARD_DEBUG_LED_NUM	0x0
#define RD_88F6183GP_BOARD_MPP_CONFIG_NUM	0x1
#define RD_88F6183GP_BOARD_DEVICE_CONFIG_NUM	0x1

MV_BOARD_TWSI_INFO	rd88f6183GpInfoBoardTwsiDev[RD_88F6183GP_BOARD_TWSI_DEF_NUM] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO rd88f6183GpInfoBoardMacInfo[RD_88F6183GP_BOARD_MAC_INFO_NUM] =
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8}
	};

MV_BOARD_GPP_INFO	rd88f6183GpInfoBoardGppInfo[RD_88F6183GP_BOARD_GPP_INFO_NUM] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RESET, 20},
	{BOARD_DEV_RTC, 31},
	{BOARD_DEV_SDIO_DETECT, 9}
	};

MV_BOARD_MPP_INFO	rd88f6183GpInfoBoardMppConfigValue[RD_88F6183GP_BOARD_MPP_CONFIG_NUM] =
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{RD_88F6183_GP_MPP0_7,			/* mpp0_7 */
	RD_88F6183_GP_MPP8_15,				/* mpp8_15 */
	RD_88F6183_GP_MPP16_23,				/* mpp16_23 */
	RD_88F6183_GP_MPP24_31}}			/* mpp24_31 */
	};

MV_DEV_CS_INFO rd88f6183GpInfoBoardDeCsInfo[RD_88F6183GP_BOARD_DEVICE_CONFIG_NUM] =
		/*{deviceCS, params, devType, devWidth}*/
		{{0, 0x8fcfffff, BOARD_DEV_NAND_FLASH, 8}}; /* NAND DEV */

MV_BOARD_INFO rd88f6183GpInfo = {
	"RD-88F6183-GP",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F6183GP_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88f6183GpInfoBoardMppConfigValue,
	0,						/* intsGppMask */
	RD_88F6183GP_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	rd88f6183GpInfoBoardDeCsInfo,
	RD_88F6183GP_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	NULL,
	RD_88F6183GP_BOARD_TWSI_DEF_NUM,		/* numBoardTwsiDev */
	rd88f6183GpInfoBoardTwsiDev,
	RD_88F6183GP_BOARD_MAC_INFO_NUM,		/* numBoardMacInfo */
	rd88f6183GpInfoBoardMacInfo,
	RD_88F6183GP_BOARD_GPP_INFO_NUM,		/* numBoardGppInfo */
	rd88f6183GpInfoBoardGppInfo,
	RD_88F6183GP_BOARD_DEBUG_LED_NUM,		/* activeLedsNumber */
	NULL,
	N_A,						/* ledsPolarity */
	RD_88F6183_GP_OE,				/* gppOutEnVal */
	RD_88F6183_GP_OE_VAL,				/* gppOutVal */
	BIT15,						/* gppPolarityVal */
	NULL						/* pSwitchInfo */
};

#define RD_88F6183AP_BOARD_PCI_IF_NUM		0x0
#define RD_88F6183AP_BOARD_TWSI_DEF_NUM		0x1
#define RD_88F6183AP_BOARD_MAC_INFO_NUM		0x1
#define RD_88F6183AP_BOARD_GPP_INFO_NUM		0x3
#define RD_88F6183AP_BOARD_DEBUG_LED_NUM	0x0
#define RD_88F6183AP_BOARD_MPP_CONFIG_NUM	0x1
#define RD_88F6183AP_BOARD_DEVICE_CONFIG_NUM	0x1

MV_BOARD_TWSI_INFO	rd88f6183ApInfoBoardTwsiDev[] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO rd88f6183ApInfoBoardMacInfo[] =
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_1000M, 0x0}
	};

MV_BOARD_SWITCH_INFO	rd88f6183ApInfoBoardSwitchInfo[] = 
	/* MV_32 linkStatusIrq, {MV_32 qdPort0, MV_32 qdPort1, MV_32 qdPort2, MV_32 qdPort3, MV_32 qdPort4}, 
		MV_32 qdCpuPort, MV_32 smiScanMode} */
	{{-1, {4, 3, 2, 1, 0}, 5, 1}};

MV_BOARD_GPP_INFO	rd88f6183ApInfoBoardGppInfo[] =
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RESET, 20},
	{BOARD_DEV_RTC, 1},
	{BOARD_DEV_SWITCH_PHY_INT, 3}
	};

MV_BOARD_MPP_INFO	rd88f6183ApInfoBoardMppConfigValue[] =
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{RD_88F6183_AP_MPP0_7,			/* mpp0_7 */
	RD_88F6183_AP_MPP8_15,				/* mpp8_15 */
	RD_88F6183_AP_MPP16_23,				/* mpp16_23 */
	RD_88F6183_AP_MPP24_31}}			/* mpp24_31 */
	};

MV_DEV_CS_INFO rd88f6183ApInfoBoardDeCsInfo[] =
		/*{deviceCS, params, devType, devWidth}*/
		 {{2, N_A, BOARD_DEV_SPI_FLASH, 8}};	   /* SPI DEV */

MV_BOARD_INFO rd88f6183ApInfo = {
	"RD-88F6183-AP-GE",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88F6183AP_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88f6183ApInfoBoardMppConfigValue,
	0,						/* intsGppMask */
	RD_88F6183AP_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	rd88f6183ApInfoBoardDeCsInfo,
	RD_88F6183AP_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	NULL,
	RD_88F6183AP_BOARD_TWSI_DEF_NUM,		/* numBoardTwsiDev */
	rd88f6183ApInfoBoardTwsiDev,
	RD_88F6183AP_BOARD_MAC_INFO_NUM,		/* numBoardMacInfo */
	rd88f6183ApInfoBoardMacInfo,
	RD_88F6183AP_BOARD_GPP_INFO_NUM,		/* numBoardGppInfo */
	rd88f6183ApInfoBoardGppInfo,
	RD_88F6183AP_BOARD_DEBUG_LED_NUM,		/* activeLedsNumber */
	NULL,
	N_A,						/* ledsPolarity */
	RD_88F6183_AP_OE,				/* gppOutEnVal */
	RD_88F6183_AP_OE_VAL,				/* gppOutVal */
	BIT15,						/* gppPolarityVal */
	rd88f6183ApInfoBoardSwitchInfo			/* pSwitchInfo */
};
MV_BOARD_INFO*	boardInfoTbl[] = 	{&db88f6183BpInfo,
					 &rd88f6183GpInfo,
					 &rd88f6183ApInfo
					};


#define	BOARD_ID_BASE				BOARD_ID_88F6183_BASE
#define MV_MAX_BOARD_ID				BOARD_ID_88F6183_MAX
