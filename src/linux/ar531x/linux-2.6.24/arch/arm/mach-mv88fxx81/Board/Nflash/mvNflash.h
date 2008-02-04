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
#ifndef __INCmvNflashProtH
#define __INCmvNflashProtH

#include "mvCommon.h"
#include "mvNflashHwIf.h"

/* Vendor ID */
#define SAMSUNG_MANUF		0xEC	/* Samsung ID code */

/* SAMSUNG NFLASH device codes */
#define K9F5608Q0C 0x35
#define K9F5608D0C 0x75
#define K9F5608U0C 0x75
#define K9F5616Q0C 0x45	/* Two upper bytes are don't care */
#define K9F5616D0C 0x55	/* Two upper bytes are don't care */
#define K9F5616U0C 0x55	/* Two upper bytes are don't care */

/* Offsets inside device and vendor ID word (word=16-bit) */
#define DEVICE_ID_OFFS	0
#define VENDOR_ID_OFFS	8

/* NAND flash constants Assaf, move to init table (for each NFLASH) */
#define NFLASH_PAGE_SIZE		 528	/* x8 and x16 page size is 582bytes	*/
#define NFLASH_DATA_PLAIN_SIZE	 256	/* x8=2plain x16=1plain				*/
#define NFLASH_PAGE_DATA_SIZE	 512	/* x8 and x16 data size is 512bytes	*/
#define NFLASH_SPARE_SIZE        16     /* 16 bytes spare for each page     */
#if 0
#define NFLASH_BLOCK_PAGES		 32		/* 1block  = 32pages				*/
#define NFLASH_DEVICE_BLOCKS	 2048	/* 1device = 2048 blocks			*/
#endif
#define NFLASH_X8_INV_BLK_OFFS	 6		/* Byte offset in spare area for 	*/
										/* invalid blocks for x8 devices	*/
#define NFLASH_X16_INV_BLK_OFFS1 0   	/* word offset 1 in spare area for 	*/
										/* invalid blocks for x16 devices	*/
#define NFLASH_X16_INV_BLK_OFFS2 4   	/* word offset 2 in spare area for 	*/
										/* invalid blocks for x16 devices	*/
										
/* NAND Flash APIs timeouts*/
#define NFLASH_PROG_ERASE_TIMEOUT 	(80000)     /* number of loops 		*/
#define NFLASH_tAR_DELAY             1          /* Micro-second delay   */

/* NAND Flash invalid sector identifer */
#define NFLASH_X8_INV_BLK_MASK	0xFF
#define NFLASH_X16_INV_BLK_MASK	0xFFFF

/* NAND Command sets */
#define READ_A_PLAIN		0x00	/* Read page plain A (x8 0-255 bytes)   */
									/* (x16 0-255 words)                    */ 
#define READ_B_PLAIN		0x01	/* Read page plain B (x8 256-511 bytes) */
#define READ_SPARE			0x50	/* Read page spare area                 */
#define READ_ID				0x90	/* Get Flash device and vendor ID		*/
#define RESET_FLASH     	0xFF    /* Reset Flash							*/
#define PAGE_PROGRAM_CMD1 	0x80    /* Page program 1st command				*/
#define PAGE_PROGRAM_CMD2 	0x10    /* Page program 2nd command				*/
#define COPY_BACK_PGR_CMD1  0x00    /* Copy-Back program 1st command		*/
#define COPY_BACK_PGR_CMD2  0x8A    /* Copy-Back program 2nd command		*/
#define FLASH_LOCK 			0x2A	/* Lock all Flash blocks                */
#define BLOCK_UNLOCK_CMD1	0x23	/* Unlock block sequance 1st command	*/ 
#define BLOCK_UNLOCK_CMD2	0x24	/* Unlock block sequance 2nd command	*/
#define FLASH_LOCK_TIGHT 	0x2C	/* Tightly lock all Flash blocks		*/
#define READ_BLOCK_LOCK_STS 0x7A	/* Get block lock status				*/
#define BLOCK_ERASE_CMD1 	0x60	/* Block erase 1st command				*/
#define BLOCK_ERASE_CMD2 	0xD0	/* Block erase 2nd command				*/
#define READ_STATUS 		0x70	/* Get Page read status 				*/


/* Block Lock Status Register (BLSR) Definitions */
#define BLSR_LOCK_TIGHT_BIT				(1 << 0)
#define BLSR_LOCK_BIT					(1 << 1)
#define BLSR_UNLOCK_BIT					(1 << 2)

/* Read Status Register (RSR) Definition */
#define RSR_PROG_ERASE_OFFS		        0
#define RSR_PROG_ERASE_MASK		        (1 << RSR_PROG_ERASE_OFFS)
#define RSR_PROG_ERASE_SUCCESS		    (0 << RSR_PROG_ERASE_OFFS)
#define RSR_PROG_ERASE_ERROR			(1 << RSR_PROG_ERASE_OFFS)
#define RSR_DEV_OPERATION_OFFS		    6
#define RSR_DEV_OPERATION_MASK		    (1 << RSR_DEV_OPERATION_OFFS)
#define RSR_DEV_OPERATION_BUSY			(0 << RSR_DEV_OPERATION_OFFS)
#define RSR_DEV_OPERATION_READY			(1 << RSR_DEV_OPERATION_OFFS)
#define RSR_WRITE_PROTECT_OFFS		    7
#define RSR_WRITE_PROTECT_MASK          (1 << RSR_WRITE_PROTECT_OFFS)
#define RSR_WRITE_PROTECTED				(0 << RSR_WRITE_PROTECT_OFFS)
#define RSR_WRITE_NOT_PROTECTED			(1 << RSR_WRITE_PROTECT_OFFS)

typedef enum _lockMode
{
	BLOCK_TIGHTLOCK,
	BLOCK_LOCK,
	BLOCK_UNLOCK,
    BLOCK_UNDEFINED_LOCK,
}NFLASH_LOCK_MODE;

typedef enum _nflashReadSts
{
    NFLASH_ERASE_SUCC,
    NFLASH_ERASE_ERROR,
    NFLASH_BUSY,
    NFLASH_READY,
    NFLASH_WRITE_PROTECTED,
    NFLASH_WRITE_NOT_PROTECTED
}NFLASH_READ_STS;

/* This structure describes the Nand Flash device block */
typedef struct _nflashBlock
{
	MV_U32  	     baseOffs;  /* Block base offset related to flash base.	*/
    MV_U32  	     size;      /* Block size in bytes            			*/
    NFLASH_LOCK_MODE lockMode;  /* Block Lock protect indicator   			*/
}NFLASH_BLK;
		  
typedef struct _nflash_struct
{
    MV_U32      devVen;		    /* Vendor ID                        */    
    MV_U32      devId;	   	    /* device ID                        */
    MV_U32      size;      	    /* Total Flash data size in bytes   */
    MV_U32      blockNum;  	    /* Flash total block number         */
    MV_U32      pageDataSize;   /* Page Total size include spare    */
    MV_U32      spareSize;      /* Page spare area size             */
    MV_U32      pagesPerBlk;    /* Number of pages in one block     */
}NFLASH_STRUCT;

typedef struct _mvNflashInfo
{
	NFLASH_STRUCT   *pNflashStruct;	/* Nflash HW structure				*/
    MV_U32          baseAddr;   	/* Flash base address  				*/	/* Assaf delete? */
    MV_U32    	    devWidth;   	/* Width of single Flash device 	*/  /* Assaf delete? */
    MV_NFLASH_HW_IF	nflashHwIf;		/* NFlash HW interface identifier	*/
} MV_NFLASH_INFO;

/* Note that read/prog API are on a page basis */
MV_STATUS mvNflashInit(MV_NFLASH_INFO *pFlash);

MV_VOID   mvNflashReset(MV_NFLASH_INFO *pFlash);
MV_U32    mvNflashPageRead(MV_NFLASH_INFO *pFlash, MV_U32 offset, MV_U32 size, MV_U8 *pData);
MV_U32    mvNflashPageSpareRead(MV_NFLASH_INFO *pFlash, MV_U32 pageNum, MV_U32 offset, MV_U32 size, MV_U8 *pData);
MV_STATUS mvNflashPageProg(MV_NFLASH_INFO *pFlash, MV_U32 offset, MV_U32 size, MV_U8 *pData);
MV_STATUS mvNflashCpBackProg(MV_NFLASH_INFO *pFlash, MV_U32 srcOffs, MV_U32 dstOffs);
MV_STATUS mvNflashBlockErase(MV_NFLASH_INFO *pFlash, MV_U32 blkNum);
MV_VOID   mvNflashBlockLock(MV_NFLASH_INFO *pFlash);
MV_VOID   mvNflashBlockLockTight(MV_NFLASH_INFO *pFlash);
MV_VOID   mvNflashBlockUnlock(MV_NFLASH_INFO *pFlash, MV_U32 startBlkNum, MV_U32 endBlkNum);


MV_U16           mvNflashIdGet(MV_NFLASH_INFO *pFlash);
NFLASH_LOCK_MODE mvNflashBlockLockStsGet(MV_NFLASH_INFO *pFlash,MV_U32 blkNum);
NFLASH_READ_STS  mvNflashReadStsRegGet(MV_NFLASH_INFO *pFlash);
MV_U32           mvNflashBlkOffsGet(MV_NFLASH_INFO *pFlash, MV_U32 blkNum);
MV_VOID          mvNflashPrint(MV_NFLASH_INFO *pFlash);

/* This API hides the Spare area internal structure and provides all 	*/
/* information required by the managment layer for block replacement	*/
MV_BOOL	  mvNflashIsBlkErased  (MV_NFLASH_INFO *pFlash, MV_U32 blkNum);
MV_BOOL	  mvNflashIsBlkInvalid (MV_NFLASH_INFO *pFlash, MV_U32 blkNum);
MV_U32    mvNflashNextBlkNumGet(MV_NFLASH_INFO *pFlash, MV_U32 blkNum);
MV_STATUS mvNflashNextBlkNumSet(MV_NFLASH_INFO *pFlash, MV_U32 blkNum, MV_U32 nextBlkNum);

#endif /* __INCmvNflashProtH */
