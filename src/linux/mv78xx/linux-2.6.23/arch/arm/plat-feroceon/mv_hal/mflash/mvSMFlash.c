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
#include "mflash/mvSMFlash.h"
#include "mflash/mvSMFlashSpec.h"
#include "mflash/mvMFlash.h"
#include "mflash/mvMFlashSpec.h"
#include "spi/mvSpi.h"
#include "spi/mvSpiCmnd.h"
#include "ctrlEnv/mvCtrlEnvLib.h"

/*#define MV_DEBUG*/
#ifdef MV_DEBUG
#define DB(x) x
#else
#define DB(x)
#endif

/* Static Functions */
static MV_STATUS mvSingleByteCmnd     (MV_U8 opcode);
static MV_STATUS mvSingleByteRead     (MV_U8 opcode, MV_U8* pData);
static MV_STATUS mvSingleByteWrite    (MV_U8 opcode, MV_U8 data);
static MV_STATUS mvWriteEnable        (MV_VOID);
static MV_STATUS mvStatusRegGet       (MV_U8* pStatus);
static MV_STATUS mvWaitOnWipClear     (MV_VOID);

/*******************************************************************************
* mvSingleByteCmnd - Issue an 8bit commmand
*
* DESCRIPTION:
*       serialize an 8bit command without any data
*
********************************************************************************/
static MV_STATUS mvSingleByteCmnd(MV_U8 opcode)
{
	MV_U8 cmd[1];
	cmd[0] = opcode;
	return mvSpiWriteThenRead(cmd, 1, NULL, 0,0);
}

/*******************************************************************************
* mvSingleByteCmnd - Issue an 8bit Read
*
* DESCRIPTION:
*       serialize an 8bit command and read 8bits of data
*
********************************************************************************/
static MV_STATUS mvSingleByteRead(MV_U8 opcode, MV_U8* pData)
{
	MV_U8 cmd[1];
	cmd[0] = opcode;
	return mvSpiWriteThenRead(cmd, 1, pData, 1,0);
}

/*******************************************************************************
* mvSingleByteWrite - Issue an 8bit Write
*
* DESCRIPTION:
*       serialize an 8bit command and 8bits of data
*
********************************************************************************/
static MV_STATUS mvSingleByteWrite(MV_U8 opcode, MV_U8 data)
{
	return mvSpiWriteThenWrite(&opcode, 1, &data, 1);
}

/*******************************************************************************
* mvWriteEnable - Perform the write enable sequence
*
* DESCRIPTION:
*       Writa enable sequence needed before program and erase commands
*
********************************************************************************/
static MV_STATUS mvWriteEnable(MV_VOID)
{
	return mvSingleByteCmnd(MV_SMFLASH_WREN_CMND_OPCD);
}

/*******************************************************************************
* mvStatusRegGet - Retreiv the value of the status register
*
* DESCRIPTION:
*       perform the RDSR sequence to get the 8bit status register
*
********************************************************************************/
static MV_STATUS mvStatusRegGet(MV_U8* pStatus)
{
	return mvSingleByteRead(MV_SMFLASH_RDSR_CMND_OPCD, pStatus);
}

/*******************************************************************************
* mvWaitOnWipClear - Block waiting for the WIP (write in progress) to be cleared
*
* DESCRIPTION:
*       Block waiting for the WIP (write in progress) to be cleared
*
********************************************************************************/
static MV_STATUS mvWaitOnWipClear(MV_VOID)
{
    MV_STATUS ret;
	MV_U32 i;
	MV_U8 status;

	for (i=0; i<MV_MFLASH_MAX_PRG_LOOP; i++)
	{
		/* Read the status register */
		if ((ret = mvStatusRegGet(&status)) != MV_OK)
			return ret;

		/* check if write in progress bit was cleared by h/w */
		if ((status & MV_SMFLASH_STATUS_REG_WIP_MASK) == 0)
			return MV_OK;
	}

    DB(mvOsPrintf("%s WARNING: Wait Timeout!\n", __FUNCTION__);)
	return MV_TIMEOUT;
}

/*
#####################################################################################
#####################################################################################
*/

/*******************************************************************************
* mvSMFlashInit - Initialize the Marvell serial flash device
*
* DESCRIPTION:
*       Perform the neccessary initialization and configuration
*
* INPUT:
*       flinfo: Flash information structure
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSMFlashInit (MV_MFLASH_INFO *pFlash)
{
    /* Check for null pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	return MV_OK;
}

/*******************************************************************************
* mvSMFlashChipErase - Erase the whole chip
*
* DESCRIPTION:
*       Perform the chip erase sequence. To set both the Main and information
*		regions to logic ones.
*
* INPUT:
*       pFlash: Flash information structure
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*
*******************************************************************************/
MV_STATUS mvSMFlashChipErase (MV_MFLASH_INFO *pFlash)
{
	MV_STATUS ret = MV_OK;

    /* Check for null pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	/* Perform the write enable sequence */
	if ((ret = mvWriteEnable()) != MV_OK)
		return ret;

	if ((ret = mvSingleByteCmnd(MV_SMFLASH_CHIP_ERASE_CMND_OPCD)) != MV_OK)
		return ret;

	/* wait for the write in progress bit to be cleared in the status register */
	if ((ret = mvWaitOnWipClear()) != MV_OK)
		return ret;

	return ret;
}

/*******************************************************************************
* mvSMFlashMainErase - Erase the Main flash region only
*
* DESCRIPTION:
*       Erase the main flash region to ones. Information region will not be
*		affected.
*
* INPUT:
*       pFlash: Flash information structure
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSMFlashMainErase (MV_MFLASH_INFO *pFlash)
{
	MV_STATUS ret = MV_OK;

    /* Check for null pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	/* Perform the write enable sequence */
	if ((ret = mvWriteEnable()) != MV_OK)
		return ret;

	if ((ret = mvSingleByteCmnd(MV_SMFLASH_MAIN_ERASE_CMND_OPCD)) != MV_OK)
		return ret;

	/* wait for the write in progress bit to be cleared in the status register */
	if ((ret = mvWaitOnWipClear()) != MV_OK)
		return ret;

	return ret;
}

/*******************************************************************************
* mvSMFlashInfErase - Erase the whole information region only
*
* DESCRIPTION:
*       Erase the whole information region without affecting the Main region.
*
* INPUT:
*       pFlash: Flash information structure
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSMFlashInfErase (MV_MFLASH_INFO *pFlash)
{
	MV_STATUS ret = MV_OK;

    /* Check for null pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	/* Perform the write enable sequence */
	if ((ret = mvWriteEnable()) != MV_OK)
		return ret;

	if ((ret = mvSingleByteCmnd(MV_SMFLASH_IPG_ERASE_CMND_OPCD)) != MV_OK)
		return ret;

	/* wait for the write in progress bit to be cleared in the status register */
	if ((ret = mvWaitOnWipClear()) != MV_OK)
		return ret;

	return ret;
}

/*******************************************************************************
* mvSMFlashSecErase - Erase a single sector of the Main region
*
* DESCRIPTION:
*       Perform the sequence to erase a Main region sector
*
* INPUT:
*       pFlash: Flash information structure
*		secNumber: sector number to erase.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSMFlashSecErase (MV_MFLASH_INFO *pFlash, MV_U32 secAddr)
{
	MV_STATUS ret;
	MV_U8 cmd[MV_SMFLASH_PAGE_ERASE_CMND_LEN];

    /* Check for null pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	cmd[0] = MV_SMFLASH_PAGE_ERASE_CMND_OPCD;
	cmd[1] = ((secAddr >> 16) & 0xFF);		/* address bits 16 - 23 */
	cmd[2] = ((secAddr >> 8) & 0xFF);		/* address bits 8 - 15 */
	cmd[3] = (secAddr & 0xFF);				/* address bits 0 - 7 */

	/* Perform the write enable sequence */
	if ((ret = mvWriteEnable()) != MV_OK)
		return ret;

	if ((ret = mvSpiWriteThenRead(cmd, MV_SMFLASH_PAGE_ERASE_CMND_LEN, NULL, 0, 0)) != MV_OK)
		return ret;

	/* wait for the write in progress bit to be cleared in the status register */
	if ((ret = mvWaitOnWipClear()) != MV_OK)
		return ret;

	return ret;
}



/*******************************************************************************
* mvSMFlash64bWr - Program 64 bytes of Data alligned to 64 boundaries
*
* DESCRIPTION:
*       Program to main region 64 bytes of Data alligned to 64 boundaries
*
* INPUT:
*       pFlash: Flash information structure
*		offset: address to start the programming from (alligned to 64 bytes)
*		pBlock: pointer to the 64 bytes to program
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSMFlash64bWr(MV_MFLASH_INFO *pFlash, MV_U32 offset, MV_U8 *pBlock)
{
	MV_STATUS ret;
	MV_U8 cmd[MV_SMFLASH_PRGRM_CMND_LEN];

    /* Check for null pointer */
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

	/* check allignment */
	if (offset & MV_SMFLASH_PAGE_ALLIGN_MASK)
    {
        DB(mvOsPrintf("%s WARNING: Allignment error!\n", __FUNCTION__);)
		return MV_BAD_PARAM;
    }

	/* encode the command to send */
	cmd[0] = MV_SMFLASH_PRGRM_CMND_OPCD;
	cmd[1] = ((offset >> 16) & 0xFF);
	cmd[2] = ((offset >> 8) & 0xFF);
	cmd[3] = (offset & 0xFF);

	/* Perform the write enable sequence */
	if ((ret = mvWriteEnable()) != MV_OK)
		return ret;

	/* serialize the command and then followed by the data to program */
	if ((ret = mvSpiWriteThenWrite(cmd, MV_SMFLASH_PRGRM_CMND_LEN, pBlock, MV_SMFLASH_PAGE_SIZE)) != MV_OK)
		return ret;

	/* wait for the write in progress bit to be cleared in the status register */
	if ((ret = mvWaitOnWipClear()) != MV_OK)
		return ret;

	return MV_OK;
}

/*******************************************************************************
* mvSMFlash64bWrVerify - Program and verify 64 bytes of Data alligned to
*						 64 bytes boundaries
*
* DESCRIPTION:
*       Program and verifyto main region 64 bytes of Data alligned to 64
*		bytes boundaries
*
* INPUT:
*       pFlash: Flash information structure
*		offset: address to start the programming from (alligned to 64 bytes)
*		pBlock: pointer to the 64 bytes to program
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSMFlash64bWrVerify(MV_MFLASH_INFO *pFlash, MV_U32 offset, MV_U8 *pBlock)
{
	MV_STATUS ret;
	MV_U8 cmd[MV_SMFLASH_PRGRM_CMP_CMND_LEN];
    MV_U8 status;

    /* Check for null pointer */
    if ((pFlash == NULL) || (pBlock == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	/* check allignment */
	if (offset & MV_SMFLASH_PAGE_ALLIGN_MASK)
    {
        DB(mvOsPrintf("%s WARNING: Allignment error!\n", __FUNCTION__);)
		return MV_BAD_PARAM;
    }

	/* encode the command to send */
	cmd[0] = MV_SMFLASH_PRGRM_CMP_CMND_OPCD;
	cmd[1] = ((offset >> 16) & 0xFF);
	cmd[2] = ((offset >> 8) & 0xFF);
	cmd[3] = (offset & 0xFF);

	/* Perform the write enable sequence */
	if ((ret = mvWriteEnable()) != MV_OK)
		return ret;

	/* serialize the command and then followed by the data to program */
	if ((ret = mvSpiWriteThenWrite(cmd, MV_SMFLASH_PRGRM_CMP_CMND_LEN, pBlock, MV_SMFLASH_PAGE_SIZE)) != MV_OK)
		return ret;

	/* wait for the write in progress bit to be cleared in the status register */
	if ((ret = mvWaitOnWipClear()) != MV_OK)
		return ret;

	/* Read the status register */
	if ((ret = mvStatusRegGet(&status)) != MV_OK)
		return ret;

	/* check if result of the compare operartion */
	if (status & MV_SMFLASH_STATUS_REG_CMP_MASK)
    {
        DB(mvOsPrintf("%s WARNING: Compare after write error!\n", __FUNCTION__);)
		return MV_FAIL;
    }

	return MV_OK;
}

/*******************************************************************************
* mvSMFlash64bInfWr - Information region program 64 bytes of Data alligned
*					  to 64 bytes boundaries
*
* DESCRIPTION:
*       Program to information region 64 bytes of Data alligned to 64 boundaries
*
* INPUT:
*       pFlash: Flash information structure
*		offset: address to start the programming from (alligned to 64 bytes)
*		pBlock: pointer to the 64 bytes to program
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSMFlash64bInfWr(MV_MFLASH_INFO *pFlash, MV_U32 offset, MV_U8 *pBlock)
{
	MV_STATUS ret;
	MV_U8 cmd[MV_SMFLASH_IPRGRM_CMND_LEN];

    /* Check for null pointer */
    if ((pFlash == NULL) || (pBlock == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	/* check allignment */
	if (offset & MV_SMFLASH_PAGE_ALLIGN_MASK)
    {
        DB(mvOsPrintf("%s WARNING: Allignment error!\n", __FUNCTION__);)
		return MV_BAD_PARAM;
    }

	/* encode the command to send */
	cmd[0] = MV_SMFLASH_IPRGRM_CMND_OPCD;
	cmd[1] = ((offset >> 16) & 0xFF);
	cmd[2] = ((offset >> 8) & 0xFF);
	cmd[3] = (offset & 0xFF);

	/* Perform the write enable sequence */
	if ((ret = mvWriteEnable()) != MV_OK)
		return ret;

	/* serialize the command and then followed by the data to program */
	if ((ret = mvSpiWriteThenWrite(cmd, MV_SMFLASH_IPRGRM_CMND_LEN, pBlock, MV_SMFLASH_PAGE_SIZE)) != MV_OK)
		return ret;

	/* wait for the write in progress bit to be cleared in the status register */
	if ((ret = mvWaitOnWipClear()) != MV_OK)
		return ret;

	return MV_OK;
}

/*******************************************************************************
* mvSMFlash64bInfWrVerify - Program and verify 64 bytes of Data ti the
*							information region alligned to 64 boundaries
*
* DESCRIPTION:
*       Program and then verify to the information region 64 bytes of Data
*		alligned to 64 boundaries
*
* INPUT:
*       pFlash: Flash information structure
*		offset: address to start the programming from (alligned to 64 bytes)
*		pBlock: pointer to the 64 bytes to program
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSMFlash64bInfWrVerify(MV_MFLASH_INFO *pFlash, MV_U32 offset, MV_U8 *pBlock)
{
	MV_STATUS ret;
	MV_U8 cmd[MV_SMFLASH_IPRGRM_CMP_CMND_LEN];
    MV_U8 status;

    /* Check for null pointer */
    if ((pFlash == NULL) || (pBlock == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	/* check allignment */
	if (offset & MV_SMFLASH_PAGE_ALLIGN_MASK)
    {
        DB(mvOsPrintf("%s WARNING: Allignment error!\n", __FUNCTION__);)
		return MV_BAD_PARAM;
    }

	/* encode the command to send */
	cmd[0] = MV_SMFLASH_IPRGRM_CMP_CMND_OPCD;
	cmd[1] = ((offset >> 16) & 0xFF);
	cmd[2] = ((offset >> 8) & 0xFF);
	cmd[3] = (offset & 0xFF);

	/* Perform the write enable sequence */
	if ((ret = mvWriteEnable()) != MV_OK)
		return ret;

	/* serialize the command and then followed by the data to program */
	if ((ret = mvSpiWriteThenWrite(cmd, MV_SMFLASH_IPRGRM_CMP_CMND_LEN, pBlock, MV_SMFLASH_PAGE_SIZE)) != MV_OK)
		return ret;

	/* wait for the write in progress bit to be cleared in the status register */
	if ((ret = mvWaitOnWipClear()) != MV_OK)
		return ret;

    /* Read the status register */
	if ((ret = mvStatusRegGet(&status)) != MV_OK)
		return ret;

	/* check if result of the compare operartion */
	if (status & MV_SMFLASH_STATUS_REG_CMP_MASK)
    {
        DB(mvOsPrintf("%s WARNING: Compare after information region write error!\n", __FUNCTION__);)
		return MV_FAIL;
    }

	return MV_OK;
}

/*******************************************************************************
* mvSMFlashBlockRd - Read from the main region into a buffer
*
* DESCRIPTION:
*       Issue the main region read sequence. Size and offset are not limited
*		except to the size of the flash.
*
* INPUT:
*       pFlash: Flash information structure
*		offset: address in the main region to start the reading from.
*		blockSize: size of the buffer to be read
*		pBlock: pointer to the buffer to hold the data read from the flash
*
* OUTPUT:
*		pBlock: pointer to the buffer holding the data from the flash
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSMFlashBlockRd(MV_MFLASH_INFO *pFlash, MV_U32 offset, MV_U32 blockSize,
						   MV_U8 *pBlock)
{
	MV_STATUS ret;
	MV_U8 cmd[MV_SMFLASH_READ_CMND_LEN];

    /* Check for null pointer */
    if ((pFlash == NULL) || (pBlock == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	/* check that requested data is in the region */
    if ((offset + blockSize) > (pFlash->sectorNumber * pFlash->sectorSize))
    {
        DB(mvOsPrintf("%s WARNING: Read exceeds flash size!\n", __FUNCTION__);)
        return MV_BAD_PARAM;
    }

	/* encode the command to send */
	cmd[0] = MV_SMFLASH_READ_CMND_OPCD;
	cmd[1] = ((offset >> 16) & 0xFF);
	cmd[2] = ((offset >> 8) & 0xFF);
	cmd[3] = (offset & 0xFF);

	/* Perform the write enable sequence */
	if ((ret = mvWriteEnable()) != MV_OK)
		return ret;

	/* serialize the command and then followed by the data to program */
	if ((ret = mvSpiWriteThenRead(cmd, MV_SMFLASH_READ_CMND_LEN, pBlock, blockSize, 0)) != MV_OK)
		return ret;

	return MV_OK;
}

/*******************************************************************************
* mvSMFlashBlockInfRd - Read from the information region into a buffer
*
* DESCRIPTION:
*       Issue the information region read sequence. Size and offset are not
*		limited except to the size of the information region.
*
* INPUT:
*       pFlash: Flash information structure
*		offset: address in the information region to start the reading from.
*		blockSize: size of the buffer to be read
*		pBlock: pointer to the buffer to hold the data read from the flash
*
* OUTPUT:
*		pBlock: pointer to the buffer holding the data from the flash
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSMFlashBlockInfRd(MV_MFLASH_INFO *pFlash, MV_U32 offset, MV_U32 blockSize,
							  MV_U8 *pBlock)
{
	MV_STATUS ret;
	MV_U8 cmd[MV_SMFLASH_IREAD_CMND_LEN];

    /* Check for null pointer */
    if ((pFlash == NULL) || (pBlock == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	/* check that requested data is in the region */
    if ((offset + blockSize) > pFlash->infoSize)
    {
        DB(mvOsPrintf("%s WARNING: Read exceeds information region size!\n", __FUNCTION__);)
		return MV_BAD_PARAM;
    }

	/* encode the command to send */
	cmd[0] = MV_SMFLASH_IREAD_CMND_OPCD;
	cmd[1] = ((offset >> 16) & 0xFF);
	cmd[2] = ((offset >> 8) & 0xFF);
	cmd[3] = (offset & 0xFF);

	/* Perform the write enable sequence */
	if ((ret = mvWriteEnable()) != MV_OK)
		return ret;

	/* serialize the command and then followed by the data to program */
	if ((ret = mvSpiWriteThenRead(cmd, MV_SMFLASH_IREAD_CMND_LEN, pBlock, blockSize, 0)) != MV_OK)
		return ret;

	return MV_OK;
}

/*******************************************************************************
* mvSMFlashReadConfig3 - Read the value of the configuration register
*
* DESCRIPTION:
*       Read the value of the configuration register #3
*
* INPUT:
*       pFlash: Flash information structure
*		configReg: pointer to the 8bit variable to read in the config register
*
* OUTPUT:
*		configReg: pointer to the 8bit varable holding the config register
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSMFlashReadConfig3(MV_MFLASH_INFO *pFlash, MV_U8 * pConfigReg)
{
    /* Check for null pointer */
    if ((pFlash == NULL) || (pConfigReg == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	return mvSingleByteRead(MV_SMFLASH_READ_CFG_3_CMND_OPCD, pConfigReg);
}

/*******************************************************************************
* mvSMFlashReadConfig4 - Read the value of the configuration register
*
* DESCRIPTION:
*       Read the value of the configuration register #4
*
* INPUT:
*       pFlash: Flash information structure
*		configReg: pointer to the 8bit variable to read in the config register
*
* OUTPUT:
*		configReg: pointer to the 8bit varable holding the config register
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSMFlashReadConfig4(MV_MFLASH_INFO *pFlash, MV_U8 * pConfigReg)
{
    /* Check for null pointer */
    if ((pFlash == NULL) || (pConfigReg == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	return mvSingleByteRead(MV_SMFLASH_READ_CFG_4_CMND_OPCD, pConfigReg);
}

/*******************************************************************************
* mvSMFlashSetConfig3 - Set the value of the configuration register
*
* DESCRIPTION:
*       Set the value of the configuration register #3
*
* INPUT:
*       pFlash: Flash information structure
*		configReg: 8bit variable to be written to the config register
*
* OUTPUT:
*		None.
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSMFlashSetConfig3(MV_MFLASH_INFO *pFlash, MV_U8 configReg)
{
    /* Check for null pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	return mvSingleByteWrite(MV_SMFLASH_SET_CFG_3_CMND_OPCD, configReg);
}

/*******************************************************************************
* mvSMFlashSetConfig4 - Set the value of the configuration register
*
* DESCRIPTION:
*       Set the value of the configuration register #4
*
* INPUT:
*       pFlash: Flash information structure
*		configReg: 8bit variable to be written to the config register
*
* OUTPUT:
*		None.
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSMFlashSetConfig4(MV_MFLASH_INFO *pFlash, MV_U8 configReg)
{
    /* Check for null pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	return mvSingleByteWrite(MV_SMFLASH_SET_CFG_4_CMND_OPCD, configReg);
}

/*******************************************************************************
* mvSMFlashWriteProtectSet - Write protect the whole flash device
*
* DESCRIPTION:
*       Enable/Disable the write protection on the whole flash device.
*
* INPUT:
*       pFlash: Flash information structure
*		wp: enable/disable (MV_TRUE/MV_FALSE) write protection
*
* OUTPUT:
*		None
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSMFlashWriteProtectSet(MV_MFLASH_INFO *pFlash, MV_BOOL wp)
{
    MV_STATUS ret;
	MV_U8 reg;

    /* Check for null pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	/* Read the serial configuration register #4 */
	if ((ret = mvSMFlashReadConfig4(pFlash, &reg)) != MV_OK)
		return ret;

	/* set the WP bit according to the wp input */
	if (wp)
		reg &= ~MV_SMFLASH_SRL_CFG4_WP_MASK;
	else
		reg |= MV_SMFLASH_SRL_CFG4_WP_MASK;

	/* write back the serial configuration register #4 */
	if ((ret = mvSMFlashSetConfig4(pFlash, reg)) != MV_OK)
		return ret;

	return MV_OK;
}


/*******************************************************************************
* mvSMFlashSectorSizeSet - Set the sector size
*
* DESCRIPTION:
*       Set the sector size (4K or 32K)
*
* INPUT:
*       pFlash: Flash information structure
*		secSize: requested sector size
*
* OUTPUT:
*		None
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSMFlashSectorSizeSet(MV_MFLASH_INFO *pFlash, MV_U32 secSize)
{
    MV_STATUS ret;
	MV_U8 reg;

    /* Check for null pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	/* Read the serial configuration register #4 */
	if ((ret = mvSMFlashReadConfig4(pFlash, &reg)) != MV_OK)
		return ret;

	/* set the PAGE4K bit according to the secSize input */
	switch (secSize)
	{
		case MV_MFLASH_SECTOR_SIZE_BIG:
			reg &= ~MV_SMFLASH_SRL_CFG4_PG_SIZE_MASK;	/* 32K sectors */
			break;

		case MV_MFLASH_SECTOR_SIZE_SMALL:
			reg |= MV_SMFLASH_SRL_CFG4_PG_SIZE_MASK;		/* 4K sectors */
			break;

		default:
            DB(mvOsPrintf("%s WARNING: Invalid parameter sector size!\n", __FUNCTION__);)
			return MV_BAD_PARAM;
	}

	/* write back the serial configuration register #4 */
	if ((ret = mvSMFlashSetConfig4(pFlash, reg)) != MV_OK)
		return ret;

	return MV_OK;
}

/*******************************************************************************
* mvSMFlashShutdownSet - Put the flash device in power save mode
*
* DESCRIPTION:
*       Put the flash device in power save mode
*
* INPUT:
*       pFlash: Flash information structure
*
* OUTPUT:
*		None
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSMFlashShutdownSet(MV_MFLASH_INFO *pFlash)
{
    /* Check for null pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	return mvSingleByteCmnd(MV_SMFLASH_SHUTDOWN_CMND_OPCD);
}

/*******************************************************************************
* mvSMFlashIdGet - Retreive the Manufacturer ID and Device ID
*
* DESCRIPTION:
*       Read from the flash device the
*
* INPUT:
*       pFlash: Flash information structure
*
* OUTPUT:
*		None
*
* RETURN:
*       Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSMFlashIdGet(MV_MFLASH_INFO *pFlash, MV_U32 * pManfCode, MV_U16 * pDevCode)
{
	MV_STATUS ret = MV_OK;
	MV_U8 cmd[MV_SMFLASH_RDID_CMND_LEN];
	MV_U8 id[MV_SMFLASH_RDID_RPLY_LEN];

    /* Check for null pointer */
    if ((pFlash == NULL) || (pManfCode == NULL) || (pDevCode == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	cmd[0] = MV_SMFLASH_RDID_CMND_OPCD;

	if ((ret = mvSpiWriteThenRead(cmd, MV_SMFLASH_RDID_CMND_LEN, id, MV_SMFLASH_RDID_RPLY_LEN, 0)) != MV_OK)
		return ret;

	*pManfCode = ((id[0] << 24) | (id[1] << 16) | (id[2] << 8) | (id[3]));
	*pDevCode = ((id[4] << 8) | (id[5]));

	return ret;
}



