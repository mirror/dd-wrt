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


/*#include "mvCpu.h"*/
#include "boardEnv/mvBoardEnvLib.h"
#include "cpu/mvCpu.h"
#include "twsi/mvTwsi.h"

/* defines  */
#ifdef MV_DEBUG
	#define DB(x)	x
#else
	#define DB(x)
#endif	

/* locals */
static int cpuCoreIdGet(MV_VOID);
/*******************************************************************************
* mvCpuSampleAtResetGet - Get the CPU sample at reset register value
*
* DESCRIPTION:
*       Relevent for 1160 only.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit sample at reset register
*
*******************************************************************************/
MV_U32 mvCpuSampleAtResetGet(MV_VOID)
{
    	MV_U8 data[4];
    	MV_U32 mvSampleAtReset = 0;

	/* Read 88F1160 S@R register */
	MV_TWSI_SLAVE	twsiSlave;
	twsiSlave.slaveAddr.type = ADDR7_BIT;
	twsiSlave.slaveAddr.address = CPU_TWSI_DEBUG_ADDR;
	twsiSlave.validOffset = MV_TRUE;
	twsiSlave.offset = 0;
	twsiSlave.moreThen256 = MV_TRUE;

    	if( MV_OK != mvTwsiRead (0, &twsiSlave, data, 4))
    	{
		return 0xffffffff;
    	}
	else
	{
		mvSampleAtReset = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
	}

	return mvSampleAtReset;
}

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

/* Mul constant */
int sys2cpu_clk_ratio_m[] = {1, 3, 2, 5, 3, 7, 4, 9, 5, 1, 6};
/* Div constant */
int sys2cpu_clk_ratio_n[] = {1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1};

MV_U32 mvCpuPclkGet(MV_VOID)
{
	int p = 0;
	MV_U32	tmpPClkRate = MV_REG_READ(CPU_RESET_SAMPLE_L_REG);
	
	p = ((tmpPClkRate & MSAR_SYSCLK2CPU_MASK) >> MSAR_SYSCLK2CPU_OFFS);
	tmpPClkRate = sys2cpu_clk_ratio_m[p];
	tmpPClkRate *= mvBoardSysClkGet();

	return (tmpPClkRate/sys2cpu_clk_ratio_n[p]);
}

MV_U32 mvCpuL2ClkGet(MV_VOID)
{
	MV_U32 	tmpL2ClkRate = MV_REG_READ(CPU_RESET_SAMPLE_L_REG);
	
	tmpL2ClkRate = ((tmpL2ClkRate & MSAR_CPUL2CLK_MASK) >> MSAR_CPUL2CLK_OFFS);
	tmpL2ClkRate++;

	return (mvCpuPclkGet() / tmpL2ClkRate);
}
/*******************************************************************************
* mvAHBclkGet - Get the AHB Clk 
*
* DESCRIPTION:
*       This routine extract the AHB core clock. Relevent for 1160 only.
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
MV_U32 mvAHBclkGet(MV_VOID)
{
   	MV_U32 mvSampleAtReset = 0;
	MV_U32 mvSysClkDiv = 0;
	MV_U32 	tmpPClkRate=0;

	/* Read 88F1160 S@R register */
	mvSampleAtReset = mvCpuSampleAtResetGet();
	if (mvSampleAtReset & BIT12)
	{
		mvSysClkDiv = 32 + (2 * ((mvSampleAtReset >> 7) & 0x1f));
	}
	else
	{
		mvSysClkDiv = (mvSampleAtReset >> 7) & 0x1f;
	}

	tmpPClkRate = mvCpuPclkGet() / mvSysClkDiv;

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
	case CPU_PART_MRVL571:
		mvOsSPrintf(pNameBuff, "%s (Rev %d)", "MRVL571",mvOsCpuRevGet());
		break;
	case CPU_PART_MRVL521:
		mvOsSPrintf(pNameBuff, "%s (Rev %d)", "MRVL521",mvOsCpuRevGet());
		break;
        default:
		mvOsSPrintf(pNameBuff,"??? (0x%04x) (Rev %d)",cpuModel,mvOsCpuRevGet());
    }  /* switch  */
}


/*******************************************************************************
* whoAmI - Get CPU name
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
unsigned int whoAmI(void)
{
#if !defined(MV78XX0_Z0)	
	if (mvCtrlModelGet() != MV_78200_DEV_ID)
		return (unsigned int)MASTER_CPU;

	return (cpuCoreIdGet() > 0) ? SLAVE_CPU: MASTER_CPU;
#else
	return MASTER_CPU;
#endif

}

static int cpuCoreIdGet(MV_VOID)
{
	MV_U32 value;

	__asm__ __volatile__("mrc p15, 1, %0, c15, c1, 0   @ read control reg\n"
		: "=r" (value) :: "memory");
	return !!(value & (1 << 14));
}


