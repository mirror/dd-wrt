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

#ifndef __INCmvAmdFlashH
#define __INCmvAmdFlashH

#include "mvFlashCom.h"


/* AMD Flash IDs */
#define AMD_FID_LV040B		0x4F		/* 29LV040B ID				*/
						/* 4 Mbit, 512K x 8,			*/
						/* 8 64K x 8 uniform sectors		*/

#define AMD_FID_F040B		0xA4		/* 29F040B ID				*/
						/* 4 Mbit, 512K x 8,			*/
						/* 8 64K x 8 uniform sectors		*/
#define STM_FID_M29W040B 	0xE3		/* M29W040B ID				*/
						/* 4 Mbit, 512K x 8,			*/
						/* 8 64K x 8 uniform sectors		*/
#define AMD_FID_F080B		0xD5		/* 29F080  ID  ( 1 M)			*/
						/* 8 Mbit, 512K x 16,			*/
						/* 8 64K x 16 uniform sectors		*/
#define AMD_FID_F016D		0xAD	/* 29F016  ID  ( 2 M x 8)		*/
#define AMD_FID_F032B		0x41	/* 29F032  ID  ( 4 M x 8)		*/
#define AMD_FID_LV116DT		0xC7	/* 29LV116DT   ( 2 M x 8, top boot sect)*/
#define AMD_FID_LV016B		0xc8	/* 29LV016 ID  ( 2 M x 8)		*/

#define AMD_FID_LV400T		0x22B9	/* 29LV400T ID ( 4 M, top boot sector)	*/
#define AMD_FID_LV400B		0x22BA	/* 29LV400B ID ( 4 M, bottom boot sect) */

#define AMD_FID_LV033C		0xA3	/* 29LV033C ID ( 4 M x 8)		*/
#define AMD_FID_LV065D		0x93	/* 29LV065D ID ( 8 M x 8)		*/

#define AMD_FID_LV800T		0x22DA	/* 29LV800T ID ( 8 M, top boot sector)	*/
#define AMD_FID_LV800B		0x225B	/* 29LV800B ID ( 8 M, bottom boot sect) */

#define AMD_FID_LV160T		0x22C4	/* 29LV160T ID (16 M, top boot sector)	*/
#define AMD_FID_LV160B		0x2249	/* 29LV160B ID (16 M, bottom boot sect) */

#define AMD_FID_LV320T		0x22F6	/* 29LV320T ID (32 M, top boot sector)	*/
#define AMD_FID_LV320B		0x22F9	/* 29LV320B ID (32 M, bottom boot sect) */

#define AMD_FID_DL322T		0x2255	/* 29DL322T ID (32 M, top boot sector)	*/
#define AMD_FID_DL322B		0x2256	/* 29DL322B ID (32 M, bottom boot sect) */
#define AMD_FID_DL323T		0x2250	/* 29DL323T ID (32 M, top boot sector)	*/
#define AMD_FID_DL323B		0x2253	/* 29DL323B ID (32 M, bottom boot sect) */
#define AMD_FID_DL324T		0x225C	/* 29DL324T ID (32 M, top boot sector)	*/
#define AMD_FID_DL324B		0x225F	/* 29DL324B ID (32 M, bottom boot sect) */

#define AMD_S29GL128N		0x227E  /* S29GL128N spansion 128Mbit flash     */
#define AMD_FID_DL640		0x227E	/* 29DL640D ID (64 M, dual boot sectors)*/
#define AMD_FID_MIRROR		0x227E	/* 1st ID word for MirrorBit family 	*/
#define AMD_FID_LV640U_2	0x220C	/* 2d ID word for AM29LV640M at 0x38	*/
#define AMD_FID_LV640U_3	0x2201	/* 3d ID word for AM29LV640M at 0x3c 	*/
#define AMD_FID_LV128U_2 	0x2212	/* 2d ID word for AM29LV128M at 0x38 	*/
#define AMD_FID_LV128U_3 	0x2200	/* 3d ID word for AM29LV128M at 0x3c 	*/

#define AMD_FID_LV640U	 	0x22D7	/* 29LV640U ID (64 M, uniform sectors)	*/

#define STM_FID_29W040B  	0xE3	/* M29W040B ID (4M = 512K x 8)  */

/* SST Flash IDs */
#define SST_39VF_020 		0xD6	/* SST39VF020 (256KB = 4K * 64) */


/* Amd Flash APIs */
#define AMD_EARASE_MILI_TIMEOUT	8000		/* mili seconds 	*/
#define AMD_PROG_TIMEOUT		0xA0000 	/* number of loops 	*/

/* Commands */
#define AMD_CHIP_CMD_RST		0xF0		/* reset flash 			*/
#define AMD_CHIP_UNLOCK_CMD1	0xAA		/* 1st data for unlock 		*/
#define AMD_CHIP_UNLOCK_ADDR1	0x555		/* 1st addr for unlock 		*/
#define SST_CHIP_UNLOCK_ADDR1	0x5555		/* 1st addr for unlock 		*/
#define AMD_CHIP_UNLOCK_CMD2	0x55		/* 2nd data for unlock 		*/
#define AMD_CHIP_UNLOCK_ADDR2	0x2AA		/* 2nd addr for unlock 		*/
#define SST_CHIP_UNLOCK_ADDR2	0x2AAA		/* 2nd addr for unlock 		*/
#define AMD_CHIP_CMD_PROG		0xA0		/* 1st data for program command */
#define AMD_CHIP_ADDR_PROG		0x555		/* 1st addr for program command */
#define SST_CHIP_ADDR_PROG		0x5555		/* 1st addr for program command */
#define AMD_CHIP_CMD_ERASE1		0x80		/* 1st data for erase command 	*/
#define AMD_CHIP_ADDR_ERASE1	0x555		/* 1st addr for erase command 	*/
#define SST_CHIP_ADDR_ERASE1	0x5555		/* 1st addr for erase command 	*/
#define AMD_CHIP_CMD_ERASE2		0xAA		/* 2nd data for erase command 	*/
#define AMD_CHIP_ADDR_ERASE2	0x555		/* 2nd addr for erase command 	*/
#define SST_CHIP_ADDR_ERASE2	0x5555		/* 2nd addr for erase command 	*/
#define AMD_CHIP_CMD_ERASE3		0x55		/* 3rd data for erase command 	*/
#define AMD_CHIP_ADDR_ERASE3	0x2AA		/* 3rd addr for erase command 	*/
#define SST_CHIP_ADDR_ERASE3	0x2AAA		/* 3rd addr for erase command 	*/
#define AMD_CHIP_CMD_ERASE4		0x30		/* 4rd data for erase command 	*/

/* there are few ways to check if the AMD flash is busy or not:				*/
/* 1) by checking DQ7 [and 5 - optional ]						*/
/* 2) by checking toggle bit DQ7 (linux)						*/
/* here we used the first option.			 				*/
/* status register bits */
#define AMD_CHIP_STAT_DQ7_MASK	0x80		/* Device is ready 		*/

/* ID and Lock Configuration */
#define AMD_CHIP_RD_ID_MAN		0x01		/* Manufacturer code = 0x89 	*/

MV_STATUS amdFlashSecErase(MV_FLASH_INFO *pFlash, MV_U32 secNum);
MV_VOID amdFlashReset(MV_FLASH_INFO *pFlash);
MV_STATUS amdFlashProg(MV_FLASH_INFO *pFlash,MV_U32 offset, MV_U32 data);

#endif /* __INCmvAmdFlashH */
