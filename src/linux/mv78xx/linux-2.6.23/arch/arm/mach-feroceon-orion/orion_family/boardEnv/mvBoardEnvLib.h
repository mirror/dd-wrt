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
#ifndef __INCmvBoardEnvLibh
#define __INCmvBoardEnvLibh

/* defines */
/* The below constant macros defines the board I2C EEPROM data offsets */



#include "ctrlEnv/mvCtrlEnvLib.h"
#include "mvSysHwConfig.h"
#include "boardEnv/mvBoardEnvSpec.h"


/* DUART stuff for Tclk detection only */
#define DUART_BAUD_RATE			115200
#define MAX_CLOCK_MARGINE		5000000	/* Maximum detected clock margine */


#define BOARD_ETH_PORT_NUM  MV_ETH_MAX_PORTS
#define BOARD_ETH_SWITCH_PORT_NUM	5

#define	MV_BOARD_MAX_USB_IF		2
#define MV_BOARD_MAX_MPP		4
#define MV_BOARD_NAME_LEN  		0x20

typedef struct _boardData
{
   MV_U32 magic;
   MV_U16 boardId;
   MV_U8 boardVer;
   MV_U8 boardRev;
   MV_U32 reserved1;
   MV_U32 reserved2;

}BOARD_DATA;

typedef enum _devBoardClass
{
	BOARD_DEV_NOR_FLASH,
	BOARD_DEV_NAND_FLASH,
	BOARD_DEV_SEVEN_SEG,
	BOARD_DEV_FPGA,
	BOARD_DEV_SRAM,
	BOARD_DEV_RTC,
	BOARD_DEV_PEX_TO_PCI_BRIDGE,
	BOARD_DEV_MV_SWITCH,
	BOARD_DEV_USB_VBUS,
	BOARD_DEV_REF_CLCK,
	BOARD_DEV_VOIP_SLIC,
	BOARD_DEV_BUTTON,
	BOARD_DEV_POWER_BUTTON,
	BOARD_DEV_RESTOR_BUTTON,
	BOARD_DEV_HDD0_POWER,
	BOARD_DEV_HDD1_POWER,
	BOARD_DEV_FAN_POWER,
	BOARD_DEV_SPI_FLASH,
	BOARD_DEV_RESET,
	BOARD_DEV_POWER_ON_LED,
	BOARD_DEV_HDD_POWER,
    BOARD_DEV_AUDIO_DEC,
	BOARD_DEV_SDIO_DETECT,
	BOARD_DEV_SWITCH_PHY_INT,
	BOARD_DEV_OTHER,
}MV_BOARD_DEV_CLASS;


typedef struct _pciBoardSlot
{
	MV_U8	pciSlotGppIntMap[4];

}MV_BOARD_PCI_SLOT;


typedef struct _pciBoardIf
{
	MV_U8	pciDevNum;
	MV_U8	pciGppIntMap[4];
}MV_BOARD_PCI_IF;


typedef struct _devCsInfo
{
    MV_U8		deviceCS;
    MV_U32		params;
    MV_U32		devClass;	/* MV_BOARD_DEV_CLASS */
    MV_U8		devWidth;

}MV_DEV_CS_INFO;


#define MV_BOARD_PHY_FORCE_10MB		0x0
#define MV_BOARD_PHY_FORCE_100MB	0x1
#define MV_BOARD_PHY_FORCE_1000MB	0x2
#define MV_BOARD_PHY_SPEED_AUTO		0x3

typedef struct _boardSwitchInfo
{
	MV_32	linkStatusIrq;
	MV_32	qdPort[BOARD_ETH_SWITCH_PORT_NUM];
	MV_32	qdCpuPort;
	MV_32	smiScanMode; /* 1 for SMI_MANUAL_MODE, 0 otherwise */

}MV_BOARD_SWITCH_INFO;

typedef struct _boardLedInfo
{
	MV_U8	activeLedsNumber;
	MV_U8	ledsPolarity;	/* '0' or '1' to turn on led */
	MV_U8*	gppPinNum; 		/* Pointer to GPP values */

}MV_BOARD_LED_INFO;

typedef struct _boardGppInfo
{
	MV_BOARD_DEV_CLASS	devClass;
	MV_U8	gppPinNum;

}MV_BOARD_GPP_INFO;


typedef struct _boardTwsiInfo
{
	MV_BOARD_DEV_CLASS	devClass;
	MV_U8	twsiDevAddr;
	MV_U8	twsiDevAddrType;

}MV_BOARD_TWSI_INFO;


typedef enum _boardMacSpeed
{
	BOARD_MAC_SPEED_10M,
	BOARD_MAC_SPEED_100M,
	BOARD_MAC_SPEED_1000M,
	BOARD_MAC_SPEED_AUTO,

}MV_BOARD_MAC_SPEED;

typedef struct _boardMacInfo
{
	MV_BOARD_MAC_SPEED	boardMacSpeed;
	MV_U8	boardEthSmiAddr;

}MV_BOARD_MAC_INFO;

typedef struct _boardMppInfo
{
	MV_U32		mppGroup[MV_BOARD_MAX_MPP];

}MV_BOARD_MPP_INFO;

typedef struct _boardInfo
{
	char 			   	boardName[MV_BOARD_NAME_LEN];
	MV_U8				numBoardMppConfigValue;
	MV_BOARD_MPP_INFO*		pBoardMppConfigValue;
    	MV_U32				intsGppMask;
	MV_U8				numBoardDeviceIf;
    	MV_DEV_CS_INFO*			pDevCsInfo;
	MV_U8				numBoardPciIf;
	MV_BOARD_PCI_IF*		pBoardPciIf;
	MV_U8				numBoardTwsiDev;
	MV_BOARD_TWSI_INFO*		pBoardTwsiDev;
	MV_U8				numBoardMacInfo;
	MV_BOARD_MAC_INFO*		pBoardMacInfo;
	MV_U8				numBoardGppInfo;
	MV_BOARD_GPP_INFO*		pBoardGppInfo;
    	MV_U8				activeLedsNumber;
	MV_U8*				pLedGppPin;
	MV_U8				ledsPolarity;	/* '0' or '1' to turn on led */
	/* GPP values */
	MV_U32				gppOutEnVal;
	MV_U32				gppOutVal;
	MV_U32				gppPolarityVal;
	/* Switch Configuration */
	MV_BOARD_SWITCH_INFO*		pSwitchInfo;
}MV_BOARD_INFO;



MV_VOID 	mvBoardEnvInit(MV_VOID);
MV_U32      	mvBoardIdGet(MV_VOID);
MV_U16      	mvBoardModelGet(MV_VOID);
MV_U16      	mvBoardRevGet(MV_VOID);
MV_STATUS	mvBoardNameGet(char *pNameBuff);
MV_32      	mvBoardPhyAddrGet(MV_U32 ethPortNum);
MV_BOARD_MAC_SPEED      mvBoardMacSpeedGet(MV_U32 ethPortNum);
MV_32		mvBoardLinkStatusIrqGet(MV_U32 ethPortNum);
MV_32		mvBoardSwitchPortGet(MV_U32 ethPortNum, MV_U8 boardPortNum);
MV_32		mvBoardSwitchCpuPortGet(MV_U32 ethPortNum);
MV_32		mvBoardSmiScanModeGet(MV_U32 ethPortNum);
MV_BOOL     	mvBoardIsPortInSgmii(MV_U32 ethPortNum);
MV_U32 		mvBoardTclkGet(MV_VOID);
MV_U32      	mvBoardSysClkGet(MV_VOID);
MV_VOID     	mvBoardDebug7Seg(MV_U32 hexNum);
MV_32      	mvBoardMppGet(MV_U32 mppGroupNum);
MV_U8		mvBoardRtcTwsiAddrTypeGet(MV_VOID);
MV_U8		mvBoardRtcTwsiAddrGet(MV_VOID);
MV_U8		mvBoardA2DTwsiAddrTypeGet(MV_VOID);
MV_U8		mvBoardA2DTwsiAddrGet(MV_VOID);
MV_VOID	    	mvBoardReset(MV_VOID);
MV_BOOL 	mvBoardSpecInitGet(MV_U32* regOff, MV_U32* data);

/* Board devices API managments */
MV_32  	    mvBoardGetDevicesNumber(MV_BOARD_DEV_CLASS devClass);
MV_32  	    mvBoardGetDeviceBaseAddr(MV_32 devNum, MV_BOARD_DEV_CLASS devClass);
MV_32	    mvBoardGetDeviceBusWidth(MV_32 devNum, MV_BOARD_DEV_CLASS devClass);
MV_32  	    mvBoardGetDeviceWidth(MV_32 devNum, MV_BOARD_DEV_CLASS devClass);
MV_32  	    mvBoardGetDeviceWinSize(MV_32 devNum, MV_BOARD_DEV_CLASS devClass);

/* Gpio Pic Connections API */
MV_32 	    mvBoarGpioPinNumGet(MV_BOARD_DEV_CLASS class);
MV_U8	    mvBoardStatusLedPinNumGet(MV_VOID);
MV_STATUS   mvBoardStatusLed(MV_BOOL status);
MV_32 	    mvBoardUSBVbusGpioPinGet(int devId);
MV_U32      mvBoardPexBridgeIntPinGet(MV_U32 devNum, MV_U32 intPin);
MV_32	    mvBoardResetGpioPinGet(MV_VOID);
MV_32 	    mvBoardRTCGpioPinGet(MV_VOID);
MV_32 	    mvBoardGpioIntMaskGet(MV_VOID);
MV_32 	    mvBoardSlicGpioPinGet(MV_U32 slicNum);

#if defined(MV_INCLUDE_SDIO)
MV_32	    mvBoardSDIOGpioPinGet(MV_VOID);
#endif
#if defined(MV_INCLUDE_PCI)
MV_32      mvBoardPciGpioPinGet(MV_U32 pciIf, MV_U32 idSel, MV_U32 intPin);
/* Misc */
MV_32 	    mvBoardFirstPciSlotDevNumGet(MV_U32 pciIf);
MV_32 	    mvBoardPciSlotsNumGet(MV_U32 pciIf);
MV_BOOL	    mvBoardIsOurPciSlot(MV_U32 busNum, MV_U32 slotNum);
MV_BOOL     mvBoardPciIsMonarch(void);
#endif /* defined(MV_INCLUDE_PCI) */
#if defined(MV_INCLUDE_NAND)
MV_32 	mvBoardNandWidthGet(void);
#endif /* defined(MV_INCLUDE_NAND) */
#endif /* __INCmvBoardEnvLibh */
