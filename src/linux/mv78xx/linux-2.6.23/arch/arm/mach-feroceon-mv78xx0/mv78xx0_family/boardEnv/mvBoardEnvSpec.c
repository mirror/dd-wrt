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

#include "boardEnv/mvBoardEnvLib.h"
#include "ctrlEnv/sys/mvCpuIf.h"
#include "cntmr/mvCntmr.h"
#include "device/mvDevice.h"

/* defines  */
#ifdef DEBUG
	#define DB(x)	x
#else
	#define DB(x)
#endif	


/*******************************************************************************
* mvBoardPciGpioPinGet - Get board PCI interrupt level.
*
* DESCRIPTION:
*		This function returns the value of Gpp Pin that is connected 
*		to the specified IDSEL and interrupt pin (A,B,C,D). For example, If 
*		IDSEL 8 (device 8) interrupt A is connected to GPIO pin 4 the function 
*		will return the value 4.
*		This function supports multiple PCI interfaces.
*
* INPUT:
*		pciIf  - PCI interface number. 
*		devNum - device number (IDSEL).
*		intPin - Interrupt pin (A=1, B=2, C=3, D=4).
*
* OUTPUT:
*		None.
*
* RETURN:
*       GPIO pin number. The function return -1 for bad parameters.
*
*******************************************************************************/
MV_32 mvBoardPciGpioPinGet(MV_U32 pciIf, MV_U32 devNum, MV_U32 intPin)
{
	/*No PCI*/
	return MV_ERROR;
}

/*******************************************************************************
* mvBoardWDGpioPinGet - Get board Watchdog NMI GPP pin number.
*
* DESCRIPTION:
*		This function returns the number of Gpp Pin that is connected 
*		to the watchdog NMI interrupt.
*
* INPUT:
*		None.
*
* OUTPUT:
*		None.
*
* RETURN:
*       GPIO pin number. The function return -1 for bad parameters.
*
*******************************************************************************/
MV_32 mvBoardWDGpioPinGet(MV_VOID)
{
	int intPin = -1;
	MV_U32 boardID;

	boardID = mvBoardIdGet();

	if ((boardID == DB_78XX0_ID) || (boardID == RD_78XX0_AMC_ID) || 
	    (boardID == RD_78XX0_MASA_ID) || (boardID == RD_78XX0_H3C_ID))
	{
    		return MV_ERROR;
    	}

    return intPin;
}

/*******************************************************************************
* mvBoardDbgLedGpioMaskGet - Get board debug LED GPP bit mask.
*
* DESCRIPTION:
*		This function returns a bit-mask of Gpp Pin that is connected 
*		to the debug LED.
*
* INPUT:
*		None.
*
* OUTPUT:
*		None.
*
* RETURN:
*       32-bit bit-mask. The function return -1 for bad parameters.
*
*******************************************************************************/
MV_32 mvBoardDbgLedGpioMaskGet(MV_VOID)
{
	MV_U32 boardID;
	MV_U32 intPin = -1;

	boardID = mvBoardIdGet();
	switch(boardID){
	case DB_78XX0_ID:
#ifdef MV_INCLUDE_MODEM_ON_TTYS1
		intPin = 0;
#else
    		return MV_ERROR;
#endif
		break;
	case RD_78XX0_AMC_ID:
		intPin = 0;
		break;
	case RD_78XX0_MASA_ID:
		intPin = (BIT22 | BIT23);
		break;
	}

    return intPin;
}

