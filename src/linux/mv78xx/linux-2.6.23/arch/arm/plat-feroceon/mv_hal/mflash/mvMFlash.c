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
#include "mflash/mvMFlash.h"
#include "mflash/mvPMFlash.h"
#include "mflash/mvSMFlash.h" 
#include "mflash/mvMFlashSpec.h"
#include "mflash/mvPMFlashSpec.h"
#include "mflash/mvSMFlashSpec.h"

/*#define MV_DEBUG*/
#ifdef MV_DEBUG
#define DB(x) x
#else
#define DB(x)
#endif

/* Static Functions */
static MV_STATUS mvMFlashBlockByRegionWr(MV_MFLASH_INFO *pFlash, MV_U32 offset, \
                                         MV_U32 blockSize, MV_U8 *pBlock, MV_BOOL verify, \
                                         MV_BOOL main);
static MV_STATUS mvMFlash64bInfWr       (MV_MFLASH_INFO *pFlash, MV_U32 offset, \
                                         MV_U8 *pBuff, MV_BOOL verify);
static MV_STATUS mvMFlash64bWr          (MV_MFLASH_INFO *pFlash, MV_U32 offset, \
                                         MV_U8 *pBuff, MV_BOOL verify);

/*******************************************************************************
* mvMFlashInit - Initialize a Marvell Flash device
*
* DESCRIPTION:
*       This function performs the necessary initialization for a Marvell 
*       Sunol flash.
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*           pFlash->baseAddr: flash base address.
*           pFlash->ifMode: The mode used SPI or Parallel.
*       
* OUTPUT:
*       pFlash: pointer to the tructure with Marvell Flash information detected
*           pFlash->flashModel: flash model deteced
*           pFlash->sectorSize: Size of each sector (unified sector size)
*           pFlash->sectorNumber: Number of sectors in the flash
*           pFlash->infoSize: Size of the Information region.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvMFlashInit (MV_MFLASH_INFO *pFlash)
{
    MV_U32  manufId;
    MV_U16  devId;
    MV_U8  reg;
    MV_STATUS ret;

    /* check for NULL pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

    if (pFlash->ifMode == MV_MFLASH_PARALLEL)
    {
        /* perform the basic initialization */
        if ((ret = mvPMFlashInit(pFlash)) != MV_OK)
            return ret;

        /* Try to read the device ID and decide the device model */
        if ((ret = mvPMFlashIdGet(pFlash, &manufId, &devId)) != MV_OK)
            return ret;

        /* Get the Serial interface configuration register to check sector size */
        if ((ret = mvPMFlashReadConfig4(pFlash, &reg)) != MV_OK)
            return ret;
    }
    else if (pFlash->ifMode == MV_MFLASH_SPI)
    {   
        /* perform the basic initialization */
        if ((ret = mvSMFlashInit(pFlash)) != MV_OK)
            return ret;

        /* Try to read the device ID and decide the device model */
        if ((ret = mvSMFlashIdGet(pFlash, &manufId, &devId)) != MV_OK)
            return ret;

        /* Get the Serial interface configuration register to check sector size */
        if ((ret = mvSMFlashReadConfig4(pFlash, &reg)) != MV_OK)
            return ret;
    }
    else
    {
        mvOsPrintf("%s ERROR: Invalid interface mode!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

    /* check the manufacturer Id to be the same as MARVELL */
    if (manufId != MV_MFLASH_MANUFACTURER_ID)
    {
        mvOsPrintf("%s ERROR: Flash unknown manufacturer ID!\n", __FUNCTION__);
        return MV_NOT_SUPPORTED;
    }

    /* based on device ID and the PAGE4K bit fill model, sector size and number */
    switch (devId & MV_MFLASH_DEVICE_ID_MASK)
    {
        case MV_MFLASH_DEVICE_ID_SUNOL_1:
            pFlash->flashModel = MV_MFLASH_SUNOL_1;
            pFlash->infoSize = MV_MFLASH_INF_MEM_SZ_SUNOL_1;
            if (reg & MV_SMFLASH_SRL_CFG4_PG_SIZE_MASK)  /* 1 - page is 4K */
            {
                pFlash->sectorSize = MV_MFLASH_SECTOR_SIZE_SMALL;
                pFlash->sectorNumber = MV_MFLASH_SMALL_SEC_NUM_SUNOL_1;
            }
            else    /* 0 - page is 32K */
            {
                pFlash->sectorSize = MV_MFLASH_SECTOR_SIZE_BIG;
                pFlash->sectorNumber = MV_MFLASH_BIG_SEC_NUM_SUNOL_1;
            }
            break;

        case MV_MFLASH_DEVICE_ID_SUNOL_2:
            pFlash->flashModel = MV_MFLASH_SUNOL_2;
            pFlash->infoSize = MV_MFLASH_INF_MEM_SZ_SUNOL_2_3;
            if (reg & MV_SMFLASH_SRL_CFG4_PG_SIZE_MASK)  /* 1 - page is 4K */
            {
                pFlash->sectorSize = MV_MFLASH_SECTOR_SIZE_SMALL;
                pFlash->sectorNumber = MV_MFLASH_SMALL_SEC_NUM_SUNOL_2;
            }
            else    /* 0 - page is 32K */
            {
                pFlash->sectorSize = MV_MFLASH_SECTOR_SIZE_BIG;
                pFlash->sectorNumber = MV_MFLASH_BIG_SEC_NUM_SUNOL_2;
            }
            break;

        case MV_MFLASH_DEVICE_ID_SUNOL_3:
            pFlash->flashModel = MV_MFLASH_SUNOL_3;
            pFlash->infoSize = MV_MFLASH_INF_MEM_SZ_SUNOL_2_3;
            if (reg & MV_SMFLASH_SRL_CFG4_PG_SIZE_MASK)  /* 1 - page is 4K */
            {
                pFlash->sectorSize = MV_MFLASH_SECTOR_SIZE_SMALL;
                pFlash->sectorNumber = MV_MFLASH_SMALL_SEC_NUM_SUNOL_3;
            }
            else    /* 0 - page is 32K */
            {
                pFlash->sectorSize = MV_MFLASH_SECTOR_SIZE_BIG;
                pFlash->sectorNumber = MV_MFLASH_BIG_SEC_NUM_SUNOL_3;
            }
            break;

        default:
            mvOsPrintf("%s ERROR: Unknown Flash Device ID!\n", __FUNCTION__);
            pFlash->flashModel = MV_MFLASH_MODEL_UNKNOWN;
            return MV_NOT_SUPPORTED;
    }

	return MV_OK;
}


/*******************************************************************************
* mvMFlashChipErase - Erase the whole flash
*
* DESCRIPTION:
*       Erase the whole flash (both the Main and Information region)
*
* INPUT:
*       pFlash: pointer to the MFlash information structure
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvMFlashChipErase (MV_MFLASH_INFO *pFlash)
{
    /* check for NULL pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	if (pFlash->ifMode == MV_MFLASH_PARALLEL)
        return mvPMFlashChipErase(pFlash);
    else if (pFlash->ifMode == MV_MFLASH_SPI)
        return mvSMFlashChipErase(pFlash);

    mvOsPrintf("%s ERROR: Invalid interface mode!\n", __FUNCTION__);
    return MV_BAD_PARAM;
}   

/*******************************************************************************
* mvMFlashMainErase - Erase the main flash region only
*
* DESCRIPTION:
*       Erase the Main flash region
*
* INPUT:
*       pFlash: pointer to the MFlash information structure
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvMFlashMainErase (MV_MFLASH_INFO *pFlash)
{
    /* check for NULL pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	if (pFlash->ifMode == MV_MFLASH_PARALLEL)
        return mvPMFlashMainErase(pFlash);
    else if (pFlash->ifMode == MV_MFLASH_SPI)
        return mvSMFlashMainErase(pFlash);

    mvOsPrintf("%s ERROR: Invalid interface mode!\n", __FUNCTION__);
    return MV_BAD_PARAM;
}

/*******************************************************************************
* mvMFlashInfErase - Erase the information flash region only
*
* DESCRIPTION:
*       Erase the information flash region
*
* INPUT:
*       pFlash: pointer to the MFlash information structure
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvMFlashInfErase (MV_MFLASH_INFO *pFlash)
{
    /* check for NULL pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	if (pFlash->ifMode == MV_MFLASH_PARALLEL)
        return mvPMFlashInfErase(pFlash);
    else if (pFlash->ifMode == MV_MFLASH_SPI)
        return mvSMFlashInfErase(pFlash);

    mvOsPrintf("%s ERROR: Invalid interface mode!\n", __FUNCTION__);
    return MV_BAD_PARAM;
}


/*******************************************************************************
* mvMFlashSecErase - Erase the single sector of the main flash region 
*
* DESCRIPTION:
*       Erase one sector of the main flash region
*
* INPUT:
*       pFlash: pointer to the MFlash information structure
*		secOffset: sector offset within the MFlash
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvMFlashSecErase (MV_MFLASH_INFO *pFlash, MV_U32 secNumber)
{
    MV_U32 i;
    MV_U32 * pW = (MV_U32*) ((secNumber * pFlash->sectorSize) + pFlash->baseAddr);
    MV_U32 erasedWord = 0xFFFFFFFF;
    MV_U32 wordsPerSector = (pFlash->sectorSize / sizeof(MV_U32));
    MV_BOOL eraseNeeded = MV_FALSE;

    /* check for NULL pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

    /* First compare to FF and check if erase is needed */
    for (i=0; i<wordsPerSector; i++)
    {
        if (memcmp(pW, &erasedWord, sizeof(MV_U32)) != 0)
        {
            eraseNeeded = MV_TRUE;
            break;
        }

        ++pW;
    }

    if (!eraseNeeded)
        return MV_OK;

	if (pFlash->ifMode == MV_MFLASH_PARALLEL)
        return mvPMFlashSecErase(pFlash, secNumber);
    else if (pFlash->ifMode == MV_MFLASH_SPI)
        return mvSMFlashSecErase(pFlash, secNumber);

    mvOsPrintf("%s ERROR: Invalid interface mode!\n", __FUNCTION__);
    return MV_BAD_PARAM;
}

/*******************************************************************************
* mvMFlash64bWr - Perform the alligned write according to the SPI mode and verify
*                 type.
*
* DESCRIPTION:
*       Select the appropriate write api according to the SPI mode and the 
*       verify flag.
*
*******************************************************************************/
static MV_STATUS mvMFlash64bWr (MV_MFLASH_INFO *pFlash, MV_U32 offset, 
                                          MV_U8 *pBuff, MV_BOOL verify)
{
    MV_STATUS ret; 

    /* check for NULL pointer */
#ifndef CONFIG_MARVELL
    if(NULL == pBuff)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }
#endif

    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

    /* call the api based on the SPI mode and the verify option */
    if (pFlash->ifMode == MV_MFLASH_PARALLEL)
    {        
	    if (verify)
	    {          
		    if ((ret = mvPMFlash64bWrVerify(pFlash, offset, pBuff)) != MV_OK)
			    return ret;
	    }
	    else
	    {
		    if ((ret = mvPMFlash64bWr(pFlash, offset, pBuff)) != MV_OK)
			    return ret;
	    }
    }      
    else if (pFlash->ifMode == MV_MFLASH_SPI)
    {
        if (verify)
	    {
		    if ((ret = mvSMFlash64bWrVerify(pFlash, offset, pBuff)) != MV_OK)
			    return ret;
	    }
	    else
	    {
		    if ((ret = mvSMFlash64bWr(pFlash, offset, pBuff)) != MV_OK)
			    return ret;
	    }
    }
    else
    {
        mvOsPrintf("%s ERROR: Invalid interface mode!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

    return MV_OK;
}

/*******************************************************************************
* mvMFlash64bInfWr - Perform the alligned write to the information region based 
*                    on the SPI mode and verify type.
*
* DESCRIPTION:
*       Select the appropriate write api according to the SPI mode and the 
*       verify flag.
*
*******************************************************************************/
static MV_STATUS mvMFlash64bInfWr (MV_MFLASH_INFO *pFlash, MV_U32 offset, 
                                           MV_U8 *pBuff, MV_BOOL verify)
{
    MV_STATUS ret; 

    /* check for NULL pointer */
#ifndef CONFIG_MARVELL
    if(NULL == pBuff)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }
#endif

    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

    /* call the api based on the SPI mode and the verify option */
    if (pFlash->ifMode == MV_MFLASH_PARALLEL)
    {
	    if (verify)
	    {          
		    if ((ret = mvPMFlash64bInfWrVerify(pFlash, offset, pBuff)) != MV_OK)
			    return ret;
	    }
	    else
	    {
		    if ((ret = mvPMFlash64bInfWr(pFlash, offset, pBuff)) != MV_OK)
			    return ret;
	    }
    }      
    else if (pFlash->ifMode == MV_MFLASH_SPI)
    {
        if (verify)
	    {
		    if ((ret = mvSMFlash64bInfWrVerify(pFlash, offset, pBuff)) != MV_OK)
			    return ret;
	 
	    }
	    else
	    {
		    if ((ret = mvSMFlash64bInfWr(pFlash, offset, pBuff)) != MV_OK)
			    return ret;
	    }
    }
    else
    {
        mvOsPrintf("%s ERROR: Invalid interface mode!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

    return MV_OK;
}



/*******************************************************************************
* mvMFlashBlockByRegionWr - Write a buffer with a custom length and allignment
*                           based on the region (main or information)
*
* DESCRIPTION:
*       Program a varient sized block in the selected region of the MFlash
*       and perform the verify based on flag.
*
*******************************************************************************/
static MV_STATUS mvMFlashBlockByRegionWr(MV_MFLASH_INFO *pFlash, MV_U32 offset, 
                          MV_U32 blockSize, MV_U8 *pBlock, MV_BOOL verify, MV_BOOL main)
{
    MV_U8 tempBuff[MV_MFLASH_PROG_CHUNK_SIZE];
	MV_U32 data2write	= blockSize;
	MV_U32 preAllOfst	= (offset & MV_MFLASH_PROG_ALIGN_MASK);
    MV_U32 preAllSz		= (preAllOfst ? (MV_MFLASH_PROG_CHUNK_SIZE - preAllOfst) : 0);	
	MV_U32 writeOffset	= (offset & ~MV_MFLASH_PROG_ALIGN_MASK);
    MV_STATUS ret;

    /* check for NULL pointer */
#ifndef CONFIG_MARVELL
    if(NULL == pBlock)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }
#endif

    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	/* verify that the write commands does not exceed the size */
    if ((offset + blockSize) >  (pFlash->sectorNumber * pFlash->sectorSize))
    {
        DB(mvOsPrintf("%s WARNING: Exceeded flash size!!\n", __FUNCTION__);)
		return MV_BAD_PARAM;
    }
	
	/* check if the total block size is less than the first chunk remainder */
	if (data2write < preAllSz)
		preAllSz = data2write;

	/* check if programing does not start at a 64byte alligned offset */
	if (preAllSz)
	{
        /* first copy the original data */
        if (main)
        {
            if ((ret = mvMFlashBlockRd(pFlash, writeOffset, MV_MFLASH_PROG_CHUNK_SIZE, tempBuff)) != MV_OK)
                return ret;
        }
        else
        {
            if ((ret = mvMFlashBlockInfRd(pFlash, writeOffset, MV_MFLASH_PROG_CHUNK_SIZE, tempBuff)) != MV_OK)
                return ret;
        }

		/* overwrite the data to be changed */
		mvOsMemcpy((MV_VOID*)&tempBuff[preAllOfst], pBlock, preAllSz);

        /* Perform the 64 bytes write based on the mode and verify type */
        if (main)
        {
            if ((ret = mvMFlash64bWr(pFlash, writeOffset, tempBuff, verify)) != MV_OK)
                return ret;
        }
        else
        {
            if ((ret = mvMFlash64bInfWr(pFlash, writeOffset, tempBuff, verify)) != MV_OK)
                return ret;
        }
		
		/* increment pointers and counters */
		writeOffset += MV_MFLASH_PROG_CHUNK_SIZE;
		data2write -= preAllSz;
		pBlock += preAllSz;
	}

	/* program the data that fits in complete 64bytes chunks */
	while (data2write >= MV_MFLASH_PROG_CHUNK_SIZE)
	{
        /* Perform the 64 bytes write based on the mode and verify type */
        if (main)
        {
            if ((ret = mvMFlash64bWr(pFlash, writeOffset, pBlock, verify)) != MV_OK)
                return ret;
        }
        else
        {
            if ((ret = mvMFlash64bInfWr(pFlash, writeOffset, pBlock, verify)) != MV_OK)
                return ret;
        }

		/* increment pointers and counters */
		writeOffset += MV_MFLASH_PROG_CHUNK_SIZE;
		data2write -= MV_MFLASH_PROG_CHUNK_SIZE;
		pBlock += MV_MFLASH_PROG_CHUNK_SIZE;
	}

	/* program the last partial chunk */
	if (data2write)
	{
		/* first copy the original data */
        if (main)
        {
            if ((ret = mvMFlashBlockRd(pFlash, writeOffset, MV_MFLASH_PROG_CHUNK_SIZE, tempBuff)) != MV_OK)
                return ret;
        }
        else
        {
            if ((ret = mvMFlashBlockInfRd(pFlash, writeOffset, MV_MFLASH_PROG_CHUNK_SIZE, tempBuff)) != MV_OK)
                return ret;
        }

		/* overwrite the data to be changed */
		mvOsMemcpy(tempBuff, pBlock, data2write);

        /* Perform the 64 bytes write based on the mode and verify type */
        if (main)
        {
            if ((ret = mvMFlash64bWr(pFlash, writeOffset, tempBuff, verify)) != MV_OK)
                return ret;
        }
        else
        {
            if ((ret = mvMFlash64bInfWr(pFlash, writeOffset, tempBuff, verify)) != MV_OK)
                return ret;
        }
	}

    return MV_OK;
}

/*******************************************************************************
* mvMFlashBlockWr - Write a buffer with a custom length and allignment
*
* DESCRIPTION:
*       Program a varient sized block in the Main region of the MFlash.
*
* INPUT:
*       pFlash: pointer to the MFlash information structure
*		offset: offset within the Information region (limited to 1024)
*		blockSize: size in bytes of the buffer to be programmed.
*		pBlock: pointer to the 64 bytes buffer to be programed
*		verify: bollean flag controlling the compare after program feature
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvMFlashBlockWr(MV_MFLASH_INFO *pFlash, MV_U32 offset, 
                          MV_U32 blockSize, MV_U8 *pBlock, MV_BOOL verify)
{
    /* check for NULL pointers */
#ifndef CONFIG_MARVELL
    if(NULL == pBlock)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }
#endif

    if (NULL == pFlash)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

    return mvMFlashBlockByRegionWr(pFlash, offset, blockSize, pBlock, verify, MV_TRUE /* main region */);
}


/*******************************************************************************
* mvMFlashInfBlockWr - Write a buffer with a custom length and allignment to 
*                      the information region of the flash
*
* DESCRIPTION:
*       Program a varient sized block in the Information region of the MFlash.
*
* INPUT:
*       pFlash: pointer to the MFlash information structure
*		offset: offset within the Information region (limited to 1024)
*		blockSize: size in bytes of the buffer to be programmed.
*		pBlock: pointer to the 64 bytes buffer to be programed
*		verify: bollean flag controlling the compare after program feature
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvMFlashInfBlockWr(MV_MFLASH_INFO *pFlash, MV_U32 offset, 
                             MV_U32 blockSize, MV_U8 *pBlock, MV_BOOL verify)
{
    /* check for NULL pointers */
    if ((pFlash == NULL) || (pBlock == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

    return mvMFlashBlockByRegionWr(pFlash, offset, blockSize, pBlock, verify, MV_FALSE /* info region */);
}

/*******************************************************************************
* mvMFlashBlockRd - Read a block of Memory from the Main Flash 
*
* DESCRIPTION:
*       Read a block of Memory from the Main Flash region
*
* INPUT:
*       pFlash: pointer to the MFlash information structure
*		offset: offset to read from the main region of the MFlash
*		blockSize: number of bytes to read from the offset
*		pBlock: pointer of the buffer to fill
*
* OUTPUT:
*		pBlock: pointer of the buffer holding the data read from flash.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvMFlashBlockRd (MV_MFLASH_INFO *pFlash, MV_U32 offset, 
                           MV_U32 blockSize, MV_U8 *pBlock)
{
    /* check for NULL pointers */
    if ((pFlash == NULL) || (pBlock == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

    if (pFlash->ifMode == MV_MFLASH_PARALLEL)
        return mvPMFlashBlockRd(pFlash, offset, blockSize, pBlock);
    else if (pFlash->ifMode == MV_MFLASH_SPI)
        return mvSMFlashBlockRd(pFlash, offset, blockSize, pBlock);

    mvOsPrintf("%s ERROR: Invalid interface mode!\n", __FUNCTION__);
    return MV_BAD_PARAM;
}

/*******************************************************************************
* mvMFlashSecErase - Erase the single sector of the main flash region 
*
* DESCRIPTION:
*       Erase one sector of the main flash region
*
* INPUT:
*       pFlash: pointer to the MFlash information structure
*		secOffset: sector offset within the MFlash
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvMFlashBlockInfRd (MV_MFLASH_INFO *pFlash, MV_U32 offset, 
                              MV_U32 blockSize, MV_U8 *pBlock)
{
    /* check for NULL pointers */
    if ((pFlash == NULL) || (pBlock == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

    if (pFlash->ifMode == MV_MFLASH_PARALLEL)
        return mvPMFlashBlockInfRd(pFlash, offset, blockSize, pBlock);
    else if (pFlash->ifMode == MV_MFLASH_SPI)
        return mvSMFlashBlockInfRd(pFlash, offset, blockSize, pBlock);

    mvOsPrintf("%s ERROR: Invalid interface mode!\n", __FUNCTION__);
    return MV_BAD_PARAM;
}

/*******************************************************************************
* mvMFlashReadConfig - Read the Configuration register
*
* DESCRIPTION:
*       Perform the Read CONFIGx register RAB
*
* INPUT:
*       pFlash: pointer to the MFlash information structure
*       regNum: register number to read (1-4 in parallel and 3-4 in spi)
*		pConfigReg: pointer to read in the configuration register
*
* OUTPUT:
*		pConfigReg: pointer with the data read from the configuration register
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvMFlashReadConfig (MV_MFLASH_INFO *pFlash, MV_U32 regNum, MV_U8 * pConfigReg)
{
    /* check for NULL pointer */
    if ((pFlash == NULL) || (pConfigReg == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

    if (pFlash->ifMode == MV_MFLASH_PARALLEL)
    {
        switch (regNum)
        {
            case 1:
                return mvPMFlashReadConfig1(pFlash, pConfigReg);

            case 2:
                return mvPMFlashReadConfig2(pFlash, pConfigReg);

            case 3:
                return mvPMFlashReadConfig3(pFlash, pConfigReg);

            case 4:
                return mvPMFlashReadConfig4(pFlash, pConfigReg);

            default:
                DB(mvOsPrintf("%s WARNING: Not supported in parallel mode!!\n", __FUNCTION__);)
                return MV_NOT_SUPPORTED;
        }
    }
    else if (pFlash->ifMode == MV_MFLASH_SPI)
    {
        switch (regNum)
        {
            case 3:
                return mvSMFlashReadConfig3(pFlash, pConfigReg);

            case 4:
                return mvSMFlashReadConfig4(pFlash, pConfigReg);

            default:
                DB(mvOsPrintf("%s WARNING: Not supported in SPI mode!!\n", __FUNCTION__);)
                return MV_NOT_SUPPORTED;
        }
    }

    mvOsPrintf("%s ERROR: Invalid interface mode!\n", __FUNCTION__);
    return MV_BAD_PARAM;
}

/*******************************************************************************
* mvMFlashSetConfig - Write to the Configuration register
*
* DESCRIPTION:
*       Perform the write CONFIGx register RAB
*
* INPUT:
*       pFlash: pointer to the MFlash information structure
*       regNum: register number to write (1-4 in parallel and 3-4 in spi)
*		configReg: Data to write in the configuration register
*
* OUTPUT:
*		None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvMFlashSetConfig	(MV_MFLASH_INFO *pFlash, MV_U32 regNum, MV_U8 configReg)
{
    /* check for NULL pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

    if (pFlash->ifMode == MV_MFLASH_PARALLEL)
    {
        switch (regNum)
        {
            case 1:
                return mvPMFlashSetConfig1(pFlash, configReg);

            case 2:
                return mvPMFlashSetConfig2(pFlash, configReg);

            case 3:
                return mvPMFlashSetConfig3(pFlash, configReg);

            case 4:
                return mvPMFlashSetConfig4(pFlash, configReg);

            default:
                DB(mvOsPrintf("%s WARNING: Not supported in parallel mode!!\n", __FUNCTION__);)
                return MV_NOT_SUPPORTED;
        }
    }
    else if (pFlash->ifMode == MV_MFLASH_SPI)
    {
        switch (regNum)
        {
            case 3:
                return mvSMFlashSetConfig3(pFlash, configReg);

            case 4:
                return mvSMFlashSetConfig4(pFlash, configReg);

            default:
                DB(mvOsPrintf("%s WARNING: Not supported in SPI mode!!\n", __FUNCTION__);)
                return MV_NOT_SUPPORTED;
        }
    }

    mvOsPrintf("%s ERROR: Invalid interface mode!\n", __FUNCTION__);
    return MV_BAD_PARAM;
}

/*******************************************************************************
* mvMFlashSetSlewRate - Set the slew rate for parallel interface
*
* DESCRIPTION:
*       Perform the set slew rate register RAB
*
* INPUT:
*       pFlash: pointer to the MFlash information structure
*		configReg: Data to write in the slew rate register
*
* OUTPUT:
*		None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvMFlashSetSlewRate (MV_MFLASH_INFO *pFlash, MV_U8 configReg)
{
    /* check for NULL pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

    if (pFlash->ifMode == MV_MFLASH_PARALLEL)
        return mvPMFlashSetSlewRate(pFlash, configReg);

    DB(mvOsPrintf("%s WARNING: Invalid interface mode!\n", __FUNCTION__);)
    return MV_BAD_PARAM;
}   

/*******************************************************************************
* mvMFlashWriteProtectSet - Set the write protection feature status
*
* DESCRIPTION:
*       Enable or disable the write protection
*
* INPUT:
*       pFlash: pointer to the MFlash information structure
*		wp: write protection status (enable = MV_TRUE, disable = MVFALSE)
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvMFlashWriteProtectSet (MV_MFLASH_INFO *pFlash, MV_BOOL wp)
{
    MV_U32 reg;

    /* check for NULL pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }
    
    /* read, modify then write the register */
    reg = MV_REG_READ(MV_PMFLASH_IF_CFG_REG);
    if (wp) /* active low */
        reg &= ~MV_PMFLASH_WP_PROTECT_MASK;
    else
        reg |= MV_PMFLASH_WP_PROTECT_MASK;

    MV_REG_WRITE(MV_PMFLASH_IF_CFG_REG, reg);

    return MV_OK;

#if 0
	if (pFlash->ifMode == MV_MFLASH_PARALLEL)
        return mvPMFlashWriteProtectSet(pFlash, wp);
    else if (pFlash->ifMode == MV_MFLASH_SPI)
        return mvSMFlashWriteProtectSet(pFlash, wp);

    mvOsPrintf("%s ERROR: Invalid interface mode!\n", __FUNCTION__);
    return MV_BAD_PARAM;
#endif
}

/*******************************************************************************
* mvMFlashWriteProtectGet - Get the write protection feature status
*
* DESCRIPTION:
*       Retreive the write protection status from the hardware (from the MFlash
*               controller and not from the flash device configuration register)
*
* INPUT:
*       pFlash: pointer to the MFlash information structure
*		wp: pointer to the variable to retreive the write protection status in
*
* OUTPUT:
*       None.
*		wp: pointer to the variable holding the write protection status 
*           (enable = MV_TRUE, disable = MVFALSE)
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvMFlashWriteProtectGet(MV_MFLASH_INFO *pFlash, MV_BOOL * pWp)
{
    MV_U32 reg;

    /* check for NULL pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }
    
    /* read the register and check the bit status */
    reg = MV_REG_READ(MV_PMFLASH_IF_CFG_REG);
    if (reg & MV_PMFLASH_WP_PROTECT_MASK)
        *pWp = MV_FALSE;
    else
        *pWp = MV_TRUE;

    return MV_OK;
}

/*******************************************************************************
* mvMFlashSectorSizeSet - Set the sector size
*
* DESCRIPTION:
*       Set the sector size of the MFLASH main region
*
* INPUT:
*       pFlash: pointer to the MFlash information structure
*		secSize: size of sector in bytes (either 4K or 32K)
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvMFlashSectorSizeSet (MV_MFLASH_INFO *pFlash, MV_U32 secSize)
{
	MV_STATUS ret;
    /* check for NULL pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	if (pFlash->ifMode == MV_MFLASH_PARALLEL)
        ret = mvPMFlashSectorSizeSet(pFlash, secSize);
    else if (pFlash->ifMode == MV_MFLASH_SPI)
        ret = mvSMFlashSectorSizeSet(pFlash, secSize);
    else
    {
	    mvOsPrintf("%s ERROR: Invalid interface mode!\n", __FUNCTION__);
    	return MV_BAD_PARAM;
    }
    
    if (ret != MV_OK)
    	return ret;
    	
    /* update the sector size and number in the structure */
    switch (pFlash->flashModel)
    {
        case MV_MFLASH_SUNOL_1:
            if (secSize ==  MV_MFLASH_SECTOR_SIZE_SMALL) 
            {
                pFlash->sectorSize = MV_MFLASH_SECTOR_SIZE_SMALL;
                pFlash->sectorNumber = MV_MFLASH_SMALL_SEC_NUM_SUNOL_1;
            }
            else 
            {
                pFlash->sectorSize = MV_MFLASH_SECTOR_SIZE_BIG;
                pFlash->sectorNumber = MV_MFLASH_BIG_SEC_NUM_SUNOL_1;
            }
            break;

        case MV_MFLASH_SUNOL_2:
            if (secSize ==  MV_MFLASH_SECTOR_SIZE_SMALL) 
            {
                pFlash->sectorSize = MV_MFLASH_SECTOR_SIZE_SMALL;
                pFlash->sectorNumber = MV_MFLASH_SMALL_SEC_NUM_SUNOL_2;
            }
            else 
            {
                pFlash->sectorSize = MV_MFLASH_SECTOR_SIZE_BIG;
                pFlash->sectorNumber = MV_MFLASH_BIG_SEC_NUM_SUNOL_2;
            }
            break;

        case MV_MFLASH_SUNOL_3:
            if (secSize ==  MV_MFLASH_SECTOR_SIZE_SMALL) 
            {
                pFlash->sectorSize = MV_MFLASH_SECTOR_SIZE_SMALL;
                pFlash->sectorNumber = MV_MFLASH_SMALL_SEC_NUM_SUNOL_3;
            }
            else 
            {
                pFlash->sectorSize = MV_MFLASH_SECTOR_SIZE_BIG;
                pFlash->sectorNumber = MV_MFLASH_BIG_SEC_NUM_SUNOL_3;
            }
            break;

        default:
            mvOsPrintf("%s ERROR: Unknown Flash Type!\n", __FUNCTION__);
            return MV_NOT_SUPPORTED;
    }
    
    return MV_OK;
}


/*******************************************************************************
* mvMFlashPrefetchSet - Set the Prefetch mode enable/disable
*
* DESCRIPTION:
*       Enable (MV_TRUE) or Disable (MV_FALSE) the prefetch mode
*
* INPUT:
*       pFlash: pointer to the MFlash information structure
*		prefetch: enable/disable (MV_TRUE/MV_FALSE)
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvMFlashPrefetchSet (MV_MFLASH_INFO *pFlash, MV_BOOL prefetch)
{
    /* check for NULL pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	if (pFlash->ifMode == MV_MFLASH_PARALLEL)
        return mvPMFlashPrefetchSet(pFlash, prefetch);

    DB(mvOsPrintf("%s WARNING: Invalid interface mode!\n", __FUNCTION__);)
    return MV_BAD_PARAM;
}

/*******************************************************************************
* mvMFlashShutdownSet - Shutdown the voltage regulator in the flash device
*
* DESCRIPTION:
*       Causes the device to enter in a power save mode untill the next access 
*		to the flash.
*
* INPUT:
*       pFlash: pointer to the MFlash information structure
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvMFlashShutdownSet(MV_MFLASH_INFO *pFlash)
{
    /* check for NULL pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	if (pFlash->ifMode == MV_MFLASH_PARALLEL)
        return mvPMFlashShutdownSet(pFlash);
    else if (pFlash->ifMode == MV_MFLASH_SPI)
        return mvSMFlashShutdownSet(pFlash);

    mvOsPrintf("%s ERROR: Invalid interface mode!\n", __FUNCTION__);
    return MV_BAD_PARAM;
}

/*******************************************************************************
* mvMFlashIdGet - Retreive the MFlash's manufacturer and device IDs
*
* DESCRIPTION:
*       Read from the flash the 32bit manufacturer Id and the 16bit device ID
*
* INPUT:
*       pFlash: pointer to the MFlash information structure
*       pManfId: pointer to the 32bit variable to read the manufacturer ID in
*       pDevId: pointer to the 16bit variable to read the device ID in
*
* OUTPUT:
*       pManfId: pointer to the 32bit variable holding the manufacturer ID
*       pDevId: pointer to the 16bit variable holding the device ID
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvMFlashIdGet (MV_MFLASH_INFO *pFlash, MV_U32 * pManfId, MV_U16 * pDevId)
{
    /* check for NULL pointer */
    if ((pFlash == NULL) || (pManfId == NULL) || (pDevId == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	if (pFlash->ifMode == MV_MFLASH_PARALLEL)
        return mvPMFlashIdGet(pFlash, pManfId, pDevId);
    else if (pFlash->ifMode == MV_MFLASH_SPI)
        return mvSMFlashIdGet(pFlash, pManfId, pDevId);

    mvOsPrintf("%s ERROR: Invalid interface mode!\n", __FUNCTION__);
    return MV_BAD_PARAM;
}

/*******************************************************************************
* mvMFlashReset - Reset the flash device and the MFlash glue logic
*
* DESCRIPTION:
*       Perfom a hard reset for the Marvell Flash and the controller interface
*
* INPUT:
*       pFlash: pointer to the MFlash information structure
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvMFlashReset(MV_MFLASH_INFO *pFlash)
{
	MV_U32 reg;

    /* check for NULL pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	/* set both bits 4 and 5 */
	reg = MV_REG_READ(MV_PMFLASH_IF_CFG_REG);
	reg |= (MV_PMFLASH_RESET_MASK | MV_PMFLASH_IF_RESET_MASK);
	MV_REG_WRITE(MV_PMFLASH_IF_CFG_REG, reg);

	/* Insert a short delay */
	mvOsDelay(1);   /* 1ms */

	/* reset both bits 4 and 5 */
	reg = MV_REG_READ(MV_PMFLASH_IF_CFG_REG);
	reg &= ~(MV_PMFLASH_RESET_MASK | MV_PMFLASH_IF_RESET_MASK);
	MV_REG_WRITE(MV_PMFLASH_IF_CFG_REG, reg);

	return MV_OK;
}



