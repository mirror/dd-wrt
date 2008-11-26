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

#include "ctrlEnv/mvCtrlEnvSpec.h"
#include "boardEnv/mvBoardEnvSpec.h"
#include "ctrlEnv/mvCtrlEnvLib.h"
#include "mvSysHwConfig.h"

#define BD_NAME_OFFS
#define BD_MODEL_MAJOR_OFFS
#define BD_MODEL_MINOR_OFFS
#define BD_REV_MAJOR_OFFS
#define BD_REV_MINOR_OFFS


/* Board specific */
/* =============================== */

/* boards ID numbers */

/* New board ID numbers */

#define DB_78XX0_ID	    0x00000050	
#define DB_78200_ID	    0x00000090	
#define DB_78XX0_BP_MLL_ID	1683
#define RD_78XX0_AMC_ID	    0x00000060	
#define RD_78XX0_MASA_ID    0x00000070
#define RD_78XX0_H3C_ID     0x00000080

#define MAX_BOARD_NAME_LEN  0x20
#define UNKNOWN_BOARD_ID  	0x0

/* DUART stuff for Tclk detection only */
#define DUART_BAUD_RATE			115200
#define MAX_CLOCK_MARGINE		5000000	/* Maximum detected clock margine */


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
	BOARD_DEV_OTHER,   
} MV_BOARD_DEV_CLASS;


typedef struct _devCsInfo
{
    MV_U8		deviceCS;
    MV_U32		params;
    MV_U32		devClass;	/* MV_BOARD_DEV_CLASS */
    MV_U8		devWidth;
	
}MV_DEV_CS_INFO;

typedef enum _boardMacSpeed
{
	BOARD_MAC_SPEED_10M,
	BOARD_MAC_SPEED_100M,
	BOARD_MAC_SPEED_1000M,
	BOARD_MAC_SPEED_AUTO,

}MV_BOARD_MAC_SPEED;

/* Locals */


MV_VOID 	mvBoardEnvInit(MV_VOID);
MV_U32      mvBoardIdGet(MV_VOID);
MV_STATUS	mvBoardNameGet(char *pNameBuff);
MV_U32      mvBoardPhyAddrGet(MV_U32 ethPortNum);
MV_BOARD_MAC_SPEED      mvBoardMacSpeedGet(MV_U32 ethPortNum);
MV_BOOL 	mvBoardIsPortInSgmii(MV_U32 ethPortNum);
MV_32 		mvBoardTclkGet(MV_VOID);
MV_32      	mvBoardSysClkGet(MV_VOID);
MV_VOID     mvBoardDebugLed(MV_U32 hexNum);
MV_VOID     mvBoardDebug7Seg(MV_U32 hexNum);

MV_U32      mvBoardMppGet(MV_U32 mppGroupNum);

MV_U8		mvBoardRtcTwsiAddrTypeGet(MV_VOID);
MV_U8		mvBoardRtcTwsiAddrGet(MV_VOID);


MV_32	    mvBoardPciGpioPinGet(MV_U32 pciIf, MV_U32 idSel, MV_U32 intPin);
MV_32	    mvBoardWDGpioPinGet(MV_VOID);
MV_32	    mvBoardDbgLedGpioMaskGet(MV_VOID);

MV_32	    mvBoardGpioIntMaskGet(MV_VOID);
MV_32	    mvBoardUSBVbusGpioPinGet(int devId);
MV_32	    mvBoardUSBVbusEnGpioPinGet(int devId);

MV_VOID		mvBoardReset(MV_VOID);
MV_TARGET   	mvBoardGetDevCSNum(MV_32 devNum, MV_BOARD_DEV_CLASS devClass);
MV_32 		mvBoardGetDevicesNumber(MV_BOARD_DEV_CLASS devClass);
MV_U32 		mvBoardGetDeviceBaseAddr(MV_32 devNum, MV_BOARD_DEV_CLASS devClass);
MV_U32		mvBoardGetDeviceWinSize(MV_32 devNum, MV_BOARD_DEV_CLASS devClass);
MV_U32		mvBoardGetDeviceBusWidth(MV_32 devNum, MV_BOARD_DEV_CLASS devClass);
MV_32		mvBoardGetDeviceWidth(MV_32 devNum, MV_BOARD_DEV_CLASS devClass);

MV_BOOL	    	mvBoardIsBootFromNand(MV_VOID);
MV_BOOL 	mvBoardIsBootFromSpi(MV_VOID);
MV_BOOL 	mvBoardIsBootFromNor(MV_VOID);
MV_VOID 	mvBoardEthPhyInit(MV_VOID);
MV_BOOL		mvBoardSpecInitGet(MV_U32* regOff, MV_U32* data);
MV_U32 		mvBoardFlashParamGet(MV_VOID);
MV_U32 		mvBoardGetMaxUsbIf(MV_VOID);
#endif /* __INCmvBoardEnvLibh */
