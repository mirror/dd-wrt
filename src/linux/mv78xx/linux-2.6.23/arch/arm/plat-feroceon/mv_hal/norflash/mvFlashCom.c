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

#include "mvFlashCom.h"
#include "mvIntelFlash.h"
#include "mvAmdFlash.h"

#undef MV_DEBUG

#ifdef MV_DEBUG
#define DB(x) x
#else
#define DB(x)
#endif


static MV_VOID sizePrint (MV_U32 size, MV_U8 *s);

/*******************************************************************************
* mvFlashInWhichSec - Return which Sector rap the offset address.
*
* DESCRIPTION:
*
* INPUT:
*       pFlash	- Flash identifier structure.
*       offset	- offset address.
*
* OUTPUT:
*      None
*
* RETURN:
*       MV_U32	- The Sector Number that the offset sits in.
*	BAD_SEC_NUM if not found.
*
*******************************************************************************/
MV_U32 mvFlashInWhichSec(MV_FLASH_INFO *pFlash, MV_U32 offset)
{
	MV_U32 secNum;
	if(NULL == pFlash)
		return 0;

	for( secNum = 0; secNum < mvFlashNumOfSecsGet(pFlash); secNum++){
		if((offset >= mvFlashSecOffsGet(pFlash, secNum)) &&
		   (offset < mvFlashSecOffsGet(pFlash, secNum) +
					 mvFlashSecSizeGet(pFlash, secNum)) )
		{
			return secNum;
		}
	}
	/* return illegal sector Number */
	return FLASH_BAD_SEC_NUM;
}


/**************************************************/
/* get information from the MV_FLASH_INFO struct  */
/**************************************************/
/* get the Number of Device which sits in parallel on the bus */
MV_U32   mvFlashNumOfDevGet(MV_FLASH_INFO *pFlash)
{
	if(NULL == pFlash)
		return 0;
	if(mvFlashBusWidthGet(pFlash) >= mvFlashDevWidthGet(pFlash))
		return mvFlashBusWidthGet(pFlash) / mvFlashDevWidthGet(pFlash);
	return 1;
}
/* get the Flash Lock type HW/SW */
MV_BOOL	mvFlashIsHwLock(MV_FLASH_INFO *pFlash)
{
	if(NULL == pFlash)
		return MV_FALSE;

	return pFlash->flashSpec.HwProtect;
}
/* get the lock status of a sector in the flash */
MV_BOOL   mvFlashSecLockGet(MV_FLASH_INFO *pFlash, MV_U32 secNum)
{
	if((NULL == pFlash)|| (secNum > mvFlashNumOfSecsGet(pFlash)) )
		return MV_FALSE;

	return pFlash->sector[secNum].protect;
}
/* get the size of a sector in the flash */
MV_U32 mvFlashSecSizeGet(MV_FLASH_INFO *pFlash, MV_U32 secNum)
{
	if((NULL == pFlash) || (secNum > mvFlashNumOfSecsGet(pFlash)))
		return 0;

	return pFlash->sector[secNum].size;
}
/* get the num of sectors in the flash */
MV_U32 mvFlashNumOfSecsGet(MV_FLASH_INFO *pFlash)
{
	if(NULL == pFlash)
		return 0;

	return pFlash->flashSpec.sectorNum;
}
/* get the flash size */
MV_U32 mvFlashSizeGet(MV_FLASH_INFO *pFlash)
{
	if(NULL == pFlash)
		return 0;

	return pFlash->flashSpec.size * mvFlashNumOfDevGet(pFlash);
}
/* get the sector offset */
MV_U32 mvFlashSecOffsGet(MV_FLASH_INFO *pFlash, MV_U32 secNum)
{
	if((NULL == pFlash)|| (secNum > mvFlashNumOfSecsGet(pFlash)))
		return 0;

	return pFlash->sector[secNum].baseOffs;
}
/* get the sector types TOP/BOT/REG */
FLASH_SEC_TYPE	mvFlashSecTypeGet(MV_FLASH_INFO *pFlash)
{
	if(NULL == pFlash)
		return 0;

	return pFlash->flashSpec.secType;
}
/* get the flash Vendor ID */
MV_U32 mvFlashVenIdGet(MV_FLASH_INFO *pFlash)
{
	if(NULL == pFlash)
		return 0;

	return pFlash->flashSpec.flashVen;
}
/* get the flash device id */
MV_U32 mvFlashDevIdGet(MV_FLASH_INFO *pFlash)
{
	if(NULL == pFlash)
		return 0;

	return pFlash->flashSpec.flashId;
}
/* get the flash base address */
MV_U32 mvFlashBaseAddrGet(MV_FLASH_INFO *pFlash)
{
	if(NULL == pFlash)
		return 0;

	return pFlash->baseAddr;
}
/* get the flash bus width */
MV_U32 mvFlashBusWidthGet(MV_FLASH_INFO *pFlash)
{
	if(NULL == pFlash)
		return 0;

	return pFlash->busWidth;
}
/* get the flash device width */
MV_U32 mvFlashDevWidthGet(MV_FLASH_INFO *pFlash)
{
	if(NULL == pFlash)
		return 0;

	return pFlash->devWidth;
}


/*******************************************************************************
* flashDataExt - Extend Data.
* DESCRIPTION:
*	Should be used only for FLASH CFI command sequence.
*
* 	Prepare the Data according to the Flash Width and Bus Width.
* 	If flash width = 2 and bus width = 1 data = 0x55 -> data = 0x55
* 	If flash width = 2 and bus width = 4 data = 0x55 -> data = 0x550055
* 	If flash width = 1 and bus width = 4 data = 0x55 -> data = 0x55555555
*
* INPUT:
*       data    - Data to be expended.
*	pFlash	- flash information.
*
* OUTPUT:
*	None
*
* RETURN:
*	MV_U32 	- Data after extension.
*	OxFFFFFFFF if pFlash is Null
*
*******************************************************************************/
MV_U32 flashDataExt( MV_FLASH_INFO *pFlash, MV_U32 data)
{
	MV_U32	i;
	if(NULL == pFlash)
		return 0xFFFFFFFF;

	for(i = 0; i < pFlash->busWidth ; i+= pFlash->devWidth)
	{
		data |= data << 8*i;
	}
	return data;
}
/******************************************************************************
* flashAddrExt - Extend Addr.
* DESCRIPTION:
*	Should be used only for FLASH CFI command sequence.
*
* 	Prepare the Addr according to the Flash width and the bus width,
* 	and add the sector offset.
*	If flash width = 2 and bus width = 1 then it means we are using 16 Bit
* 	flash in 8 Bit mode, we should make sure that we shift the addr in 1 bit
* 	since in 16 Bit flash A0 isn't connected. and A1 of the MV dev address will
* 	go to A1 of the Flash.
* 	If flash width = 2 and bus width = 4 then it means we are using 2 16 Bit
* 	flash, (for the 16 Bit flash A0 isn't connected) and since when we refer to
* 	address 0x4 we actually want to refer to 0x2 of each device, then we will
* 	connect A2 of the MV Dev addres to A1 of the flash.
*
* INPUT:
*       addr    - Addr to be expended.
*	pFlash	- flash information.
*	secNum 	- Sector Number.
*
* OUTPUT:
*	None
*
* RETURN:
*	MV_U32 	- Data after extension.
*	0 if pFlash is Null.
*
*******************************************************************************/
MV_U32 flashAddrExt(MV_FLASH_INFO *pFlash, MV_U32 addr, MV_U32 secNum )
{
	MV_U32 shift;
	if(NULL == pFlash)
		return 0;

	shift = (pFlash->busWidth > pFlash->devWidth ) ?
										pFlash->busWidth : pFlash->devWidth;
	addr = addr * shift;

	/* Add flash sector Offset.*/
	addr += (mvFlashSecOffsGet(pFlash,secNum) + mvFlashBaseAddrGet(pFlash));

	return addr;
}

/*******************************************************************************
* flashCmdSet - Write converted data to the flash converted address+sector base.
*
* DESCRIPTION:
*		Convert data based on the bus width and the flash device width
*		and write it to secoffset + converted address.
*		Should be used only for FLASH command sequence.
*
* INPUT:
*       addr    - Address offset.
*	secNum	- In which sector the address is sitting.
*       data    - Data to be written.
*	pFlash	- flash information.
*
* OUTPUT:
*	None
*
* RETURN:
*	None
*
*******************************************************************************/
MV_VOID  flashCmdSet(MV_FLASH_INFO *pFlash, MV_U32 addr, MV_U32 secNum,
																MV_U32 data)
{
	if(NULL == pFlash)
		return;

	/* prepare the Data according to the Flash Width and Bus Width.			*/
	data = flashDataExt(pFlash, data);
	addr = flashAddrExt(pFlash, addr, secNum);

	flashBusWidthWr(pFlash,addr,data);

	return;
}

/*******************************************************************************
* flashBusWidthRd - read BusWidth Bits from address.
*
* DESCRIPTION:
*		read BusWidth Bits from address. Note that access to Flash registers
*		is always in LE	mode as the Flash registers are in LE mode.
*
* INPUT:
*       addr    - Address offset.
*	pFlash	- flash information.
*
* OUTPUT:
*	None
*
* RETURN:
*	MV_U32 - contain the Bus Width Bits read from the address.
*
*******************************************************************************/
MV_U32 flashBusWidthRd(MV_FLASH_INFO *pFlash, MV_U32 addr)
{
	MV_U32 val;

	switch(pFlash->busWidth)
    	{
		case 1:
			val = (MV_U32)MV_FL_8_READ(addr);
			break;
	        case 2:
			val = (MV_U32)MV_FL_16_READ(addr);
			break;
		case 4:
			val = MV_FL_32_READ(addr);
			break;
		default:
			mvOsPrintf("%s ERROR: Bus Width %d Bytes isn't supported.\n",
                        __FUNCTION__, pFlash->busWidth);
			return 0;
	}
	/* mvOsPrintf("Addr = 0x%x, val = 0x%x, width %d\n", addr, val, pFlash->busWidth); */

	return val;
}

/*******************************************************************************
* flashBusWidthWr - write BusWidth Bits from address.
*
* DESCRIPTION:
*		write BusWidth Bits to address. Note that access to Flash registers
*		is always in LE	mode as the Flash registers are in LE mode.
*
* INPUT:
*       addr    - Address offset.
*	data	- data to be written.
*	pFlash	- flash information.
*
* OUTPUT:
*	None
*
* RETURN:
*	None
*
*******************************************************************************/
MV_VOID flashBusWidthWr(MV_FLASH_INFO *pFlash, MV_U32 addr, MV_U32 data)
{
	/* mvOsPrintf("Addr = 0x%x, data = 0x%x, width %d\n", addr, data, pFlash->busWidth); */
	switch(pFlash->busWidth)
    	{
		case 1:
			MV_FL_8_WRITE(addr,(MV_U8)data);
			break;
	        case 2:
			MV_FL_16_WRITE(addr,(MV_U16)data);
			break;
		case 4:
			MV_FL_32_WRITE(addr,data);
			break;
		default:
			mvOsPrintf("%s ERROR: Bus Width %d Bytes isn't supported.\n",
                        __FUNCTION__, pFlash->busWidth);
			return;
	}

	return ;
}

/*******************************************************************************
* flashBusWidthDataWr - write BusWidth Bits of data from address.
*
* DESCRIPTION:
*		This function is used to perform data write to Flash, not like
*		flashBusWidthWr which is used for commands.
*
* INPUT:
*   addr    - Address offset.
*	data	- data to be written.
*	pFlash	- flash information.
*
* OUTPUT:
*	None
*
* RETURN:
*	None
*
*******************************************************************************/
MV_VOID flashBusWidthDataWr(MV_FLASH_INFO *pFlash, MV_U32 addr, MV_U32 data)
{
	/*mvOsPrintf("Addr = 0x%x, data = 0x%x, width %d\n", addr, data, pFlash->busWidth);*/
	switch(pFlash->busWidth)
    	{
		case 1:
			MV_FL_8_DATA_WRITE(addr,(MV_U8)data);
			break;
	        case 2:
			MV_FL_16_DATA_WRITE(addr,(MV_U16)data);
			break;
		case 4:
			MV_FL_32_DATA_WRITE(addr,data);
			break;
		default:
			mvOsPrintf("%s ERROR: Bus Width %d Bytes isn't supported.\n",
                        __FUNCTION__, pFlash->busWidth);
			return;
	}

	return ;
}


/*******************************************************************************
*  flashPrint - Print flash information structure.
*
* DESCRIPTION:
*	Prints all the feilds in the flash info structure.
*
* INPUT:
*       pFlash	- Flash information.
*
* OUTPUT:
*       None
*
* RETURN:
*	None
*
*******************************************************************************/
MV_VOID flashPrint(MV_FLASH_INFO *pFlash)
{
	MV_U32 i;


	if ((NULL == pFlash) || (mvFlashVenIdGet(pFlash) == 0))
	{
		mvOsOutput ("missing or unknown FLASH type\n");
		return;
	}

	switch (mvFlashVenIdGet(pFlash)) {
		case STM_MANUF:
			mvOsOutput ("STM ");
			break;
		case AMD_MANUF:
			mvOsOutput ("AMD ");
			break;
		case FUJ_MANUF:
			mvOsOutput ("FUJITSU ");
			break;
		case INTEL_MANUF:
			mvOsOutput ("INTEL ");
			break;
		case SST_MANUF:
			mvOsOutput ("SST ");
			break;
		case MX_MANUF:
			mvOsOutput ("MX ");
			break;
		default:
			mvOsOutput ("Unknown Vendor 0x%x",mvFlashVenIdGet(pFlash));
			break;
        }

	switch (mvFlashDevIdGet(pFlash)) {
	case AMD_FID_LV040B:
		mvOsOutput ("AM29LV040B (4 Mbit, bottom boot sect)");
		break;
	case AMD_FID_LV400B:
		mvOsOutput ("AM29LV400B (4 Mbit, bottom boot sect)");
		break;
	case AMD_FID_LV400T:
		mvOsOutput ("AM29LV400T (4 Mbit, top boot sector)");
		break;
	case AMD_FID_LV800B:
		mvOsOutput ("AM29LV800B (8 Mbit, bottom boot sect)");
		break;
	case AMD_FID_LV800T:
		mvOsOutput ("AM29LV800T (8 Mbit, top boot sector)");
		break;
	case AMD_FID_LV160B:
		mvOsOutput ("AM29LV160B (16 Mbit, bottom boot sect)");
		break;
	case AMD_FID_LV160T:
		mvOsOutput ("AM29LV160T (16 Mbit, top boot sector)");
		break;
	case AMD_FID_LV320B:
		mvOsOutput ("AM29LV320B (32 Mbit, bottom boot sect)");
		break;
	case AMD_FID_LV320T:
		mvOsOutput ("AM29LV320T (32 Mbit, top boot sector)");
		break;
	case AMD_S29GL128N:
		mvOsOutput ("SPANSION S29GL128N (128 Mbit) - AMD MirrorBit-compat");
		break;
	case  STM_FID_29W040B:
		mvOsOutput ("M29W040B (4Mbit = 512K x 8) ");
		break;
	case INTEL_FID_28F320J3A:
		mvOsOutput ("28F320J3A (32 Mbit)");
		break;
	case INTEL_FID_28F640J3A:
		mvOsOutput ("28F640J3A (64 Mbit)");
		break;
	case INTEL_FID_28F128J3A:
		mvOsOutput ("28F128J3A (128 Mbit)");
		break;
	case INTEL_FID_28F128P30T:
		mvOsOutput ("28F128P30 TOP (128 Mbit)");
		break;
	case INTEL_FID_28F128P30B:
		mvOsOutput ("28F128P30 BOTTOM (128 Mbit)");
		break;
	case INTEL_FID_28F256P30T:
		mvOsOutput ("28F256P30 TOP (256 Mbit)");
		break;
#if defined (DB_88F1281)
	case INTEL_FID_28F256P30B:
		mvOsOutput ("28F256P30 BOTTOM (128 Mbit)");
		break;
#else
	case INTEL_FID_28F256P30B:
		mvOsOutput ("28F256P30 BOTTOM (256 Mbit)");
		break;
#endif
	case SST_39VF_020:
		mvOsOutput ("SST39VF020 (2 Mbit)");
		break;
	default:
		mvOsOutput ("Unknown Chip Type id 0x%x",mvFlashDevIdGet(pFlash));
		break;
	}
	if(mvFlashNumOfDevGet(pFlash) > 1)
		mvOsOutput(" X %d",mvFlashNumOfDevGet(pFlash));

	mvOsOutput("\nSize: ");
	sizePrint(mvFlashSizeGet(pFlash),(MV_U8*)" in ");
	mvOsOutput("%d Sectors\n",mvFlashNumOfSecsGet(pFlash));
	mvOsOutput("Bus Width: %dbit, device Width: %dbit, type: ",
			   (8 * mvFlashBusWidthGet(pFlash)), (8 * mvFlashDevWidthGet(pFlash)));

	switch (mvFlashSecTypeGet(pFlash)) {
		case TOP:   	  	mvOsOutput ("TOP");		break;
		case BOTTOM:     	mvOsOutput ("BOTTOM");		break;
		case REGULAR:     	mvOsOutput ("REGULAR");		break;
		default:                mvOsOutput ("Unknown Type");	break;
        }
	mvOsOutput(".\n");


	mvOsOutput ("  Sector Start Addresses:");
	for (i=0; i<mvFlashNumOfSecsGet(pFlash); ++i) {
		if ((i % 5) == 0)
			mvOsOutput ("\n   ");
		mvOsOutput (" %08lX%s",
			(mvFlashSecOffsGet(pFlash, i) + mvFlashBaseAddrGet(pFlash)),
			mvFlashSecLockGet(pFlash,i) ? " (RO)" : "     "
		);
	}
	mvOsOutput("\n");

	return;
}

/***************************************************************************
* print sizes as "xxx kB", "xxx.y kB", "xxx MB" or "xxx.y MB" as needed;
* allow for optional trailing string (like "\n")
***************************************************************************/
static MV_VOID sizePrint (MV_U32 size, MV_U8 *s)
{
	MV_U32 m, n;
	MV_U32 d = 1 << 20;		/* 1 MB */
	MV_U8  c = 'M';

	if (size < d) {			/* print in kB */
		c = 'k';
		d = 1 << 10;
	}

	n = size / d;

	m = (10 * (size - (n * d)) + (d / 2) ) / d;

	if (m >= 10) {
		m -= 10;
		n += 1;
	}

	mvOsOutput ("%2d", n);
	if (m) {
		mvOsOutput (".%d", m);
	}
	mvOsOutput (" %cB%s", c, s);
}

