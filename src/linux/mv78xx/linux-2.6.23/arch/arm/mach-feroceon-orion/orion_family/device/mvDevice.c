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

#include "mvDevice.h"

/* defines  */       
#ifdef MV_DEBUG         
	#define DB(x)	x
#else                
	#define DB(x)    
#endif	             



/*******************************************************************************
* mvDevPramSet - Set device interface bank parameters
*
* DESCRIPTION:
*       This function sets a device bank parameters to a given device.
*
* INPUT:
*       device      - Device number. See MV_DEVICE enumerator.
*       *pDevParams - Device bank parameter struct.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvDevIfPramSet(MV_DEVICE device, MV_DEVICE_PARAM *pDevParams)
{
	MV_U32 devParam;

	/* check parameters */
	if (device >= MV_DEV_MAX_CS)
	{
		DB(mvOsPrintf("mvDevIfPramSet: ERR. Invalid Device num %d\n", device));
		return MV_BAD_PARAM;

	}
	if (pDevParams->turnOff  > MAX_DBP_TURNOFF)
	{
		DB(mvOsPrintf("mvDevIfPramSet: ERR. turnOff parameter out of range\n"));
		return MV_ERROR;
	}
	if (pDevParams->acc2First > MAX_DBP_ACC2FIRST)
	{
		DB(mvOsPrintf("mvDevIfPramSet: ERR. acc2First out of range\n"));
		return MV_ERROR;
	}
	if (pDevParams->acc2Next > MAX_DBP_ACC2NEXT)
	{
		DB(mvOsPrintf("mvDevIfPramSet: ERR. acc2Next out of range\n"));
		return MV_ERROR;
	}
	if (pDevParams->ale2Wr > MAX_DBP_ALE2WR)
	{
		DB(mvOsPrintf("mvDevIfPramSet: ERR. ale2Wr parameter out of range\n"));
		return MV_ERROR;
	}
	if (pDevParams->wrLow > MAX_DBP_WRLOW)
	{
		DB(mvOsPrintf("mvDevIfPramSet: ERR. wrLow parameter out of range\n"));
		return MV_ERROR;
	}
	if (pDevParams->wrHigh > MAX_DBP_WRHIGH)
	{
		DB(mvOsPrintf("mvDevIfPramSet: ERR. wrHigh parameter out of range\n"));
		return MV_ERROR;
	}
	if ((pDevParams->badrSkew << DBP_BADRSKEW_OFFS) > DBP_BADRSKEW_2CYCLE )
	{
		DB(mvOsPrintf("mvDevIfPramSet: ERR. badrSkew parameter out of range\n"));
		return MV_ERROR;
	}
	if ((pDevParams->deviceWidth != 8 )&&
		(pDevParams->deviceWidth != 16 )&&
		(pDevParams->deviceWidth != 32 ))
	{
		DB(mvOsPrintf("mvDevIfPramSet: ERR. deviceWidth out of range\n"));
		return MV_ERROR;
	}


		devParam = MV_REG_READ(DEV_BANK_PARAM_REG(device));


	/* setting values */
	devParam |= (pDevParams->turnOff << DBP_TURNOFF_OFFS);
	devParam |= (pDevParams->acc2First << DBP_ACC2FIRST_OFFS);
	devParam |= (pDevParams->acc2Next << DBP_ACC2NEXT_OFFS);
	devParam |= (pDevParams->ale2Wr << DBP_ALE2WR_OFFS);
	devParam |= (pDevParams->wrLow << DBP_WRLOW_OFFS);
	devParam |= (pDevParams->wrHigh << DBP_WRHIGH_OFFS);
	devParam |= (pDevParams->badrSkew << DBP_BADRSKEW_OFFS);
	
	
	switch (pDevParams->deviceWidth)
	{
	case 8:
		devParam |= DBP_DEVWIDTH_8BIT;
		break;
	case 16:
		devParam |= DBP_DEVWIDTH_16BIT;
		break;
	case 32:
		devParam |= DBP_DEVWIDTH_32BIT;
		break;
	default:
		DB(mvOsPrintf("mvDevIfPramSet: ERR. deviceWidth invalid\n"));
		return MV_ERROR;
		break;
	}
    
		MV_REG_WRITE(DEV_BANK_PARAM_REG(device),devParam);

	return MV_OK;
}

/*******************************************************************************
* mvDevPramget - Get device interface bank parameters
*
* DESCRIPTION:
*       This function retrieves a device bank parameter settings.
*
* INPUT:
*       device      - Device number. See MV_DEVICE enumerator.
*
* OUTPUT:
*       *pDevParams - Device bank parameter struct.
*
* RETURN:
*       MV_BAD_PARAM for bad parameters ,MV_ERROR on error ! otherwise MV_OK
*
*******************************************************************************/
MV_STATUS mvDevPramGet(MV_DEVICE device, MV_DEVICE_PARAM *pDevParams)
{
	MV_U32 devParam;

	/* check parameters */
	if (device >= MV_DEV_MAX_CS)
	{
		DB(mvOsPrintf("mvDevIfPramSet: ERR. Invalid Device num %d\n", device));
		return MV_BAD_PARAM;

	}
	
		devParam = MV_REG_READ(DEV_BANK_PARAM_REG(device));

	pDevParams->turnOff = (devParam & DBP_TURNOFF_MASK) >> DBP_TURNOFF_OFFS;
	pDevParams->acc2First = (devParam & DBP_ACC2FIRST_MASK)>>DBP_ACC2FIRST_OFFS;
	pDevParams->acc2Next = (devParam & DBP_ACC2NEXT_MASK) >> DBP_ACC2NEXT_OFFS;
	pDevParams->ale2Wr = (devParam & DBP_ALE2WR_MASK) >> DBP_ALE2WR_OFFS;
	pDevParams->wrLow = (devParam & DBP_WRLOW_MASK) >> DBP_WRLOW_OFFS;
	pDevParams->wrHigh = (devParam & DBP_WRHIGH_MASK) >> DBP_WRHIGH_OFFS;
	pDevParams->badrSkew = (devParam & DBP_BADRSKEW_MASK) >> DBP_BADRSKEW_OFFS;



	switch (devParam & DBP_DEVWIDTH_MASK)
	{
	case DBP_DEVWIDTH_8BIT:
		pDevParams->deviceWidth=8;
		break;
	case DBP_DEVWIDTH_16BIT:
		pDevParams->deviceWidth=16;
		break;
	case  DBP_DEVWIDTH_32BIT:
		pDevParams->deviceWidth=32;
		break;
	default:
		DB(mvOsPrintf("mvDevIfPramSet: ERR. invalid deviceWidth\n"));
		return MV_ERROR;
		break;
	}


	return MV_OK;
}

/*******************************************************************************
* mvDevWidthGet - Get device width parameter
*
* DESCRIPTION:
*       This function gets width parameter of a given device.
*
* INPUT:
*       device - Device number. See MV_DEVICE enumerator.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Device width in bytes.
*
*******************************************************************************/
MV_U32 mvDevWidthGet(MV_DEVICE device)
{

	MV_U32 devParam;

	/* check parameters */
	if (device >= MV_DEV_MAX_CS)
	{
		DB(mvOsPrintf("mvDevIfPramSet: ERR. Invalid Device num %d\n", device));
		return MV_BAD_PARAM;

	}

		devParam = MV_REG_READ(DEV_BANK_PARAM_REG(device));
	
	devParam = (devParam & DBP_DEVWIDTH_MASK) >> DBP_DEVWIDTH_OFFS;

	return (MV_U32)(0x8 << devParam);

}

/*******************************************************************************
* mvDevNandSet - Set NAND chip-select and care mode
*
* DESCRIPTION:
*       This function set the NAND flash controller registers with NAND 
*       device chip-select.
*
* INPUT:
*       devNum   - Device number. See MV_DEVICE enumerator.
*       careMode - NAND device care mode (0 = Don't care, '1' = care).
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvDevNandSet(MV_DEVICE devNum, MV_BOOL careMode)
{
    MV_U32 nfCtrlReg;   /* NAND Flash Control Register */
    
    /* Set chip select */
    nfCtrlReg = MV_REG_READ(DEV_NAND_CTRL_REG);

    nfCtrlReg |= (DINFCR_NF_CS_MASK(devNum));

    if (careMode)
    	nfCtrlReg |= (DINFCR_NF_ACT_CE_MASK(devNum));
    else
    	nfCtrlReg &= ~(DINFCR_NF_ACT_CE_MASK(devNum));
	
    
    MV_REG_WRITE(DEV_NAND_CTRL_REG, nfCtrlReg);

}


