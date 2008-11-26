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


/* includes */
#include "ctrlEnv/sys/mvCpuIf.h"
#include "ctrlEnv/sys/mvAhbToMbusRegs.h"
#include "cpu/mvCpu.h"
#include "ctrlEnv/sys/mvSysDram.h"
#include "ctrlEnv/mvCtrlEnvLib.h"
#include "mvSysHwConfig.h"
#ifdef MV78200
#include "mv78200/mvSocUnitMap.h"
#endif
/*#define MV_DEBUG*/
/* defines  */

#ifdef MV_DEBUG
	#define DB(x)	x
#else
	#define DB(x)
#endif	

/* typedefs */
/* CPU address decode registers offsets are inconsecutive. This struct 		*/
/* describes address decode register offsets					*/
typedef struct _cpuDecRegOffs           
{
    MV_U32 baseRegOffs;	/* Base address register offset.			*/
    MV_U32 sizeRegOffs;	/* Size address register offset. 			*/
			/* Used as size itself for windows with no size 	*/
			/* registers (e.g. internal registers) 			*/
}CPU_DEC_REG_OFFS;

/* CPU address remap registers offsets are inconsecutive. This struct 		*/
/* describes address remap register offsets					*/
typedef struct _cpuRemapRegOffs           
{
    MV_U32 lowRegOffs;		/* Low 32-bit remap register offset		*/
    MV_U32 highRegOffs;    	/* High 32 bit remap register offset		*/
}CPU_REMAP_REG_OFFS;

#if defined (MV_BRIDGE_SYNC_REORDER)
MV_U32 *mvUncachedParam = NULL;
#endif

/* locals   */

/* CPU address decode table. Note that table entry number must match its    */
/* target enumerator. For example, table entry '4' must describe Deivce CS0 */
/* target which is represent by DEVICE_CS0 enumerator (4).                  */


/* locals   */
/* static functions */
static MV_BOOL cpuTargetWinOverlap(MV_U32 cpu, MV_TARGET target, MV_ADDR_WIN *pAddrWin);
#ifdef MV_INCLUDE_MONT_EXT
MV_STATUS mvBoardSpecWinMapOverride(MV_VOID);
#endif
MV_TARGET sampleAtResetTargetArray[] = BOOT_TARGETS_NAME_ARRAY;
/*******************************************************************************
* mvCpuIfInit - Initialize Controller CPU interface
*
* DESCRIPTION:
*       This function initialize Controller CPU interface:
*       1. Set CPU interface configuration registers.
*       2. Set CPU master Pizza arbiter control according to static 
*          configuration described in configuration file.
*       3. Opens CPU address decode windows. DRAM windows are assumed to be 
*		   already set (auto detection).
*
* INPUT:
*       cpu      	- CPU id
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_STATUS mvCpuIfInitForCpu(MV_U32 cpu, MV_CPU_DEC_WIN *cpuAddrWinMap)
{    
	MV_U32 regVal;
	MV_TARGET target;
	MV_ADDR_WIN addrWin;
    	MV_CPU_DEC_WIN *winMap = cpuAddrWinMap;

	if (cpuAddrWinMap == NULL)
	{
		DB(mvOsPrintf("mvCpuIfInit:ERR. cpuAddrWinMap == NULL\n"));
		return MV_ERROR;
	}

	/* Set ARM Configuration register */
	regVal  = MV_REG_READ(CPU_CONFIG_REG(cpu));
	regVal &= ~CPU_CONFIG_DEFAULT_MASK;
	regVal |= CPU_CONFIG_DEFAULT;


	MV_REG_WRITE(CPU_CONFIG_REG(cpu),regVal);

	/* Set ARM Control and Status register */
	regVal  = MV_REG_READ(CPU_CTRL_STAT_REG(cpu));
	regVal &= ~CPU_CTRL_STAT_DEFAULT_MASK;
	regVal |= CPU_CTRL_STAT_DEFAULT;
	MV_REG_WRITE(CPU_CTRL_STAT_REG(cpu),regVal);


	/* First disable all CPU target windows  */
	for (target = 0; winMap[target].enable != TBL_TERM; target++)
    	{
		if ((MV_TARGET_IS_DRAM(target))||(target == INTER_REGS))
		{
			continue;
		}

#ifdef MV_MEM_OVER_PEX_WA
	/* If the target PEX or PCI and memory is over PEX or PCI we don't touch this CPU windows */
		if (MV_TARGET_IS_PEX(target))
		{
			continue;
		}
#endif
		mvCpuIfTargetWinEnableForCpu(cpu, MV_CHANGE_BOOT_CS(target),MV_FALSE);
	}

	/* Go through all targets in user table until table terminator			*/
	for (target = 0; winMap[target].enable != TBL_TERM; target++)
    	{
	/* if DRAM auto sizing is used do not initialized DRAM target windows, 	*/
	/* assuming this already has been done earlier.							*/
#ifdef	MV_DRAM_AUTO_SIZE
		if (MV_TARGET_IS_DRAM(target))
		{
			continue;
		}
#endif

#ifdef MV_MEM_OVER_PEX_WA
	/* If the target PEX or PCI and memory is over PEX or PCI we don't touch this CPU windows */
		if (MV_TARGET_IS_PEX(target))
		{
			continue;
		}
#endif
	/* If the target attribute is the same as the boot device attribute */
	/* then it's stays disable */
#if !defined(MV78XX0_Z0)
		if (mvBoardIsBootFromSpi() && (target == SPI_CS))
			continue;
#endif
		if (MV_TARGET_IS_AS_BOOT(target))	
		{
			continue;
		}

		if((0 == winMap[target].addrWin.size) ||
		   (DIS == winMap[target].enable))

		{
			
			if (MV_OK != mvCpuIfTargetWinEnableForCpu(cpu, target, MV_FALSE))
			{
				DB(mvOsPrintf("mvCpuIfInit:ERR. mvCpuIfTargetWinEnable fail\n"));
				return MV_ERROR;
			}

		}
		else
		{
			if (MV_OK != mvCpuIfTargetWinSetForCpu(cpu, 
							       target, 
							       &winMap[target]))
			{
				DB(mvOsPrintf("mvCpuIfInit:ERR. mvCpuIfTargetWinSet fail\n"));


				return MV_ERROR;
			}

			addrWin.baseLow = winMap[target].addrWin.baseLow;
			addrWin.baseHigh = winMap[target].addrWin.baseHigh;
			if (0xffffffff == mvAhbToMbusWinRemap(cpu, winMap[target].winNum ,&addrWin))
			{
				DB(mvOsPrintf("mvCpuIfInit:WARN. mvAhbToMbusWinRemap can't remap winNum=%d\n",
							  winMap[target].winNum));
			}
		}
    }
	return MV_OK;
}

MV_STATUS mvCpuIfInit(MV_CPU_DEC_WIN *cpuAddrWinMap)
{
	return mvCpuIfInitForCpu(whoAmI(), cpuAddrWinMap);
}


#if defined (MV_BRIDGE_SYNC_REORDER)
MV_STATUS mvCpuIfBridgeReorderWAInit(void)
{	
    MV_ULONG tmpPhysAddress;
    mvUncachedParam = mvOsIoUncachedMalloc(NULL, 4, &tmpPhysAddress, NULL);
    if (mvUncachedParam == NULL) {
	mvOsPrintf("Uncached memory allocation failed\n"); 
	return MV_ERROR;
    }
    return MV_OK;
}
#endif

/*******************************************************************************
* mvCpuIfTargetWinSet - Set CPU-to-peripheral target address window
*
* DESCRIPTION:
*       This function sets a peripheral target (e.g. SDRAM bank0, PCI0_MEM0) 
*       address window, also known as address decode window.  
*       A new address decode window is set for specified target address window.
*       If address decode window parameter structure enables the window, 
*       the routine will also enable the target window, allowing CPU to access
*       the target window.
*
* INPUT:
*		cpu			- cpu id
*       target      - Peripheral target enumerator.
*       pAddrDecWin - CPU target window data structure.
*
* OUTPUT:
*       N/A
*
* RETURN:
*       MV_OK if CPU target window was set correctly, MV_ERROR in case of  
*       address window overlapps with other active CPU target window or
*		trying to assign 36bit base address while CPU does not support that.
*       The function returns MV_NOT_SUPPORTED, if the target is unsupported.
*
*******************************************************************************/
MV_STATUS mvCpuIfTargetWinSetForCpu(MV_U32 cpu, MV_TARGET target, MV_CPU_DEC_WIN *pAddrDecWin)
{
	MV_AHB_TO_MBUS_DEC_WIN decWin;
	MV_U32 existingWinNum;
	MV_DRAM_DEC_WIN addrDecWin;

	target = MV_CHANGE_BOOT_CS(target);

	/* Check parameters */
	if (cpu >= MV_MAX_CPU)
	{
		mvOsPrintf("mvCpuIfTargetWinSet: ERR. Invalid cpu %d\n", cpu);
		return MV_ERROR;
	}

	if (target >= MAX_TARGETS)
	{
		mvOsPrintf("mvCpuIfTargetWinSet: target %d is Illegal\n", target);
		return MV_ERROR;
	}

    	/* 2) Check if the requested window overlaps with current windows		*/
    	if (MV_TRUE == cpuTargetWinOverlap(cpu, target, &pAddrDecWin->addrWin))
	{
        	mvOsPrintf("mvCpuIfTargetWinSet: ERR. Target %d overlap\n", target);
		return MV_BAD_PARAM;
	}

	if (MV_TARGET_IS_DRAM(target))
	{
		/* copy relevant data to MV_DRAM_DEC_WIN structure */
		addrDecWin.addrWin.baseHigh = pAddrDecWin->addrWin.baseHigh;
		addrDecWin.addrWin.baseLow = pAddrDecWin->addrWin.baseLow;
		addrDecWin.addrWin.size = pAddrDecWin->addrWin.size;
		addrDecWin.enable = pAddrDecWin->enable;

		if (MV_OK != mvDramIfWinSet(cpu,target,&addrDecWin))
		{
			mvOsPrintf("mvCpuIfTargetWinSet: mvDramIfWinSet Failed\n");
			return MV_ERROR;
		}
		
	}
	else
	{
		/* copy relevant data to MV_AHB_TO_MBUS_DEC_WIN structure */
		decWin.addrWin.baseLow = pAddrDecWin->addrWin.baseLow;
		decWin.addrWin.baseHigh = pAddrDecWin->addrWin.baseHigh;
		decWin.addrWin.size = pAddrDecWin->addrWin.size;
		decWin.enable = pAddrDecWin->enable;
		decWin.target = target;

		existingWinNum = mvAhbToMbusWinTargetGet(cpu, target);

		/* check if there is already another Window configured
		for this target */
		if ((existingWinNum < MAX_AHB_TO_MBUS_WINS )&&
			(existingWinNum != pAddrDecWin->winNum))
		{
			/* if we want to enable the new winow number 
			passed by the user , then the old one should
			be disabled */
			if (MV_TRUE == pAddrDecWin->enable)
			{
				/* be sure it is disabled */
				mvAhbToMbusWinEnable(cpu, existingWinNum , MV_FALSE);
			}
		}

		if (mvAhbToMbusWinSet(cpu, pAddrDecWin->winNum,&decWin) != MV_OK)
		{
			mvOsPrintf("mvCpuIfTargetWinSet: mvAhbToMbusWinSet Failed\n");
			return MV_ERROR;
		}
	}

    return MV_OK;
}

MV_STATUS mvCpuIfTargetWinSet( MV_TARGET target, MV_CPU_DEC_WIN *pAddrDecWin)
{
	return mvCpuIfTargetWinSetForCpu(whoAmI(), target, pAddrDecWin);
}

/*******************************************************************************
* mvCpuIfTargetWinGetForCpu - Get CPU-to-peripheral target address window
*
* DESCRIPTION:
*		Get the CPU peripheral target address window.
*
* INPUT:
*       target - Peripheral target enumerator
*
* OUTPUT:
*       pAddrDecWin - CPU target window information data structure.
*
* RETURN:
*       MV_OK if target exist, MV_ERROR otherwise.
*
*******************************************************************************/
MV_STATUS mvCpuIfTargetWinGetForCpu(MV_U32 cpu, MV_TARGET target, MV_CPU_DEC_WIN *pAddrDecWin)
{

	MV_U32 winNum=0xffffffff;
	MV_AHB_TO_MBUS_DEC_WIN decWin;
	MV_DRAM_DEC_WIN addrDecWin;

	target = MV_CHANGE_BOOT_CS(target);

	/* Check parameters */
	if (cpu >= MV_MAX_CPU)
	{
		mvOsPrintf("mvCpuIfTargetWinGet: ERR. Invalid cpu %d\n", cpu);
		return MV_ERROR;
	}

	if (target >= MAX_TARGETS)
	{
		mvOsPrintf("mvCpuIfTargetWinGet: target %d is Illigal\n", target);
		return MV_ERROR;
	}

	if (MV_TARGET_IS_DRAM(target))
	{
		if (mvDramIfWinGet(cpu,target,&addrDecWin) != MV_OK)
		{
			mvOsPrintf("mvCpuIfTargetWinGet: Failed to get window target %d\n",
					   target);
			return MV_ERROR;
		}

		/* copy relevant data to MV_CPU_DEC_WIN structure */
		pAddrDecWin->addrWin.baseLow = addrDecWin.addrWin.baseLow;
		pAddrDecWin->addrWin.baseHigh = addrDecWin.addrWin.baseHigh;
		pAddrDecWin->addrWin.size = addrDecWin.addrWin.size;
		pAddrDecWin->enable = addrDecWin.enable;
		pAddrDecWin->winNum = 0xffffffff;

	}
	else
	{
		/* get the Window number associated with this target */

		winNum = mvAhbToMbusWinTargetGet(cpu, target);
		if (winNum >= MAX_AHB_TO_MBUS_WINS)
		{
			return MV_NO_SUCH;

		}

		if (mvAhbToMbusWinGet(cpu, winNum , &decWin) != MV_OK)
		{
			mvOsPrintf("%s: mvAhbToMbusWinGet Failed at winNum = %d\n",
					   __FUNCTION__, winNum);
			return MV_ERROR;

		}

		/* copy relevant data to MV_CPU_DEC_WIN structure */
		pAddrDecWin->addrWin.baseLow = decWin.addrWin.baseLow;
		pAddrDecWin->addrWin.baseHigh = decWin.addrWin.baseHigh;
		pAddrDecWin->addrWin.size = decWin.addrWin.size;
		pAddrDecWin->enable = decWin.enable;
		pAddrDecWin->winNum = winNum;

	}

	return MV_OK;
}


/*******************************************************************************
* mvCpuIfTargetWinGet - Get CPU-to-peripheral target address window
*
* DESCRIPTION:
*		Get the CPU peripheral target address window.
*
* INPUT:
*       target - Peripheral target enumerator
*
* OUTPUT:
*       pAddrDecWin - CPU target window information data structure.
*
* RETURN:
*       MV_OK if target exist, MV_ERROR otherwise.
*
*******************************************************************************/
MV_STATUS mvCpuIfTargetWinGet(MV_TARGET target, MV_CPU_DEC_WIN *pAddrDecWin)
{
	return mvCpuIfTargetWinGetForCpu(whoAmI(), target, pAddrDecWin);
}

/*******************************************************************************
* mvCpuIfTargetWinEnable - Enable/disable a CPU address decode window
*
* DESCRIPTION:
*       This function enable/disable a CPU address decode window.
*       if parameter 'enable' == MV_TRUE the routine will enable the 
*       window, thus enabling CPU accesses (before enabling the window it is 
*       tested for overlapping). Otherwise, the window will be disabled.
*
* INPUT:
*       cpu    - CPU id
*       target - Peripheral target enumerator.
*       enable - Enable/disable parameter.
*
* OUTPUT:
*       N/A
*
* RETURN:
*       MV_ERROR if protection window number was wrong, or the window 
*       overlapps other target window.
*
*******************************************************************************/
MV_STATUS mvCpuIfTargetWinEnableForCpu(MV_U32 cpu, MV_TARGET target,MV_BOOL enable)
{
	MV_U32 winNum, temp;
	MV_CPU_DEC_WIN addrDecWin;

	/* Check parameters */
	if (cpu >= MV_MAX_CPU)
	{
		mvOsPrintf("mvCpuIfTargetWinEnable: ERR. Invalid cpu %d\n", cpu);
		return MV_ERROR;
	}

	target = MV_CHANGE_BOOT_CS(target);

	/* Check parameters */
	if (target >= MAX_TARGETS)
	{
		mvOsPrintf("mvCpuIfTargetWinEnable: target %d is Illigal\n", target);
		return MV_ERROR;
	}

	/* get the window and check if it exist */
	temp = mvCpuIfTargetWinGetForCpu(cpu, target, &addrDecWin);
	if (MV_NO_SUCH == temp)
	{
		return (enable? MV_ERROR: MV_OK);
	}
	else if( MV_OK != temp)
	{
		mvOsPrintf("%s: ERR. Getting target %d failed.\n",__FUNCTION__, target);
		return MV_ERROR;
	}


	/* check overlap */

	if (MV_TRUE == enable)
	{
		if (MV_TRUE == cpuTargetWinOverlap(cpu, target, &addrDecWin.addrWin))
		{
			DB(mvOsPrintf("%s: ERR. Target %d overlap\n",__FUNCTION__, target));
			return MV_ERROR;
		}
		
	}


	if (MV_TARGET_IS_DRAM(target))
	{
		if (mvDramIfWinEnable(cpu, target, enable) != MV_OK)
		{
			mvOsPrintf("mvCpuIfTargetWinGet: mvDramIfWinEnable Failed at \n");
			return MV_ERROR;

		}

	}
	else
	{
		/* get the Window number associated with this target */

		winNum = mvAhbToMbusWinTargetGet(cpu, target);

		if (winNum >= MAX_AHB_TO_MBUS_WINS)
		{
			return (enable? MV_ERROR: MV_OK);
		}

		if (mvAhbToMbusWinEnable(cpu, winNum, enable) != MV_OK)
		{
			mvOsPrintf("mvCpuIfTargetWinGet: Failed to enable window = %d\n",
					   winNum);
			return MV_ERROR;

		}

	}

	return MV_OK;
}

MV_STATUS mvCpuIfTargetWinEnable( MV_TARGET target,MV_BOOL enable)
{
	return mvCpuIfTargetWinEnableForCpu(whoAmI(),  target, enable);
}

/*******************************************************************************
* mvCpuIfTargetWinSizeGet - Get CPU target address window size
*
* DESCRIPTION:
*		Get the size of CPU-to-peripheral target window.
*
* INPUT:
*       cpu    - CPU id
*       target - Peripheral target enumerator
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit size. Function also returns '0' if window is closed.
*		Function returns 0xFFFFFFFF in case of an error.
*
*******************************************************************************/
MV_U32    mvCpuIfTargetWinSizeGetForCpu(MV_U32 cpu, MV_TARGET target)
{
	MV_CPU_DEC_WIN addrDecWin;

	target = MV_CHANGE_BOOT_CS(target);

	/* Check parameters */
	if (cpu >= MV_MAX_CPU)
	{
		mvOsPrintf("mvCpuIfTargetWinSizeGet: ERR. Invalid cpu %d\n", cpu);
		return 0;
	}

	if (target >= MAX_TARGETS)
	{
		mvOsPrintf("mvCpuIfTargetWinSizeGet: target %d is Illigal\n", target);
		return 0;
	}

    /* Get the winNum window */
	if (MV_OK != mvCpuIfTargetWinGetForCpu(cpu, target, &addrDecWin))
	{
		mvOsPrintf("mvCpuIfTargetWinSizeGet:ERR. Getting target %d failed.\n", 
                                                                        target);
		return 0;
	}

	/* Check if window is enabled   */
	if (addrDecWin.enable == MV_TRUE)
    {
		return (addrDecWin.addrWin.size);
    }
    else
    {
        return 0;		/* Window disabled. return 0 */
    }
}

MV_U32    mvCpuIfTargetWinSizeGet(MV_TARGET target)
{
	return mvCpuIfTargetWinSizeGetForCpu(whoAmI(), target);
}

/*******************************************************************************
* mvCpuIfTargetWinBaseLowGet - Get CPU target address window base low
*
* DESCRIPTION:
*       CPU-to-peripheral target address window base is constructed of 
*       two parts: Low and high.
*		This function gets the CPU peripheral target low base address.
*
* INPUT:
*       cpu    - CPU id
*       target - Peripheral target enumerator
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit low base address.
*
*******************************************************************************/
MV_U32   mvCpuIfTargetWinBaseLowGetForCpu(MV_U32 cpu, MV_TARGET target)
{
	MV_CPU_DEC_WIN addrDecWin;
    
	target = MV_CHANGE_BOOT_CS(target);

	/* Check parameters */
	if (cpu >= MV_MAX_CPU)
	{
		mvOsPrintf("mvCpuIfTargetWinBaseLowGet: ERR. Invalid cpu %d\n", cpu);
		return 0xffffffff;
	}

	if (target >= MAX_TARGETS)
	{
		mvOsPrintf("mvCpuIfTargetWinBaseLowGet: target %d is Illigal\n", target);
		return 0xffffffff;
	}

    /* Get the target window */
	if (MV_OK != mvCpuIfTargetWinGetForCpu(cpu, target, &addrDecWin))
	{
		mvOsPrintf("mvCpuIfTargetWinBaseLowGet:ERR. Getting target %d failed.\n", 
                                                                        target);
		return 0xffffffff;
	}

	if (MV_FALSE == addrDecWin.enable)
	{
		return 0xffffffff;
	}
	return (addrDecWin.addrWin.baseLow);
}

MV_U32   mvCpuIfTargetWinBaseLowGet(MV_TARGET target)
{
	return mvCpuIfTargetWinBaseLowGetForCpu(whoAmI(), target);
}

/*******************************************************************************
* mvCpuIfTargetWinBaseHighGet - Get CPU target address window base high
*
* DESCRIPTION:
*       CPU-to-peripheral target address window base is constructed of 
*       two parts: Low and high.
*		This function gets the CPU peripheral target high base address.
*
* INPUT:
*       target - Peripheral target enumerator
*
* OUTPUT:
*       cpu    - CPU id
*       None.
*
* RETURN:
*       32bit high base address.
*
*******************************************************************************/
MV_U32    mvCpuIfTargetWinBaseHighGetForCpu(MV_U32 cpu, MV_TARGET target)
{
	MV_CPU_DEC_WIN addrDecWin;
    
	target = MV_CHANGE_BOOT_CS(target);

	/* Check parameters */

	if (cpu >= MV_MAX_CPU)
	{
		mvOsPrintf("mvCpuIfTargetWinBaseHighGet: ERR. Invalid cpu %d\n", cpu);
		return 0xffffffff;
	}

	if (target >= MAX_TARGETS)
	{
		mvOsPrintf("mvCpuIfTargetWinBaseLowGet: target %d is Illigal\n", target);
		return 0xffffffff;
	}

    /* Get the target window */
	if (MV_OK != mvCpuIfTargetWinGetForCpu(cpu, target, &addrDecWin))
	{
		mvOsPrintf("mvCpuIfTargetWinBaseHighGet:ERR. Getting target %d failed.\n", 
                                                                        target);
		return 0xffffffff;
	}

	if (MV_FALSE == addrDecWin.enable)
	{
		return 0;
	}

	return (addrDecWin.addrWin.baseHigh);
}

MV_U32    mvCpuIfTargetWinBaseHighGet(MV_TARGET target)
{
	return mvCpuIfTargetWinBaseHighGetForCpu(whoAmI(), target);
}

#if defined(MV_INCLUDE_PEX)
/*******************************************************************************
* mvCpuIfPciIfRemap - Set CPU remap register for address windows.
*
* DESCRIPTION:
*
* INPUT:
*       pciTarget   - Peripheral target enumerator. Must be a PCI target.
*       pAddrDecWin - CPU target window information data structure.
*                     Note that caller has to fill in the base field only. The
*                     size field is ignored.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_ERROR if target is not a PCI one, MV_OK otherwise.
*
*******************************************************************************/
MV_U32 mvCpuIfPciIfRemap(MV_TARGET pciIfTarget, MV_ADDR_WIN *pAddrDecWin)
{
	if (MV_TARGET_IS_PEX(pciIfTarget))
	{
		return mvCpuIfPexRemap(pciIfTarget,pAddrDecWin);
	}
	return 0;
}

/*******************************************************************************
* mvCpuIfPexRemap - Set CPU remap register for address windows.
*
* DESCRIPTION:
*
* INPUT:
* 		cpu 		- CPU id
*       pexTarget   - Peripheral target enumerator. Must be a PEX target.
*       pAddrDecWin - CPU target window information data structure.
*                     Note that caller has to fill in the base field only. The 
*                     size field is ignored.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_ERROR if target is not a PEX one, MV_OK otherwise.
*
*******************************************************************************/
MV_U32 mvCpuIfPexRemapForCpu(MV_U32 cpu, MV_TARGET pexTarget, MV_ADDR_WIN *pAddrDecWin)
{
	MV_U32 winNum;

	/* Check parameters */

	if (cpu >= MV_MAX_CPU)
	{
		mvOsPrintf("mvCpuIfPexRemap: ERR. Invalid cpu %d\n", cpu);
		return 0xffffffff;
	}

	if (!MV_TARGET_IS_PEX(pexTarget))
	{
		mvOsPrintf("mvCpuIfPexRemap: target %d is Illigal\n",pexTarget);
		return 0xffffffff;
	}
	
	/* get the Window number associated with this target */
	winNum = mvAhbToMbusWinTargetGet(cpu, pexTarget);

	if (winNum >= MAX_AHB_TO_MBUS_WINS)
	{
		mvOsPrintf("mvCpuIfPexRemap: mvAhbToMbusWinTargetGet Failed\n");
		return 0xffffffff;

	}
	
	return mvAhbToMbusWinRemap(cpu, winNum, pAddrDecWin);
}

MV_U32 mvCpuIfPexRemap(MV_TARGET pexTarget, MV_ADDR_WIN *pAddrDecWin)
{
	return mvCpuIfPexRemapForCpu(whoAmI(), pexTarget, pAddrDecWin);
}

#endif


/*******************************************************************************
* cpuTargetWinOverlap - Detect CPU address decode windows overlapping
*
* DESCRIPTION:
*       An unpredicted behaviur is expected in case CPU address decode 
*       windows overlapps.
*       This function detects CPU address decode windows overlapping of a 
*       specified target. The function does not check the target itself for 
*       overlapping. The function also skipps disabled address decode windows.
*
* INPUT:
*       cpu			- cpu Id 
*       target      - Peripheral target enumerator.
*       pAddrDecWin - An address decode window struct.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE if the given address window overlaps current address
*       decode map, MV_FALSE otherwise.
*
*******************************************************************************/
static MV_BOOL cpuTargetWinOverlap(MV_U32 cpu, MV_TARGET target, MV_ADDR_WIN *pAddrWin)
{
    MV_U32 			targetNum;
    MV_CPU_DEC_WIN 	addrDecWin;
	MV_STATUS		status;

	
	for(targetNum = 0; targetNum < MAX_TARGETS; targetNum++)
    {

		/* don't check our target or illegal targets */
        if (targetNum == target)
        {
            continue;
        }       

		/* Get window parameters 	*/
		status = mvCpuIfTargetWinGetForCpu(cpu, targetNum, &addrDecWin);
        if(MV_NO_SUCH == status)
        {
            continue;
        }
		if(MV_OK != status)
		{
			DB(mvOsPrintf("cpuTargetWinOverlap: ERR. TargetWinGet failed\n"));
            return MV_TRUE;
		}

		/* Do not check disabled windows	*/
		if (MV_FALSE == addrDecWin.enable)
		{
			continue;
		}
        
        if(MV_TRUE == ctrlWinOverlapTest(pAddrWin, &addrDecWin.addrWin))
		{                    
	    DB(mvOsPrintf("cpuTargetWinOverlap: Required target %d overlap current %d\n", 
															target, targetNum));
			return MV_TRUE;           
		}
    }
	return MV_FALSE;
}



/*******************************************************************************
* mvCpuIfAddDecShow - Print the CPU address decode map.
*
* DESCRIPTION:
*		This function print the CPU address decode map.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/

MV_VOID mvCpuIfAddrDecShow(MV_U32 cpu)
{
	MV_CPU_DEC_WIN win;
	MV_U32 target;

	if (cpu >= MV_MAX_CPU)
	{
		mvOsPrintf("mvCpuIfAddrDecShow: ERR. Invalid cpu %d\n", cpu);
		return;
	}

	mvOsOutput( "\n" );
	mvOsOutput( "CPU %d, CPU Interface\n", cpu);
	mvOsOutput( "-------------\n" );

	for( target = 0; target < MAX_TARGETS; target++ ) 
	{

		memset( &win, 0, sizeof(MV_CPU_DEC_WIN) );

		mvOsOutput( "%s ",mvCtrlTargetNameGet(target));
		mvOsOutput( "...." );

		if( mvCpuIfTargetWinGetForCpu(cpu, target, &win ) == MV_OK )
		{
			if( win.enable )
			{
				mvOsOutput( "base %08x, ", win.addrWin.baseLow );
				mvSizePrint( win.addrWin.size );
				mvOsOutput( "\n" );

            }
			else
				mvOsOutput( "disable\n" );
		}
		else if( mvCpuIfTargetWinGetForCpu(cpu,  target, &win ) == MV_NO_SUCH )
		{
				mvOsOutput( "no such\n" );
		}

	}

}


/*******************************************************************************/

#if defined(MV78XX0)

#define MV_PROC_STR_SIZE 50

void mvCpuIfGetL2EccMode(MV_8 *buf)
{
    MV_U32 regVal = MV_REG_READ(CPU_CONFIG_REG(0));
    if (regVal & BIT22)
	mvOsSPrintf(buf, "L2 ECC Enabled");
    else
	mvOsSPrintf(buf, "L2 ECC Disabled");
}

void mvCpuIfGetL2Mode(MV_8 *buf)
{
    MV_U32 regVal = 0;
    __asm volatile ("mrc p15, 1, %0, c15, c1, 0" : "=r" (regVal)); /* Read Marvell extra features register */
    if (regVal & BIT22)
	    mvOsSPrintf(buf, "L2 Enabled in %s mode", 
			(MV_REG_READ(CPU_CTRL_STAT_REG(whoAmI())) & CCSR_L2WT) ? 
			"Write-Trough" : "Write-Back");
    else
	mvOsSPrintf(buf, "L2 Disabled");
}

void mvCpuIfGetL2PrefetchMode(MV_8 *buf)
{
    MV_U32 regVal = 0;
    __asm volatile ("mrc	p15, 1, %0, c15, c1, 0" : "=r" (regVal)); /* Read Marvell extra features register */
    if (regVal & BIT24)
	mvOsSPrintf(buf, "L2 Prefetch Disabled");
    else
	mvOsSPrintf(buf, "L2 Prefetch Enabled");
}

void mvCpuIfGetVfpMode(MV_8 *buf)
{

}

void mvCpuIfGetWriteAllocMode(MV_8 *buf)
{
    MV_U32 regVal = 0;
    __asm volatile ("mrc	p15, 1, %0, c15, c1, 0" : "=r" (regVal)); /* Read Marvell extra features register */
    if (regVal & BIT28)
	mvOsSPrintf(buf, "Write Allocate Enabled");
    else
	mvOsSPrintf(buf, "Write Allocate Disabled");
}

void mvCpuIfGetCpuStreamMode(MV_8 *buf)
{
    MV_U32 regVal = 0;
    __asm volatile ("mrc	p15, 1, %0, c15, c1, 0" : "=r" (regVal)); /* Read Marvell extra features register */
    if (regVal & BIT29)
	mvOsSPrintf(buf, "CPU Streaming Enabled");
    else
	mvOsSPrintf(buf, "CPU Streaming Disabled");
}

void mvCpuIfGetDramEccMode(MV_8 *buf)
{
    if (MV_REG_READ(SDRAM_CONFIG_REG) & SDRAM_ECC_EN)
    {
        mvOsSPrintf(buf, "DRAM ECC enabled");
    }
    else
    {
        mvOsSPrintf(buf, "DRAM ECC Disabled");
    }
}

void mvCpuIfGetCasLatency(MV_8 *buf)
{
    MV_U32 sdramCasLat = mvDramIfCalGet();

    mvOsSPrintf(buf, "CAS Latency %d.%d", sdramCasLat/10, sdramCasLat%10);
}


MV_U32 mvCpuIfPrintSystemConfig(MV_8 *buffer, MV_U32 index)
{
  MV_U32 count = 0;
  
  MV_8 L2_ECC_str[MV_PROC_STR_SIZE];
  MV_8 L2_En_str[MV_PROC_STR_SIZE];
  MV_8 L2_Prefetch_str[MV_PROC_STR_SIZE];
  /*MV_8 VFP_mode_str[MV_PROC_STR_SIZE];*/
  MV_8 Write_Alloc_str[MV_PROC_STR_SIZE];
  MV_8 Cpu_Stream_str[MV_PROC_STR_SIZE];
  MV_8 Dram_ECC_str[MV_PROC_STR_SIZE];
  MV_8 Cas_Latency_str[MV_PROC_STR_SIZE];
#ifdef MV78200
  MV_8 Soc_Unit_Map_str[MV_PROC_STR_SIZE*3];
#endif
  mvCpuIfGetL2Mode(L2_En_str);
  mvCpuIfGetL2EccMode(L2_ECC_str); 
  mvCpuIfGetL2PrefetchMode(L2_Prefetch_str);
  /*mvCpuIfGetVfpMode(VFP_mode_str);*/
  mvCpuIfGetWriteAllocMode(Write_Alloc_str);
  mvCpuIfGetCpuStreamMode(Cpu_Stream_str);
  mvCpuIfGetDramEccMode(Dram_ECC_str);
  mvCpuIfGetCasLatency(Cas_Latency_str);
#ifdef MV78200
  mvSocUnitMapPrint(Soc_Unit_Map_str);
#endif
  count += mvOsSPrintf(buffer + count + index, "%s\n", L2_En_str);
  count += mvOsSPrintf(buffer + count + index, "%s\n", L2_ECC_str);
  count += mvOsSPrintf(buffer + count + index, "%s\n", L2_Prefetch_str);
  /*count += mvOsSPrintf(buffer + count + index, "%s\n", VFP_mode_str);*/
  count += mvOsSPrintf(buffer + count + index, "%s\n", Write_Alloc_str);
  count += mvOsSPrintf(buffer + count + index, "%s\n", Cpu_Stream_str);
  count += mvOsSPrintf(buffer + count + index, "%s\n", Dram_ECC_str);
  count += mvOsSPrintf(buffer + count + index, "%s\n", Cas_Latency_str);
#ifdef MV78200
  count += mvOsSPrintf(buffer + count + index, "%s\n", Soc_Unit_Map_str);
#endif
  return count;
}

#endif

