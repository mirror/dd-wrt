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
#include "mflash/mvMFlashSpec.h"
#include "mflash/mvPMFlash.h"
#include "mflash/mvPMFlashSpec.h"
#include "ctrlEnv/mvCtrlEnvLib.h" 

/*#define MV_DEBUG*/
#ifdef MV_DEBUG
#define DB(x) x
#else
#define DB(x)
#endif

/* MACROS */

/* Static Functions */
static MV_STATUS waitOnCmdDone              (MV_VOID);
static MV_STATUS waitOnEraseBitClear        (MV_VOID);
static MV_STATUS waitOnInfReadBitClear      (MV_VOID);
static MV_STATUS mvPMFlashReadSR            (MV_U8 * pStatusReg);
static MV_STATUS mvPMFlashReadRegSeq        (MV_U32 prgCmnd, MV_U8 * pStatusReg);
static MV_STATUS mvPMFlashWriteRegSeq       (MV_U32 prgCmnd, MV_U32 reg);
static MV_STATUS mvPMFlashProgSeq           (MV_MFLASH_INFO *pFlash, MV_U32 prgCmnd, MV_U32 offset, MV_U8 *pBlock);
static MV_STATUS mvPMFlashEraseSeq          (MV_MFLASH_INFO *pFlash, MV_U32 eraseCmnd);
static MV_STATUS mvPMFlashIndirectReadSeq   (MV_MFLASH_INFO *pFlash, MV_U32 readCmnd, MV_U8* p8Bytes);

/*******************************************************************************
* waitOnCmdDone - Wait for command done bit
*
* DESCRIPTION:
*       Block waiting for command done. Returns Error Status
*
********************************************************************************/
static MV_STATUS waitOnCmdDone(MV_VOID)
{
	MV_U32 i;
	/* loop over waiting for the command done bit to be set */
	for (i=0; i<MV_MFLASH_MAX_CMD_DONE_LOOP; i++)
		if (MV_REG_READ(MV_PMFLASH_IF_STATUS_REG) & MV_PMFLASH_CMD_DONE_MASK)
			return MV_OK;

    DB(mvOsPrintf("%s WARNING: Command Timeout!\n", __FUNCTION__);)
	return MV_TIMEOUT;
}

/*******************************************************************************
* waitOnEraseBitClear - Wait for erase bit to be cleared by H/W
*
* DESCRIPTION:
*       Block waiting for erase bit (bit13 in Flash command opcode register)
*		to be cleared by hardware as an indication that the command was accepted.
*
********************************************************************************/
static MV_STATUS waitOnEraseBitClear(MV_VOID)
{
	MV_U32 i;
	/* loop over waiting for the command done bit to be set */
	for (i=0; i<MV_MFLASH_MAX_CMD_DONE_LOOP; i++)
		if ((MV_REG_READ(MV_PMFLASH_CMD_OPCODE_REG) & MV_PMFLASH_ERASE_STRT_MASK) == 0)
			return MV_OK;

    DB(mvOsPrintf("%s WARNING: Command Timeout!\n", __FUNCTION__);)
	return MV_TIMEOUT;
}

/*******************************************************************************
* waitOnRdStatusBitClear - Wait for Read status bit to be cleared by H/W
*
* DESCRIPTION:
*       Block waiting for Read bit (bit14 in Flash command opcode register)
*		to be cleared by hardware as an indication that the command was executed.
*
********************************************************************************/
static MV_STATUS waitOnRdStatusBitClear(MV_VOID)
{
	MV_U32 i;
	/* loop over waiting for the command done bit to be set */
	for (i=0; i<MV_MFLASH_MAX_CMD_DONE_LOOP; i++)
		if ((MV_REG_READ(MV_PMFLASH_CMD_OPCODE_REG) & MV_PMFLASH_RDSR_SET_MASK) == 0)
			return MV_OK;

    DB(mvOsPrintf("%s WARNING: Command Timeout!\n", __FUNCTION__);)
	return MV_TIMEOUT;
}

/*******************************************************************************
* waitOnInfReadBitClear - Wait for Information Read bit to be cleared by H/W
*
* DESCRIPTION:
*       Block waiting for IREAD (bit15 in Flash command opcode register)
*		to be cleared by hardware as an indication that the requested 8bytes
*		are ready to be ready by s/w.
*
********************************************************************************/
static MV_STATUS waitOnInfReadBitClear(MV_VOID)
{
	MV_U32 i;
	/* loop over waiting for the command done bit to be set */
	for (i=0; i<MV_MFLASH_MAX_CMD_DONE_LOOP; i++)
		if ((MV_REG_READ(MV_PMFLASH_CMD_OPCODE_REG) & MV_PMFLASH_INF_READ_STRT_MASK) == 0)
			return MV_OK;

    DB(mvOsPrintf("%s WARNING: Command Timeout!\n", __FUNCTION__);)
	return MV_TIMEOUT;
}

/*******************************************************************************
* mvPMFlashReadRegSeq - Read internal register of the MFLASH
*
* DESCRIPTION:
*       Perform the Read internal register RAB
*
********************************************************************************/
static MV_STATUS mvPMFlashReadRegSeq(MV_U32 prgCmnd, MV_U8 * pStatusReg)
{	
	MV_U32 temp;
    MV_STATUS ret;

	/* set commnand opcode = RDSR */
	temp = MV_REG_READ(MV_PMFLASH_CMD_OPCODE_REG);
	temp = ((temp & ~MV_PMFLASH_CMD_OPCD_MASK) | prgCmnd | MV_PMFLASH_EAD9_8_MASK);
	MV_REG_WRITE(MV_PMFLASH_CMD_OPCODE_REG, temp);

	/* trigger the operation */
	MV_REG_BIT_SET(MV_PMFLASH_CMD_OPCODE_REG, MV_PMFLASH_RDSR_SET_MASK);	

	/* wait on the read status bit to be cleared */
	if ((ret = waitOnRdStatusBitClear()) != MV_OK)
		return ret;

	/* wait for command done bit - this is supposed to be immediate */
	if ((ret = waitOnCmdDone()) != MV_OK)
		return ret;

	/* read the satus register */
	*pStatusReg = (MV_REG_READ(MV_PMFLASH_STATUS_REG) & MV_PMFLASH_STATUS_MASK);
	
	return MV_OK;
}

/*******************************************************************************
* mvPMFlashWriteRegSeq - Write internal register of the MFLASH
*
* DESCRIPTION:
*       Perform the write internal register RAB
*
********************************************************************************/
static MV_STATUS mvPMFlashWriteRegSeq(MV_U32 prgCmnd, MV_U32 reg)
{	
    MV_STATUS ret;
	MV_U32 temp;

	/* set data to be written in the address low register */
	MV_REG_WRITE(MV_PMFLASH_ADDR_LOW_REG, (reg & 0xFFFF));

	/* set commnand opcode = prgCmnd */
	temp = MV_REG_READ(MV_PMFLASH_CMD_OPCODE_REG);
	temp = ((temp & ~MV_PMFLASH_CMD_OPCD_MASK) | prgCmnd | MV_PMFLASH_EAD9_8_MASK);
	MV_REG_WRITE(MV_PMFLASH_CMD_OPCODE_REG, temp);

	/* trigger the operation */
	MV_REG_BIT_SET(MV_PMFLASH_CMD_OPCODE_REG, MV_PMFLASH_ERASE_STRT_MASK);	

	/* wait for erase bit to be cleared */
	if ((ret = waitOnEraseBitClear()) != MV_OK)
		return ret;
	
	return MV_OK;
}

/*******************************************************************************
* mvPMFlashReadSR - Read the Status register
*
* DESCRIPTION:
*       Perform the Read status register RAB
*
********************************************************************************/
static MV_STATUS mvPMFlashReadSR(MV_U8 * pStatusReg)
{	
	return mvPMFlashReadRegSeq(MV_PMFLASH_OPCD_RDSR, pStatusReg);
}

/*******************************************************************************
* mvPMFlashProgSeq - Perform the program sequence for the MFLASH
*
* DESCRIPTION:
*       program a 64 byte block (with or without compare after program) on 
*		both the main and information regions of the flash.
*
********************************************************************************/
static MV_STATUS mvPMFlashProgSeq(MV_MFLASH_INFO *pFlash, MV_U32 prgCmnd, MV_U32 offset, 
                                  MV_U8 *pBlock)
{
	MV_U32 i;
	MV_U16 * pBuff;
	MV_U8 status;
	MV_U32 temp;
    MV_STATUS ret;

	/* check that the programming offset is alligned according to requirement */
	if (offset & MV_MFLASH_PROG_ALIGN_MASK)
    {
        DB(mvOsPrintf("%s WARNING: Programming allignment problem!\n", __FUNCTION__);)
		return MV_NOT_ALIGNED;
    }

	/* Set the flash address low */
	MV_REG_WRITE(MV_PMFLASH_ADDR_LOW_REG, (offset & 0xFFFF));

	/* set the flash address high */
	MV_REG_WRITE(MV_PMFLASH_ADDR_HI_REG, ((offset >> 16) & 0xFFFF));

	/* set commnand opcode = prgCmnd */
	temp = MV_REG_READ(MV_PMFLASH_CMD_OPCODE_REG);
	temp = ((temp & ~MV_PMFLASH_CMD_OPCD_MASK) | prgCmnd | MV_PMFLASH_EAD9_8_MASK);
	MV_REG_WRITE(MV_PMFLASH_CMD_OPCODE_REG, temp);

	/* start the programming operation - set bit 12 to 1 */
	MV_REG_BIT_SET(MV_PMFLASH_CMD_OPCODE_REG, MV_PMFLASH_WR_OPER_STRT_MASK);	

	/* serialize the whole 64 byte buffer */
	pBuff = (MV_U16*)pBlock;
	for (i=0; i<MV_PMFLASH_PROG_WORDS_PER_CHUNK; i++)
	{
		MV_REG_WRITE(MV_PMFLASH_DATA_REG, MV_16BIT_LE(*pBuff));
		pBuff++;
	}

	/* stop the programming operation - set bit 12 to 0 */
	MV_REG_BIT_RESET(MV_PMFLASH_CMD_OPCODE_REG, MV_PMFLASH_WR_OPER_STRT_MASK);	

	for (i=0; i<MV_MFLASH_MAX_PRG_LOOP; i++)
	{
		if ((ret = mvPMFlashReadSR(&status)) != MV_OK)
			return ret;

		/* check the WIP bit if cleared (indicating end of programming) */
		if ((status & MV_PMFLASH_STAT_WIP_MASK) == 0)
			return MV_OK;
	}

    DB(mvOsPrintf("%s WARNING: Command Timeout!\n", __FUNCTION__);)
	return MV_TIMEOUT;
}

/*******************************************************************************
* mvPMFlashEraseSeq - Perform the erase sequence for the MFLASH
*
* DESCRIPTION:
*       Erase a complete region in the MFlash (Main, Information or the 
*		whole flash).
*
********************************************************************************/
static MV_STATUS mvPMFlashEraseSeq(MV_MFLASH_INFO *pFlash, MV_U32 eraseCmnd)
{
	MV_U32 i;
	MV_U8 status;
	MV_U32 temp;
    MV_STATUS ret;

	/* set commnand opcode = prgCmnd */
	temp = MV_REG_READ(MV_PMFLASH_CMD_OPCODE_REG);
	temp = ((temp & ~MV_PMFLASH_CMD_OPCD_MASK) | eraseCmnd | MV_PMFLASH_EAD9_8_MASK);
	MV_REG_WRITE(MV_PMFLASH_CMD_OPCODE_REG, temp);

	/* Trigger the erase operation - set bit 13 to 1 */
	MV_REG_BIT_SET(MV_PMFLASH_CMD_OPCODE_REG, MV_PMFLASH_ERASE_STRT_MASK);	

	/* wait for the command to be sent to the device */
	if ((ret = waitOnEraseBitClear()) != MV_OK)
		return ret;

	/* wait for the erase command to be completed */
	for (i=0; i<MV_MFLASH_MAX_PRG_LOOP; i++)
	{
		/* read the Status register */
		if ((ret = mvPMFlashReadSR(&status)) != MV_OK)
			return ret;

		/* check the WIP bit if cleared (indicating end of programming) */
		if ((status & MV_PMFLASH_STAT_WIP_MASK) == 0)
			return MV_OK;
	}

    DB(mvOsPrintf("%s WARNING: Command Timeout!\n", __FUNCTION__);)
	return MV_TIMEOUT;
}

/*******************************************************************************
* mvPMFlashIndirectReadSeq - Perform the Information READ sequence
*
* DESCRIPTION:
*       Read indirectly from the FLASH information region
*
********************************************************************************/
static MV_STATUS mvPMFlashIndirectReadSeq(MV_MFLASH_INFO *pFlash, MV_U32 readCmnd, MV_U8* p8Bytes)
{
	MV_U32 i;
	MV_U16 * dataPrt = (MV_U16*)p8Bytes;
	MV_U32 temp;
    MV_STATUS ret;

	/* set commnand opcode = prgCmnd */
	temp = MV_REG_READ(MV_PMFLASH_CMD_OPCODE_REG);
	temp = ((temp & ~MV_PMFLASH_CMD_OPCD_MASK) | readCmnd | MV_PMFLASH_EAD9_8_MASK);
	MV_REG_WRITE(MV_PMFLASH_CMD_OPCODE_REG, temp);

	/* Trigger the erase operation - set bit 13 to 1 */
	MV_REG_BIT_SET(MV_PMFLASH_CMD_OPCODE_REG, MV_PMFLASH_INF_READ_STRT_MASK);
	
	/* block waiting for bit to be cleared by hardware */
	if ((ret = waitOnInfReadBitClear()) != MV_OK)
		return ret;

	/* read the 8bytes from the Data register into the buffer in 4 iterations */
	for (i=0; i<4; i++)
	{
		*dataPrt = MV_REG_READ(MV_PMFLASH_DATA_REG);
		dataPrt++;
	}

	return MV_OK;
}

/*
#####################################################################################
#####################################################################################
*/

/*******************************************************************************
* mvPMFlashInit - Perform basic initialize for the MFlash device in paralle mode
*
* DESCRIPTION:
*       This function performs the necessary initialization for a Marvell 
*       Sunol flash.
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvPMFlashInit (MV_MFLASH_INFO *pFlash)
{
    MV_STATUS ret;

    /* Check for null pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	/* set the static timing parameters */
	if ((ret = mvPMFlashSetConfig1(pFlash, ((MV_PMFLASH_T2 << 4) | MV_PMFLASH_T1))) != MV_OK)
        return ret;

	if ((ret = mvPMFlashSetConfig3(pFlash, MV_PMFLASH_T7)) != MV_OK)
        return ret;

	if ((ret = mvPMFlashSetSlewRate(pFlash, ((MV_PMFLASH_PCLK_OUT << 4) | 
		                         (MV_PMFLASH_SLEW_2_3 << 2) | 
						          MV_PMFLASH_SLEW_0_1))) != MV_OK)
        return ret;

	return MV_OK;
}

/*******************************************************************************
* mvPMFlash64bWr - Program (no Verify) main flash data
*
* DESCRIPTION:
*       Program an alligned 64byte block in the main flash region of the MFlash
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*		offset: offset within the Information region (limited to 1024)
*		pBlock: pointer to the 64 bytes buffer to be programed
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvPMFlash64bWr (MV_MFLASH_INFO *pFlash, MV_U32 offset, MV_U8 *pBlock)
{
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

	return mvPMFlashProgSeq(pFlash, MV_PMFLASH_OPCD_PROGRAM, offset, pBlock);
}

/*******************************************************************************
* mvPMFlash64bWrVerify - Program and Verify Main flash data
*
* DESCRIPTION:
*       Program and Verify a 64byte alligned block in the main region of the 
*       MFlash
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*		offset: offset within the main region
*		pBlock: pointer to the 64 bytes buffer to be programed
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvPMFlash64bWrVerify (MV_MFLASH_INFO *pFlash, MV_U32 offset, MV_U8 *pBlock)
{
    MV_STATUS ret;
    MV_U8 status;

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

    /* Perform the program sequence */
	if ((ret = mvPMFlashProgSeq(pFlash, MV_PMFLASH_OPCD_PRG_CMP, offset, pBlock)) != MV_OK)
        return ret;

    /* Read the status register */
	if ((ret = mvPMFlashReadSR(&status)) != MV_OK)
		return ret;

	/* check the result of the compare operartion */
	if (status & MV_PMFLASH_STAT_CMP_MASK)
    {
        DB(mvOsPrintf("%s WARNING: Compare after write Failed!\n", __FUNCTION__);)
		return MV_FAIL;
    }

    return MV_OK;
}

/*******************************************************************************
* mvPMFlash64bInfWr - Program (no Verify) Information data
*
* DESCRIPTION:
*       Program a 64byte alligned block in the information region of the 
*       MFlash - Offset is limited to 1024
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*		offset: offset within the Information region (limited to 1024)
*		pBlock: pointer to the 64 bytes buffer to be programed
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvPMFlash64bInfWr (MV_MFLASH_INFO *pFlash, MV_U32 offset, MV_U8 *pBlock)
{
    /* Check for null pointer */
    if ((pFlash == NULL) || (pBlock == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	return mvPMFlashProgSeq(pFlash, MV_PMFLASH_OPCD_IPROGRAM, offset, pBlock);
}

/*******************************************************************************
* mvPMFlash64bInfWrVerify - Program and Verify Information data
*
* DESCRIPTION:
*       Program and Verify a 64byte alligned block in the information region 
*       of the MFlash - Offset is limited to 1024
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*		offset: offset within the Information region (limited to 1024)
*		pBlock: pointer to the 64 bytes buffer to be programed
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvPMFlash64bInfWrVerify (MV_MFLASH_INFO *pFlash, MV_U32 offset, MV_U8 *pBlock)
{
	MV_STATUS ret;
    MV_U8 status;

    /* Check for null pointer */
    if ((pFlash == NULL) || (pBlock == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

    /* Perform the program sequence */
	if ((ret = mvPMFlashProgSeq(pFlash, MV_PMFLASH_OPCD_IPRG_CMP, offset, pBlock)) != MV_OK)
        return ret;

    /* Read the status register */
	if ((ret = mvPMFlashReadSR(&status)) != MV_OK)
		return ret;

	/* check the result of the compare operartion */
	if (status & MV_PMFLASH_STAT_CMP_MASK)
    {
        DB(mvOsPrintf("%s WARNING: Compare after write Failed!\n", __FUNCTION__);)
		return MV_FAIL;
    }

    return MV_OK;
}

/*******************************************************************************
* mvPMFlashChipErase - Erase the whole flash
*
* DESCRIPTION:
*       Erase the whole flash (both the Main and Information region).
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvPMFlashChipErase (MV_MFLASH_INFO *pFlash)
{
    /* Check for null pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	return mvPMFlashEraseSeq(pFlash, MV_PMFLASH_OPCD_CHIP_ERASE);
}

/*******************************************************************************
* mvPMFlashMainErase - Erase the main flash region only
*
* DESCRIPTION:
*       Erase the Main flash region only. The information region will not be
*       affected.
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvPMFlashMainErase (MV_MFLASH_INFO *pFlash)
{
    /* Check for null pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	return mvPMFlashEraseSeq(pFlash, MV_PMFLASH_OPCD_MAIN_ERASE);
}

/*******************************************************************************
* mvPMFlashInfErase - Erase the information flash region only
*
* DESCRIPTION:
*       Erase the information flash region. The main flash region will not be
*       affected.
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvPMFlashInfErase (MV_MFLASH_INFO *pFlash)
{
    /* Check for null pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	return mvPMFlashEraseSeq(pFlash, MV_PMFLASH_OPCD_IPAGE_ERASE);
}


/*******************************************************************************
* mvPMFlashSecErase - Erase the single sector of the main flash region 
*
* DESCRIPTION:
*       Erase one sector of the main flash region
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*		secNumber: sector number to erase
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvPMFlashSecErase (MV_MFLASH_INFO *pFlash, MV_U32 secNumber)
{
    /* Check for null pointer */
    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	/* set the sector address in the flash address low */
	MV_REG_WRITE(MV_PMFLASH_ADDR_LOW_REG, (secNumber & 0xFFFF));

	/* Perform the erase sequence*/
	return mvPMFlashEraseSeq(pFlash, MV_PMFLASH_OPCD_PAGE_ERASE);
}


/*******************************************************************************
* mvPMFlashBlockRd - Read a block of Memory from the Main Flash 
*
* DESCRIPTION:
*       Read a block of Memory from the Main Flash 
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*		offset: offset to read from the main region of the MFlash
*		blockSize: number of bytes to read from the offset
*
* OUTPUT:
*		pBlock: pointer of the buffer to fill
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvPMFlashBlockRd (MV_MFLASH_INFO *pFlash, MV_U32 offset, MV_U32 blockSize, 
                            MV_U8 *pBlock)
{
    /* Check for null pointer */
    if ((pFlash == NULL) || (pBlock == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	/* check that requested data is in the region */
    if ((offset + blockSize) > (pFlash->sectorNumber * pFlash->sectorSize))
    {
        DB(mvOsPrintf("%s WARNING: Read allignment problem!\n", __FUNCTION__);)
        return MV_BAD_PARAM;
    }
	
    /* Read using direct access */
	mvOsMemcpy(pBlock, (MV_VOID*)(pFlash->baseAddr + offset), blockSize);

	return MV_OK;
}

/*******************************************************************************
* mvPMFlashBlockInfRd - Read a block of Memory from the Information region 
*
* DESCRIPTION:
*       Read a block of Memory from the information region
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*		offset: offset to read from the main region of the MFlash
*		blockSize: number of bytes to read from the offset
*
* OUTPUT:
*		pBlock: pointer of the buffer to fill
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvPMFlashBlockInfRd (MV_MFLASH_INFO *pFlash, MV_U32 offset, MV_U32 blockSize, 
                               MV_U8 *pBlock)
{
    MV_STATUS ret;
	MV_U8 tempBuff[MV_PMFLASH_IREAD_CHUNK_SIZE];
	MV_U32 data2read	= blockSize;
	MV_U32 preAllOfst	= (offset & MV_PMFLASH_IREAD_ALIGN_MASK);
    MV_U32 preAllSz		= (preAllOfst ? (MV_PMFLASH_IREAD_CHUNK_SIZE - preAllOfst) : 0);	
	MV_U32 readOffset	= (offset & ~MV_PMFLASH_IREAD_ALIGN_MASK);

    /* Check for null pointer */
    if ((pFlash == NULL) || (pBlock == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	/* sanity check on sizes and offsets based on chip model */
    if ((offset + blockSize) > pFlash->infoSize)
    {
        DB(mvOsPrintf("%s WARNING: Read exceeds flash size!\n", __FUNCTION__);)
        return MV_BAD_PARAM;
    }

	/* set the ADDDRESS HI to zero - since only the low address is needed */
	MV_REG_WRITE(MV_PMFLASH_ADDR_HI_REG, 0); /* only low address is needed */

	/* check if the total block size is less than the first chunk remainder */
	if (data2read < preAllSz)
		preAllSz = data2read;

	/* Check if read offset is not alligned to 8bytes; then perform a read alligned 
	   to 8 bytes and copy only the necessary data */
	if (preAllOfst)
	{
		/* set the read offset address in the flash address Low */
		MV_REG_WRITE(MV_PMFLASH_ADDR_LOW_REG, readOffset);
		if ((ret = mvPMFlashIndirectReadSeq(pFlash, MV_PMFLASH_OPCD_IREAD, tempBuff)) != MV_OK)
			return ret;

		/* copy to the buffer only the needed data */
		mvOsMemcpy (pBlock, &tempBuff[preAllOfst], preAllSz);

		/* increment the pointers and offsets */
		data2read -= preAllSz;
		readOffset += MV_PMFLASH_IREAD_CHUNK_SIZE;
		pBlock += preAllSz;
	}

	/* perform all reads that are alligned to 8bytes and full 8bytes */
	while (data2read >= MV_PMFLASH_IREAD_CHUNK_SIZE)
	{
		/* set the read offset address in the flash address Low */
		MV_REG_WRITE(MV_PMFLASH_ADDR_LOW_REG, readOffset);
		if ((ret = mvPMFlashIndirectReadSeq(pFlash, MV_PMFLASH_OPCD_IREAD, pBlock)) != MV_OK)
			return ret;

		/* increment the pointers and offsets */
		data2read -= MV_PMFLASH_IREAD_CHUNK_SIZE;
		readOffset += MV_PMFLASH_IREAD_CHUNK_SIZE;
		pBlock += MV_PMFLASH_IREAD_CHUNK_SIZE;
	}
	
	/* check if we need to perform a partial read at the end */
	if (data2read)
	{
		/* set the read offset address in the flash address Low */
		MV_REG_WRITE(MV_PMFLASH_ADDR_LOW_REG, readOffset);
		if ((ret = mvPMFlashIndirectReadSeq(pFlash, MV_PMFLASH_OPCD_IREAD, tempBuff)) != MV_OK)
			return ret;
		
		/* copy to the buffer only the needed data */
		mvOsMemcpy (pBlock, tempBuff, data2read);
	}

	return MV_OK;
}

/*******************************************************************************
* mvPMFlashReadConfig1 - Read the Configuration register
*
* DESCRIPTION:
*       Perform the Read CONFIG1 register RAB
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*		pConfigReg: pointer to the variable to fill
*
* OUTPUT:
*       pConfigReg: pointer to the variable holding the register value
*
* RETURN:
*       Success or Error code.
*       
********************************************************************************/
MV_STATUS mvPMFlashReadConfig1(MV_MFLASH_INFO *pFlash, MV_U8 * pConfigReg)
{	
    if ((pFlash == NULL) || (pConfigReg == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	return mvPMFlashReadRegSeq(MV_PMFLASH_OPCD_READ_CFG1, pConfigReg);
}

/*******************************************************************************
* mvPMFlashReadConfig2 - Read the Configuration register
*
* DESCRIPTION:
*       Perform the Read CONFIG2 register RAB
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*		pConfigReg: pointer to the variable to fill
*
* OUTPUT:
*       pConfigReg: pointer to the variable holding the register value
*
* RETURN:
*       Success or Error code.
*       
********************************************************************************/
MV_STATUS mvPMFlashReadConfig2(MV_MFLASH_INFO *pFlash, MV_U8 * pConfigReg)
{	
	if ((pFlash == NULL) || (pConfigReg == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	return mvPMFlashReadRegSeq(MV_PMFLASH_OPCD_READ_CFG2, pConfigReg);
}

/*******************************************************************************
* mvPMFlashReadConfig3 - Read the Configuration register
*
* DESCRIPTION:
*       Perform the Read CONFIG3 register RAB
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*		pConfigReg: pointer to the variable to fill
*
* OUTPUT:
*       pConfigReg: pointer to the variable holding the register value
*
* RETURN:
*       Success or Error code.
*       
********************************************************************************/
MV_STATUS mvPMFlashReadConfig3(MV_MFLASH_INFO *pFlash, MV_U8 * pConfigReg)
{	
	if ((pFlash == NULL) || (pConfigReg == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	return mvPMFlashReadRegSeq(MV_PMFLASH_OPCD_READ_CFG3, pConfigReg);
}

/*******************************************************************************
* mvPMFlashReadConfig4 - Read the Configuration register
*
* DESCRIPTION:
*       Perform the Read CONFIG4 register RAB
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*		pConfigReg: pointer to the variable to fill
*
* OUTPUT:
*       pConfigReg: pointer to the variable holding the register value
*
* RETURN:
*       Success or Error code.
*       
********************************************************************************/
MV_STATUS mvPMFlashReadConfig4(MV_MFLASH_INFO *pFlash, MV_U8 * pConfigReg)
{	
	if ((pFlash == NULL) || (pConfigReg == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	return mvPMFlashReadRegSeq(MV_PMFLASH_OPCD_READ_CFG4, pConfigReg);
}

/*******************************************************************************
* mvPMFlashSetConfig1 - Write the Configuration register
*
* DESCRIPTION:
*       Perform the SET CONFIG1 register RAB
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*		configReg: value to set in the configuration register
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
********************************************************************************/
MV_STATUS mvPMFlashSetConfig1(MV_MFLASH_INFO *pFlash, MV_U8 configReg)
{	
	if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	return mvPMFlashWriteRegSeq(MV_PMFLASH_OPCD_SET_CFG1, configReg);
}

/*******************************************************************************
* mvPMFlashSetConfig2 - Write the Configuration register
*
* DESCRIPTION:
*       Perform the SET CONFIG2 register RAB
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*		configReg: value to set in the configuration register
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
********************************************************************************/
MV_STATUS mvPMFlashSetConfig2(MV_MFLASH_INFO *pFlash, MV_U8 configReg)
{	
	if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	return mvPMFlashWriteRegSeq(MV_PMFLASH_OPCD_SET_CFG2, configReg);
}

/*******************************************************************************
* mvPMFlashSetConfig3 - Write the Configuration register
*
* DESCRIPTION:
*       Perform the SET CONFIG3 register RAB
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*		configReg: value to set in the configuration register
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
********************************************************************************/
MV_STATUS mvPMFlashSetConfig3(MV_MFLASH_INFO *pFlash, MV_U8 configReg)
{	
	if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	return mvPMFlashWriteRegSeq(MV_PMFLASH_OPCD_SET_CFG3, configReg);
}

/*******************************************************************************
* mvPMFlashSetConfig4 - Write the Configuration register
*
* DESCRIPTION:
*       Perform the SET CONFIG4 register RAB
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*		configReg: value to set in the configuration register
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
********************************************************************************/
MV_STATUS mvPMFlashSetConfig4(MV_MFLASH_INFO *pFlash, MV_U8 configReg)
{	
	if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	return mvPMFlashWriteRegSeq(MV_PMFLASH_OPCD_SET_CFG4, configReg);
}
/*******************************************************************************
* mvPMFlashSetConfig4 - Write the Configuration register
*
* DESCRIPTION:
*       Perform the SET CONFIG4 register RAB
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*		configReg: value to set in the configuration register
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
********************************************************************************/
MV_STATUS mvPMFlashSetSlewRate (MV_MFLASH_INFO *pFlash, MV_U8 configReg)
{
	if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	return mvPMFlashWriteRegSeq(MV_PMFLASH_OPCD_SET_SLEW, configReg);
}

/*******************************************************************************
* mvPMFlashWriteProtectSet - Set the write protection feature status
*
* DESCRIPTION:
*       Enable or disable the write protection
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
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
MV_STATUS mvPMFlashWriteProtectSet (MV_MFLASH_INFO *pFlash, MV_BOOL wp)
{
    MV_STATUS ret;
	MV_U8 tempReg;

    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;	
    }

	/* Read the original value */
	if ((ret = mvPMFlashReadConfig4(pFlash, &tempReg)) != MV_OK)
    {
        DB(mvOsPrintf("%s WARNING: Failed to read Config register!\n", __FUNCTION__);)
		return ret;
    }

	/* set the WP bit */
	if (wp) /* write protect active hi */
		tempReg |= MV_PMFLASH_CFG4_WP_MASK;
	else
		tempReg &= ~MV_PMFLASH_CFG4_WP_MASK;

	/* write the new register value */
	if ((ret = mvPMFlashSetConfig4(pFlash, tempReg)) != MV_OK)
		return ret;

	return MV_OK;
}

/*******************************************************************************
* mvPMFlashWriteProtectGet - Get the write protection feature status
*
* DESCRIPTION:
*       Read from the h/w the write protection status
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*		pWp: pointer to the variable to read in the write protection status
*
* OUTPUT:
*		pWp: pointer to the variable holding the write protection status
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvPMFlashWriteProtectGet(MV_MFLASH_INFO *pFlash, MV_BOOL * pWp)
{
    MV_STATUS ret;
	MV_U8 tempReg;

    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;	
    }

	/* Read the original value */
	if ((ret = mvPMFlashReadConfig4(pFlash, &tempReg)) != MV_OK)
    {
        DB(mvOsPrintf("%s WARNING: Failed to read Config register!\n", __FUNCTION__);)
		return ret;
    }

	/* Read the WP bit */
	if (tempReg &= ~MV_PMFLASH_CFG4_WP_MASK)
        *pWp = MV_TRUE;
    else
        *pWp = MV_FALSE;

	return MV_OK;
}

/*******************************************************************************
* mvPMFlashSectorSizeSet - Set the sector size (4K or 32K)
*
* DESCRIPTION:
*       Set the sector size of the MFLASH main region
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
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
MV_STATUS mvPMFlashSectorSizeSet (MV_MFLASH_INFO *pFlash, MV_U32 secSize)
{
    MV_STATUS ret;
	MV_U8 tempReg;

    if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;	
    }

	/* Read the original value */
	if ((ret = mvPMFlashReadConfig4(pFlash, &tempReg)) != MV_OK)
		return ret;

	/* set the sector size bit */
	switch (secSize)
	{
		case MV_MFLASH_SECTOR_SIZE_SMALL:
			tempReg |= MV_PMFLASH_CFG4_PG_SZ_MASK;		/* 1 - 4K */
			break;

		case MV_MFLASH_SECTOR_SIZE_BIG:
			tempReg &= ~MV_PMFLASH_CFG4_PG_SZ_MASK;		/* 0 - 32K */
			break;

		default:
            DB(mvOsPrintf("%s WARNING: Invalid parameter sector size!\n", __FUNCTION__);)
			return MV_BAD_PARAM;
	}

	/* write the new register value */
	if ((ret = mvPMFlashSetConfig4(pFlash, tempReg)) != MV_OK)
		return ret;

	return MV_OK;
}


/*******************************************************************************
* mvPMFlashPrefetchSet - Set the Prefetch mode enable/disable
*
* DESCRIPTION:
*       Enable (MV_TRUE) or Disable (MV_FALSE) the prefetch mode
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*		prefetch: enable/disable (true/false)
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvPMFlashPrefetchSet (MV_MFLASH_INFO *pFlash, MV_BOOL prefetch)
{
    MV_STATUS ret;
	MV_U8 tempReg;

	if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	/* Read the original value */
	if ((ret = mvPMFlashReadConfig2(pFlash, &tempReg)) != MV_OK)
		return ret;

	/* set the prefetch enable bit */
	if (prefetch) 
		tempReg |= MV_PMFLASH_CFG2_PRFTCH_MASK;		/* prefetch mode active HI */
	else
		tempReg &= ~MV_PMFLASH_CFG2_PRFTCH_MASK;

	/* write the new register value */
	if ((ret = mvPMFlashSetConfig2(pFlash, tempReg)) != MV_OK)
		return ret;

	return MV_OK;
}

/*******************************************************************************
* mvPMFlashShutdownSet - Shutdown the voltage regulator in the flash device
*
* DESCRIPTION:
*       Causes the device to enter in a power save mode untill the next access 
*		to the flash.
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvPMFlashShutdownSet(MV_MFLASH_INFO *pFlash)
{
	if (pFlash == NULL)
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	return mvPMFlashEraseSeq(pFlash, MV_PMFLASH_OPCD_SHUTDOWN);	
}

/*******************************************************************************
* mvPMFlashIdGet - Retreive the MFlash manufacturer and device IDs
*
* DESCRIPTION:
*       Read from the Mflash the 32bit manufacturer ID and the 16bit devide Id.
*
* INPUT:
*       pFlash: Structure with Marvell Flash information
*       pManfCode: pointer to the 32bit to fill the manufacturer Id
*       pDevCode: pointer to the 16bit to fill the device Id
*
* OUTPUT:
*       pManfCode: pointer to the 32bit holding the manufacturer Id
*       pDevCode: pointer to the 16bit holding the device Id
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvPMFlashIdGet (MV_MFLASH_INFO *pFlash, MV_U32 * pManfCode, MV_U16 * pDevCode)
{
	MV_U32 temp;	
    MV_STATUS ret;

	if ((pFlash == NULL) || (pManfCode == NULL) || (pDevCode == NULL))
    {
        mvOsPrintf("%s ERROR: Null pointer parameter!\n", __FUNCTION__);
        return MV_BAD_PARAM;
    }

	/* first confiure the Inf Read Data length = 3 (i.e 3*16bit = 6bytes)
	  This is because we need to read 6 bytes (4bytes Manuf. ID and 2 bytes Dev ID) */
	MV_REG_WRITE(MV_PMFLASH_IF_CTRL_REG, MV_PMFLASH_READ_ID_16BIT_COUNT);

	/* set commnand opcode = prgCmnd */
	temp = MV_REG_READ(MV_PMFLASH_CMD_OPCODE_REG);
	temp = ((temp & ~MV_PMFLASH_CMD_OPCD_MASK) | MV_PMFLASH_OPCD_READ_ID | MV_PMFLASH_EAD9_8_MASK);
	MV_REG_WRITE(MV_PMFLASH_CMD_OPCODE_REG, temp);

	/* Trigger the erase operation - set bit 13 to 1 */
	MV_REG_BIT_SET(MV_PMFLASH_CMD_OPCODE_REG, MV_PMFLASH_INF_READ_STRT_MASK);
	
	/* block waiting for bit to be cleared by hardware */
	if ((ret = waitOnInfReadBitClear()) != MV_OK)
		return ret;

	/* read the 6bytes from the Data register into the buffer in 3 iterations */
	/* First re-organize the Manufacturer ID */
	*pManfCode = 0;
	temp = MV_REG_READ(MV_PMFLASH_DATA_REG);		/* First 16bit */
	*pManfCode |= ((temp << 24) & 0xFF000000);
	*pManfCode |= ((temp << 8)  & 0x00FF0000);
	temp = MV_REG_READ(MV_PMFLASH_DATA_REG);		/* Second 16bit */
	*pManfCode |= ((temp << 8) & 0x0000FF00);
	*pManfCode |= ((temp >> 8) & 0x000000FF);

	/* Then re-organize the Device ID */
	*pDevCode = 0;
	temp = MV_REG_READ(MV_PMFLASH_DATA_REG);		/* Third 16bit */
	*pDevCode |= ((temp << 8) & 0x0000FF00);
	*pDevCode |= ((temp >> 8) & 0x000000FF);

	/* Set the word count back to the defualt for IREAD commands = 4*16bit */
	MV_REG_WRITE(MV_PMFLASH_IF_CTRL_REG, MV_PMFLASH_IREAD_16BIT_COUNT);

	return MV_OK;
}


