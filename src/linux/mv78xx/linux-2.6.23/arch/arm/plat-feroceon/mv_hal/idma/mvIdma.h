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

/*******************************************************************************
* mvIdma.h - Header File for :
*
* DESCRIPTION:
*       This file contains Marvell Controller IDMA HW library API.
*       NOTE: This HW library API assumes IDMA source, destination and 
*       descriptors are cache coherent. 
*
* DEPENDENCIES:
*       None.
*
*******************************************************************************/


#ifndef __INCmvIdmah
#define __INCmvIdmah

#include "mvCommon.h"
#include "mvOs.h"
#include "idma/mvIdmaRegs.h"
#include "ctrlEnv/mvCtrlEnvSpec.h"

/* defines  */
#define MV_IDMA_DESC_ALIGNMENT  0x10  /* 16bytes aligment restriction       */

/* typedefs */


/* This struct describes IDMA descriptor structure                          */
typedef struct _mvDmaDesc
{
    MV_U32 	byteCnt;        /* The total number of bytes to transfer        */
    MV_U32 	phySrcAdd;	    /* The physical source address                  */
    MV_U32 	phyDestAdd;     /* The physical destination address             */
    MV_U32	phyNextDescPtr; /* If we are using chain mode DMA transfer,     */
				            /* then this pointer should point to the        */
                            /* physical address of the next descriptor,     */
                            /* otherwise it should be NULL.                 */
}MV_DMA_DESC;


/* mvIdma.h API list */
MV_VOID mvDmaHalInit (MV_U32);
MV_STATUS mvDmaCtrlLowSet (MV_U32 chan, MV_U32 ctrlWord);
MV_STATUS mvDmaCtrlHighSet(MV_U32 chan, MV_U32 ctrlWord);
MV_STATUS mvDmaTransfer(MV_U32 chan, MV_U32 phySrc, MV_U32 phyDst, MV_U32 size, 
                                                    MV_U32 phyNextDescPtr);
MV_STATUS mvDmaMemInit(MV_U32 chan, MV_U32 ptrnPtr, MV_U32 startPtr, MV_U32 size);
MV_STATE  mvDmaStateGet(MV_U32 chan);
MV_STATUS mvDmaCommandSet(MV_U32 chan, MV_COMMAND command);

MV_VOID mvIdmaRegs(MV_U32 chan);

#endif /* __INCmvIdmah */


