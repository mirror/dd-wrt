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


#include "mvCpu.h"
#include "mvBoardEnvLib.h"


/* defines  */
#ifdef MV_DEBUG
	#define DB(x)	x
#else
	#define DB(x)
#endif	

/* locals */

/*******************************************************************************
* mvCpuPclkGet - Get the CPU pClk (pipe clock)
*
* DESCRIPTION:
*       This routine extract the CPU core clock.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit clock cycles in Hertz.
*
*******************************************************************************/
MV_U32 mvCpuPclkGet(MV_VOID)
{
	MV_U32 	tmpPClkRate=0;
	MV_U32	tmp;
	

#ifdef SYSCLK_AUTO_DETECT
		
		tmp = MV_REG_READ(MPP_SAMPLE_AT_RESET);

		tmpPClkRate = tmp & MSAR_ARMDDRCLCK_MASK;
		
		if (mvCtrlModelGet() == MV_5281_DEV_ID)
			if(tmp & MSAR_ARMDDRCLCK_H_MASK)
				tmpPClkRate |= BIT8;

		switch (tmpPClkRate)
		{
		case MSAR_ARMDDRCLCK_333_167:
			tmpPClkRate = 333000000;
			break;
		case MSAR_ARMDDRCLCK_400_200:
			tmpPClkRate = 400000000;
			break;
		case MSAR_ARMDDRCLCK_400_133:
			tmpPClkRate = 400000000;
			break;
		case MSAR_ARMDDRCLCK_500_167:
			tmpPClkRate = 500000000;
			break;
		case MSAR_ARMDDRCLCK_533_133:
			tmpPClkRate = 533000000;
			break;
		case MSAR_ARMDDRCLCK_600_200:
			tmpPClkRate = 600000000;
			break;
		case MSAR_ARMDDRCLCK_667_167:
			tmpPClkRate = 667000000;
			break;
		case MSAR_ARMDDRCLCK_800_200:	
			tmpPClkRate = 800000000;
			break;
	   case MSAR_ARMDDRCLCK_550_183:
				tmpPClkRate = 550000000;
				break;
		case MSAR_ARMDDRCLCK_480_160:
			tmpPClkRate = 480000000;
			break;
	   case MSAR_ARMDDRCLCK_466_233:
			  tmpPClkRate = 466000000;
				break;
	   case MSAR_ARMDDRCLCK_500_250:
				tmpPClkRate = 500000000;
				break;
		case MSAR_ARMDDRCLCK_525_175:
			tmpPClkRate = 525000000;
			break;
	   case MSAR_ARMDDRCLCK_533_266:
				tmpPClkRate = 533000000;
				break;
	   case MSAR_ARMDDRCLCK_600_300:
				tmpPClkRate = 600000000;
				break;
		case MSAR_ARMDDRCLCK_450_150:
			tmpPClkRate = 450000000;
			break;
		case MSAR_ARMDDRCLCK_533_178:
			tmpPClkRate = 533000000;
			break;
		case MSAR_ARMDDRCLCK_575_192:
			tmpPClkRate = 575000000;
			break;
		case MSAR_ARMDDRCLCK_700_175:
			tmpPClkRate = 700000000;
			break;
		case MSAR_ARMDDRCLCK_733_183:
			tmpPClkRate = 733000000;
			break;
		case MSAR_ARMDDRCLCK_750_187:
			tmpPClkRate = 750000000;
			break;
		case MSAR_ARMDDRCLCK_775_194:
			tmpPClkRate = 775000000;
			break;
		case MSAR_ARMDDRCLCK_500_125:
			tmpPClkRate = 500000000;
			break;
		case MSAR_ARMDDRCLCK_500_100:
			tmpPClkRate = 500000000;
			break;
		case MSAR_ARMDDRCLCK_600_150:
			tmpPClkRate = 600000000;
			break;


		}

#else
		tmpPClkRate = MV_BOARD_DEFAULT_PCLK;
#endif


	return tmpPClkRate;
}

/*******************************************************************************
* mvCpuNameGet - Get CPU name
*
* DESCRIPTION:
*       This function returns a string describing the CPU model and revision.
*
* INPUT:
*       None.
*
* OUTPUT:
*       pNameBuff - Buffer to contain board name string. Minimum size 32 chars.
*
* RETURN:
*       None.
*******************************************************************************/
MV_VOID mvCpuNameGet(char *pNameBuff)
{
    MV_U32 cpuModel;
    
    cpuModel = mvOsCpuPartGet();

    /* The CPU module is indicated in the Processor Version Register (PVR) */
    switch(cpuModel)
    {
        case CPU_PART_ARM926:
            mvOsSPrintf(pNameBuff, "%s (Rev %d)", "ARM926",mvOsCpuRevGet());
            break;
	case CPU_PART_ARM946:
		mvOsSPrintf(pNameBuff, "%s (Rev %d)", "ARM946",mvOsCpuRevGet());
		break;
        default:
            mvOsSPrintf(pNameBuff,"??? (0x%04x) (Rev %d)",cpuModel,mvOsCpuRevGet());
            break;
    }  /* switch  */

    return;
}



