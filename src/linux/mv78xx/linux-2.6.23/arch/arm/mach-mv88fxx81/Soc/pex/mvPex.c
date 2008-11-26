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

#include "mvPex.h"
#include "mvPexConfig.h"
#include "mvDeviceRegs.h"
#include "mvCpuIf.h"

/* defines  */       
#ifdef MV_DEBUG         
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
have maximum of 9 CPU target windows ! the following array is a priority
array where the lowest index has the highest priotiy and the highest 
index has the lowest priority of being cnfigured */

MV_U32	pexDevBarPrioTable[] =
{
#if defined(MV_88F5181)
    DEVICE_CS0,
    DEVICE_CS1,
    DEVICE_CS2,
#elif defined (MV_88F1181)                           
    FLASH_CS,
#else                                                
#   error "CHIP not selected"                        
#endif                                               
	DEV_BOOCS,
	TBL_TERM
};


/* PEX Wins registers offsets are inconsecutive. This struct describes WIN	*/
/* register offsets	and its function where its is located.					*/
/* Also, PEX address remap registers offsets are inconsecutive. This struct	*/
/* describes address remap register offsets									*/
typedef struct _pexWinRegInfo
{
    MV_U32 baseLowRegOffs;
	MV_U32 baseHighRegOffs;
	MV_U32 sizeRegOffs;
	MV_U32 remapLowRegOffs;
	MV_U32 remapHighRegOffs;

}PEX_WIN_REG_INFO;

static MV_STATUS pexWinOverlapDetect(MV_U32 pexIf, MV_U32 winNum,
									 MV_ADDR_WIN *pAddrWin);
static MV_STATUS pexWinRegInfoGet(MV_U32 pexIf, MV_U32 winNum,
								  PEX_WIN_REG_INFO *pWinRegInfo);

static MV_STATUS pexBarIsValid(MV_U32 baseLow, MV_U32 size);

static MV_BOOL pexIsWinWithinBar(MV_U32 pexIf,MV_ADDR_WIN *pAddrWin);
static MV_BOOL pexBarOverlapDetect(MV_U32 pexIf,MV_U32 barNum,
								   MV_ADDR_WIN *pAddrWin);

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
MV_STATUS mvPexInit(MV_U32 pexIf)
{
	MV_U32 	       bar;
	MV_U32		   winNum,winIndex;
	MV_PEX_BAR	  	pexBar;
	MV_PEX_DEC_WIN 	pexWin;
	MV_CPU_DEC_WIN 	addrDecWin;
	MV_TARGET 		target;
	MV_U32			pexStartWindow;
	MV_U32			pexCurrWin=0;
	MV_U32			status;
	/* default and exapntion rom 
	are always configured */
	MV_U32 		    maxBase=0,sizeOfMaxBase=0;

	/* Parameter checking   */
	if(pexIf >= mvCtrlPexMaxIfGet())
	{
		mvOsPrintf("mvPexInit: ERR. Invalid PEX interface %d\n", pexIf);
		return MV_BAD_PARAM;
	}
	
	/* First implement Guideline (GL# PCI Express-2) Wrong Default Value    */
    /* to Transmitter Output Current (TXAMP) Relevant for: 88F5181-A1/B0/B1 */
    /* and 88F5281-B0                                                       */
    {
        MV_U32 regVal;

        /* Read current value of TXAMP */
        MV_REG_WRITE(0x41b00, 0x80820000);   /* Write the read command   */

        regVal = MV_REG_READ(0x41b00);      /* Extract the data         */

        /* Prepare new data for write */
        regVal &= ~0x7;                     /* Clear bits [2:0]         */
        regVal |=  0x4;                     /* Set the new value        */
        regVal &= ~0x80000000;              /* Set "write" command      */
        MV_REG_WRITE(0x41b00, regVal);      /* Write the write command  */

    }
    
    /* Start with bars */
	/* First disable all PEX bars*/
	for (bar = 0; bar < PEX_MAX_BARS; bar++)
    {
		if (PEX_INTER_REGS_BAR != bar)
		{
			if (MV_OK != mvPexBarEnable(pexIf, bar, MV_FALSE))
			{
				mvOsPrintf("mvPexInit:mvPexBarEnable bar =%d failed \n",bar);
				return MV_ERROR;
			}
			
		}

	}

	/* and disable all PEX target windows  */
	for (winNum = 0; winNum < PEX_MAX_TARGET_WIN - 2; winNum++)
    {
		if (MV_OK != mvPexTargetWinEnable(pexIf, winNum, MV_FALSE))
		{
			mvOsPrintf("mvPexInit:mvPexTargetWinEnable winNum =%d failed \n",
					   winNum);
			return MV_ERROR;

		}
	}

	/* Now, go through all bars*/



/******************************************************************************/
/*                       Internal registers bar                               */
/******************************************************************************/
	bar = PEX_INTER_REGS_BAR;

	/* we only open the bar , no need to open windows for this bar */

	/* first get the CS attribute from the CPU Interface */
	if (MV_OK !=mvCpuIfTargetWinGet(INTER_REGS,&addrDecWin))
	{
		mvOsPrintf("mvPexInit: ERR. mvCpuIfTargetWinGet failed target =%d\n",INTER_REGS);
		return MV_ERROR;
	}

	pexBar.addrWin.baseHigh = addrDecWin.addrWin.baseHigh;
	pexBar.addrWin.baseLow = addrDecWin.addrWin.baseLow;
	pexBar.addrWin.size = addrDecWin.addrWin.size;
	pexBar.enable = MV_TRUE;

	if (MV_OK != mvPexBarSet(pexIf, bar, &pexBar))
	{
		mvOsPrintf("mvPexInit: ERR. mvPexBarSet %d failed\n", bar);
		return MV_ERROR;
	}
	
/******************************************************************************/
/*                                DRAM bar                                    */
/******************************************************************************/

	bar = PEX_DRAM_BAR;

	pexBar.addrWin.size = 0;

	for (target = SDRAM_CS0;target <= SDRAM_CS3; target++ )
	{

		status = mvCpuIfTargetWinGet(target,&addrDecWin);

        if((MV_NO_SUCH == status)&&(target != SDRAM_CS0))
        {
            continue;
        }

		/* first get attributes from CPU If */
		if (MV_OK != status)
		{
			mvOsPrintf("mvPexInit: ERR. mvCpuIfTargetWinGet failed target =%d\n",target);
			return MV_ERROR;
		}
		if (addrDecWin.enable == MV_TRUE)
		{				
			/* the base is the base of DRAM CS0 always */
			if (SDRAM_CS0 == target )
			{
				pexBar.addrWin.baseHigh = addrDecWin.addrWin.baseHigh;
				pexBar.addrWin.baseLow = addrDecWin.addrWin.baseLow;

			}

			/* increment the bar size to be the sum of the size of all
			DRAM chips selecs */
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

	}

	/* check if the size of the bar is illeggal */
	if (-1 == ctrlSizeToReg(pexBar.addrWin.size, PXBCR_BAR_SIZE_ALIGNMENT))
	{
		/* try to get a good size */
		pexBar.addrWin.size = ctrlSizeRegRoundUp(pexBar.addrWin.size,
												 PXBCR_BAR_SIZE_ALIGNMENT);
	}
    
	/* check if the sizeand base are valid */
	if (MV_TRUE == pexBarOverlapDetect(pexIf,bar,&pexBar.addrWin))
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
	pexBar.addrWin.size = 0;
	pexBar.addrWin.baseLow = 0xffffffff;
	pexBar.addrWin.baseHigh = 0;
	maxBase = 0;

	for (target = DEV_TO_TARGET(START_DEV_CS);target < DEV_TO_TARGET(MV_DEV_MAX_CS); target++ )
	{

		status = mvCpuIfTargetWinGet(target,&addrDecWin);

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

					/* we subtract 2 always because the default and expantion
					rom windows are always configured */
					if ( pexCurrWin  < PEX_MAX_TARGET_WIN - 2)
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

						if (MV_OK != mvPexTargetWinSet(pexIf,pexCurrWin++,
													   &pexWin))
						{
							mvOsPrintf("mvPexInit: ERR. Window Set failed\n");
							return MV_ERROR;
						}
					}
				}
			}
		}
	}

	pexBar.addrWin.size = maxBase - pexBar.addrWin.baseLow + sizeOfMaxBase;
	pexBar.enable = MV_TRUE;

	/* check if the size of the bar is illegal */
	if (-1 == ctrlSizeToReg(pexBar.addrWin.size, PXBCR_BAR_SIZE_ALIGNMENT))
	{
		/* try to get a good size */
		pexBar.addrWin.size = ctrlSizeRegRoundUp(pexBar.addrWin.size,
												 PXBCR_BAR_SIZE_ALIGNMENT);
	}

	/* check if the size and base are valid */
	if (MV_TRUE == pexBarOverlapDetect(pexIf,bar,&pexBar.addrWin))
	{
		mvOsPrintf("mvPexInit:Warning :Bar %d size is illigal\n",bar);
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

	
	/* set bus and dev numbers */
	if (pexIf == 0)
	{
		mvPexLocalBusNumSet(pexIf, PEX0_HOST_BUS_NUM);
		mvPexLocalDevNumSet(pexIf, PEX0_HOST_DEV_NUM);
	}
	#if defined (MV_88F1181)                           
	else /* PEX1 */
	{
		mvPexLocalBusNumSet(pexIf, PEX1_HOST_BUS_NUM);
		mvPexLocalDevNumSet(pexIf, PEX1_HOST_DEV_NUM);

	}
	#elif defined (MV_88F5181)                           
	#else                                                
	#   error "CHIP not selected"                        
	#endif                                               
	

	/* Local device master Enable */
	mvPexMasterEnable(pexIf, MV_TRUE);

	/* Local device slave Enable */
	mvPexSlaveEnable(pexIf, mvPexLocalBusNumGet(pexIf),
					 mvPexLocalDevNumGet(pexIf), MV_TRUE);

	/* now wait 60 ns to be sure the link is valid (spec compliant) */ 
	mvOsDelay(1); /* 1 milisecond */

	return MV_OK;

}

/*******************************************************************************
* mvPexModeGet - Get Pex Mode
*
* DESCRIPTION:
*
* INPUT:
*       pexIf   - PEX interface number.
*
* OUTPUT:
*       pexMode - Pex mode structure
*
* RETURN:
*       MV_OK on success , MV_ERROR otherwise
*
*******************************************************************************/
MV_U32 mvPexModeGet(MV_U32 pexIf,MV_PEX_MODE *pexMode)
{
	MV_U32 pexData;

	/* Parameter checking   */
	if (PEX_DEFAULT_IF != pexIf)
	{
		if (pexIf >= mvCtrlPexMaxIfGet())
		{
			mvOsPrintf("mvPexModeGet: ERR. Invalid PEX interface %d\n",pexIf);
			return MV_ERROR;
		}
	}

	pexData = MV_REG_READ(PEX_CTRL_REG(pexIf));

	switch (pexData & PXCR_DEV_TYPE_CTRL_MASK)
	{
	case PXCR_DEV_TYPE_CTRL_CMPLX:

		pexMode->pexType = MV_PEX_ROOT_COMPLEX;
		break;
	case PXCR_DEV_TYPE_CTRL_POINT:

		pexMode->pexType = MV_PEX_END_POINT;
		break;

	}

	return MV_OK;
}


/* PEX configuration space read write */

/*******************************************************************************
* mvPexConfigRead - Read from configuration space
*
* DESCRIPTION:
*       This function performs a 32 bit read from PEX configuration space.
*       It supports both type 0 and type 1 of Configuration Transactions 
*       (local and over bridge). In order to read from local bus segment, use 
*       bus number retrieved from mvPexLocalBusNumGet(). Other bus numbers 
*       will result configuration transaction of type 1 (over bridge).
*
* INPUT:
*       pexIf   - PEX interface number.
*       bus     - PEX segment bus number.
*       dev     - PEX device number.
*       func    - Function number.
*       regOffs - Register offset.       
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit register data, 0xffffffff on error
*
*******************************************************************************/
MV_U32 mvPexConfigRead (MV_U32 pexIf, MV_U32 bus, MV_U32 dev, MV_U32 func, 
                        MV_U32 regOff)
{
	MV_U32 pexData = 0;
	MV_U32	localDev,localBus;

	/* Parameter checking   */
	if (PEX_DEFAULT_IF != pexIf)
	{
		if (pexIf >= mvCtrlPexMaxIfGet())
		{
			mvOsPrintf("mvPexConfigRead: ERR. Invalid PEX interface %d\n",pexIf);
			return 0xFFFFFFFF;
		}
	}

	if (dev >= MAX_PEX_DEVICES)
	{
		DB(mvOsPrintf("mvPexConfigRead: ERR. device number illigal %d\n", dev));
		return 0xFFFFFFFF;
	}
	
	if (func >= MAX_PEX_FUNCS)
	{
		DB(mvOsPrintf("mvPexConfigRead: ERR. function num illigal %d\n", func));
		return 0xFFFFFFFF;
	}
	
	if (bus >= MAX_PEX_BUSSES)
	{
		DB(mvOsPrintf("mvPexConfigRead: ERR. bus number illigal %d\n", bus));
		return MV_ERROR;
	}
    	     
    DB(mvOsPrintf("pexIf %d, bus %d, dev %d, func %d, regOff 0x%x\n",
                   pexIf, bus, dev, func, regOff));
	
	localDev = mvPexLocalDevNumGet(pexIf);
	localBus = mvPexLocalBusNumGet(pexIf);
                                     
    /* Speed up the process. In case on no link, return MV_ERROR */
    if ((dev != localDev) || (bus != localBus))
    {
        pexData = MV_REG_READ(PEX_STATUS_REG(pexIf));

        if ((pexData & PXSR_DL_DOWN))
        {
            return MV_ERROR;
        }
    }

    /* in PCI Express we have only one device number */
	/* and this number is the first number we encounter 
	else that the localDev*/
	/* spec pex define return on config read/write on any device */
	if (bus == localBus)
	{
		if (localDev == 0)
		{
			/* if local dev is 0 then the first number we encounter 
			after 0 is 1 */
			if ((dev != 1)&&(dev != localDev))
			{
				return MV_ERROR;
			}	
		}
		else
		{
			/* if local dev is not 0 then the first number we encounter 
			is 0 */
	
			if ((dev != 0)&&(dev != localDev))
			{
				return MV_ERROR;
			}
		}
		if(func != 0 ) /* i.e bridge */
		{
			return MV_ERROR;
		}
	}
    
	/* Creating PEX address to be passed */
	pexData |= (bus << PXCAR_BUS_NUM_OFFS);
	pexData |= (dev << PXCAR_DEVICE_NUM_OFFS);
	pexData |= (func << PXCAR_FUNC_NUM_OFFS);
	pexData |= (regOff & PXCAR_REG_NUM_MASK); /* lgacy register space */
	/* extended register space */
	pexData |=(((regOff & PXCAR_REAL_EXT_REG_NUM_MASK) >> 
				PXCAR_REAL_EXT_REG_NUM_OFFS) << PXCAR_EXT_REG_NUM_OFFS);

	pexData |= PXCAR_CONFIG_EN; 
	
	/* Write the address to the PEX configuration address register */
	MV_REG_WRITE(PEX_CFG_ADDR_REG(pexIf), pexData);

	DB(mvOsPrintf("mvPexConfigRead:address pexData=%x ",pexData));
    
	
	/* In order to let the PEX controller absorbed the address of the read 	*/
	/* transaction we perform a validity check that the address was written */
	if(pexData != MV_REG_READ(PEX_CFG_ADDR_REG(pexIf)))
	{
		return MV_ERROR;
	}

	/* cleaning Master Abort */
	MV_REG_BIT_SET(PEX_CFG_DIRECT_ACCESS(pexIf,PEX_STATUS_AND_COMMAND), 
				   PXSAC_MABORT);

	/* Guideline (GL# PCI Express-1) Erroneous Read Data on Configuration   */
	/* This guideline is relevant for all devices except of the following devices:
	   88F5281-BO and above, and 88F5181L-A0 and above  */
    if (((dev != localDev) || (bus != localBus)) && 
		(
		!((MV_5281_DEV_ID == mvCtrlModelGet()) && (mvCtrlRevGet() >= MV_5281_B0_REV))&&
		!((MV_5181_DEV_ID == mvCtrlModelGet()) && (mvCtrlRevGet() >= MV_5181L_A0_REV))
		))
	{

		/* PCI-Express configuration read work-around */

		/* we will use one of the Punit (AHBToMbus) windows to access the xbar 
		and read the data from there */
		/*
		Need to configure the 2 free Punit (AHB to MBus bridge) 
		address decoding windows:
		Configure the flash Window to handle Configuration space requests 
		for PEX0/1:
		1.    write 0x7931/0x7941 to the flash window and the size, 
		      79-xbar attr (pci cfg), 3/4-xbar target (pex0/1), 1-WinEn
		2.    write base to flash window 
		
		Configuration transactions from the CPU should write/read the data 
		to/from address of the form:
		addr[31:28] = 0x5 (for PEX0) or 0x6 (for PEX1)
		addr[27:24] = extended register number
		addr[23:16] = bus number
		addr[15:11] = device number
		addr[10:8]   = function number
		addr[7:0]     = register number
		*/

		#include "mvAhbToMbus.h"
		{
			MV_U32 winNum;
			MV_AHB_TO_MBUS_DEC_WIN originWin;
			MV_U32 pciAddr=0;
			MV_U32 remapLow=0,remapHigh=0;

			/* 
			We will use DEV_CS2\Flash window for this workarround 
			*/
            
			winNum = mvAhbToMbusWinTargetGet(PEX_CONFIG_RW_WA_TARGET);

			/* save remap values if exist */
			if ((1 == winNum)||(0 == winNum))
			{
				remapLow = MV_REG_READ(AHB_TO_MBUS_WIN_REMAP_LOW_REG(winNum));
				remapHigh = MV_REG_READ(AHB_TO_MBUS_WIN_REMAP_HIGH_REG(winNum));

			}
			

			/* save the original window values */
			mvAhbToMbusWinGet(winNum,&originWin);

			if (PEX_CONFIG_RW_WA_USE_ORIGINAL_WIN_VALUES)
			{
				/* set the window as xbar window */
				(pexIf) ?
				MV_REG_WRITE(AHB_TO_MBUS_WIN_CTRL_REG(winNum), 
					(0x7931 | (((originWin.addrWin.size >> 16)-1) ) << 16)) 
				:
				MV_REG_WRITE(AHB_TO_MBUS_WIN_CTRL_REG(winNum), 
					(0x7941 | (((originWin.addrWin.size >> 16)-1) ) << 16)) ;

				MV_REG_WRITE(AHB_TO_MBUS_WIN_BASE_REG(winNum),
							 originWin.addrWin.baseLow);

				/*pciAddr = originWin.addrWin.baseLow;*/
				pciAddr = (MV_U32)CPU_MEMIO_UNCACHED_ADDR(
					(MV_U32)originWin.addrWin.baseLow);
			
			}
			else
			{
				/* set the window as xbar window */
				(pexIf) ?
				MV_REG_WRITE(AHB_TO_MBUS_WIN_CTRL_REG(winNum), 
					(0x7931 | (((PEX_CONFIG_RW_WA_SIZE >> 16)-1) ) << 16)) 
				:
				MV_REG_WRITE(AHB_TO_MBUS_WIN_CTRL_REG(winNum), 
					(0x7941 | (((PEX_CONFIG_RW_WA_SIZE >> 16)-1) ) << 16)) ;


				MV_REG_WRITE(AHB_TO_MBUS_WIN_BASE_REG(winNum),
							 PEX_CONFIG_RW_WA_BASE);

				pciAddr = (MV_U32)CPU_MEMIO_UNCACHED_ADDR(PEX_CONFIG_RW_WA_BASE);
			}
			
			
			/* remap should be as base */
			if ((1 == winNum)||(0 == winNum))
			{
			   MV_REG_WRITE(AHB_TO_MBUS_WIN_REMAP_LOW_REG(winNum),pciAddr);
			   MV_REG_WRITE(AHB_TO_MBUS_WIN_REMAP_HIGH_REG(winNum),0);

			}

			/* extended register space */
			pciAddr |= (bus << 16);
			pciAddr |= (dev << 11);
			pciAddr |= (func << 8);
			pciAddr |= (regOff & PXCAR_REG_NUM_MASK); /* lgacy register space */

			pexData = *(MV_U32*)pciAddr; 
			pexData = MV_32BIT_LE(pexData); /* Data always in LE */

			/* restore the original window values */
			mvAhbToMbusWinSet(winNum,&originWin);

			/* restore original remap values*/
			if ((1 == winNum)||(0 == winNum))
			{
			   MV_REG_WRITE(AHB_TO_MBUS_WIN_REMAP_LOW_REG(winNum),remapLow);
			   MV_REG_WRITE(AHB_TO_MBUS_WIN_REMAP_HIGH_REG(winNum),remapHigh);

			}
		}
	}
	else
	{
		/* Read the Data returned in the PEX Data register */
		pexData = MV_REG_READ(PEX_CFG_DATA_REG(pexIf));

	}

	DB(mvOsPrintf(" got : %x \n",pexData));
	
	return pexData;
}

/*******************************************************************************
* mvPexConfigWrite - Write to configuration space
*
* DESCRIPTION:
*       This function performs a 32 bit write to PEX configuration space.
*       It supports both type 0 and type 1 of Configuration Transactions 
*       (local and over bridge). In order to write to local bus segment, use 
*       bus number retrieved from mvPexLocalBusNumGet(). Other bus numbers 
*       will result configuration transaction of type 1 (over bridge).
*
* INPUT:
*       pexIf   - PEX interface number.
*       bus     - PEX segment bus number.
*       dev     - PEX device number.
*       func    - Function number.
*       regOffs - Register offset.       
*       data    - 32bit data.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPexConfigWrite(MV_U32 pexIf, MV_U32 bus, MV_U32 dev, 
                           MV_U32 func, MV_U32 regOff, MV_U32 data)
{
	MV_U32 pexData = 0;
	MV_U32	localDev,localBus;

	/* Parameter checking   */
	if (PEX_DEFAULT_IF != pexIf)
	{
		if (pexIf >= mvCtrlPexMaxIfGet())
		{
			mvOsPrintf("mvPexConfigWrite: ERR. Invalid PEX interface %d\n", 
																		pexIf);
			return MV_ERROR;
		}
	}

	if (dev >= MAX_PEX_DEVICES)
	{
		mvOsPrintf("mvPexConfigWrite: ERR. device number illigal %d\n",dev);
		return MV_BAD_PARAM;
	}

	if (func >= MAX_PEX_FUNCS)
	{
		mvOsPrintf("mvPexConfigWrite: ERR. function number illigal %d\n", func);
		return MV_ERROR;
	}

	if (bus >= MAX_PEX_BUSSES)
	{
		mvOsPrintf("mvPexConfigWrite: ERR. bus number illigal %d\n", bus);
		return MV_ERROR;
	}



	localDev = mvPexLocalDevNumGet(pexIf);
	localBus = mvPexLocalBusNumGet(pexIf);

	
	/* in PCI Express we have only one device number other than ourselves*/
	/* and this number is the first number we encounter 
		else than the localDev that can be any valid dev number*/
	/* pex spec define return on config read/write on any device */
	if (bus == localBus)
	{

		if (localDev == 0)
		{
			/* if local dev is 0 then the first number we encounter 
			after 0 is 1 */
			if ((dev != 1)&&(dev != localDev))
			{
				return MV_ERROR;
			}
	
		}
		else
		{
			/* if local dev is not 0 then the first number we encounter 
			is 0 */
	
			if ((dev != 0)&&(dev != localDev))
			{
				return MV_ERROR;
			}
		}

		
	}

	/* if we are not accessing ourselves , then check the link */
	if ((dev != localDev) || (bus != localBus) )
	{
		/* workarround */
		/* when no link return MV_ERROR */

		pexData = MV_REG_READ(PEX_STATUS_REG(pexIf));

		if ((pexData & PXSR_DL_DOWN))
		{
			return MV_ERROR;
		}

	}

	pexData =0;

	/* Creating PEX address to be passed */
	pexData |= (bus << PXCAR_BUS_NUM_OFFS);
	pexData |= (dev << PXCAR_DEVICE_NUM_OFFS);
	pexData |= (func << PXCAR_FUNC_NUM_OFFS);
	pexData |= (regOff & PXCAR_REG_NUM_MASK); /* lgacy register space */
	/* extended register space */
	pexData |=(((regOff & PXCAR_REAL_EXT_REG_NUM_MASK) >> 
				PXCAR_REAL_EXT_REG_NUM_OFFS) << PXCAR_EXT_REG_NUM_OFFS);
	pexData |= PXCAR_CONFIG_EN; 
	
	DB(mvOsPrintf("	CfWr:If=%x bus=%x func=%x dev=%x regOff=%x data=%x \n",
		   pexIf,bus,func,dev,regOff,data,pexData) ); 

	/* Write the address to the PEX configuration address register */
	MV_REG_WRITE(PEX_CFG_ADDR_REG(pexIf), pexData);

	/* In order to let the PEX controller absorbed the address of the read 	*/
	/* transaction we perform a validity check that the address was written */
	if(pexData != MV_REG_READ(PEX_CFG_ADDR_REG(pexIf)))
	{
		return MV_ERROR;
	}

	/* Write the Data passed to the PEX Data register */
	MV_REG_WRITE(PEX_CFG_DATA_REG(pexIf), data);

	return MV_OK;
}

/*******************************************************************************
* mvPexMasterEnable - Enable/disale PEX interface master transactions.
*
* DESCRIPTION:
*       This function performs read modified write to PEX command status 
*       (offset 0x4) to set/reset bit 2. After this bit is set, the PEX 
*       master is allowed to gain ownership on the bus, otherwise it is 
*       incapable to do so.
*
* INPUT:
*       pexIf  - PEX interface number.
*       enable - Enable/disable parameter.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPexMasterEnable(MV_U32 pexIf, MV_BOOL enable)
{
	MV_U32 pexCommandStatus;
	MV_U32 localBus;
	MV_U32 localDev;

	/* Parameter checking   */
	if (pexIf >= mvCtrlPexMaxIfGet())
	{
		mvOsPrintf("mvPexMasterEnable: ERR. Invalid PEX interface %d\n", pexIf);
		return MV_ERROR;
	}

	localBus = mvPexLocalBusNumGet(pexIf);
	localDev = mvPexLocalDevNumGet(pexIf);
	
	pexCommandStatus = MV_REG_READ(PEX_CFG_DIRECT_ACCESS(pexIf,
													PEX_STATUS_AND_COMMAND));


	if (MV_TRUE == enable)
	{
		pexCommandStatus |= PXSAC_MASTER_EN;
	}
	else
	{
		pexCommandStatus &= ~PXSAC_MASTER_EN;
	}

	
	MV_REG_WRITE(PEX_CFG_DIRECT_ACCESS(pexIf,PEX_STATUS_AND_COMMAND),
				 pexCommandStatus);

	return MV_OK;
}


/*******************************************************************************
* mvPexSlaveEnable - Enable/disale PEX interface slave transactions.
*
* DESCRIPTION:
*       This function performs read modified write to PEX command status 
*       (offset 0x4) to set/reset bit 0 and 1. After those bits are set, 
*       the PEX slave is allowed to respond to PEX IO space access (bit 0) 
*       and PEX memory space access (bit 1). 
*
* INPUT:
*       pexIf  - PEX interface number.
*       dev     - PEX device number.
*       enable - Enable/disable parameter.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPexSlaveEnable(MV_U32 pexIf, MV_U32 bus,MV_U32 dev, MV_BOOL enable)
{
	MV_U32 pexCommandStatus;
	MV_U32 RegOffs;

	/* Parameter checking   */
	if (pexIf >= mvCtrlPexMaxIfGet())
	{
		mvOsPrintf("mvPexSlaveEnable: ERR. Invalid PEX interface %d\n", pexIf);
		return MV_BAD_PARAM;
	}
	if (dev >= MAX_PEX_DEVICES)
	{
		mvOsPrintf("mvPexLocalDevNumSet: ERR. device number illigal %d\n", dev);
		return MV_BAD_PARAM;

	}

	
	RegOffs = PEX_STATUS_AND_COMMAND;
	
	pexCommandStatus = mvPexConfigRead(pexIf, bus, dev, 0, RegOffs);

    if (MV_TRUE == enable)
	{
		pexCommandStatus |= (PXSAC_IO_EN | PXSAC_MEM_EN);
	}
	else                             
	{
		pexCommandStatus &= ~(PXSAC_IO_EN | PXSAC_MEM_EN);
	}

	mvPexConfigWrite(pexIf, bus, dev, 0, RegOffs, pexCommandStatus);

	return MV_OK;

}

/*******************************************************************************
* mvPexLocalBusNumSet - Set PEX interface local bus number.
*
* DESCRIPTION:
*       This function sets given PEX interface its local bus number.
*       Note: In case the PEX interface is PEX-X, the information is read-only.
*
* INPUT:
*       pexIf  - PEX interface number.
*       busNum - Bus number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_NOT_ALLOWED in case PEX interface is PEX-X. 
*		MV_BAD_PARAM on bad parameters ,
*       otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPexLocalBusNumSet(MV_U32 pexIf, MV_U32 busNum)
{
	MV_U32 pexStatus;
	MV_U32 localBus;
	MV_U32 localDev;


	/* Parameter checking   */
	if (pexIf >= mvCtrlPexMaxIfGet())
	{
		mvOsPrintf("mvPexLocalBusNumSet: ERR. Invalid PEX interface %d\n",pexIf);
		return MV_BAD_PARAM;
	}
	if (busNum >= MAX_PEX_BUSSES)
	{
		mvOsPrintf("mvPexLocalBusNumSet: ERR. bus number illigal %d\n", busNum);
		return MV_ERROR;

	}

	localBus = mvPexLocalBusNumGet(pexIf);
	localDev = mvPexLocalDevNumGet(pexIf);



	pexStatus  = MV_REG_READ(PEX_STATUS_REG(pexIf));

	pexStatus &= ~PXSR_PEX_BUS_NUM_MASK;

	pexStatus |= (busNum << PXSR_PEX_BUS_NUM_OFFS) & PXSR_PEX_BUS_NUM_MASK;

	MV_REG_WRITE(PEX_STATUS_REG(pexIf), pexStatus);


	return MV_OK;
}


/*******************************************************************************
* mvPexLocalBusNumGet - Get PEX interface local bus number.
*
* DESCRIPTION:
*       This function gets the local bus number of a given PEX interface.
*
* INPUT:
*       pexIf  - PEX interface number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Local bus number.0xffffffff on Error
*
*******************************************************************************/
MV_U32 mvPexLocalBusNumGet(MV_U32 pexIf)
{
	MV_U32 pexStatus;

	/* Parameter checking   */
	if (PEX_DEFAULT_IF != pexIf)
	{
		if (pexIf >= mvCtrlPexMaxIfGet())
		{
			mvOsPrintf("mvPexLocalBusNumGet: ERR. Invalid PEX interface %d\n", 
													   					pexIf);
			return 0xFFFFFFFF;
		}
	}


	pexStatus  = MV_REG_READ(PEX_STATUS_REG(pexIf));

	pexStatus &= PXSR_PEX_BUS_NUM_MASK;

	return (pexStatus >> PXSR_PEX_BUS_NUM_OFFS);

}


/*******************************************************************************
* mvPexLocalDevNumSet - Set PEX interface local device number.
*
* DESCRIPTION:
*       This function sets given PEX interface its local device number.
*       Note: In case the PEX interface is PEX-X, the information is read-only.
*
* INPUT:
*       pexIf  - PEX interface number.
*       devNum - Device number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_NOT_ALLOWED in case PEX interface is PEX-X. 
*		MV_BAD_PARAM on bad parameters ,
*       otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPexLocalDevNumSet(MV_U32 pexIf, MV_U32 devNum)
{
	MV_U32 pexStatus;
	MV_U32 localBus;
	MV_U32 localDev;

	/* Parameter checking   */
	if (pexIf >= mvCtrlPexMaxIfGet())
	{
		mvOsPrintf("mvPexLocalDevNumSet: ERR. Invalid PEX interface %d\n",pexIf);
		return MV_BAD_PARAM;
	}
	if (devNum >= MAX_PEX_DEVICES)
	{
		mvOsPrintf("mvPexLocalDevNumSet: ERR. device number illigal %d\n", 
																	   devNum);
		return MV_BAD_PARAM;

	}
	
	localBus = mvPexLocalBusNumGet(pexIf);
	localDev = mvPexLocalDevNumGet(pexIf);


	pexStatus  = MV_REG_READ(PEX_STATUS_REG(pexIf));

	pexStatus &= ~PXSR_PEX_DEV_NUM_MASK;

	pexStatus |= (devNum << PXSR_PEX_DEV_NUM_OFFS) & PXSR_PEX_DEV_NUM_MASK;

	MV_REG_WRITE(PEX_STATUS_REG(pexIf), pexStatus);


	return MV_OK;
}

/*******************************************************************************
* mvPexLocalDevNumGet - Get PEX interface local device number.
*
* DESCRIPTION:
*       This function gets the local device number of a given PEX interface.
*
* INPUT:
*       pexIf  - PEX interface number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Local device number. 0xffffffff on Error
*
*******************************************************************************/
MV_U32 mvPexLocalDevNumGet(MV_U32 pexIf)
{
	MV_U32 pexStatus;

	/* Parameter checking   */
	
	if (PEX_DEFAULT_IF != pexIf)
	{
		if (pexIf >= mvCtrlPexMaxIfGet())
		{
			mvOsPrintf("mvPexLocalDevNumGet: ERR. Invalid PEX interface %d\n", 
																   		pexIf);
			return 0xFFFFFFFF;
		}
	}
	
	pexStatus  = MV_REG_READ(PEX_STATUS_REG(pexIf));

	pexStatus &= PXSR_PEX_DEV_NUM_MASK;

	return (pexStatus >> PXSR_PEX_DEV_NUM_OFFS);
}

/*******************************************************************************
* mvPexTargetWinSet - Set PEX to peripheral target address window BAR
*
* DESCRIPTION:
*
* INPUT:
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
MV_STATUS mvPexTargetWinSet(MV_U32 pexIf, MV_U32 winNum, 
                            MV_PEX_DEC_WIN *pAddrDecWin)
{

	MV_DEC_REGS decRegs;
	PEX_WIN_REG_INFO winRegInfo;
	MV_TARGET_ATTRIB targetAttribs;

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

	/* get the pex Window registers offsets */
	pexWinRegInfoGet(pexIf,winNum,&winRegInfo);


	if (MV_TRUE == pAddrDecWin->enable)
	{

		/* 2) Check if the requested window overlaps with current windows  */
		if (MV_TRUE == pexWinOverlapDetect(pexIf,winNum, &pAddrDecWin->addrWin))
		{
			mvOsPrintf("mvPexTargetWinSet: ERR. Target %d overlap\n", winNum);
			return MV_BAD_PARAM;
		}
	
		/* 2) Check if the requested window overlaps with current windows  */
		if (MV_FALSE == pexIsWinWithinBar(pexIf,&pAddrDecWin->addrWin))
		{
			mvOsPrintf("mvPexTargetWinSet: Win %d should be in bar boundries\n",
					   winNum);
			return MV_BAD_PARAM;
		}

	}



	/* read base register*/
	
	if (winRegInfo.baseLowRegOffs)
	{
		decRegs.baseReg = MV_REG_READ(winRegInfo.baseLowRegOffs);
	}
	else
	{
		decRegs.baseReg = 0;
	}

	if (winRegInfo.sizeRegOffs)
	{
		decRegs.sizeReg = MV_REG_READ(winRegInfo.sizeRegOffs);
	}
	else
	{
		decRegs.sizeReg =0;
	}
	
	if (MV_OK != mvCtrlAddrDecToReg(&(pAddrDecWin->addrWin),&decRegs))
	{
		mvOsPrintf("mvPexTargetWinSet:mvCtrlAddrDecToReg Failed\n");
		return MV_ERROR;		
	}

	/* enable\Disable */
	if (MV_TRUE == pAddrDecWin->enable)
	{
		decRegs.sizeReg |= PXWCR_WIN_EN;
	}
	else
	{
		decRegs.sizeReg &= ~PXWCR_WIN_EN;
	}


	/* clear bit location */
	decRegs.sizeReg &= ~PXWCR_WIN_BAR_MAP_MASK;

	/* set bar Mapping */
	if (pAddrDecWin->targetBar == 1)
	{
		decRegs.sizeReg |= PXWCR_WIN_BAR_MAP_BAR1;
	}
	else if (pAddrDecWin->targetBar == 2)
	{
		decRegs.sizeReg |= PXWCR_WIN_BAR_MAP_BAR2;
	}

	mvCtrlAttribGet(pAddrDecWin->target,&targetAttribs);

	/* set attributes */
	decRegs.sizeReg &= ~PXWCR_ATTRIB_MASK;
	decRegs.sizeReg |= targetAttribs.attrib << PXWCR_ATTRIB_OFFS;
	/* set target ID */
	decRegs.sizeReg &= ~PXWCR_TARGET_MASK;
	decRegs.sizeReg |= targetAttribs.targetId << PXWCR_TARGET_OFFS;


	/* 3) Write to address decode Base Address Register                   */

	if (winRegInfo.baseLowRegOffs)
	{
		MV_REG_WRITE(winRegInfo.baseLowRegOffs, decRegs.baseReg);
	}

	/* write size reg */
	if (winRegInfo.sizeRegOffs)
	{
		if ((MV_PEX_WIN_DEFAULT == winNum)||
			(MV_PEX_WIN_EXP_ROM == winNum))
		{
			/* clear size because there is no size field*/
			decRegs.sizeReg &= ~PXWCR_SIZE_MASK;

			/* clear enable because there is no enable field*/
			decRegs.sizeReg &= ~PXWCR_WIN_EN;

		}

		MV_REG_WRITE(winRegInfo.sizeRegOffs, decRegs.sizeReg);
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
*       pexIf - PEX interface number.
*       bar   - BAR to be accessed by slave.
*
* OUTPUT:
*       pAddrBarWin - PEX target window information data structure.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvPexTargetWinGet(MV_U32 pexIf, MV_U32 winNum, 
                            MV_PEX_DEC_WIN *pAddrDecWin)
{
	MV_TARGET_ATTRIB targetAttrib;
	MV_DEC_REGS decRegs;

	PEX_WIN_REG_INFO winRegInfo;

	/* Parameter checking   */
	if(pexIf >= mvCtrlPexMaxIfGet())
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
	pexWinRegInfoGet(pexIf,winNum,&winRegInfo);

	/* read base register*/
	if (winRegInfo.baseLowRegOffs)
	{
		decRegs.baseReg = MV_REG_READ(winRegInfo.baseLowRegOffs);
	}
	else
	{
		decRegs.baseReg = 0;
	}

	/* read size reg */
	if (winRegInfo.sizeRegOffs)
	{
		decRegs.sizeReg = MV_REG_READ(winRegInfo.sizeRegOffs);
	}
	else
	{
		decRegs.sizeReg =0;
	}

	if (MV_OK != mvCtrlRegToAddrDec(&decRegs,&(pAddrDecWin->addrWin)))
	{
		mvOsPrintf("mvPexTargetWinGet: mvCtrlRegToAddrDec Failed \n");
		return MV_ERROR;

	}

	if (decRegs.sizeReg & PXWCR_WIN_EN)
	{
		pAddrDecWin->enable = MV_TRUE;
	}
	else
	{
		pAddrDecWin->enable = MV_FALSE;	  

	}


	#if 0
    if (-1 == pAddrDecWin->addrWin.size)
	{
		return MV_ERROR;
	}
	#endif


	/* get target bar */
	if ((decRegs.sizeReg & PXWCR_WIN_BAR_MAP_MASK) == PXWCR_WIN_BAR_MAP_BAR1 )
	{
		pAddrDecWin->targetBar = 1;
	} 
	else if ((decRegs.sizeReg & PXWCR_WIN_BAR_MAP_MASK) == 
			 PXWCR_WIN_BAR_MAP_BAR2 )
	{
		pAddrDecWin->targetBar = 2;
	}

	/* attrib and targetId */
	pAddrDecWin->attrib = (decRegs.sizeReg & PXWCR_ATTRIB_MASK) >> 
													PXWCR_ATTRIB_OFFS;
	pAddrDecWin->targetId = (decRegs.sizeReg & PXWCR_TARGET_MASK) >> 
													PXWCR_TARGET_OFFS;

	targetAttrib.attrib = pAddrDecWin->attrib;
	targetAttrib.targetId = pAddrDecWin->targetId;

	pAddrDecWin->target = mvCtrlTargetGet(&targetAttrib);

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
*       bar    - BAR to be accessed by slave.
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
	PEX_WIN_REG_INFO winRegInfo;
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


	/* get the pex Window registers offsets */
	pexWinRegInfoGet(pexIf,winNum,&winRegInfo);


	/* if the address windows is disabled , we only disable the appropriare
	pex window and ignore other settings */

	if (MV_FALSE == enable)
	{

		/* this is not relevant to default and expantion rom
		windows */
		if (winRegInfo.sizeRegOffs)
		{
			if ((MV_PEX_WIN_DEFAULT != winNum)&&
				(MV_PEX_WIN_EXP_ROM != winNum))
			{
				MV_REG_BIT_RESET(winRegInfo.sizeRegOffs, PXWCR_WIN_EN);
			}
		}

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

		if (MV_FALSE == pexIsWinWithinBar(pexIf,&addrDecWin.addrWin))
		{
			mvOsPrintf("mvPexTargetWinEnable: Win %d should be in bar boundries\n",
					   winNum);
			return MV_BAD_PARAM;
		}


		/* this is not relevant to default and expantion rom
		windows */
		if (winRegInfo.sizeRegOffs)
		{
			if ((MV_PEX_WIN_DEFAULT != winNum)&&
				(MV_PEX_WIN_EXP_ROM != winNum))
			{
				MV_REG_BIT_SET(winRegInfo.sizeRegOffs, PXWCR_WIN_EN);
			}
		}


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
*       pexIf    - PEX interface number.
*       bar      - Peripheral target enumerator accessed by slave.
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

	PEX_WIN_REG_INFO winRegInfo;
	
	/* Parameter checking   */
	if (pexIf >= mvCtrlPexMaxIfGet())
	{
		mvOsPrintf("mvPexTargetWinRemap: ERR. Invalid PEX interface num %d\n", 
																		pexIf);
		return MV_BAD_PARAM;
	}
	if (MV_PEX_WIN_DEFAULT == winNum)
	{
		mvOsPrintf("mvPexTargetWinRemap: ERR. Invalid PEX win num %d\n", 
																		winNum);
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

	pexWinRegInfoGet(pexIf, winNum, &winRegInfo);

	/* Set remap low register value */
	MV_REG_WRITE(winRegInfo.remapLowRegOffs, pAddrWin->addrWin.baseLow);
	
	/* Skip base high settings if the BAR has only base low (32-bit)		*/
	if (0 != winRegInfo.remapHighRegOffs)
	{
		MV_REG_WRITE(winRegInfo.remapHighRegOffs, pAddrWin->addrWin.baseHigh);
	}


	if (pAddrWin->enable == MV_TRUE)
	{
		MV_REG_BIT_SET(winRegInfo.remapLowRegOffs,PXWRR_REMAP_EN);
	}
	else
	{
		MV_REG_BIT_RESET(winRegInfo.remapLowRegOffs,PXWRR_REMAP_EN);
	}

	return MV_OK;
}

/*******************************************************************************
* mvPexTargetWinRemapEnable - 
*
* DESCRIPTION:
*
* INPUT:
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
	PEX_WIN_REG_INFO winRegInfo;
	
	/* Parameter checking   */
	if (pexIf >= mvCtrlPexMaxIfGet())
	{
		mvOsPrintf("mvPexTargetWinRemap: ERR. Invalid PEX interface num %d\n", 
																		pexIf);
		return MV_BAD_PARAM;
	}
	if (MV_PEX_WIN_DEFAULT == winNum)
	{
		mvOsPrintf("mvPexTargetWinRemap: ERR. Invalid PEX win num %d\n", 
																		winNum);
		return MV_BAD_PARAM;

	}


	pexWinRegInfoGet(pexIf, winNum, &winRegInfo);

	if (enable == MV_TRUE)
	{
		MV_REG_BIT_SET(winRegInfo.remapLowRegOffs,PXWRR_REMAP_EN);
	}
	else
	{
		MV_REG_BIT_RESET(winRegInfo.remapLowRegOffs,PXWRR_REMAP_EN);
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
		mvOsPrintf("mvPexBarSet: Size zero is Illigal\n" );
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
	regBaseLow = MV_REG_READ(PEX_CFG_DIRECT_ACCESS(pexIf,
												   PEX_MV_BAR_BASE(barNum)));

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

	
	for(win = 0; win < PEX_MAX_TARGET_WIN -2 ; win++)
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
			mvOsPrintf("pexWinOverlapDetect: winNum %d overlap current %d\n", 
															winNum, win);
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
			mvOsPrintf("pexBarOverlapDetect: winNum %d overlap current %d\n", 
															barNum, bar);
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
*       pexIf - PEX interface number.
*		bar	  - The PEX BAR in question.	
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

	if ((winNum >= 0)&&(winNum <=3))
	{
		pWinRegInfo->baseLowRegOffs   = PEX_WIN0_3_BASE_REG(pexIf,winNum);
		pWinRegInfo->baseHighRegOffs  = 0;
		pWinRegInfo->sizeRegOffs      = PEX_WIN0_3_CTRL_REG(pexIf,winNum);
		pWinRegInfo->remapLowRegOffs  = PEX_WIN0_3_REMAP_REG(pexIf,winNum);
		pWinRegInfo->remapHighRegOffs = 0;
	}
	else if ((winNum >= 4)&&(winNum <=5))
	{
		pWinRegInfo->baseLowRegOffs   = PEX_WIN4_5_BASE_REG(pexIf,winNum);
		pWinRegInfo->baseHighRegOffs  = 0;
		pWinRegInfo->sizeRegOffs      = PEX_WIN4_5_CTRL_REG(pexIf,winNum);
		pWinRegInfo->remapLowRegOffs  = PEX_WIN4_5_REMAP_REG(pexIf,winNum);
		pWinRegInfo->remapHighRegOffs = PEX_WIN4_5_REMAP_HIGH_REG(pexIf,winNum);

	}
	else if (MV_PEX_WIN_DEFAULT == winNum)
	{
		pWinRegInfo->baseLowRegOffs   = 0;
		pWinRegInfo->baseHighRegOffs  = 0;
		pWinRegInfo->sizeRegOffs      = PEX_WIN_DEFAULT_CTRL_REG(pexIf);
		pWinRegInfo->remapLowRegOffs  = 0;
		pWinRegInfo->remapHighRegOffs = 0;
	}
	else if (MV_PEX_WIN_EXP_ROM == winNum)
	{
		pWinRegInfo->baseLowRegOffs   = 0;
		pWinRegInfo->baseHighRegOffs  = 0;
		pWinRegInfo->sizeRegOffs      = PEX_WIN_EXP_ROM_CTRL_REG(pexIf);
		pWinRegInfo->remapLowRegOffs  = PEX_WIN_EXP_ROM_REMAP_REG(pexIf);
		pWinRegInfo->remapHighRegOffs = 0;

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
	MV_PEX_DEC_WIN win;
	MV_U32 pexIf;
	MV_U32 bar,winNum;

	for( pexIf = MV_PEX_START_IF; pexIf < mvCtrlPexMaxIfGet(); pexIf++ )
	{
		mvOsOutput( "\n" );
		mvOsOutput( "PEX%d:\n", pexIf );
		mvOsOutput( "-----\n" );

		mvOsOutput( "\nPex Bars \n\n");

		for( bar = 0; bar < PEX_MAX_BARS; bar++ ) 
		{
			memset( &pexBar, 0, sizeof(MV_PEX_BAR) );

			mvOsOutput( "%s ", pexBarNameGet(bar) );

			if( mvPexBarGet( pexIf, bar, &pexBar ) == MV_OK )
			{
				if( pexBar.enable )
				{
                    mvOsOutput( "base %08x, ", pexBar.addrWin.baseLow );
                    mvSizePrint( pexBar.addrWin.size );
                    mvOsOutput( "\n" );
				}
				else
					mvOsOutput( "disable\n" );
			}
		}
		mvOsOutput( "\nPex Decode Windows\n\n");

		for( winNum = 0; winNum < PEX_MAX_TARGET_WIN - 2; winNum++)
		{
			memset( &win, 0,sizeof(MV_PEX_DEC_WIN) );

			mvOsOutput( "win%d - ", winNum );

			if ( mvPexTargetWinGet(pexIf,winNum,&win) == MV_OK)
			{
				if (win.enable)
				{
					mvOsOutput( "%s base %08x, ",
					mvCtrlTargetNameGet(win.target), win.addrWin.baseLow );
					mvOsOutput( "...." );
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
}


