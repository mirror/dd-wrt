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

#ifndef __INCmvPMFlashH
#define __INCmvPMFlashH

#include "mvMFlash.h"

/* Parallel mode function prototypes */

/* Init */
MV_STATUS	mvPMFlashInit			(MV_MFLASH_INFO *pFlash);

/* Erase */
MV_STATUS 	mvPMFlashChipErase  	(MV_MFLASH_INFO *pFlash); /* Erase the whole chip */
MV_STATUS 	mvPMFlashMainErase	  	(MV_MFLASH_INFO *pFlash); /* Erase the whole main region */
MV_STATUS 	mvPMFlashInfErase	  	(MV_MFLASH_INFO *pFlash); /* Erase the whole information region */
MV_STATUS 	mvPMFlashSecErase 		(MV_MFLASH_INFO *pFlash, MV_U32 secNumber); /* Single Main sector erase */

/* write */
MV_STATUS   mvPMFlash64bWr          (MV_MFLASH_INFO *pFlash, MV_U32 offset, MV_U8 *pBlock);
MV_STATUS   mvPMFlash64bWrVerify    (MV_MFLASH_INFO *pFlash, MV_U32 offset, MV_U8 *pBlock);
MV_STATUS   mvPMFlash64bInfWr       (MV_MFLASH_INFO *pFlash, MV_U32 offset, MV_U8 *pBlock);
MV_STATUS   mvPMFlash64bInfWrVerify (MV_MFLASH_INFO *pFlash, MV_U32 offset, MV_U8 *pBlock);

/* Read */
MV_STATUS   mvPMFlashBlockRd        (MV_MFLASH_INFO *pFlash, MV_U32 offset, \
                                     MV_U32 blockSize, MV_U8 *pBlock);
MV_STATUS   mvPMFlashBlockInfRd     (MV_MFLASH_INFO *pFlash, MV_U32 offset, \
                                     MV_U32 blockSize, MV_U8 *pBlock);

/* Configuration registers Read/Write */
MV_STATUS	mvPMFlashReadConfig1	(MV_MFLASH_INFO *pFlash, MV_U8 * pConfigReg);
MV_STATUS	mvPMFlashReadConfig2	(MV_MFLASH_INFO *pFlash, MV_U8 * pConfigReg);
MV_STATUS	mvPMFlashReadConfig3 	(MV_MFLASH_INFO *pFlash, MV_U8 * pConfigReg);
MV_STATUS	mvPMFlashReadConfig4	(MV_MFLASH_INFO *pFlash, MV_U8 * pConfigReg);
MV_STATUS	mvPMFlashSetConfig1		(MV_MFLASH_INFO *pFlash, MV_U8 configReg);
MV_STATUS	mvPMFlashSetConfig2		(MV_MFLASH_INFO *pFlash, MV_U8 configReg);
MV_STATUS	mvPMFlashSetConfig3		(MV_MFLASH_INFO *pFlash, MV_U8 configReg);
MV_STATUS	mvPMFlashSetConfig4		(MV_MFLASH_INFO *pFlash, MV_U8 configReg);
MV_STATUS	mvPMFlashSetSlewRate	(MV_MFLASH_INFO *pFlash, MV_U8 configReg);

/* write protection */
MV_STATUS 	mvPMFlashWriteProtectSet(MV_MFLASH_INFO *pFlash, MV_BOOL wp);
MV_STATUS 	mvPMFlashWriteProtectGet(MV_MFLASH_INFO *pFlash, MV_BOOL * pWp);

/* Set the Sector Size option */
MV_STATUS 	mvPMFlashSectorSizeSet	(MV_MFLASH_INFO *pFlash, MV_U32 secSize);

/* Prefetch Operation Set */
MV_STATUS	mvPMFlashPrefetchSet    (MV_MFLASH_INFO *pFlash, MV_BOOL prefetch);

/* Shutdown/operate (true/false) the device */
MV_STATUS 	mvPMFlashShutdownSet    (MV_MFLASH_INFO *pFlash);

/* Retreive the Flash ID */
MV_STATUS	mvPMFlashIdGet			(MV_MFLASH_INFO *pFlash, MV_U32 * pManfCode, MV_U16 * pDevCode);

#endif /* __INCmvPMFlashH */
