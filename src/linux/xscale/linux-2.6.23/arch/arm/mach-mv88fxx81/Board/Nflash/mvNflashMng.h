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
#ifndef __INCmvNflashMngH
#define __INCmvNflashMngH

#include "mvNflashDev.h"
#include "mvCtrlEnvLib.h"



/* typedefs */

/* This struct describes a Flash device     */
typedef struct _mvNflashInfo
{
    MV_NFLASH_INFO	nflashInfo;	/* Flash information             */
    NFLASH_BLK_NODE	*pFlashBlk;	/* Flash block list             */
} MV_NFLASH_MNG;

/* This structure describes a flash Block for block replacement algorithem	*/
typedef struct _nflashBlkNode;
{
    MV_BOOL     valid		/* Block validity					*/
	MV_BOOL     erased		/* Is block erased					*/
    MV_BOOL     locked      /* Is this block locked             */
	NFLASH_BLK	nflashBlk;	/* Nflash device block 				*/
	NFLASH_BLK	*pNextBlk;	/* Pointer to Next block in device	*/
}NFLASH_BLK_NODE;

/* This structure describes a flash page for ECC implementation           */
typedef struct _nflashPage
{
    MV_U8  	plainAData[ECC_BLOCK_SIZE]; /* Plain A data 0-255 bytes   */
    MV_U8  	plainBData[ECC_BLOCK_SIZE];	/* Plain B data 256-511 bytes */
    MV_U8	plainAEcc[ECC_SIZE];		/* Plain A ECC array		  */
    MV_U8	plainBEcc[ECC_SIZE];		/* Plain B ECC array		  */
}NFLASH_PAGE;


/* Note that the block numbers in the following APIs are virtual block 	*/
/* numbers and offsets are virtual offsets.								*/
/* get info */
MV_U32      mvNflashNumOfDevGet (MV_FLASH_INFO *pFlash);
MV_BOOL	    mvNflashIsHwLock    (MV_FLASH_INFO *pFlash);
MV_BOOL     mvNflashBlkLockGet  (MV_FLASH_INFO *pFlash, MV_U32 vBlkNum);
MV_U32      mvNflashInWhichBlk  (MV_FLASH_INFO *pFlash, MV_U32 vOffs);
MV_U32      mvNflashBlkSizeGet  (MV_FLASH_INFO *pFlash, MV_U32 vBlkNum);
MV_U32      mvNflashNumOfBlkGet (MV_FLASH_INFO *pFlash);
MV_U32      mvNflashSizeGet     (MV_FLASH_INFO *pFlash);
MV_U32      mvNflashBlkOffsGet  (MV_FLASH_INFO *pFlash, MV_U32 vBlkNum);
MV_U32      mvNflashVenIdGet    (MV_FLASH_INFO *pFlash);
MV_U32      mvNflashDevIdGet    (MV_FLASH_INFO *pFlash);
MV_U32      mvNflashBaseAddrGet (MV_FLASH_INFO *pFlash);
MV_U32      mvNflashBusWidthGet (MV_FLASH_INFO *pFlash);
MV_U32		mvNflashDevWidthGet	(MV_FLASH_INFO *pFlash);

MV_VOID	nflashPrint  (MV_FLASH_INFO *pFlash);

/** APIs **/
/* Init */
MV_U32	  	mvNflashInit		(MV_FLASH_INFO *pFlash);
/* Erase */
MV_STATUS 	mvNflashErase	  	(MV_FLASH_INFO *pFlash);
MV_STATUS 	mvNflashBlkErase	(MV_FLASH_INFO *pFlash, MV_U32 vBlkNum);
MV_BOOL   	mvNflashIsBlkErased (MV_FLASH_INFO *pFlash, MV_U32 vBlkNum);
/* read/write */
MV_U32 	    mvNflashBlockWr 	(MV_FLASH_INFO *pFlash, MV_U32 vOffs, 
											MV_U32 blockSize, MV_U8 *pBlock);
MV_U32 		mvNflashBlockRd 	(MV_FLASH_INFO *pFlash, MV_U32 vOffs, 
											MV_U32 blockSize, MV_U8 *pBlock);
/* protection */
MV_STATUS mvNflashBlkLockSet(MV_FLASH_INFO *pFlash, MV_U32 vBlkNum, 
							                                LOCK_MODE lockMode);


#endif /* __INCmvNflashMngH */
