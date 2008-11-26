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

//#define MV_DEBUG
/* defines  */       
#ifdef MV_DEBUG         
	#define DB(x)	x
#else                
	#define DB(x)    
#endif	             

/* locals */         
typedef struct
{
	MV_U32 data;
	MV_U32 mask;
}PEX_HEADER_DATA;

/* local function forwad decleration */
MV_U32 mvPexHwConfigRead (MV_U32 pexIf, MV_U32 bus, MV_U32 dev, MV_U32 func, 
                        MV_U32 regOff);
MV_STATUS mvPexHwConfigWrite(MV_U32 pexIf, MV_U32 bus, MV_U32 dev, 
                           MV_U32 func, MV_U32 regOff, MV_U32 data);
void resetPexConfig(MV_U32 pexIf, MV_U32 bus, MV_U32 dev);


PEX_HEADER_DATA configHdr[16] = 
{
{0x888811ab, 0x00000000}, /*[device ID, vendor ID] */
{0x00100007, 0x0000ffff}, /*[status register, command register] */  
{0x0604000e, 0x00000000}, /*[programming interface, sub class code, class code, revision ID] */
{0x00010008, 0x00000000},  /*[BIST, header type, latency time, cache line] */
{0x00000000, 0x00000000},  /*[base address 0] */             
{0x00000000, 0x00000000},  /*[base address 1] */             
{0x00000000, 0x00ffffff},  /*[secondary latency timersubordinate bus number, secondary bus number, primary bus number] */         
{0x0000f101, 0x00000000},  /*[secondary status ,IO limit, IO base] */
{0x9ff0a000, 0x00000000},  /*[memory limit, memory base] */                
{0x0001fff1, 0x00000000},  /*[prefetch memory limit, prefetch memory base] */       
{0xffffffff, 0x00000000},  /*[prefetch memory base upper] */ 
{0x00000000, 0x00000000},  /*[prefetch memory limit upper] */
{0xeffff000, 0x00000000},  /*[IO limit upper 16 bits, IO base upper 16 bits] */      
{0x00000000, 0x00000000},  /*[reserved, capability pointer] */ 
{0x00000000, 0x00000000},  /*[expansion ROM base address] */ 
{0x00000000, 0x000000FF},  /*[bridge control, interrupt pin, interrupt line] */             
};


#define HEADER_WRITE(data, offset) configHdr[offset/4].data = ((configHdr[offset/4].data & ~configHdr[offset/4].mask) | \
																(data & configHdr[offset/4].mask))
#define HEADER_READ(offset) configHdr[offset/4].data

/*******************************************************************************
* mvVrtBrgPexInit - Initialize PEX interfaces
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
MV_STATUS mvPexVrtBrgInit(MV_U32 pexIf)
{
	/* reset PEX tree to recover previous U-boot/Boot configurations */
	MV_U32 localBus = mvPexLocalBusNumGet(pexIf);


	resetPexConfig(pexIf, localBus, 1);
	return MV_OK;
}


MV_U32 mvPexVrtBrgConfigRead (MV_U32 pexIf, MV_U32 bus, MV_U32 dev, MV_U32 func, 
                        MV_U32 regOff)
{
	
	MV_U32 localBus = mvPexLocalBusNumGet(pexIf);
	MV_U32 localDev = mvPexLocalDevNumGet(pexIf);

	if(bus == localBus)
	{
		if(dev > 1)
		{
/* on the local device allow only device #0 & #1 */
			return 0xffffffff;
		}
		else
		if (dev == localDev)
		{
			/* read the memory controller registers */
			return mvPexHwConfigRead (pexIf, bus, dev, func, regOff);
		}
		else
		{
			/* access the virtual brg header */
			return HEADER_READ(regOff);
		}
	}
	else
	if(bus == (localBus + 1))
	{
		/* access the device behind the virtual bridge */
		if((dev == localDev) || (dev > 1))
		{
			return 0xffffffff;
		}
		else
		{
			/* access the device behind the virtual bridge, in this case 
			*  change the bus number to the local bus number in order to 
			*  generate type 0 config cycle
			*/
			return mvPexHwConfigRead (pexIf, localBus, dev, func, regOff);
		}
	}
	/* for all other devices use the HW function to get the 
	*  requested registers
	*/
	return mvPexHwConfigRead (pexIf, bus, dev, func, regOff);

}


MV_STATUS mvPexVrtBrgConfigWrite(MV_U32 pexIf, MV_U32 bus, MV_U32 dev, 
                           MV_U32 func, MV_U32 regOff, MV_U32 data)
{
	MV_U32 localBus = mvPexLocalBusNumGet(pexIf);
	MV_U32 localDev = mvPexLocalDevNumGet(pexIf);

	if(bus == localBus)
	{
		if(dev > 1)
		{
			/* on the local device allow only device #0 & #1 */
			return MV_ERROR;
		}
		else
		if (dev == localDev)
		{
			/* read the memory controller registers */
			return mvPexHwConfigWrite (pexIf, bus, dev, func, regOff, data);
			
		}
		else
		{
			/* access the virtual brg header */
			HEADER_WRITE(data, regOff);
			return MV_OK;
		}
	}
	else
	if(bus == (localBus + 1))
	{
		/* access the device behind the virtual bridge */
		if((dev == localDev) || (dev > 1))
		{
			return MV_ERROR;
		}
		else
		{
			/* access the device behind the virtual bridge, in this case 
			*  change the bus number to the local bus number in order to 
			*  generate type 0 config cycle
			*/
			return mvPexHwConfigWrite (pexIf, localBus, dev, func, regOff, data);
		}
	}
	/* for all other devices use the HW function to get the 
	*  requested registers
	*/
	return mvPexHwConfigWrite (pexIf, bus, dev, func, regOff, data);

}


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
MV_U32 mvPexHwConfigRead (MV_U32 pexIf, MV_U32 bus, MV_U32 dev, MV_U32 func, 
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
	pexData = 0; /* ofir */
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


	/* In order to let the PEX controller absorbed the address of the read 	*/
	/* transaction we perform a validity check that the address was written */
	if(pexData != MV_REG_READ(PEX_CFG_ADDR_REG(pexIf)))
	{
		return MV_ERROR;
	}

	/* cleaning Master Abort */
	MV_REG_BIT_SET(PEX_CFG_DIRECT_ACCESS(pexIf,PEX_STATUS_AND_COMMAND), 
				   PXSAC_MABORT);

	    /* Clear CPU pipe. Important where CPU can perform OOO execution */
	    CPU_PIPE_FLUSH;


	/* Read the Data returned in the PEX Data register */
	pexData = MV_REG_READ(PEX_CFG_DATA_REG(pexIf));


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
MV_STATUS mvPexHwConfigWrite(MV_U32 pexIf, MV_U32 bus, MV_U32 dev, 
                           MV_U32 func, MV_U32 regOff, MV_U32 data)
{
	MV_U32 pexData = 0;
	MV_U32	localDev,localBus;

	/* Parameter checking   */
	if (PEX_DEFAULT_IF != pexIf)
	{
		if (pexIf >= mvCtrlPexMaxIfGet())
		{
			mvOsPrintf("mvPexConfigWrite: ERR. Invalid PEX interface %d\n", pexIf);
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

	/* Clear CPU pipe. Important where CPU can perform OOO execution */
    	CPU_PIPE_FLUSH;


	/* Write the Data passed to the PEX Data register */
	MV_REG_WRITE(PEX_CFG_DATA_REG(pexIf), data);

	return MV_OK;
}



void resetPexConfig(MV_U32 pexIf, MV_U32 bus, MV_U32 dev)
{
	MV_U32 tData;
	MV_U32 i;

	/* restore the PEX configuration to initialization state */
	/* in case PEX P2P call recursive and reset config */
	tData = mvPexHwConfigRead (pexIf, bus, dev, 0x0, 0x0);
	if(tData != 0xffffffff)
	{
		/* agent had been found - check whether P2P */
		tData = mvPexHwConfigRead (pexIf, bus, dev, 0x0, 0x8);
		if((tData & 0xffff0000) == 0x06040000)
		{/* P2P */
			/* get the sec bus and the subordinate */
			MV_U32 secBus;	
			tData = mvPexHwConfigRead (pexIf, bus, dev, 0x0, 0x18);
			secBus = ((tData >> 8) & 0xff);
			/* now scan on sec bus */
			for(i = 0;i < 0xff;i++)
			{
				resetPexConfig(pexIf, secBus, i);
			}
			/* now reset this device */
			DB(mvOsPrintf("Reset bus %d dev %d\n", bus, dev));
			mvPexHwConfigWrite(pexIf, bus, dev, 0x0, 0x18, 0x0); 
		}
	}
}


