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
#include "mvOs.h"
#include "mvNflashHwIf.h"
/*#include "mvDevice.h" assaf */


/* Marvell Nand controller commands */
#define CLE_CMD	 (pNflashHwIf->devBaseAddr + (0x1 << (pNflashHwIf->devWidth>>4)))
#define ALE_CMD	 (pNflashHwIf->devBaseAddr + (0x2 << (pNflashHwIf->devWidth>>4)))
#define DATA_CMD (pNflashHwIf->devBaseAddr)


/*******************************************************************************
* mvNflashHwIfInit - Initialize the HW interface driver.
*
* DESCRIPTION:
*      None 
*
* INPUT:
*      None 
*
* OUTPUT:
*      None 
*
* RETURN:
*      None 
*
*******************************************************************************/
MV_VOID mvNflashHwIfInit(MV_NFLASH_HW_IF *pNflashHwIf)
{
	
    /* Set the data "read" routine if user did not set it. */
    if (NULL == pNflashHwIf->nfDataGetRtn) 
    {
        if (NFLASH_DEV_WIDTH_x8 == pNflashHwIf->devWidth)
        {
            pNflashHwIf->nfDataGetRtn = mvNflash8bitDataGet;
        }
        else
        {
            pNflashHwIf->nfDataGetRtn = mvNflash16bitDataGet;
        }
    }
    
    /* Set the data "write" routine if user did not set it. */
    if (NULL == pNflashHwIf->nfDataSetRtn) 
    {
        if (NFLASH_DEV_WIDTH_x8 == pNflashHwIf->devWidth)
        {
            pNflashHwIf->nfDataSetRtn = mvNflash8bitDataSet;
        }
        else
        {
            pNflashHwIf->nfDataSetRtn = mvNflash16bitDataSet;
        }
    }
}


/*******************************************************************************
* mvNflashCommandSet - Set command.
*
* DESCRIPTION:
*		Set a Flash command. Note that the Flash interface for 
*		commands and addresses is 8-bit wide.
*
* INPUT:
*		pNflashHwIf - Nand flash HW interface identifier.
*       command	    - Flash command.
*
* OUTPUT:
*      None 
*
* RETURN:
*      None 
*
*******************************************************************************/
MV_VOID mvNflashCommandSet(MV_NFLASH_HW_IF *pNflashHwIf, MV_U8 command)
{
	MV_MEMIO8_WRITE(CLE_CMD, command);
}


/*******************************************************************************
* mvNflashAddrSet - Set address.
*
* DESCRIPTION:
*		Set the address of a Flash command. Note that the Flash interface for 
*		commands and addresses is 8-bit wide. If the address is for example 
*		24 addresses (x8 device) or 23 addresses (x16 device) this function 
*		will be called three times.
*
* INPUT:
*		pNflashHwIf - Nand flash HW interface identifier.
*       addr        - address.
*
* OUTPUT:
*      None 
*
* RETURN:
*      None 
*
*******************************************************************************/
MV_VOID mvNflashAddrSet(MV_NFLASH_HW_IF *pNflashHwIf, MV_U8 addr)
{	
	MV_MEMIO8_WRITE(ALE_CMD, addr);
}


/*******************************************************************************
* mvNflash8bitDataSet - Set 8-bit data to be written.
*
* DESCRIPTION:
*		Set 8-bit data to be written on the HW bus. 
*
* INPUT:
*		pNflashHwIf - Nand flash HW interface identifier.
*       pSrc - Pointer to source data.
*       size - size of copied data in bytes (8-bits).
*
* OUTPUT:
*      None 
*
* RETURN:
*      MV_OK.
*
*******************************************************************************/
MV_STATUS mvNflash8bitDataSet (MV_NFLASH_HW_IF *pNflashHwIf, void *pSrc, MV_U32 size)
{
	int i;

    for (i = 0; i < size; i++)
    {
        MV_MEMIO8_WRITE(DATA_CMD, *(MV_U8*)pSrc++); 
    }
    
    return MV_OK;
}


/*******************************************************************************
* mvNflash16bitDataSet - Set 16-bit data to be written.
*
* DESCRIPTION:
*		Set 16-bit data to be written on the HW bus. 
*
* INPUT:
*		pNflashHwIf - Nand flash HW interface identifier.
*       pSrc - Pointer to source data.
*       size - size of copied data in words (16-bits).
*               
* OUTPUT:
*      None 
*
* RETURN:
*      MV_OK.
*
*******************************************************************************/
MV_STATUS mvNflash16bitDataSet(MV_NFLASH_HW_IF *pNflashHwIf, void *pSrc, MV_U32 size)
{
	int i;

    /* Note, the size is in bytes (8-bit) while reads are in words (16-bit) */
    for (i = 0; i < size; i++)
    {
        MV_MEMIO16_WRITE(DATA_CMD, *((MV_U16*)pSrc)++);
    }
    
    return MV_OK;
}


/*******************************************************************************
* mvNflash8bitDataGet - Get 8-bit data.
*
* DESCRIPTION:
*		Get 8-bit data from the HW bus. 
*
* INPUT:
*		pNflashHwIf - Nand flash HW interface identifier.
*       pDst - Where to copy the data.
*       size - size of copied data in bytes (8-bits).
*
* OUTPUT:
*      None 
*
* RETURN:
*      MV_OK.
*
* NOTE: 
*
*******************************************************************************/
MV_STATUS mvNflash8bitDataGet (MV_NFLASH_HW_IF *pNflashHwIf, void *pDst, MV_U32 size)
{
	int i;

    for (i = 0; i < size; i++)
    {
        *(MV_U8*)pDst++ = MV_MEMIO8_READ(DATA_CMD);
    }

    return MV_OK;
}

/*******************************************************************************
* mvNflash16bitDataGet - Get 16-bit data.
*
* DESCRIPTION:
*		Get 16-bit data from the HW bus. 
*
* INPUT:
*		pNflashHwIf - Nand flash HW interface identifier.
*       pDst - Where to copy the data.
*       size - size of copied data in words (16-bits).
*
* OUTPUT:
*      pData is filled with data from NFlash.
*
* RETURN:
*      MV_OK.
*
* NOTE: 
*
*******************************************************************************/
MV_STATUS mvNflash16bitDataGet(MV_NFLASH_HW_IF *pNflashHwIf, void *pDst, MV_U32 size)
{
	int i;

    /* Note, the size is in bytes (8-bit) while reads are in words (16-bit) */
    for (i = 0; i < size; i++)
    {
        *((MV_U16*)pDst)++ = MV_MEMIO16_READ(DATA_CMD);
    }
    
    return MV_OK;
}

