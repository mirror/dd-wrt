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

#ifndef __INCmvIntelFlashH
#define __INCmvIntelFlashH

#include "mvFlashCom.h"


/* Mt Flash IDs */
#define MT_FID_28F400_T	0x4470	/* 28F400B3 ID ( 4 M, top boot sector)		*/
#define MT_FID_28F400_B	0x4471	/* 28F400B3 ID ( 4 M, bottom boot sect) 	*/


/* Intel Flash IDs */
#define INTEL_FID_28F016S    0x66a0	/* 28F016S[VS] ID (16M = 512k x 16)	*/
#define INTEL_FID_28F800B3T  0x8892	/*  8M = 512K x 16 top boot sector	*/
#define INTEL_FID_28F800B3B  0x8893	/*  8M = 512K x 16 bottom boot sector	*/
#define INTEL_FID_28F160B3T  0x8890	/*  16M = 1M x 16 top boot sector	*/
#define INTEL_FID_28F160B3B  0x8891	/*  16M = 1M x 16 bottom boot sector	*/
#define INTEL_FID_28F320B3T  0x8896	/*  32M = 2M x 16 top boot sector	*/
#define INTEL_FID_28F320B3B  0x8897	/*  32M = 2M x 16 bottom boot sector	*/
#define INTEL_FID_28F640B3T  0x8898	/*  64M = 4M x 16 top boot sector	*/
#define INTEL_FID_28F640B3B  0x8899	/*  64M = 4M x 16 bottom boot sector	*/
#define INTEL_FID_28F160F3B  0x88F4	/*  16M = 1M x 16 bottom boot sector	*/

#define INTEL_FID_28F800C3T  0x88C0	/*  8M = 512K x 16 top boot sector	*/
#define INTEL_FID_28F800C3B  0x88C1	/*  8M = 512K x 16 bottom boot sector	*/
#define INTEL_FID_28F160C3T  0x88C2	/*  16M = 1M x 16 top boot sector	*/
#define INTEL_FID_28F160C3B  0x88C3	/*  16M = 1M x 16 bottom boot sector	*/
#define INTEL_FID_28F320C3T  0x88C4	/*  32M = 2M x 16 top boot sector	*/
#define INTEL_FID_28F320C3B  0x88C5	/*  32M = 2M x 16 bottom boot sector	*/
#define INTEL_FID_28F640C3T  0x88CC	/*  64M = 4M x 16 top boot sector	*/
#define INTEL_FID_28F640C3B  0x88CD	/*  64M = 4M x 16 bottom boot sector	*/

#define INTEL_FID_28F128J3   0x8918	/*  16M = 8M x 16 x 128 		*/
#define INTEL_FID_28F320J5   0x0014	/*  32M = 128K x  32			*/
#define INTEL_FID_28F640J5   0x0015	/*  64M = 128K x  64			*/
#define INTEL_FID_28F320J3A  0x0016	/*  32M = 128K x  32			*/
#define INTEL_FID_28F640J3A  0x0017	/*  64M = 128K x  64			*/
#define INTEL_FID_28F128J3A  0x0018	/* 128M = 128K x 128			*/
#define INTEL_FID_28F256L18T 0x880D	/* 256M = 128K x 255 + 32k x 4 		*/

#define INTEL_FID_28F160S3   0x00D0	/*  16M = 512K x  32 (64kB x 32)	*/
#define INTEL_FID_28F320S3   0x00D4	/*  32M = 512K x  64 (64kB x 64)	*/
#define INTEL_FID_28F128P30T 0x8818	/*  16M = 128K x  127 (32kB x 4) top boot sector */
#define INTEL_FID_28F128P30B 0x881B	/*  16M = 128K x  127 (32kB x 4) bottom boot sector */
#define INTEL_FID_28F256P30T 0x8919	/*  32M = 128K x  255 (32kB x 4) top boot sector */
#if defined (DB_88F1281)
#define INTEL_FID_28F256P30B 0x1C	/*  16M = 64K x  255 (16kB x 4) bottom boot sector */
#else
#define INTEL_FID_28F256P30B 0x891C	/*  32M = 128K x  255 (32kB x 4) bottom boot sector */
#endif


/* Intel Flash APIs timeouts*/
#define INTEL_EARASE_MILI_TIMEOUT		(8000*10) 	/* mili Sec 		*/
#define INTEL_PROG_TIMEOUT				(0xA0000*10)	/* number of loops 	*/
#define INTEL_LOCK_MILI_TIMEOUT			(8000*10) 	/* mili Sec		*/

/* Commands */
#define INTEL_CHIP_CMD_RST			0xFF	/* reset flash 					*/
#define INTEL_CHIP_CMD_RD_ID		0x90	/* read the id and lock bits 	*/
#define INTEL_CHIP_CMD_RD_QUERY		0x98	/* read device capabilities 	*/
#define INTEL_CHIP_CMD_RD_STAT		0x70	/* read the status register 	*/
#define INTEL_CHIP_CMD_CLR_STAT		0x50	/* clear the staus register		*/
#define INTEL_CHIP_CMD_WR_BUF		0xE8	/* write buffer command			*/
#define INTEL_CHIP_CMD_CONFIRM_BUF	0xD0	/* write buffer command			*/
#define INTEL_CHIP_CMD_PROG			0x40	/* program word command 		*/
#define INTEL_CHIP_CMD_ERASE1		0x20	/* 1st word for block erase 	*/
#define INTEL_CHIP_CMD_ERASE2		0xD0	/* 2nd word for block erase 	*/
#define INTEL_CHIP_CMD_ERASE_SUSP	0xB0	/* suspend block erase 			*/
#define INTEL_CHIP_CMD_LOCK			0x60	/* 1st word for all lock cmds 	*/
#define INTEL_CHIP_CMD_SET_LOCK_BLK	 0x01	/* 2nd wrd set block lock bit 	*/
#define INTEL_CHIP_CMD_SET_LOCK_MSTR 0xF1	/* 2nd wrd set master lck bit 	*/
#define INTEL_CHIP_CMD_CLR_LOCK_BLK	0xD0	/* 2nd wrd clear blk lck bit 	*/



/* status register bits */
#define INTEL_CHIP_STAT_DPS		0x02		/* Device Protect Status 		*/
#define INTEL_CHIP_STAT_VPPS	0x08		/* VPP Status 					*/
#define INTEL_CHIP_STAT_PSLBS	0x10		/* Program+Set Lock Bit Stat 	*/
#define INTEL_CHIP_STAT_ECLBS	0x20		/* Erase+Clr Lock Bit Stat 		*/
#define INTEL_CHIP_STAT_ESS		0x40		/* Erase Suspend Status 		*/
#define INTEL_CHIP_STAT_RDY		0x80		/* WSM Mach Status, 1=rdy 		*/

#define INTEL_CHIP_STAT_ERR		(INTEL_CHIP_STAT_VPPS | INTEL_CHIP_STAT_DPS | \
				    INTEL_CHIP_STAT_ECLBS | INTEL_CHIP_STAT_PSLBS)

/* Lock Configuration */
#define INTEL_CHIP_RD_ID_LOCK		0x01		/* Bit 0 of each byte 		*/

MV_STATUS intelFlashSecErase(MV_FLASH_INFO *pFlash, MV_U32 secNum);
MV_VOID   intelFlashReset(MV_FLASH_INFO *pFlash);
MV_STATUS intelFlashProg(MV_FLASH_INFO *pFlash,MV_U32 offset, MV_U32 data);
MV_BOOL   intelFlashSecLockGet(MV_FLASH_INFO *pFlash,MV_U32 secNum);
MV_STATUS intelFlashSecLock(MV_FLASH_INFO *pFlash, MV_U32 secNum, 
																MV_BOOL enable);
MV_U32 intelFlashGetHwBuffSize(MV_FLASH_INFO *pFlash);
MV_STATUS intelFlashHwBufferProg(MV_FLASH_INFO *pFlash,MV_U32 offset, MV_U32 byteCount, 
																		MV_U8* pData);

#endif /* __INCmvIntelFlashH */
