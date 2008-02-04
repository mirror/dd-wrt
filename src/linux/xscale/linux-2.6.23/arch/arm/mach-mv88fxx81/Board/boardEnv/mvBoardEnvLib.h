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



#include "mvCtrlEnvLib.h"
#include "mvSysHwConfig.h"
#include "mvBoardEnvSpec.h"


/* DUART stuff for Tclk detection only */
#define DUART_BAUD_RATE			115200
#define MAX_CLOCK_MARGINE		5000000	/* Maximum detected clock margine */


#define BOARD_ETH_PORT_NUM  1

#define MV_BOARD_MAX_PCI_SLOTS	3
#define MV_BOARD_MAX_PCI_IF		1
#define MV_BOARD_MAX_PORTS		1
#define	MV_BOARD_MAX_USB_IF		2
#define MV_BOARD_MAX_DEV		4
#define MV_BOARD_MAX_TWSI_DEV	8
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



typedef enum _devBoardType
{
	BOARD_DEV_NOR_FLASH,
	BOARD_DEV_NAND_FLASH,
	BOARD_DEV_SEVEN_SEG,
	BOARD_DEV_FPGA,
	BOARD_DEV_SRAM,
	BOARD_DEV_RTC,
	BOARD_DEV_PEX_TO_PCI_BRIDGE,
	BOARD_DEV_MV_6063_SWITCH,
	BOARD_DEV_USB_VBUS,
	BOARD_DEV_REF_CLCK,

}MV_BOARD_DEV_TYPE;


typedef struct _pciBoardSlot
{
	MV_U8	pciSlotGppIntMap[4];

}MV_BOARD_PCI_SLOT;


typedef struct _pciBoardIf
{
	MV_U8				firstSlotDevNum;
	MV_U8				pciSlotsNum;
	MV_BOARD_PCI_SLOT	pciSlot[MV_BOARD_MAX_PCI_SLOTS];

}MV_BOARD_PCI_IF;


typedef struct _devCsInfo
{
    MV_U32		params;
	MV_U32		devType;	/* MV_BOARD_DEV_TYPE */
    MV_U8		devWidth;
	
}MV_DEV_CS_INFO;

/* 
1) 
struct _devTwsiDevInfo is defined for future use to support the following APIs:

MV_U8 mvBoardGetDeviceTwsiAddr(MV_32 devNum, MV_BOARD_DEV_TYPE devType);	
MV_U8 mvBoardGetDeviceTwsiAddrType(MV_32 devNum, MV_BOARD_DEV_TYPE devType);

So the user can search for board device either on Device CSs or Twsi Bus.

And to do this array: MV_DEV_TWSI_INFO twsiDevInfo[MV_BOARD_MAX_TWSI_DEV] 
should be added to the MV_BOARD_INFO structure.

typedef struct _devTwsiInfo
{
	MV_U32	devType;
	MV_U8	twsiDevAddr;
	MV_U8	twsiDevAddrType;
	
}MV_DEV_TWSI_INFO;


*/




typedef struct _boardInfo
{
	char 				boardName[MV_BOARD_NAME_LEN];
	MV_U32				mppGroup[MV_BOARD_MAX_MPP];
    MV_U32				intsGppMask;
    MV_DEV_CS_INFO		devCsInfo[MV_BOARD_MAX_DEV];
	MV_BOARD_PCI_IF		pciBoardIf[MV_BOARD_MAX_PCI_IF];
	MV_U8				rtcTwsiAddr;
	MV_U8				rtcTwsiAddrType;
	MV_U8				pexPciBridgeTwsiAddr;
	MV_U8				pexPciBridgeTwsiAddrType;
	MV_U8				ethPhyAddr[MV_BOARD_MAX_PORTS];
	MV_U8				rtcIntPin;
	MV_U8				switchIntPin;
	MV_U8				vbusUsbGppPin[MV_BOARD_MAX_USB_IF];
    MV_U8				activeLedsNumber;
	MV_U8				ledGppPin[8];
	MV_U8				ledsPolarity;	/* '0' or '1' to turn on led */
	MV_U8				refClkGppPin;

}MV_BOARD_INFO;


MV_VOID 	mvBoardEnvInit(MV_VOID);
MV_U32      mvBoardIdGet(MV_VOID);
MV_U16      mvBoardModelGet(MV_VOID);
MV_U16      mvBoardRevGet(MV_VOID);
MV_STATUS	mvBoardNameGet(char *pNameBuff);
MV_U32      mvBoardPhyAddrGet(MV_U32 ethPortNum);
MV_U32 		mvBoardTclkGet(MV_VOID);
MV_U32      mvBoardSysClkGet(MV_VOID);

MV_VOID     mvBoardDebug7Seg(MV_U32 hexNum);
MV_U32      mvBoardMppGet(MV_U32 mppGroupNum);

MV_U8		mvBoardRtcTwsiAddrTypeGet(MV_VOID);
MV_U8		mvBoardRtcTwsiAddrGet(MV_VOID);

/* Board devices API managments */
MV_32  	    mvBoardGetDevicesNumber(MV_BOARD_DEV_TYPE devType);
MV_32  	    mvBoardGetDeviceBaseAddr(MV_32 devNum, MV_BOARD_DEV_TYPE devType);
MV_32	    mvBoardGetDeviceBusWidth(MV_32 devNum, MV_BOARD_DEV_TYPE devType);
MV_32  	    mvBoardGetDeviceWidth(MV_32 devNum, MV_BOARD_DEV_TYPE devType);

/* Gpio Pic Connections API */
MV_U32 	    mvBoardUSBVbusGpioPinGet(int devId);
MV_U32      mvBoardPexBridgeIntPinGet(MV_U32 devNum, MV_U32 intPin);
MV_U32      mvBoardPciGpioPinGet(MV_U32 pciIf, MV_U32 idSel, MV_U32 intPin);
MV_U32 	    mvBoardRTCGpioPinGet(MV_VOID);
MV_U32 	    mvBoardGpioIntMaskGet(MV_VOID);
MV_U32 	    mvBoardSlicGpioPinGet(MV_U32 slicNum);

/* Misc */
MV_U32 	    mvBoardFirstPciSlotDevNumGet(MV_U32 pciIf);
MV_U32 	    mvBoardPciSlotsNumGet(MV_U32 pciIf);

#endif /* __INCmvBoardEnvLibh */
