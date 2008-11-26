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
* mvEthPolicy.h - Header File for RX and TX policy of Giga Ethernet driver
*
* DESCRIPTION:
*
* DEPENDENCIES:
*       None.
*
*******************************************************************************/

#ifndef __mvEthPolicy_h__
#define __mvEthPolicy_h__

#include "mvOs.h"
#include "mvEth.h"

#define MV_ETH_TX_POLICY_MAX_MACDA  16

typedef struct
{
	MV_U8*		pHeader;
	int		headerSize;
	int		txQ;

} MV_ETH_TX_POLICY_ENTRY;

typedef struct
{
    MV_U8                   macDa[MV_MAC_ADDR_SIZE];
    MV_ETH_TX_POLICY_ENTRY  policy;
}MV_ETH_TX_POLICY_MACDA;

/* Rx policy struct */
typedef struct
{
    int                 port;
    MV_ETH_PRIO_MODE    rxPrioMode;
    int                 rxQuota[MV_ETH_RX_Q_NUM];
    int                 rxCurQ;
    int                 rxCurQuota;
}ETH_RX_POLICY;

/* Tx policy struct */
typedef struct
{
    int                     port;
    MV_ETH_TX_POLICY_ENTRY  txPolDef;
    int                     txPolMaxDa;
    MV_ETH_TX_POLICY_MACDA  txPolDa[MV_ETH_TX_POLICY_MAX_MACDA];
}ETH_TX_POLICY;

/* RX Policy */
void*       mvEthRxPolicyInit(int port, int defQuota, MV_ETH_PRIO_MODE defMode);
void*       mvEthRxPolicyHndlGet(int port);

MV_STATUS	mvEthRxPolicyModeSet(void* pPortHndl, MV_ETH_PRIO_MODE prioMode);
MV_STATUS	mvEthRxPolicyQueueCfg(void* pPortHndl, int rxQueue, int rxQuota);
int         mvEthRxPolicyGet(void* pPolicyHndl, MV_U32 cause);
void        mvEthRxPolicyShow(void* pPolicyHndl);

/* TX Policy */
void*       mvEthTxPolicyInit(int port, MV_ETH_TX_POLICY_ENTRY* pDefPolicy);
void*       mvEthTxPolicyHndlGet(int port);

MV_STATUS	mvEthTxPolicyDef(void* pTxPolHndl, MV_ETH_TX_POLICY_ENTRY* pPolicy);
MV_STATUS	mvEthTxPolicyAdd(void* pTxPolHndl, MV_ETH_TX_POLICY_MACDA* pDaPolicy);
MV_STATUS	mvEthTxPolicyDel(void* pTxPolHndl, MV_U8* pMacAddr);

int     mvEthTxPolicyGet(void* pTxPolicyHndl, MV_PKT_INFO* pPktInfo, 
                         MV_ETH_TX_POLICY_ENTRY* pTxPolicyEntry);

void        mvEthTxPolicyShow(void* pPolicyHndl);

#endif /* __mvEthPolicy_h__ */

