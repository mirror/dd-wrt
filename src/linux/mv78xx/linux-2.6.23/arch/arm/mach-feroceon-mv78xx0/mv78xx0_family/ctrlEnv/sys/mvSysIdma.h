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
#ifndef __INCmvSysIdmah
#define __INCmvSysIdmah

#include "ctrlEnv/sys/mvCpuIf.h"
#include "ctrlEnv/mvCtrlEnvLib.h"
#include "ctrlEnv/mvCtrlEnvAddrDec.h"

/* General IDMA */
#define IDMA_MAX_ADDR_DEC_WIN	8	/* Maximum address decode windows		*/
#define IDMA_MAX_OVERRIDE_WIN	4	/* Maximum address override windows		*/

/*  IDMA Address Decoding Base and size Registers  							*/ 
#define IDMA_BASE_ADDR_REG(winNum)     		(0x60a00 + ((winNum) * 8))
#define IDMA_SIZE_REG(winNum)          		(0x60a04 + ((winNum) * 8))
 
/* IDMA Address Decoding High Address Remap,. Note that only window 0 - 3 	*/
/* has remap capabilities													*/
#define IDMA_HIGH_ADDR_REMAP_REG(winNum)    (0x60a60 + ((winNum) * 4))

/* IDMA Base Addres enable register*/
#define IDMA_BASE_ADDR_ENABLE_REG			0x60a80

/* IDMA Access Protection Registers                    						*/
#define IDMA_ACCESS_PROTECT_REG(chan)		(0x60a70 + ((chan) * 4))
  /*  IDMA Headers Retarget Registers   */
#define IDMA_HEADERS_RETARGET_CTRL_REG      0x60a84
#define IDMA_HEADERS_RETARGET_BASE_REG      0x60a88


/* Base Addr reg */
#define IDMA_WIN_TARGET_OFFS 0 /* The target interface associated with window*/
#define IDMA_WIN_TARGET_MASK (0xf << IDMA_WIN_TARGET_OFFS)
#define IDMA_WIN_ATTR_OFFS   8 /* The target attributes Associated with window*/
#define IDMA_WIN_ATTR_MASK   (0xff << IDMA_WIN_ATTR_OFFS)

/* IDMA Base Address Enable Register (IBAER)								*/
#define IBAER_ENABLE_OFFS			0
#define IBAER_ENABLE_MASK			(0xFF << IBAER_ENABLE_OFFS)
#define IBAER_ENABLE(winNum)		(1 << (winNum))

/* IDMA Channel Access Protect Register (ICAPR)*/
#define ICAPR_PROT_NO_ACCESS            NO_ACCESS_ALLOWED
#define ICAPR_PROT_READ_ONLY            READ_ONLY
#define ICAPR_PROT_FULL_ACCESS          FULL_ACCESS
#define ICAPR_PROT_WIN_OFFS(winNum)     (2 * (winNum))
#define ICAPR_PROT_WIN_MASK(winNum)     (0x3 << ICAPR_PROT_WIN_OFFS(winNum))

/* This struct describes address decode override types                      */
typedef enum _mvDmaOverride
{
    DMA_SRC_ADDR,           /* Override source address                      */
    DMA_DST_ADDR,           /* Override destination address                 */
    DMA_NEXT_DESC           /* Override next descriptor address             */
}MV_DMA_OVERRIDE;


typedef struct _mvDmaDecWin
{
        MV_TARGET     target;
        MV_ADDR_WIN   addrWin; /* An address window*/
        MV_BOOL       enable;  /* Address decode window is enabled/disabled */
 
}MV_DMA_DEC_WIN;

MV_STATUS mvDmaInit (MV_VOID);

MV_STATUS mvDmaWinSet(MV_U32 winNum, MV_DMA_DEC_WIN *pAddrDecWin);
MV_STATUS mvDmaWinGet(MV_U32 winNum, MV_DMA_DEC_WIN *pAddrDecWin);
MV_STATUS mvDmaWinEnable(MV_U32 winNum,MV_BOOL enable);
MV_U32  mvDmaWinTargetGet(MV_TARGET target);

MV_STATUS mvDmaProtWinSet (MV_U32 chan, MV_U32 winNum, MV_ACCESS_RIGHTS access);
MV_STATUS mvDmaOverrideSet(MV_U32 chan, MV_U32 winNum, MV_DMA_OVERRIDE override);
MV_STATUS mvDmaPciRemap(MV_U32 winNum, MV_U32 addrHigh);

MV_VOID mvDmaAddrDecShow(MV_VOID);
MV_VOID mvDmaAllStop(MV_VOID);

#endif /* __INCmvSysIdmaBarh */

