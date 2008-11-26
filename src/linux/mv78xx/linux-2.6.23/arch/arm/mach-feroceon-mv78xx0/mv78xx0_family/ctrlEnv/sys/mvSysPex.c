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

#include "ctrlEnv/sys/mvSysPex.h"
#include "cpu/mvCpu.h"
#include "pex/mvPex.h"

#ifdef MV_DEBUG         
#define mvOsPrintf      printf
#define DB(x)	x
#else                
#define DB(x)    
#endif	             

/* locals */         
/* this structure describes the mapping between a Pex Window and a CPU target*/
typedef struct _pexWinToTarget
{
	MV_TARGET target;
	MV_BOOL	  enable;
}PEX_WIN_TO_TARGET;

/* this array is a priority array that define How Pex windows should be 
configured , We have only 6 Pex Windows that can be configured , but we
have morethat that CPU target windows ! the following array is a priority
array where the lowest index has the highest priotiy and the highest 
index has the lowest priority of being cnfigured */
MV_U32	pexDevBarPrioTable[] =
{
    DEVICE_CS0,
    DEVICE_CS1,
    DEVICE_CS2,
    DEV_BOOCS,
    TBL_TERM
};

/* PEX Wins registers offsets are inconsecutive. This struct describes WIN	*/
/* register offsets	and its function where its is located.					*/
/* Also, PEX address remap registers offsets are inconsecutive. This struct	*/
/* describes address remap register offsets									*/
typedef struct _pexWinRegInfo
{
    MV_U32 winCtrlRegOffs;
	MV_U32 winBaseRegOffs;
	MV_U32 winRemapLowRegOffs;
	MV_U32 winRemapHighRegOffs;

}PEX_WIN_REG_INFO;

static MV_STATUS pexWinOverlapDetect(MV_U32 pexIf, MV_U32 winNum,
									 MV_ADDR_WIN *pAddrWin);
static MV_STATUS pexWinRegInfoGet(MV_U32 pexIf, MV_U32 winNum,
								  PEX_WIN_REG_INFO *pWinRegInfo);
static MV_STATUS pexBarIsValid(MV_U32 baseLow, MV_U32 size);
static MV_BOOL pexIsWinWithinBar(MV_U32 pexIf,MV_ADDR_WIN *pAddrWin);
static MV_BOOL pexBarOverlapDetect(MV_U32 pexIf,MV_U32 barNum,
								   MV_ADDR_WIN *pAddrWin);
static MV_VOID mvPexDecWinShow(MV_U32 pexIf);

/*******************************************************************************
* mvPexInit - Initialize PEX interfaces
*
* DESCRIPTION:
*
* This function is responsible of intialization of the Pex Interface , It 
* configure the Pex Bars and Windows in the following manner:
*
*  Assumptions : 
*				Bar0 is always internal registers bar
*			    Bar1 is always the DRAM bar
*				Bar2 is always the Device bar
*
*  1) Sets the Internal registers bar base by obtaining the base from
*	  the CPU Interface
*  2) Sets the DRAM bar base and size by getting the base and size from
*     the CPU Interface when the size is the sum of all enabled DRAM 
*	  chip selects and the base is the base of CS0 .
*  3) Sets the Device bar base and size by getting these values from the 
*     CPU Interface when the base is the base of the lowest base of the
*     Device chip selects, and the 
*
*
* INPUT:
*
*       pexIf   -  PEX interface number.
*
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_OK if function success otherwise MV_ERROR or MV_BAD_PARAM
*
*******************************************************************************/
MV_STATUS mvPexInit(MV_U32 pexIf, MV_PEX_TYPE pexType)
{
	MV_U32 	       	bar;
	MV_U32		winNum,winIndex;
	MV_PEX_BAR	pexBar;
	MV_PEX_DEC_WIN  pexWin;
	MV_CPU_DEC_WIN 	addrDecWin;
	MV_TARGET 	target;
	MV_U32		pexStartWindow;
	MV_U32		pexCurrWin=0;
	MV_U32		status;
	/* default and expansion ROM are always configured */
	MV_U32 		maxBase=0,sizeOfMaxBase=0;
	MV_U32		dramSize = mvDramIfSizeGet();

	/* Parameter checking   */
	if(pexIf >= mvCtrlPexMaxIfGet())
	{
		mvOsPrintf("mvPexInit: ERR. Invalid PEX interface %d\n", pexIf);
		return MV_BAD_PARAM;
	}
	/* Start with bars */
	/* First disable all PEX bars*/
	for (bar = 0; bar < PEX_MAX_BARS; bar++)
	{
		if (PEX_INTER_REGS_BAR != bar)
		{
			if (MV_OK != mvPexBarEnable(pexIf, bar, MV_FALSE))
			{
				mvOsPrintf("mvPexInit: Closing all BARs failed at bar %d\n",bar);
				return MV_ERROR;
			}			
		}
	}

	/* and disable all PEX target windows  */
	for (winNum = 0; winNum < PEX_GRNERAL_WIN_NUM; winNum++)
	{
		if (MV_OK != mvPexTargetWinEnable(pexIf, winNum, MV_FALSE))
		{
			mvOsPrintf("mvPexInit: Disable all windows failed at win %d\n",winNum);
			return MV_ERROR;
		}
	}

	/* Now, go through all bars*/

/******************************************************************************/
/*                       Internal registers bar                               */
/******************************************************************************/

	/* first get the CS attribute from the CPU Interface */
	if (MV_OK !=mvCpuIfTargetWinGet(INTER_REGS,&addrDecWin))
	{
		mvOsPrintf("mvPexInit: ERR. mvCpuIfTargetWinGet failed target =%d\n",INTER_REGS);
		return MV_ERROR;
	}

	/* mvOsPrintf("mvPexInit: dramSize =%dGBYTE\n",dramSize/_1M); */
	if (dramSize > _2G)
	{
		pexBar.addrWin.baseHigh = 1;
	}
	else
	{
		pexBar.addrWin.baseHigh = addrDecWin.addrWin.baseHigh;
	}
	pexBar.addrWin.baseLow = addrDecWin.addrWin.baseLow;
	pexBar.addrWin.size = addrDecWin.addrWin.size;
	pexBar.enable = MV_TRUE;

	if (MV_OK != mvPexBarSet(pexIf, PEX_INTER_REGS_BAR, &pexBar))
	{
		mvOsPrintf("mvPexInit: ERR. mvPexBarSet %d failed\n", PEX_INTER_REGS_BAR);
		return MV_ERROR;
	}
	
/******************************************************************************/
/*                                DRAM bar                                    */
/******************************************************************************/

	bar = PEX_DRAM_BAR;
	pexBar.addrWin.baseHigh = 0xFFFFFFFF;
	pexBar.addrWin.baseLow = 0xFFFFFFFF;
	pexBar.addrWin.size = 0;

	for (target = SDRAM_CS0;target <= SDRAM_CS3; target++ )
	{
		status = mvCpuIfTargetWinGet(target, &addrDecWin);
		if((MV_NO_SUCH == status) && (target != SDRAM_CS0))
		{
			continue;
		}
		/* first get attributes from CPU If */
		if (MV_OK != status)
		{
			mvOsPrintf("mvPexInit: ERR. Getting DRAM address decode window "\
                       "%d from CPU interface failed\n",target - SDRAM_CS0);
			return MV_ERROR;
		}
		
		DB(mvOsPrintf("mvPexInit: target %d, size =%d MBYTE  \n",target,addrDecWin.addrWin.size/_1M));
		if (addrDecWin.enable == MV_TRUE)
		{				
			/* the base is the First DIMM on address 0 */
			/*if (mvDramIfGetFirstCS() == target )*/
			if ((addrDecWin.addrWin.baseHigh <= pexBar.addrWin.baseHigh) && 
				(addrDecWin.addrWin.baseLow < pexBar.addrWin.baseLow))
			{
				pexBar.addrWin.baseHigh = addrDecWin.addrWin.baseHigh;
				pexBar.addrWin.baseLow = addrDecWin.addrWin.baseLow;
			}

			/* Accumulate bar size */
			pexBar.addrWin.size += addrDecWin.addrWin.size;


			/* set a Pex window for this target ! 
			DRAM CS always will have a Pex Window , and is not a 
			part of the priority table */
			pexWin.addrWin.baseHigh = addrDecWin.addrWin.baseHigh;
			pexWin.addrWin.baseLow = addrDecWin.addrWin.baseLow;
			pexWin.addrWin.size = addrDecWin.addrWin.size;
			
			/* we disable the windows at first because we are not
			sure that it is witihin bar boundries */
			pexWin.enable =MV_FALSE;
			pexWin.target = target;
			pexWin.targetBar = bar;

			if (MV_OK != mvPexTargetWinSet(pexIf,pexCurrWin++,&pexWin))
			{
				mvOsPrintf("mvPexInit: ERR. mvPexTargetWinSet failed\n");
				return MV_ERROR;
			}
		}
		if (pexBar.addrWin.size >= _2G)
		{
			break;
		}
	}

	/* check if the size of the bar is illeggal */
	pexBar.addrWin.size = ctrlSizeRegRoundUp(pexBar.addrWin.size,											    PXBCR_BAR_SIZE_ALIGNMENT);
    
	/* check if the size and base are valid */
	if (MV_TRUE == pexBarOverlapDetect(pexIf, bar, &pexBar.addrWin))
	{
		mvOsPrintf("mvPexInit:Warning :Bar %d size is illigal\n",bar);
		mvOsPrintf("it will be disabled\n");
		mvOsPrintf("please check Pex and CPU windows configuration\n");
    }
	else
	{
		pexBar.enable = MV_TRUE;
        
		/* configure the bar */
		if (MV_OK != mvPexBarSet(pexIf, bar, &pexBar))
		{
			mvOsPrintf("mvPexInit: ERR. mvPexBarSet %d failed\n", bar);
			return MV_ERROR;
		}
	
		/* after the bar was configured then we enable the Pex windows*/
		for (winNum = 0;winNum < pexCurrWin ;winNum++)
		{
			if (MV_OK != mvPexTargetWinEnable(pexIf, winNum, MV_TRUE))
			{
				mvOsPrintf("mvPexInit: Can't enable window =%d\n",winNum);
				return MV_ERROR;
            }
	
		}
	}


/******************************************************************************/
/*                              DEVICE bar                                    */
/******************************************************************************/

	/* then device  bar*/
	bar = PEX_DEVICE_BAR;

	/* save the starting window */
	pexStartWindow = pexCurrWin;
	maxBase = 0;
	if (dramSize <= _2G)
	{
		pexBar.addrWin.size = 0;
		pexBar.addrWin.baseLow = 0xffffffff;
		pexBar.addrWin.baseHigh = 0;
	}
	else
	{
		/********************************************/
        /*      DEVICE bar  config windows over 2G  */
		/********************************************/
		target++;
		/* the base is the base of DRAM CS0 always */
		pexBar.addrWin.baseHigh = 0;
		pexBar.addrWin.baseLow 	= _2G;
		pexBar.addrWin.size 	= _2G;
		for ( ;target <= SDRAM_CS3; target++ )
		{
			status = mvCpuIfTargetWinGet(target, &addrDecWin);
			if (MV_NO_SUCH == status)
			{
				continue;
			}

			/* first get attributes from CPU If */
			if (MV_OK != status)
			{
				mvOsPrintf("mvPexInit: ERR. Getting DRAM address decode window "\
						   "%d from CPU interface failed\n",target - SDRAM_CS0);
				return MV_ERROR;
			}

			if (addrDecWin.enable == MV_TRUE)
			{

				/* set a Pex window for this target ! 
				DRAM CS always will have a Pex Window , and is not a 
				part of the priority table */
				pexWin.addrWin.baseHigh = addrDecWin.addrWin.baseHigh;
				pexWin.addrWin.baseLow 	= addrDecWin.addrWin.baseLow;
				pexWin.addrWin.size 	= addrDecWin.addrWin.size;
				DB(mvOsPrintf("mvPexInit: target %d, size =%d MBYTE  \n",target,addrDecWin.addrWin.size/_1M));

				/* we disable the windows at first because we are not
				sure that it is witihin bar boundries */
				pexWin.enable =MV_FALSE;
				pexWin.target = target;
				pexWin.targetBar = bar;
				if (MV_OK != mvPexTargetWinSet(pexIf,pexCurrWin++,&pexWin))
				{
					mvOsPrintf("mvPexInit: ERR. mvPexTargetWinSet failed\n");
					return MV_ERROR;
				}
			}
		}
	}

	for (target = DEV_TO_TARGET(DEV_CS0);
         target < DEV_TO_TARGET(MV_DEV_MAX_CS); 
         target++ )
	{
		status = mvCpuIfTargetWinGet(target, &addrDecWin);
        if (MV_NO_SUCH == status)
        {
            continue;
        }

		if (MV_OK != status)
		{
			mvOsPrintf("mvPexInit: ERR. mvCpuIfTargetWinGet failed target =%d\n",target);
			return MV_ERROR;
		}

		if (addrDecWin.enable == MV_TRUE)
		{				
			/* get the minimum base */
			if (addrDecWin.addrWin.baseLow < pexBar.addrWin.baseLow)
			{
				pexBar.addrWin.baseLow = addrDecWin.addrWin.baseLow;
			}

			/* get the maximum base */
			if (addrDecWin.addrWin.baseLow > maxBase)
			{
				maxBase = addrDecWin.addrWin.baseLow;
				sizeOfMaxBase = addrDecWin.addrWin.size;
			}

			/* search in the priority table for this target */
			for (winIndex = 0; pexDevBarPrioTable[winIndex] != TBL_TERM;
				 winIndex++)
			{
				if (pexDevBarPrioTable[winIndex] != target)
				{
					continue;
				}
				else if (pexDevBarPrioTable[winIndex] == target)
				{
					/*found it */

					/* if the index of this target in the prio table is valid 
					then we set the Pex window for this target, a valid index is 
					an index that is lower than the number of the windows that
					was not configured yet */
					if ( pexCurrWin  < PEX_GRNERAL_WIN_NUM)
					{
						/* set a Pex window for this target !  */
						pexWin.addrWin.baseHigh = addrDecWin.addrWin.baseHigh;
						pexWin.addrWin.baseLow = addrDecWin.addrWin.baseLow;
						pexWin.addrWin.size = addrDecWin.addrWin.size;
						
						/* we disable the windows at first because we are not
						sure that it is witihin bar boundries */
						pexWin.enable = MV_FALSE;
						pexWin.target = target;
						pexWin.targetBar = bar;
						if (MV_OK != mvPexTargetWinSet(pexIf,pexCurrWin++, &pexWin))
						{
							mvOsPrintf("mvPexInit: ERR. Window Set failed\n");
							return MV_ERROR;
						}
					}
				}
			}
		}
	}

	if (dramSize <= _2G)
	{
		pexBar.addrWin.size = maxBase - pexBar.addrWin.baseLow + sizeOfMaxBase;
	}
	pexBar.enable = MV_TRUE;

	pexBar.addrWin.size = ctrlSizeRegRoundUp(pexBar.addrWin.size, PXBCR_BAR_SIZE_ALIGNMENT);

	/* check if the size and base are valid */
	if (MV_TRUE == pexBarOverlapDetect(pexIf,bar,&pexBar.addrWin))
	{
		mvOsPrintf("mvPexInit:Warning :Bar %d size is illegal\n",bar);
		mvOsPrintf("it will be disabled\n");
		mvOsPrintf("please check Pex and CPU windows configuration\n");
	}
	else
	{
		if (MV_OK != mvPexBarSet(pexIf, bar, &pexBar))
		{
			mvOsPrintf("mvPexInit: ERR. mvPexBarSet %d failed\n", bar);
			return MV_ERROR;
		}

		/* now enable the windows */
		for (winNum = pexStartWindow; winNum < pexCurrWin ; winNum++)
		{
			if (MV_OK != mvPexTargetWinEnable(pexIf, winNum, MV_TRUE))
			{
				mvOsPrintf("mvPexInit:mvPexTargetWinEnable winNum =%d failed \n",
						   winNum);
				return MV_ERROR;
			}
		}
	}	
	/* WA - to set default value for default window */
	MV_REG_WRITE(PEX_WIN_DEFAULT_CTRL_REG(pexIf),0xf10);

	return mvPexHalInit(pexIf, pexType);
}

/*******************************************************************************
* mvPexTargetWinSet - Set PEX to peripheral target address window BAR
*
* DESCRIPTION:
*
* INPUT:
*       pexIf  - PEX interface number.
*       winNum - Address window number.
*
* OUTPUT:
*       N/A
*
* RETURN:
*       MV_OK if PEX BAR target window was set correctly, 
*		MV_BAD_PARAM on bad params 
*       MV_ERROR otherwise 
*       (e.g. address window overlapps with other active PEX target window).
*
*******************************************************************************/
MV_STATUS mvPexTargetWinSet(MV_U32 pexIf, MV_U32 winNum, MV_PEX_DEC_WIN *pPexAddrDecWin)
{

	MV_DEC_WIN_PARAMS   winParams;
	PEX_WIN_REG_INFO    winRegOffs;
	MV_U32              winCtrlReg = 0;
	MV_U32              winBaseReg = 0;

	/* Parameter checking   */
	if(pexIf >= mvCtrlPexMaxIfGet())
	{
		mvOsPrintf("mvPexTargetWinSet: ERR. Invalid PEX interface %d\n", pexIf);
		return MV_BAD_PARAM;
	}

	if (winNum >= PEX_MAX_TARGET_WIN)
	{
		mvOsPrintf("mvPexTargetWinSet: ERR. Invalid PEX winNum %d\n", winNum);
		return MV_BAD_PARAM;

	}

	if (MV_TRUE == pPexAddrDecWin->enable)
	{
		/* Check if the requested window overlaps with current windows  */
		if (MV_TRUE == pexWinOverlapDetect(pexIf,winNum, &pPexAddrDecWin->addrWin))
		{
			mvOsPrintf("mvPexTargetWinSet: ERR. Target %d overlap\n", winNum);
			return MV_BAD_PARAM;
		}
	
		/* Check if the requested window is within its BAR */
		if (MV_FALSE == pexIsWinWithinBar(pexIf,&pPexAddrDecWin->addrWin))
		{
			mvOsPrintf("mvPexTargetWinSet: Win %d should be in bar boundries\n",
					   winNum);
			return MV_BAD_PARAM;
		}
	}
	
	/* Translate the PEX address decode structure to register fields format */
	if (MV_OK != mvCtrlAddrDecToParams((MV_DEC_WIN*)pPexAddrDecWin, &winParams))
	{
		mvOsPrintf("Failed to translate the PEX address decode structure to "\
                   "register fields format\n");
		return MV_ERROR;		
	}
	
	/* set bar Mapping */
	winCtrlReg |= PXWCR_WIN_BAR_MAP_BAR(pPexAddrDecWin->targetBar);

	/* set attributes */
	winCtrlReg |= (winParams.attrib << PXWCR_ATTRIB_OFFS);

	/* If window is DRAM with HW cache coherency, make sure bit2 is set */
	winCtrlReg &= ~PXWCR_SLV_WR_SPLT_MASK;
	winCtrlReg |= PXWCR_SLV_WR_SPLT_128B;
	
	/* set target ID */
	winCtrlReg |= (winParams.targetId << PXWCR_TARGET_OFFS);

	/* set window size */
	winCtrlReg |= (winParams.size << PXWCR_SIZE_OFFS);

	/* enable\Disable */
	if (MV_TRUE == pPexAddrDecWin->enable)
	{
		winCtrlReg |= PXWCR_WIN_EN;
	}
    
    /* PCI Express Default Window Control Register reserve some fields */
    if (MV_PEX_WIN_DEFAULT == winNum)
    {
        winCtrlReg &= (PXWCR_TARGET_MASK | PXWCR_ATTRIB_MASK | PXWCR_SLV_WR_SPLT_MASK);
    }
    
    /* PCI Express Expansion ROM Window Control Register reserve some fields */
    if (MV_PEX_WIN_EXP_ROM == winNum)
    {
        winCtrlReg &= (PXWCR_TARGET_MASK | PXWCR_ATTRIB_MASK);
    }

    /* get the pex Window registers offsets */
	pexWinRegInfoGet(pexIf, winNum, &winRegOffs);

    /* Write to window control register */
    MV_REG_WRITE(winRegOffs.winCtrlRegOffs, winCtrlReg);
	
    /* PCI Express Default Window and Expansion ROM Window does not have    */
    /* base register                                                        */
    if (0 != winRegOffs.winBaseRegOffs)
	{
        /* Base parameter */
        winBaseReg = (winParams.baseAddr & PXWBR_BASE_MASK);

        /* Write to window base register */
        MV_REG_WRITE(winRegOffs.winBaseRegOffs, winBaseReg);
    }

    return MV_OK;

}

/*******************************************************************************
* mvPexTargetWinGet - Get PEX to peripheral target address window
*
* DESCRIPTION:
*		Get the PEX to peripheral target address window BAR.
*
* INPUT:
*       pexIf  - PEX interface number.
*       winNum - Address window number.
*
* OUTPUT:
*       pAddrBarWin - PEX target window information data structure.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPexTargetWinGet(MV_U32 pexIf, MV_U32 winNum, MV_PEX_DEC_WIN *pPexAddrDecWin)
{
	MV_DEC_WIN_PARAMS   winParams;
	PEX_WIN_REG_INFO    winRegOffs;
	MV_U32              winCtrlReg = 0;
	MV_U32              winBaseReg = 0;

	/* Parameter checking   */
	if(pexIf > mvCtrlPexMaxIfGet())
	{
		mvOsPrintf("mvPexTargetWinGet: ERR. Invalid PEX interface %d\n", pexIf);
		return MV_BAD_PARAM;
	}


	if (winNum >= PEX_MAX_TARGET_WIN)
	{
		mvOsPrintf("mvPexTargetWinGet: ERR. Invalid PEX winNum %d\n", winNum);
		return MV_BAD_PARAM;

	}

	/* get the pex Window registers offsets */
	pexWinRegInfoGet(pexIf,winNum, &winRegOffs);
	
    	/* Read Control register */
    	winCtrlReg = MV_REG_READ(winRegOffs.winCtrlRegOffs);

	/* Read size reg if there is valid base information */
	if (winRegOffs.winBaseRegOffs)
	{
		winBaseReg = MV_REG_READ(winRegOffs.winBaseRegOffs);
	}

    	/* Extract window parameters from registers */
    	winParams.attrib   = (winCtrlReg & PXWCR_ATTRIB_MASK) >> PXWCR_ATTRIB_OFFS;
	winParams.targetId = (winCtrlReg & PXWCR_TARGET_MASK) >> PXWCR_TARGET_OFFS; 
    	winParams.size     = (winCtrlReg & PXWCR_SIZE_MASK) >> PXWCR_SIZE_OFFS;
    	winParams.baseAddr = winBaseReg & PXWBR_BASE_MASK;
    
    	/* Translate the decode window parameters to address decode struct */
    	if (MV_OK != mvCtrlParamsToAddrDec(&winParams, (MV_DEC_WIN*)pPexAddrDecWin))
	{
		mvOsPrintf("Failed to translate register parameters to PEX address" \
                   " decode window structure\n");
		return MV_ERROR;		
	}

	/* get target bar */
	pPexAddrDecWin->targetBar = ((winCtrlReg & PXWCR_WIN_BAR_MAP_MASK) >> 
                              PXWCR_WIN_BAR_MAP_OFFS) + 1;

    	pPexAddrDecWin->slvWrSpltCnt = ((winCtrlReg & PXWCR_SLV_WR_SPLT_MASK) >> 
                              PXWCR_SLV_WR_SPLT_OFFS);

	pPexAddrDecWin->enable = (winCtrlReg & PXWCR_WIN_EN) >> PXWCR_WIN_EN_OFFS;

	return MV_OK;

}

/*******************************************************************************
* mvPexTargetWinEnable - Enable/disable a PEX BAR window
*
* DESCRIPTION:
*       This function enable/disable a PEX BAR window.
*       if parameter 'enable' == MV_TRUE the routine will enable the 
*       window, thus enabling PEX accesses for that BAR (before enabling the 
*       window it is tested for overlapping). Otherwise, the window will 
*       be disabled.
*
* INPUT:
*       pexIf  - PEX interface number.
*       winNum - Address window number.
*       enable - Enable/disable parameter.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPexTargetWinEnable(MV_U32 pexIf,MV_U32 winNum, MV_BOOL enable)
{
	PEX_WIN_REG_INFO winRegOffs;
	MV_PEX_DEC_WIN addrDecWin;

	/* Parameter checking   */
	if(pexIf >= mvCtrlPexMaxIfGet())
	{
		mvOsPrintf("mvPexTargetWinEnable: ERR. Invalid PEX If %d\n", pexIf);
		return MV_BAD_PARAM;
	}

	if (winNum >= PEX_MAX_TARGET_WIN)
	{
		mvOsPrintf("mvPexTargetWinEnable ERR. Invalid PEX winNum %d\n", winNum);
		return MV_BAD_PARAM;

	}

    /* Enable/Disable not relevant to default and expantion rom windows */
    if ((MV_PEX_WIN_DEFAULT == winNum) || (MV_PEX_WIN_EXP_ROM == winNum))
    {
        return MV_BAD_PARAM;
    }        

	/* get the pex Window registers offsets */
	pexWinRegInfoGet(pexIf, winNum, &winRegOffs);

	/* if the address windows is disabled , we only disable the appropriare
	pex window and ignore other settings */
	if (MV_FALSE == enable)
	{
		MV_REG_BIT_RESET(winRegOffs.winCtrlRegOffs, PXWCR_WIN_EN);
	}
	else
	{
		if (MV_OK != mvPexTargetWinGet(pexIf,winNum, &addrDecWin))
		{
			mvOsPrintf("mvPexTargetWinEnable: mvPexTargetWinGet Failed\n");
			return MV_ERROR;
		}

		/* Check if the requested window overlaps with current windows	*/
		if (MV_TRUE == pexWinOverlapDetect(pexIf,winNum, &addrDecWin.addrWin))
		{
			mvOsPrintf("mvPexTargetWinEnable: ERR. Target %d overlap\n", winNum);
			return MV_BAD_PARAM;
		}

		if (MV_FALSE == pexIsWinWithinBar(pexIf, &addrDecWin.addrWin))
		{
			mvOsPrintf("mvPexTargetWinEnable: Win %d should be in bar boundries\n",
					   winNum);
			return MV_BAD_PARAM;
		}
		
        	MV_REG_BIT_SET(winRegOffs.winCtrlRegOffs, PXWCR_WIN_EN);
	}

	return MV_OK;
}

/*******************************************************************************
* mvPexTargetWinRemap - Set PEX to target address window remap.
*
* DESCRIPTION:
*       The PEX interface supports remap of the BAR original address window.
*       For each BAR it is possible to define a remap address. For example
*       an address 0x12345678 that hits BAR 0x10 (SDRAM CS[0]) will be modified
*       according to remap register but will also be targeted to the 
*       SDRAM CS[0].
*
* INPUT:
*       pexIf  - PEX interface number.
*       winNum - Address window number.
*       pAddrWin - Address window to be checked.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPexTargetWinRemap(MV_U32 pexIf, MV_U32 winNum, 
                           MV_PEX_REMAP_WIN *pAddrWin)
{

	PEX_WIN_REG_INFO winRegOffs;
	
	/* Parameter checking   */
	if (pexIf >= mvCtrlPexMaxIfGet())
	{
		mvOsPrintf("mvPexTargetWinRemap: ERR. Invalid PEX interface num %d\n", 
																		pexIf);
		return MV_BAD_PARAM;
	}
	
    if ((MV_PEX_WIN_DEFAULT == winNum) || (winNum >= PEX_MAX_TARGET_WIN))
	{
		mvOsPrintf("mvPexTargetWinEnable ERR. Invalid PEX winNum %d\n", winNum);
		
        return MV_BAD_PARAM;
	}

	if (MV_IS_NOT_ALIGN(pAddrWin->addrWin.baseLow, PXWRR_REMAP_ALIGNMENT))
	{
		mvOsPrintf("mvPexTargetWinRemap: Error remap PEX interface %d win %d."\
				   "\nAddress 0x%08x is unaligned to size 0x%x.\n",
				   pexIf,
				   winNum,
                   pAddrWin->addrWin.baseLow,
				   pAddrWin->addrWin.size);
		
		return MV_ERROR;
	}

	pexWinRegInfoGet(pexIf, winNum, &winRegOffs);

	/* Set remap low register value */
	MV_REG_WRITE(winRegOffs.winRemapLowRegOffs, pAddrWin->addrWin.baseLow);
	
	/* Skip base high settings if the BAR has only base low (32-bit)		*/
	if (0 != winRegOffs.winRemapHighRegOffs)
	{
		MV_REG_WRITE(winRegOffs.winRemapHighRegOffs, pAddrWin->addrWin.baseHigh);
	}


	if (pAddrWin->enable == MV_TRUE)
	{
		MV_REG_BIT_SET(winRegOffs.winRemapLowRegOffs,PXWRR_REMAP_EN);
	}
	else
	{
		MV_REG_BIT_RESET(winRegOffs.winRemapLowRegOffs,PXWRR_REMAP_EN);
	}

	return MV_OK;
}

/*******************************************************************************
* mvPexTargetWinRemapEnable - 
*
* DESCRIPTION:
*
* INPUT:
*       pexIf  - PEX interface number.
*       winNum - Address window number.
*
* OUTPUT:
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPexTargetWinRemapEnable(MV_U32 pexIf, MV_U32 winNum, 
                           MV_BOOL enable)
{
	PEX_WIN_REG_INFO winRegOffs;
	
	/* Parameter checking   */
	if (pexIf >= mvCtrlPexMaxIfGet())
	{
		mvOsPrintf("mvPexTargetWinRemap: ERR. Invalid PEX interface num %d\n", 
																		pexIf);
		return MV_BAD_PARAM;
	}
	
    if ((MV_PEX_WIN_DEFAULT == winNum) || (winNum >= PEX_MAX_TARGET_WIN))
	{
		mvOsPrintf("mvPexTargetWinEnable ERR. Invalid PEX winNum %d\n", winNum);
		
        return MV_BAD_PARAM;
	}

	pexWinRegInfoGet(pexIf, winNum, &winRegOffs);

	if (enable == MV_TRUE)
	{
		MV_REG_BIT_SET(winRegOffs.winRemapLowRegOffs,PXWRR_REMAP_EN);
	}
	else
	{
		MV_REG_BIT_RESET(winRegOffs.winRemapLowRegOffs,PXWRR_REMAP_EN);
	}

	return MV_OK;

}

/*******************************************************************************
*  mvPexBarSet - Set PEX bar address and size 
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPexBarSet(MV_U32 pexIf,
						MV_U32 barNum,
						MV_PEX_BAR *pAddrWin)
{
	MV_U32 regBaseLow;
	MV_U32 regSize,sizeToReg;


	/* check parameters */
	if(pexIf >= mvCtrlPexMaxIfGet())
	{
		mvOsPrintf("mvPexBarSet: ERR. Invalid PEX interface %d\n", pexIf);
		return MV_BAD_PARAM;
	}

	if(barNum >= PEX_MAX_BARS)
	{
		mvOsPrintf("mvPexBarSet: ERR. Invalid bar number %d\n", barNum);
		return MV_BAD_PARAM;
	}
	

	if (pAddrWin->addrWin.size == 0)
	{
		mvOsPrintf("mvPexBarSet: Size zero is Illegal\n" );
		return MV_BAD_PARAM;
	}


	/* Check if the window complies with PEX spec							*/
	if (MV_TRUE != pexBarIsValid(pAddrWin->addrWin.baseLow,
								 pAddrWin->addrWin.size))
	{
        mvOsPrintf("mvPexBarSet: ERR. Target %d window invalid\n", barNum);
		return MV_BAD_PARAM;
	}

    /* 2) Check if the requested bar overlaps with current bars		*/
    if (MV_TRUE == pexBarOverlapDetect(pexIf,barNum, &pAddrWin->addrWin))
	{
        mvOsPrintf("mvPexBarSet: ERR. Target %d overlap\n", barNum);
		return MV_BAD_PARAM;
	}

	/* Get size register value according to window size						*/
	sizeToReg = ctrlSizeToReg(pAddrWin->addrWin.size, PXBCR_BAR_SIZE_ALIGNMENT);
	
	/* Read bar size */
	if (PEX_INTER_REGS_BAR != barNum) /* internal registers have no size */
	{
		regSize = MV_REG_READ(PEX_BAR_CTRL_REG(pexIf,barNum));

		/* Size parameter validity check.                                   */
		if (-1 == sizeToReg)
		{
			mvOsPrintf("mvPexBarSet: ERR. Target BAR %d size invalid.\n",barNum);
			return MV_BAD_PARAM;
		}
	
		regSize &= ~PXBCR_BAR_SIZE_MASK;
		regSize |= (sizeToReg << PXBCR_BAR_SIZE_OFFS) ;
	
		MV_REG_WRITE(PEX_BAR_CTRL_REG(pexIf,barNum),regSize);

	}

	/* set size */



	/* Read base address low */
	regBaseLow = MV_REG_READ(PEX_CFG_DIRECT_ACCESS(pexIf, PEX_MV_BAR_BASE(barNum)));

	/* clear current base */
	if (PEX_INTER_REGS_BAR == barNum)
	{
		regBaseLow &= ~PXBIR_BASE_MASK;
        	regBaseLow |= (pAddrWin->addrWin.baseLow & PXBIR_BASE_MASK);
	}
	else
	{
		regBaseLow &= ~PXBR_BASE_MASK;
		regBaseLow |= (pAddrWin->addrWin.baseLow & PXBR_BASE_MASK);
	}

	/* if we had a previous value that contain the bar type (MeM\IO), we want to
	restore it */
	regBaseLow |= PEX_BAR_DEFAULT_ATTRIB;

	/* write base low */
    	MV_REG_WRITE(PEX_CFG_DIRECT_ACCESS(pexIf,PEX_MV_BAR_BASE(barNum)),
				regBaseLow);

	if (pAddrWin->addrWin.baseHigh != 0)
	{
		/* Read base address high */
		MV_REG_WRITE(PEX_CFG_DIRECT_ACCESS(pexIf,PEX_MV_BAR_BASE_HIGH(barNum)),
								 pAddrWin->addrWin.baseHigh);

	}

	/* lastly enable the Bar */
	if (pAddrWin->enable == MV_TRUE)
	{
		if (PEX_INTER_REGS_BAR != barNum) /* internal registers 
												are enabled always */
		{
			MV_REG_BIT_SET(PEX_BAR_CTRL_REG(pexIf,barNum),PXBCR_BAR_EN);
		}
	}
	else if (MV_FALSE == pAddrWin->enable)
	{
		if (PEX_INTER_REGS_BAR != barNum) /* internal registers 
												are enabled always */
		{
			MV_REG_BIT_RESET(PEX_BAR_CTRL_REG(pexIf,barNum),PXBCR_BAR_EN);
		}

	}



	return MV_OK;
}


/*******************************************************************************
*  mvPexBarGet - Get PEX bar address and size
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPexBarGet(MV_U32 pexIf,
							MV_U32 barNum,
								MV_PEX_BAR *pAddrWin)
{
	/* check parameters */
	if(pexIf >= mvCtrlPexMaxIfGet())
	{
		mvOsPrintf("mvPexBarGet: ERR. Invalid PEX interface %d\n", pexIf);
		return MV_BAD_PARAM;
	}

	if(barNum >= PEX_MAX_BARS)
	{
		mvOsPrintf("mvPexBarGet: ERR. Invalid bar number %d\n", barNum);
		return MV_BAD_PARAM;
	}

	/* read base low */
	pAddrWin->addrWin.baseLow = 
		MV_REG_READ(PEX_CFG_DIRECT_ACCESS(pexIf,PEX_MV_BAR_BASE(barNum)));


	if (PEX_INTER_REGS_BAR == barNum)
	{
		pAddrWin->addrWin.baseLow &= PXBIR_BASE_MASK;
	}
	else
	{
		pAddrWin->addrWin.baseLow &= PXBR_BASE_MASK;
	}


	/* read base high */
	pAddrWin->addrWin.baseHigh = 
		MV_REG_READ(PEX_CFG_DIRECT_ACCESS(pexIf,PEX_MV_BAR_BASE_HIGH(barNum)));


	/* Read bar size */
	if (PEX_INTER_REGS_BAR != barNum) /* internal registers have no size */
	{
		pAddrWin->addrWin.size = MV_REG_READ(PEX_BAR_CTRL_REG(pexIf,barNum));

		/* check if enable or not */
		if (pAddrWin->addrWin.size & PXBCR_BAR_EN)
		{
			pAddrWin->enable = MV_TRUE;
		}
		else
		{
			pAddrWin->enable = MV_FALSE;
		}
			
		/* now get the size */
		pAddrWin->addrWin.size &= PXBCR_BAR_SIZE_MASK;
		pAddrWin->addrWin.size >>= PXBCR_BAR_SIZE_OFFS;

		pAddrWin->addrWin.size = ctrlRegToSize(pAddrWin->addrWin.size,
											   PXBCR_BAR_SIZE_ALIGNMENT);

	}
	else /* PEX_INTER_REGS_BAR */
	{
		pAddrWin->addrWin.size = INTER_REGS_SIZE;
		pAddrWin->enable = MV_TRUE;
	}


	return MV_OK;
}

/*******************************************************************************
*  mvPexBarEnable - 
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPexBarEnable(MV_U32 pexIf, MV_U32 barNum, MV_BOOL enable)
{

	MV_PEX_BAR pexBar;

	/* check parameters */
	if(pexIf >= mvCtrlPexMaxIfGet())
	{
		mvOsPrintf("mvPexBarEnable: ERR. Invalid PEX interface %d\n", pexIf);
		return MV_BAD_PARAM;
	}

	if(barNum >= PEX_MAX_BARS)
	{
		mvOsPrintf("mvPexBarEnable: ERR. Invalid bar number %d\n", barNum);
		return MV_BAD_PARAM;
	}

    if (PEX_INTER_REGS_BAR == barNum)
	{
		if (MV_TRUE == enable)
		{
			return MV_OK;
		}
		else
		{
			return MV_ERROR;
		}
	}


	if (MV_FALSE == enable)
	{
			/* disable bar and quit */
			MV_REG_BIT_RESET(PEX_BAR_CTRL_REG(pexIf,barNum),PXBCR_BAR_EN);
			return MV_OK;
	}

	/* else */

	if (mvPexBarGet(pexIf,barNum,&pexBar) != MV_OK)
	{
		mvOsPrintf("mvPexBarEnable: mvPexBarGet Failed\n");
		return MV_ERROR;

	}

	if (MV_TRUE == pexBar.enable)
	{
		/* it is already enabled !!! */
		return MV_OK;
	}

	/* else enable the bar*/

	pexBar.enable = MV_TRUE;

	if (mvPexBarSet(pexIf,barNum,&pexBar) != MV_OK)
	{
		mvOsPrintf("mvPexBarEnable: mvPexBarSet Failed\n");
		return MV_ERROR;

	}

	return MV_OK;
}


/*******************************************************************************
* pexWinOverlapDetect - Detect address windows overlapping
*
* DESCRIPTION:
*       This function detects address window overlapping of a given address 
*       window in PEX BARs.
*
* INPUT:
*       pAddrWin - Address window to be checked.
*       bar      - BAR to be accessed by slave.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE if the given address window overlap current address
*       decode map, MV_FALSE otherwise.
*
*******************************************************************************/
static MV_BOOL pexWinOverlapDetect(MV_U32 pexIf,
									 MV_U32 winNum,
									 MV_ADDR_WIN *pAddrWin)
{
    	MV_U32 		   win;
	MV_PEX_DEC_WIN addrDecWin;
	
	for(win = 0; win < PEX_GRNERAL_WIN_NUM ; win++)
    	{
        	/* don't check our target or illegal targets */
        	if (winNum == win)
        	{
            		continue;
        	}
        
		/* Get window parameters 	*/
		if (MV_OK != mvPexTargetWinGet(pexIf, win, &addrDecWin))
		{
			mvOsPrintf("pexWinOverlapDetect: ERR. TargetWinGet failed win=%x\n",
					   win);
            		return MV_ERROR;
		}

		/* Do not check disabled windows	*/
		if (MV_FALSE == addrDecWin.enable)
		{
			continue;
		}

        	if(MV_TRUE == ctrlWinOverlapTest(pAddrWin, &addrDecWin.addrWin))
		{                    
			mvOsPrintf("pexWinOverlapDetect: winNum %d overlap current %d\n", 												winNum, win);
			return MV_TRUE;           
		}
    	}
    
	return MV_FALSE;
}

/*******************************************************************************
* pexIsWinWithinBar - Detect if address is within PEX bar boundries
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE if the given address window overlap current address
*       decode map, MV_FALSE otherwise.
*
*******************************************************************************/
static MV_BOOL pexIsWinWithinBar(MV_U32 pexIf,
								   MV_ADDR_WIN *pAddrWin)
{
    MV_U32 		   bar;
	MV_PEX_BAR addrDecWin;

	for(bar = 0; bar < PEX_MAX_BARS; bar++)
    {        
		/* Get window parameters 	*/
		if (MV_OK != mvPexBarGet(pexIf, bar, &addrDecWin))
		{
			mvOsPrintf("pexIsWinWithinBar: ERR. mvPexBarGet failed\n");
            return MV_ERROR;
		}
		
		/* Do not check disabled bars	*/
		if (MV_FALSE == addrDecWin.enable)
		{
			continue;
		}
        
        if(MV_TRUE == ctrlWinWithinWinTest(pAddrWin, &addrDecWin.addrWin))
		{                    
			return MV_TRUE;
		}
    }
    
	return MV_FALSE;
}

/*******************************************************************************
* pexBarOverlapDetect - Detect address windows overlapping
*
* DESCRIPTION:
*       This function detects address window overlapping of a given address 
*       window in PEX BARs.
*
* INPUT:
*       pAddrWin - Address window to be checked.
*       bar      - BAR to be accessed by slave.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE if the given address window overlap current address
*       decode map, MV_FALSE otherwise.
*
*******************************************************************************/
static MV_BOOL pexBarOverlapDetect(MV_U32 pexIf,
									 MV_U32 barNum,
									 MV_ADDR_WIN *pAddrWin)
{
    MV_U32 		   bar;
	MV_PEX_BAR barDecWin;

	
	for(bar = 0; bar < PEX_MAX_BARS; bar++)
    	{
        	/* don't check our target or illegal targets */
        	if (barNum == bar)
        	{
            		continue;
        	}
        
		/* Get window parameters 	*/
		if (MV_OK != mvPexBarGet(pexIf, bar, &barDecWin))
		{
			mvOsPrintf("pexBarOverlapDetect: ERR. TargetWinGet failed\n");
            		return MV_ERROR;
		}

		/* don'nt check disabled bars */
        	if (barDecWin.enable == MV_FALSE)
		{
			continue;
		}

        	if(MV_TRUE == ctrlWinOverlapTest(pAddrWin, &barDecWin.addrWin))
		{                    
			mvOsPrintf("pexBarOverlapDetect: winNum %d overlap current %d\n", 														barNum, bar);
			return MV_TRUE;           
		}
    }
    
	return MV_FALSE;
}

/*******************************************************************************
* pexBarIsValid - Check if the given address window is valid
*
* DESCRIPTION:
*		PEX spec restrict BAR base to be aligned to BAR size.
*		This function checks if the given address window is valid.
*
* INPUT:
*       baseLow - 32bit low base address.
*       size    - Window size.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE if the address window is valid, MV_FALSE otherwise.
*
*******************************************************************************/
static MV_STATUS pexBarIsValid(MV_U32 baseLow, MV_U32 size)
{

	/* PCI spec restrict BAR base to be aligned to BAR size					*/
	if(MV_IS_NOT_ALIGN(baseLow, size))
	{
		return MV_ERROR;
	}
	else
	{
		return MV_TRUE;
	}
	
	return MV_TRUE;
}

/*******************************************************************************
* pexBarRegInfoGet - Get BAR register information
*
* DESCRIPTION:
* 		PEX BARs registers offsets are inconsecutive. 
*		This function gets a PEX BAR register information like register offsets
*		and function location of the BAR.
*
* INPUT:
*       pexIf  - PEX interface number.
*		winNum - The PEX address decode window in question.	
*
* OUTPUT:
*       pBarRegInfo - BAR register info struct.
*
* RETURN:
*		MV_BAD_PARAM when bad parameters ,MV_ERROR on error ,othewise MV_OK
*
*******************************************************************************/
static MV_STATUS pexWinRegInfoGet(MV_U32 pexIf, 
								  MV_U32 winNum,
								  PEX_WIN_REG_INFO *pWinRegInfo)
{

    if (MV_PEX_WIN_DEFAULT == winNum)
	{
		pWinRegInfo->winCtrlRegOffs      = PEX_WIN_DEFAULT_CTRL_REG(pexIf);
		pWinRegInfo->winBaseRegOffs      = 0;
		pWinRegInfo->winRemapLowRegOffs  = 0;
		pWinRegInfo->winRemapHighRegOffs = 0;
	}
	else if (MV_PEX_WIN_EXP_ROM == winNum)
	{
		pWinRegInfo->winCtrlRegOffs      = PEX_WIN_EXP_ROM_CTRL_REG(pexIf);
		pWinRegInfo->winBaseRegOffs      = 0;
		pWinRegInfo->winRemapLowRegOffs  = PEX_WIN_EXP_ROM_REMAP_REG(pexIf);
		pWinRegInfo->winRemapHighRegOffs = 0;

	}
    else 
    {
        pWinRegInfo->winCtrlRegOffs      = PEX_WIN_CTRL_REG(pexIf,winNum);
        pWinRegInfo->winBaseRegOffs      = PEX_WIN_BASE_REG(pexIf,winNum);
        pWinRegInfo->winRemapLowRegOffs  = PEX_WIN_REMAP_REG(pexIf,winNum);
        pWinRegInfo->winRemapHighRegOffs = PEX_WIN_REMAP_HIGH_REG(pexIf,winNum);
    }


	return MV_OK;
}

/*******************************************************************************
* pexBarNameGet - Get the string name of PEX BAR.
*
* DESCRIPTION:
*		This function get the string name of PEX BAR.
*
* INPUT:
*       bar - PEX bar number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       pointer to the string name of PEX BAR.
*
*******************************************************************************/
const MV_8* pexBarNameGet( MV_U32 bar )
{
	switch( bar ) 
	{
		case PEX_INTER_REGS_BAR: 
			return "Internal Regs Bar0....";
		case PEX_DRAM_BAR: 
			return "DRAM Bar1.............";
		case PEX_DEVICE_BAR: 
			return "Devices Bar2..........";
		default:
			 return "Bar unknown";
	}
}
/*******************************************************************************
* mvPexAddrDecShow - Print the PEX address decode map (BARs and windows).
*
* DESCRIPTION:
*		This function print the PEX address decode map (BARs and windows).
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
MV_VOID mvPexAddrDecShow(MV_VOID)
{
	MV_PEX_BAR pexBar;
	MV_U32 pexIf;
	MV_U32 pexIfMax = mvCtrlPexMaxIfGet();
	MV_U32 bar;

	for( pexIf = 0; pexIf < pexIfMax; pexIf++ )
	{
		mvOsOutput( "\n" );
		mvOsOutput( "PEX%d:\n", pexIf );
		mvOsOutput( "-----" );

		mvOsOutput( "\nPex Bars \n\n");

		for( bar = 0; bar < PEX_MAX_BARS; bar++ ) 
		{
			memset( &pexBar, 0, sizeof(MV_PEX_BAR) );

			mvOsOutput( "%s ", pexBarNameGet(bar) );

			if( mvPexBarGet( pexIf, bar, &pexBar ) == MV_OK )
			{
				if( pexBar.enable )
				{
                    mvOsOutput( "base %08x:%08x, ", pexBar.addrWin.baseHigh, pexBar.addrWin.baseLow );
                    mvSizePrint( pexBar.addrWin.size );
                    mvOsOutput( "\n" );
				}
				else
					mvOsOutput( "disable\n" );
			}
		}
        
        mvPexDecWinShow(pexIf);
	}
}


/*******************************************************************************
* mvPexDecWinShow - Print the PEX address decode windows.
*
* DESCRIPTION:
*		This function print the PEX address decode windows.
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
static MV_VOID mvPexDecWinShow(MV_U32 pexIf)
{
	MV_PEX_DEC_WIN win;
	int winNum;

	mvOsOutput( "\nPex Decode Windows\n\n");

	for( winNum = 0; winNum < PEX_GRNERAL_WIN_NUM; winNum++ )
	{
		memset( &win, 0, sizeof(MV_PEX_DEC_WIN) );

		mvOsOutput( "win%d - ", winNum );

        if ( mvPexTargetWinGet(pexIf,winNum,&win) == MV_OK)
		{
			if( win.enable )
			{
                mvOsOutput( "%s base %08x:%08x, ",
                mvCtrlTargetNameGet(win.target), win.addrWin.baseHigh, win.addrWin.baseLow );
                mvSizePrint( win.addrWin.size );
				mvOsOutput( "\n" );
			}
			else
				mvOsOutput( "disable\n" );
		}
	}

    memset( &win, 0,sizeof(MV_PEX_DEC_WIN) );

    mvOsOutput( "default win - " );

    if ( mvPexTargetWinGet(pexIf, MV_PEX_WIN_DEFAULT, &win) == MV_OK)
    {
        mvOsOutput( "%s ",
        mvCtrlTargetNameGet(win.target) );
        mvOsOutput( "\n" );
    }
    memset( &win, 0,sizeof(MV_PEX_DEC_WIN) );

    mvOsOutput( "Expansion ROM - " );

    if ( mvPexTargetWinGet(pexIf, MV_PEX_WIN_EXP_ROM, &win) == MV_OK)
    {
        mvOsOutput( "%s ",
        mvCtrlTargetNameGet(win.target) );
        mvOsOutput( "\n" );
    }
}

