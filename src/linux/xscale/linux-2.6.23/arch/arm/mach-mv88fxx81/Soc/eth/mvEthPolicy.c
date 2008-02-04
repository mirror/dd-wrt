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
* mvEthPolicy.c - Source file for RX and TX policy of Giga Ethernet driver
*
* DESCRIPTION:
*
* DEPENDENCIES:
*       None.
*
*******************************************************************************/


#include "mvEthPolicy.h"

/*****************************************************************************/
/**************************** RX Policy **************************************/
/*****************************************************************************/

ETH_RX_POLICY*  rxPolicy[BOARD_ETH_PORT_NUM];  


/*******************************************************************************
* mvEthRxPolicyInit - Initialize RX policy component.
*
* DESCRIPTION:
*       Create RX policy database for ethernet port, set to it to default 
*   (FIXED mode) and return port handle. 
*
* INPUT:
*       int port    - Ethernet port number
*       
*
* RETURN:   
*       void*   pRxPolicyHndl   - RX Policy component handler;
*
*******************************************************************************/
void*   mvEthRxPolicyInit(int port, int defQuota, MV_ETH_PRIO_MODE defMode)
{
    int queue;

    if( (port < 0) || (port >= BOARD_ETH_PORT_NUM) )
    {
        mvOsPrintf("ethRxPolicy: port #%d is not exist\n", port);
        return NULL;
    }
    rxPolicy[port] = mvOsMalloc(sizeof(ETH_RX_POLICY));
    if(rxPolicy[port] == NULL)
    {
        mvOsPrintf("ethRxPolicy: Port #%d, Can't allocate %d bytes\n", 
                        port, sizeof(ETH_RX_POLICY));
        return NULL;
    }
    /* Set defaults */
    memset(rxPolicy[port], 0, sizeof(ETH_RX_POLICY));    

    for(queue=0; queue<MV_ETH_RX_Q_NUM; queue++)
    {
        rxPolicy[port]->rxQuota[queue] = defQuota;
    }
    rxPolicy[port]->port = port;
    rxPolicy[port]->rxPrioMode = defMode;
    rxPolicy[port]->rxCurQ = MV_ETH_RX_Q_NUM-1;
    rxPolicy[port]->rxCurQuota = rxPolicy[port]->rxQuota[rxPolicy[port]->rxCurQ];

    return rxPolicy[port];
}

/* Get RX policy handler for specific port */
void*   mvEthRxPolicyHndlGet(int port)
{
    return rxPolicy[port];
}

/*******************************************************************************
* mvEthRxPolicyModeSet - Set receive priority policy.
*
* DESCRIPTION:
*       This function configures priority mode for processing received packets. 
*
* INPUT:
*       void*	            pRxPolHndl  - RX Policy component handler
*       MV_ETH_PRIO_MODE	prioMode    - RX priority mode (FIXED or WRR)
*       
*
* RETURN:   MV_STATUS
*       MV_OK       - Success
*       MV_FAIL     - Failed. 
*
*******************************************************************************/
MV_STATUS	mvEthRxPolicyModeSet(void* pRxPolHndl, MV_ETH_PRIO_MODE prioMode)
{
    ETH_RX_POLICY*  pRxPolicy = (ETH_RX_POLICY*)pRxPolHndl;

    pRxPolicy->rxPrioMode = prioMode;

    return MV_OK;
}

/*******************************************************************************
* mvEthRxPolicyQueueCfg - Set quota for  RX queue in WRR receive priority mode.
*
* DESCRIPTION:
*       This function configures packet based quota for each RX queue, 
*   when WRR priority mode is chosen for processing received packets. 
*
* INPUT:
*       void*	pRxPolHndl  - RX Policy component handler
*       int	    rxQueue     - RX priority mode (FIXED or WRR)
*       int     rxQuota     - packet based quota for RX queue (only in WRR mode)
*
* RETURN:   MV_STATUS
*       MV_OK       - Success
*       MV_FAIL     - Failed. 
*
*******************************************************************************/
MV_STATUS	mvEthRxPolicyQueueCfg(void* pRxPolHndl, int rxQueue, int rxQuota)
{
    ETH_RX_POLICY*  pRxPolicy = (ETH_RX_POLICY*)pRxPolHndl;

    pRxPolicy->rxQuota[rxQueue] = rxQuota;

    return MV_OK;
}

/*******************************************************************************
* mvEthRxPolicyGet - Get RX policy of ethernet port.
*
* DESCRIPTION:
*       This function choose which RX queue will be processed next
*
* INPUT:
*       void*	pRxPolHndl  - RX Policy component handler
*       MV_U32  cause	    - value of cause register, indicating 
*                           which RX queues received packets.
*
* RETURN:   
*   int	    rxQueue     - The RX queue number that will be processed next.
*                       -1 means error        
*
*******************************************************************************/
int     mvEthRxPolicyGet(void* pRxPolHndl, MV_U32 cause)
{
    int             queue;
    ETH_RX_POLICY*  pRxPolicy = (ETH_RX_POLICY*)pRxPolHndl;

#ifdef MV_RT_DEBUG
    if(cause == 0) 
    {
        mvOsPrintf("EthRxPolicy: port #%d, unexpected cause=0x%x\n", pRxPolicy->port, cause);
        return MV_INVALID;
    }
#endif /* MV_RT_DEBUG */

    if(pRxPolicy->rxPrioMode == MV_ETH_PRIO_FIXED)
    {
        for(queue=(MV_ETH_RX_Q_NUM-1); queue>=0; queue--)
        {
            if(cause & (ETH_CAUSE_RX_READY_MASK(queue)))
                return queue;
        }
    }
    else
    {
        /* Check Current RX queue */
        if(cause & ETH_CAUSE_RX_READY_MASK(pRxPolicy->rxCurQ))
        {
            if(pRxPolicy->rxCurQuota > 0)
            {
	            pRxPolicy->rxCurQuota -= 1;
                return pRxPolicy->rxCurQ;
            }
        }
        /* Look for next RX Queue */
        while(MV_TRUE)
        {
            if(pRxPolicy->rxCurQ == 0)
	            pRxPolicy->rxCurQ = MV_ETH_RX_Q_NUM-1;
            else
	            pRxPolicy->rxCurQ -= 1;

            if( ((cause & ETH_CAUSE_RX_READY_MASK(pRxPolicy->rxCurQ)) != 0) &&
                (pRxPolicy->rxQuota[pRxPolicy->rxCurQ] != 0) )
            {
                pRxPolicy->rxCurQuota = pRxPolicy->rxQuota[pRxPolicy->rxCurQ]-1;
                return pRxPolicy->rxCurQ;
            }
        }
    }
    mvOsPrintf("EthRxPolicy: Unexpected error\n");
    return MV_INVALID;
}

/*****************************************************************************/
/**************************** TX Policy **************************************/
/*****************************************************************************/


ETH_TX_POLICY*  txPolicy[BOARD_ETH_PORT_NUM];

/*******************************************************************************
* mvEthTxPolicyInit - Initialize TX policy component.
*
* DESCRIPTION:
*       Create TX policy database for ethernet port, set to it to default 
*   values and return port handle. 
*
* INPUT:
*       int port    - Ethernet port number
*       
*
* RETURN:   
*       void*   pTxPolHndl   - TX Policy component handler
*
*******************************************************************************/
void*   mvEthTxPolicyInit(int port, MV_ETH_TX_POLICY_ENTRY* pDefPolicy)
{
    int daIdx;

    if( (port < 0) || (port >= BOARD_ETH_PORT_NUM) )
    {
        mvOsPrintf("ethTxPolicy: port #%d is not exist\n", port);
        return NULL;
    }
    txPolicy[port] = mvOsMalloc(sizeof(ETH_TX_POLICY));
    if(txPolicy[port] == NULL)
    {
        mvOsPrintf("ethTxPolicy: Port #%d, Can't allocate %d bytes\n", 
                        port, sizeof(ETH_TX_POLICY));
        return NULL;
    }
    /* Set defaults */
    memset(txPolicy[port], 0, sizeof(ETH_TX_POLICY));    
    txPolicy[port]->port = port;
    mvEthTxPolicyDef(txPolicy[port],pDefPolicy);

    /* Invalidate all entries */
    txPolicy[port]->txPolMaxDa = 0;
    for(daIdx=0; daIdx<MV_ETH_TX_POLICY_MAX_MACDA; daIdx++)
        txPolicy[port]->txPolDa[daIdx].policy.txQ = MV_INVALID;

    return txPolicy[port];
}

/* Get TX policy handler for specific port */
void*   mvEthTxPolicyHndlGet(int port)
{
    return txPolicy[port];
}


/*******************************************************************************
* mvEthTxPolicyDef - Set TX default policy.
*
* DESCRIPTION:
*       This function configures TX default policy for packets that 
*   there is no information for them
*
* INPUT:
*       void*	                pTxPolHndl  - TX Policy component handler
*       MV_ETH_TX_POLICY_ENTRY  policy	    - Default TX policy
*
* RETURN:   MV_STATUS
*       MV_OK       - Success
*       MV_FAIL     - Failed. 
*
*******************************************************************************/
MV_STATUS	mvEthTxPolicyDef(void* pTxPolHndl, MV_ETH_TX_POLICY_ENTRY* pPolicy)
{
    ETH_TX_POLICY*  pTxPolicy = (ETH_TX_POLICY*)pTxPolHndl;

    /* if Tx header exist */
    if(pPolicy->pHeader != NULL) 
    {
	/* allocate memory for header */
	pTxPolicy->txPolDef.pHeader = mvOsMalloc(pPolicy->headerSize );
	if(pTxPolicy->txPolDef.pHeader == NULL)
	{
		mvOsPrintf("mvEthTxPolicyDef: Alloc failed \n");
		return MV_FAIL;
	}
	/* copy header */
	memcpy(pTxPolicy->txPolDef.pHeader, pPolicy->pHeader , pPolicy->headerSize );
        pTxPolicy->txPolDef.headerSize = pPolicy->headerSize;
    }
    else
    {
	pTxPolicy->txPolDef.pHeader = NULL;
	pTxPolicy->txPolDef.headerSize = 0;
    }
    pTxPolicy->txPolDef.txQ = pPolicy->txQ;

    return MV_OK;
}

/*******************************************************************************
* mvEthTxPolicyAdd - Add TX policy for packets with special MAC DA.
*
* DESCRIPTION:
*       This function adds TX policy for outgoing packets with special MAC DAs. 
*   Support up to 16 entries.
*
* INPUT:
*       void*	                pTxPolHndl  - TX Policy component handler
*       MV_ETH_TX_POLICY_MACDA  daPolicy	- TX policy for outgoing packets 
*                                           with specific MACDA
*
* RETURN:   MV_STATUS
*       MV_OK       - Success
*       MV_FAIL     - Failed. 
*
*******************************************************************************/
MV_STATUS	mvEthTxPolicyAdd(void* pTxPolHndl, MV_ETH_TX_POLICY_MACDA* pDaPolicy)
{
    int             idx, firstEmptyIdx = MV_INVALID;
    ETH_TX_POLICY*  pTxPolicy = (ETH_TX_POLICY*)pTxPolHndl;

    for(idx=0; idx<MV_ETH_TX_POLICY_MAX_MACDA; idx++)
    {
        if( (pTxPolicy->txPolDa[idx].policy.txQ != MV_INVALID) &&
            ( memcmp(pTxPolicy->txPolDa[idx].macDa, pDaPolicy->macDa, 
                     MV_MAC_ADDR_SIZE) == 0) )
        {
            /* entry already exist - so replace */
            firstEmptyIdx = idx;
            break;            
        }
        if( (firstEmptyIdx == MV_INVALID) && 
            (pTxPolicy->txPolDa[idx].policy.txQ == MV_INVALID) )
        {
            firstEmptyIdx = idx;
        }
    }
    if(firstEmptyIdx != MV_INVALID)
    {
        memcpy(pTxPolicy->txPolDa[firstEmptyIdx].macDa, 
                    pDaPolicy->macDa, MV_MAC_ADDR_SIZE);
	
	/* if Tx header exist */
	if(pDaPolicy->policy.pHeader != NULL) 
	{
		/* allocate memory for header */
		pTxPolicy->txPolDa[firstEmptyIdx].policy.pHeader = mvOsMalloc(pDaPolicy->policy.headerSize );
		if(pTxPolicy->txPolDa[firstEmptyIdx].policy.pHeader == NULL)
		{
			mvOsPrintf("ethTxPolicy: Alloc failed \n");
			return MV_FAIL;
		}
		/* copy header */
		memcpy(pTxPolicy->txPolDa[firstEmptyIdx].policy.pHeader , 
				pDaPolicy->policy.pHeader , pDaPolicy->policy.headerSize );
        	pTxPolicy->txPolDa[firstEmptyIdx].policy.headerSize = pDaPolicy->policy.headerSize;
	}
	else
	{
		pTxPolicy->txPolDa[firstEmptyIdx].policy.pHeader = NULL;
		pTxPolicy->txPolDa[firstEmptyIdx].policy.headerSize = 0;
	}
       	pTxPolicy->txPolDa[firstEmptyIdx].policy.txQ = pDaPolicy->policy.txQ;
        if(firstEmptyIdx >= pTxPolicy->txPolMaxDa)
            pTxPolicy->txPolMaxDa = firstEmptyIdx + 1;

        return MV_OK;
    }
    mvOsPrintf("ethTxPolicy: Can't add more MACDA entries\n");
    return MV_FULL;
}

/*******************************************************************************
* mvEthTxPolicyDel - Delete TX policy for packets with special MACDA.
*
* DESCRIPTION:
*       This function deletes existing TX policy for outgoing packets with 
*   special MAC DAs.. 
*
* INPUT:
*       void*	pTxPolHndl  - TX Policy component handler
*       MV_U8*  pMacAddr    - Pointer to MACDA for the entry will be deleted.
*
* RETURN:   MV_STATUS
*       MV_OK       - Success
*       MV_FAIL     - Failed. 
*
*******************************************************************************/
MV_STATUS	mvEthTxPolicyDel(void* pTxPolHndl, MV_U8* pMacAddr)
{
    int             idx; 
    ETH_TX_POLICY*  pTxPolicy = (ETH_TX_POLICY*)pTxPolHndl;

    for(idx=0; idx<pTxPolicy->txPolMaxDa; idx++)
    {
        if( (pTxPolicy->txPolDa[idx].policy.txQ != MV_INVALID) && 
            (memcmp(pTxPolicy->txPolDa[idx].macDa, pMacAddr, 
                                        MV_MAC_ADDR_SIZE) == 0) )
        {
            /* Entry found */
            pTxPolicy->txPolDa[idx].policy.txQ = MV_INVALID;
	    if(pTxPolicy->txPolDa[idx].policy.pHeader != NULL)
	    {
	    	mvOsFree(pTxPolicy->txPolDa[idx].policy.pHeader);
		pTxPolicy->txPolDa[idx].policy.pHeader = NULL;
		pTxPolicy->txPolDa[idx].policy.headerSize = 0;
	    }

            if(idx == (pTxPolicy->txPolMaxDa - 1))
                pTxPolicy->txPolMaxDa--;

            /* Decrease table */
            while( (pTxPolicy->txPolMaxDa > 0) && 
                   (pTxPolicy->txPolDa[pTxPolicy->txPolMaxDa-1].policy.txQ == MV_INVALID) )
                pTxPolicy->txPolMaxDa--;

            return MV_OK;
        }
    }
    mvOsPrintf("ethTxPolicy: Can't delete the MACDA entry\n");
    return MV_NO_SUCH;
}



/*******************************************************************************
* mvEthDATxPolicyGet - Get TX policy of ethernet port for the outgoing packet,
* 			base on the DA.
*
* DESCRIPTION:
*       This function return an existing TX policy for specific MACs.
*
* INPUT:
*       void*	                pPortHndl   - Pointer to port specific handler
*       MV_U8*                  MAC         - Pointer to DA MAC.
*
* OUTPUT:
*       MV_ETH_TX_POLICY_ENTRY* pTxPolEntry - pointer to TX policy entry for 
*                                             this packet
*
* RETURN:   
*       int	    txQ - TX queue to place the outgoing packet.
*
*******************************************************************************/
int     mvEthDATxPolicyGet(void* pTxPolicyHndl, MV_U8* pDA, 
                         MV_ETH_TX_POLICY_ENTRY* pTxPolicyEntry)
{
    int                     idx=0;
    ETH_TX_POLICY*          pTxPolicy = (ETH_TX_POLICY*)pTxPolicyHndl;
    MV_ETH_TX_POLICY_ENTRY* pPolicy = &pTxPolicy->txPolDef;  

    while(idx < pTxPolicy->txPolMaxDa)
    {
        if( (pTxPolicy->txPolDa[idx].policy.txQ != MV_INVALID) &&
            (memcmp(pDA, pTxPolicy->txPolDa[idx].macDa, 
                                        MV_MAC_ADDR_SIZE) == 0) )
        {
            pPolicy = &pTxPolicy->txPolDa[idx].policy;
            break;
        }
	idx++;
    }

    /* if valid entry found */
    if(idx < pTxPolicy->txPolMaxDa)
    {
    	if(pTxPolicyEntry != NULL)
    	{
    		pTxPolicyEntry->pHeader = pPolicy->pHeader;
		pTxPolicyEntry->headerSize = pPolicy->headerSize;
		pTxPolicyEntry->txQ = pPolicy->txQ;
	}
	return pPolicy->txQ;
    }
    else /* if no entry found - use defaults*/
    {
    	if(pTxPolicyEntry != NULL)
    	{
    		pTxPolicyEntry->pHeader = pTxPolicy->txPolDef.pHeader;
		pTxPolicyEntry->headerSize = pTxPolicy->txPolDef.headerSize;
		pTxPolicyEntry->txQ = pTxPolicy->txPolDef.txQ;
	}
	return pTxPolicy->txPolDef.txQ;
    }

    return 0;
}

/*******************************************************************************
* mvEthTxPolicyGet - Get TX policy of ethernet port for the outgoing packet.
*
* DESCRIPTION:
*       This function gets existing TX policy for outgoing packets.
*
* INPUT:
*       void*	                pPortHndl   - Pointer to port specific handler
*       MV_U8*                  pPktInfo    - Pointer to outgoing packet
*
* OUTPUT:
*       MV_ETH_TX_POLICY_ENTRY* pTxPolEntry - pointer to TX policy entry for 
*                                             this packet
*
* RETURN:   
*       int	    txQ - TX queue to place the outgoing packet.
*
*******************************************************************************/
int     mvEthTxPolicyGet(void* pTxPolicyHndl, MV_PKT_INFO* pPktInfo, 
                         MV_ETH_TX_POLICY_ENTRY* pTxPolicyEntry)
{
    MV_U8 mac_da[MV_MAC_ADDR_SIZE];
    MV_U32 bufCount, tot_size = 0, size;
    MV_BUF_INFO*    pBufInfo = pPktInfo->pFrags;

    /* extract the destination MAC address */
    for(bufCount=0; bufCount<pPktInfo->numFrags; bufCount++)
    {
	size = MV_MIN((MV_MAC_ADDR_SIZE - tot_size), pBufInfo[bufCount].bufSize);
       	memcpy(&mac_da[tot_size], pBufInfo[bufCount].bufVirtPtr, size);
	tot_size += size;
      	if(tot_size == MV_MAC_ADDR_SIZE)
		break;
    }

    return mvEthDATxPolicyGet(pTxPolicyHndl, mac_da, pTxPolicyEntry);
}



