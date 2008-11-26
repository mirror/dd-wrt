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

#ifndef __INCmvMFlashH
#define __INCmvMFlashH

#include "mvCommon.h"
#include "mvOs.h"
#include "ctrlEnv/mvCtrlEnvSpec.h"

/* enumerations */
typedef enum
{
    MV_MFLASH_MODEL_UNKNOWN = 0,
	MV_MFLASH_SUNOL_1 = 1,
	MV_MFLASH_SUNOL_2 = 2,
	MV_MFLASH_SUNOL_3 = 3
} MV_MFLASH_MODEL;

typedef enum
{
    MV_MFLASH_SPI,
    MV_MFLASH_PARALLEL
} MV_MFLASH_MODE;

/* Type Definitions */
typedef struct 
{
    MV_MFLASH_MODE          ifMode;         /* Interface mode to the MFlash chip */
    MV_U32					baseAddr;       /* Flash Base Address */
	MV_MFLASH_MODEL  		flashModel;		/* Marvell flash model enumeration */
    MV_U32                  sectorSize;     /* Size of each sector in the main region */
    MV_U32                  sectorNumber;   /* Number of sectors in Main region */
    MV_U32                  infoSize;       /* Size of the Information region */
} MV_MFLASH_INFO;

/* Function Prototypes */

/* Init */
MV_STATUS	mvMFlashInit			(MV_MFLASH_INFO *pFlash);

/* Erase */
MV_STATUS 	mvMFlashChipErase  	    (MV_MFLASH_INFO *pFlash); /* Erase the whole chip */
MV_STATUS 	mvMFlashMainErase	  	(MV_MFLASH_INFO *pFlash); /* Erase the whole main region */
MV_STATUS 	mvMFlashInfErase	  	(MV_MFLASH_INFO *pFlash); /* Erase the whole information region */
MV_STATUS 	mvMFlashSecErase 		(MV_MFLASH_INFO *pFlash, MV_U32 secNumber); /* Single Main sector erase */

/* write to main and information regions regardless of allignment */
MV_STATUS	mvMFlashBlockWr 		(MV_MFLASH_INFO *pFlash, MV_U32 offset, MV_U32 blockSize, MV_U8 *pBlock, MV_BOOL verify);
MV_STATUS	mvMFlashInfBlockWr 	    (MV_MFLASH_INFO *pFlash, MV_U32 offset, MV_U32 blockSize, MV_U8 *pBlock, MV_BOOL verify);

/* read from main and information regions */
MV_STATUS	mvMFlashBlockRd 		(MV_MFLASH_INFO *pFlash, MV_U32 offset, MV_U32 blockSize, MV_U8 *pBlock);
MV_STATUS	mvMFlashBlockInfRd 	    (MV_MFLASH_INFO *pFlash, MV_U32 offset, MV_U32 blockSize, MV_U8 *pBlock);

/* Configuration registers Read/Write */
MV_STATUS	mvMFlashReadConfig	    (MV_MFLASH_INFO *pFlash, MV_U32 regNum, MV_U8 * pConfigReg);
MV_STATUS	mvMFlashSetConfig		(MV_MFLASH_INFO *pFlash, MV_U32 regNum, MV_U8 configReg);
MV_STATUS	mvMFlashSetSlewRate	    (MV_MFLASH_INFO *pFlash, MV_U8 configReg);

/* write protection */
MV_STATUS 	mvMFlashWriteProtectSet (MV_MFLASH_INFO *pFlash, MV_BOOL wp);
MV_STATUS 	mvMFlashWriteProtectGet (MV_MFLASH_INFO *pFlash, MV_BOOL * pWp);

/* Set the Sector Size option */
MV_STATUS 	mvMFlashSectorSizeSet	(MV_MFLASH_INFO *pFlash, MV_U32 secSize);

/* Prefetch Operation Set */
MV_STATUS	mvMFlashPrefetchSet     (MV_MFLASH_INFO *pFlash, MV_BOOL prefetch);

/* Shutdown/operate (true/false) the device */
MV_STATUS 	mvMFlashShutdownSet     (MV_MFLASH_INFO *pFlash);

/* Retreive the Flash ID */
MV_STATUS	mvMFlashIdGet			(MV_MFLASH_INFO *pFlash, MV_U32 * pManfId, MV_U16 * pDevId);

/* Peform a hard reset to the MFlash and Interface glue */
MV_STATUS	mvMFlashReset			(MV_MFLASH_INFO *pFlash);

#endif /* __INCmvPMFlashH */
