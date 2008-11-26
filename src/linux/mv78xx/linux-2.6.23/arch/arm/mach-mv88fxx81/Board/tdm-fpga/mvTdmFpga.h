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

#ifndef _MV_TDM_FPGA_H_
#define _MV_TDM_FPGA_H_

#include "mvTypes.h"

#define MV_TDM_MAX_CHANNELS 1
#define MV_TDM_BUFF_SIZE 80

MV_STATUS mvTdmInit(void);
MV_STATUS mvTdmChInit(void *osDev, MV_U8 ch, MV_U32 *tdmCh);
MV_STATUS mvTdmChRemove(MV_U32 tdmCh);
MV_STATUS mvTdmChStart(MV_U32 tdmCh);
MV_STATUS mvTdmChStop(MV_U32 tdmCh);
MV_STATUS mvTdmChEnable(MV_U32 tdmCh);
MV_STATUS mvTdmChDisable(MV_U32 tdmCh);
MV_STATUS mvTdmChTx(MV_U32 tdmCh, MV_U8 *buff, int count);
MV_STATUS mvTdmChRx(MV_U32 tdmCh, MV_U8 *buff, int count);
MV_STATUS mvTdmIsr(void);
MV_STATUS mvTdmChEventGet(MV_U32 tdmCh, MV_U8 *offhook);
MV_BOOL mvTdmChTxReady(MV_U32 tdmCh);
MV_BOOL mvTdmChRxReady(MV_U32 tdmCh);
MV_BOOL mvTdmChExceptionReady(MV_U32 tdmCh);
MV_STATUS mvTdmChDialTone(MV_U32 tdmCh);
MV_STATUS mvTdmChBusyTone(MV_U32 tdmCh);
MV_STATUS mvTdmChStopTone(MV_U32 tdmCh);
MV_STATUS mvTdmChStartRing(MV_U32 tdmCh);
MV_STATUS mvTdmChStopRing(MV_U32 tdmCh);
MV_STATUS mvTdmChRingBackTone(MV_U32 tdmCh);
MV_VOID mvTdmShowProperties(void);
MV_VOID mvTdmChShowProperties(MV_U32 tdmCh);

#endif /* _MV_TDM_FPGA_H_ */
