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
* mvEthDebug.c - Source file for user friendly debug functions
*
* DESCRIPTION:
*
* DEPENDENCIES:
*       None.
*
*******************************************************************************/

#include "mvOs.h"
#include "mvEth.h"
#include "mvEthPolicy.h"
#include "mvCommon.h"
#include "mvTypes.h"
#include "mv802_3.h"
#include "mvDebug.h"
#include "mvCtrlEnvLib.h"
#include "mvEthPhy.h"

extern MV_BOOL         ethDescInSram;
extern MV_BOOL         ethDescSwCoher;


void    mvEthPortShow(void* pHndl);
void    mvEthQueuesShow(void* pHndl, int rxQueue, int txQueue, int mode);
/******************************************************************************/
/*                          Debug functions                                   */
/******************************************************************************/
void    ethRxCoal(int port, int usec)
{
    void*   pHndl;

    pHndl = mvEthPortHndlGet(port);
    if(pHndl != NULL)
    {
        mvEthRxCoalSet(pHndl, usec);
    }
}

void    ethTxCoal(int port, int usec)
{
    void*   pHndl;

    pHndl = mvEthPortHndlGet(port);
    if(pHndl != NULL)
    {
        mvEthTxCoalSet(pHndl, usec);
    }
}

void    ethTxQ(int port, int txQueue,                           
               MV_ETH_PRIO_MODE txPrioMode, int txQuota)
{
    void*   pHndl;

    pHndl = mvEthPortHndlGet(port);
    if(pHndl != NULL)
    {
        mvEthTxQueueConfig(pHndl, txQueue, txPrioMode, txQuota);
    }
}


void    ethBpduRxQ(int port, int bpduQueue)
{
    void*   pHndl;

    pHndl = mvEthPortHndlGet(port);
    if(pHndl != NULL)
    {
        mvEthBpduRxQueue(pHndl, bpduQueue);
    }
}

void    ethArpRxQ(int port, int arpQueue)
{
    void*   pHndl;

    pHndl = mvEthPortHndlGet(port);
    if(pHndl != NULL)
    {
        mvEthArpRxQueue(pHndl, arpQueue);
    }
}

void    ethTcpRxQ(int port, int tcpQueue)
{
    void*   pHndl;

    pHndl = mvEthPortHndlGet(port);
    if(pHndl != NULL)
    {
        mvEthTcpRxQueue(pHndl, tcpQueue);
    }
}

void    ethUdpRxQ(int port, int udpQueue)
{
    void*   pHndl;

    pHndl = mvEthPortHndlGet(port);
    if(pHndl != NULL)
    {
        mvEthUdpRxQueue(pHndl, udpQueue);
    }
}

#ifdef INCLUDE_MULTI_QUEUE
void    ethRxPolMode(int port, MV_ETH_PRIO_MODE prioMode)
{
    void*   pHndl;

    pHndl = mvEthRxPolicyHndlGet(port);
    if(pHndl != NULL)
    {
        mvEthRxPolicyModeSet(pHndl, prioMode);
    }
}

void    ethRxPolQ(int port, int queue, int quota)
{
    void*   pHndl;

    pHndl = mvEthRxPolicyHndlGet(port);
    if(pHndl != NULL)
    {
        mvEthRxPolicyQueueCfg(pHndl, queue, quota);
    }
}

void    ethRxPolicy(int port)
{
    int             queue;
    ETH_RX_POLICY*  pRxPolicy = (ETH_RX_POLICY*)mvEthRxPolicyHndlGet(port);
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)mvEthPortHndlGet(port);

    if( (pPortCtrl == NULL) || (pRxPolicy == NULL) )
    {
        return;
    }
 
    mvOsPrintf("rxDefQ=%d, arpQ=%d, bpduQ=%d, tcpQ=%d, udpQ=%d\n\n",
                pPortCtrl->portConfig.rxDefQ, pPortCtrl->portConfig.rxArpQ, 
                pPortCtrl->portConfig.rxBpduQ, 
                pPortCtrl->portConfig.rxTcpQ, pPortCtrl->portConfig.rxUdpQ); 

    mvOsPrintf("ethRxPolicy #%d: hndl=%p, mode=%s, curQ=%d, curQuota=%d\n",
                pRxPolicy->port, pRxPolicy, 
                (pRxPolicy->rxPrioMode == MV_ETH_PRIO_FIXED) ? "FIXED" : "WRR",
                pRxPolicy->rxCurQ, pRxPolicy->rxCurQuota);

    for(queue=0; queue<MV_ETH_RX_Q_NUM; queue++)
    {
        mvOsPrintf("\t rxQ #%d: rxQuota=%d\n", queue, pRxPolicy->rxQuota[queue]);
    }
}


void    ethTxPolDef(int port, int txQ, char* headerHexStr)
{
    void*                   pHndl;
    MV_ETH_TX_POLICY_ENTRY  polEntry;

    pHndl = mvEthTxPolicyHndlGet(port);
    if(pHndl != NULL)
    {
        polEntry.txQ = txQ;
        polEntry.pHeader = headerHexStr;
        if(headerHexStr != NULL)
            polEntry.headerSize = strlen(headerHexStr);
        else
            polEntry.headerSize = 0;
        mvEthTxPolicyDef(pHndl, &polEntry);
    }
}


#define MV_MAX_HEADER_LEN    64
void    ethTxPolDA(int port, char* macStr, int txQ, char* headerHexStr)
{
    void*                   pHndl;
    MV_ETH_TX_POLICY_MACDA  daPolicy;
    MV_U8           header[MV_MAX_HEADER_LEN];

    pHndl = mvEthTxPolicyHndlGet(port);
    if(pHndl == NULL)
        return;

    mvMacStrToHex(macStr, daPolicy.macDa);
    if(txQ > 0)
    {
        daPolicy.policy.txQ = txQ;
        if(headerHexStr != NULL)
        {
            /* each two char are one byte '55'-> 0x55 */
            daPolicy.policy.headerSize = strlen(headerHexStr)/2; 
            if(daPolicy.policy.headerSize > MV_MAX_HEADER_LEN) 
                return;
            
            mvAsciiToHex(headerHexStr, header);
            daPolicy.policy.pHeader = header;
        }
        else
            daPolicy.policy.headerSize = 0;

        mvEthTxPolicyAdd(pHndl, &daPolicy);
    }
    else
    {
        mvEthTxPolicyDel(pHndl, daPolicy.macDa);
    }
}

void    ethTxPolicy(int port)
{
    ETH_TX_POLICY*  pTxPolicy = (ETH_TX_POLICY*) mvEthTxPolicyHndlGet(port);
    int             idx,i,queue;
    char            macStr[MV_MAC_STR_SIZE];

    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)mvEthPortHndlGet(port);

    if((pPortCtrl == NULL) || (pTxPolicy == NULL))
    {
        return;
    }

    for(queue=0; queue<MV_ETH_TX_Q_NUM; queue++)
    {
        mvOsPrintf("TX Queue #%d: mode=%s, quota=%d\n", 
                queue,   (pPortCtrl->txQueueConfig[queue].prioMode == MV_ETH_PRIO_FIXED) ? "FIXED" : "WRR",
                pPortCtrl->txQueueConfig[queue].quota );
    }

    mvOsPrintf("\nethDefTxPolicy : hndl=%p, defQ=%d, defHeader=%p, defSize=%d, maxDa=%d\n\n",
                pTxPolicy, pTxPolicy->txPolDef.txQ,
                pTxPolicy->txPolDef.pHeader, pTxPolicy->txPolDef.headerSize,
                pTxPolicy->txPolMaxDa);
    if(pTxPolicy->txPolDef.headerSize != 0) 
    {
    for(i = 0; i < pTxPolicy->txPolDef.headerSize; i++) {
        mvOsPrintf(" %02x",pTxPolicy->txPolDef.pHeader[i]);
        if( (i & 0xf)  == 0xf ) 
            mvOsPrintf("\n");
    }
    mvOsPrintf("\n");
    }

    for(idx=0; idx<pTxPolicy->txPolMaxDa; idx++)
    {
        if(pTxPolicy->txPolDa[idx].policy.txQ == MV_INVALID)
            continue;

        mvMacHexToStr(pTxPolicy->txPolDa[idx].macDa, macStr);
        mvOsPrintf("%d. MAC = %s, txQ=%d, pHeader=%p, headerSize=%d\n",
                    idx, macStr, pTxPolicy->txPolDa[idx].policy.txQ,
                    pTxPolicy->txPolDa[idx].policy.pHeader, 
                    pTxPolicy->txPolDa[idx].policy.headerSize);
        if(pTxPolicy->txPolDa[idx].policy.headerSize != 0) 
        {
            for(i = 0; i < pTxPolicy->txPolDa[idx].policy.headerSize; i++)  
            {
                mvOsPrintf(" %02x",pTxPolicy->txPolDa[idx].policy.pHeader[i]);
                if( (i & 0xf)  == 0xf ) 
                    mvOsPrintf("\n");
            }
            mvOsPrintf("\n");
        }
    }
}
#endif /* INCLUDE_MULTI_QUEUE */

/* Print important registers of Ethernet port */
void    ethPortRegs(int port)
{
    mvOsPrintf("\t ethGiga #%d port Registers:\n", port);

    mvOsPrintf("ETH_PORT_STATUS_REG                 : 0x%X = 0x%08x\n", 
                ETH_PORT_STATUS_REG(port), 
                MV_REG_READ( ETH_PORT_STATUS_REG(port) ) );

    mvOsPrintf("ETH_PORT_SERIAL_CTRL_REG            : 0x%X = 0x%08x\n", 
                ETH_PORT_SERIAL_CTRL_REG(port), 
                MV_REG_READ( ETH_PORT_SERIAL_CTRL_REG(port) ) );

    mvOsPrintf("ETH_PORT_CONFIG_REG                 : 0x%X = 0x%08x\n", 
                ETH_PORT_CONFIG_REG(port), 
                MV_REG_READ( ETH_PORT_CONFIG_REG(port) ) );    

    mvOsPrintf("ETH_PORT_CONFIG_EXTEND_REG          : 0x%X = 0x%08x\n", 
                ETH_PORT_CONFIG_EXTEND_REG(port), 
                MV_REG_READ( ETH_PORT_CONFIG_EXTEND_REG(port) ) );    

    mvOsPrintf("ETH_SDMA_CONFIG_REG                 : 0x%X = 0x%08x\n", 
                ETH_SDMA_CONFIG_REG(port), 
                MV_REG_READ( ETH_SDMA_CONFIG_REG(port) ) );    
    
    mvOsPrintf("ETH_MAX_TRANSMIT_UNIT               : 0x%X = 0x%08x\n", 
                ETH_MAX_TRANSMIT_UNIT_REG(port), 
                MV_REG_READ( ETH_MAX_TRANSMIT_UNIT_REG(port) ) );    
    
    mvOsPrintf("ETH_TX_FIFO_URGENT_THRESH_REG       : 0x%X = 0x%08x\n", 
                ETH_TX_FIFO_URGENT_THRESH_REG(port), 
                MV_REG_READ( ETH_TX_FIFO_URGENT_THRESH_REG(port) ) );    

    mvOsPrintf("ETH_RX_QUEUE_COMMAND_REG            : 0x%X = 0x%08x\n", 
                ETH_RX_QUEUE_COMMAND_REG(port), 
                MV_REG_READ( ETH_RX_QUEUE_COMMAND_REG(port) ) );    

    mvOsPrintf("ETH_TX_QUEUE_COMMAND_REG            : 0x%X = 0x%08x\n", 
                ETH_TX_QUEUE_COMMAND_REG(port), 
                MV_REG_READ( ETH_TX_QUEUE_COMMAND_REG(port) ) );    

    mvOsPrintf("ETH_INTR_CAUSE_REG                  : 0x%X = 0x%08x\n", 
                ETH_INTR_CAUSE_REG(port), 
                MV_REG_READ( ETH_INTR_CAUSE_REG(port) ) );    

    mvOsPrintf("ETH_INTR_EXTEND_CAUSE_REG           : 0x%X = 0x%08x\n", 
                ETH_INTR_CAUSE_EXT_REG(port), 
                MV_REG_READ( ETH_INTR_CAUSE_EXT_REG(port) ) );    

    mvOsPrintf("ETH_INTR_MASK_REG                   : 0x%X = 0x%08x\n", 
                ETH_INTR_MASK_REG(port), 
                MV_REG_READ( ETH_INTR_MASK_REG(port) ) );    

    mvOsPrintf("ETH_INTR_EXTEND_MASK_REG            : 0x%X = 0x%08x\n", 
                ETH_INTR_MASK_EXT_REG(port), 
                MV_REG_READ( ETH_INTR_MASK_EXT_REG(port) ) );    

    mvOsPrintf("ETH_ACCESS_PROTECT_REG              : 0x%X = 0x%08x\n", 
                ETH_ACCESS_PROTECT_REG(port),
                MV_REG_READ( ETH_ACCESS_PROTECT_REG(port) ) );       

    mvOsPrintf("ETH_UNIT_DEBUG_0_REG                : 0x%X = 0x%08x\n", 
                ETH_UNIT_DEBUG_0_REG, MV_REG_READ( ETH_UNIT_DEBUG_0_REG ) );    

    mvOsPrintf("ETH_UNIT_DEBUG_1_REG                : 0x%X = 0x%08x\n", 
                ETH_UNIT_DEBUG_1_REG, MV_REG_READ( ETH_UNIT_DEBUG_1_REG ) );    

    mvOsPrintf("ETH_PORT_DEBUG_0_REG                : 0x%X = 0x%08x\n", 
                ETH_PORT_DEBUG_0_REG(port), MV_REG_READ( ETH_PORT_DEBUG_0_REG(port)) );    

    mvOsPrintf("ETH_PORT_DEBUG_1_REG                : 0x%X = 0x%08x\n", 
                ETH_PORT_DEBUG_1_REG(port), MV_REG_READ( ETH_PORT_DEBUG_1_REG(port)) );    

    mvOsPrintf("ETH_RX_DESCR_STAT_CMD_REG           : 0x%X = 0x%08x\n", 
                ETH_RX_DESCR_STAT_CMD_REG(port, 0), 
                MV_REG_READ( ETH_RX_DESCR_STAT_CMD_REG(port, 0) ) );    

    mvOsPrintf("ETH_RX_BYTE_COUNT_REG               : 0x%X = 0x%08x\n", 
                ETH_RX_BYTE_COUNT_REG(port, 0), 
                MV_REG_READ( ETH_RX_BYTE_COUNT_REG(port, 0) ) );    

    mvOsPrintf("ETH_RX_BUF_PTR_REG                  : 0x%X = 0x%08x\n", 
                ETH_RX_BUF_PTR_REG(port, 0), 
                MV_REG_READ( ETH_RX_BUF_PTR_REG(port, 0) ) );    

    mvOsPrintf("ETH_RX_CUR_DESC_PTR_REG             : 0x%X = 0x%08x\n", 
                ETH_RX_CUR_DESC_PTR_REG(port, 0), 
                MV_REG_READ( ETH_RX_CUR_DESC_PTR_REG(port, 0) ) );    
}


/* Print Giga Ethernet UNIT registers */
void    ethRegs(void)
{
    int win;

    mvOsPrintf("ETH_PHY_ADDR_REG                    : 0x%X = 0x%08x\n", 
                ETH_PHY_ADDR_REG, 
                MV_REG_READ(ETH_PHY_ADDR_REG) );    

    mvOsPrintf("ETH_BASE_ADDR_ENABLE_REG            : 0x%X = 0x%08x\n", 
                ETH_BASE_ADDR_ENABLE_REG, 
                MV_REG_READ( ETH_BASE_ADDR_ENABLE_REG) );    
    
    mvOsPrintf("ETH_UNIT_INTERRUPT_CAUSE_REG        : 0x%X = 0x%08x\n", 
                ETH_UNIT_INTERRUPT_CAUSE_REG, 
                MV_REG_READ( ETH_UNIT_INTERRUPT_CAUSE_REG) );    

    mvOsPrintf("ETH_UNIT_INTERRUPT_MASK_REG         : 0x%X = 0x%08x\n", 
                ETH_UNIT_INTERRUPT_MASK_REG, 
                MV_REG_READ( ETH_UNIT_INTERRUPT_MASK_REG) );    

    mvOsPrintf("ETH_UNIT_ERROR_ADDR_REG             : 0x%X = 0x%08x\n", 
                ETH_UNIT_ERROR_ADDR_REG, 
                MV_REG_READ(ETH_UNIT_ERROR_ADDR_REG) );    

    mvOsPrintf("ETH_UNIT_INTERNAL_ADDR_ERROR_REG    : 0x%X = 0x%08x\n", 
                ETH_UNIT_INTERNAL_ADDR_ERROR_REG, 
                MV_REG_READ(ETH_UNIT_INTERNAL_ADDR_ERROR_REG) );    
    
    for(win=0; win<ETH_MAX_DECODE_WIN; win++)
    {
        mvOsPrintf("\nAddrDecWin #%d\n", win);
        mvOsPrintf("\tETH_WIN_BASE_REG(win)       : 0x%X = 0x%08x\n", 
                ETH_WIN_BASE_REG(win), 
                MV_REG_READ( ETH_WIN_BASE_REG(win)) );    
        mvOsPrintf("\tETH_WIN_SIZE_REG(win)       : 0x%X = 0x%08x\n", 
                ETH_WIN_SIZE_REG(win), 
                MV_REG_READ( ETH_WIN_SIZE_REG(win)) );    
    }
}

/******************************************************************************/
/*                      MIB Counters functions                                */
/******************************************************************************/

/*******************************************************************************
* ethClearMibCounters - Clear all MIB counters
*
* DESCRIPTION:
*       This function clears all MIB counters of a specific ethernet port.
*       A read from the MIB counter will reset the counter.
*
* INPUT:
*       int    port -  Ethernet Port number.
*
* RETURN: None
*
*******************************************************************************/
void ethClearCounters(int port)
{
    void*   pHndl;

    pHndl = mvEthPortHndlGet(port);
    if(pHndl != NULL)
        mvEthMibCountersClear(pHndl);

    return;
}


/* Print counters of the Ethernet port */
void    ethPortCounters(int port)
{
    MV_U32  regValue, regValHigh;
    void*   pHndl;

    pHndl = mvEthPortHndlGet(port);
    if(pHndl == NULL)
        return;

    mvOsPrintf("\n\t Port #%d MIB Counters\n\n", port);

    mvOsPrintf("GoodFramesReceived          = %u\n", 
              mvEthMibCounterRead(pHndl, ETH_MIB_GOOD_FRAMES_RECEIVED, NULL));
    mvOsPrintf("BadFramesReceived           = %u\n", 
              mvEthMibCounterRead(pHndl, ETH_MIB_BAD_FRAMES_RECEIVED, NULL));
    mvOsPrintf("BroadcastFramesReceived     = %u\n", 
              mvEthMibCounterRead(pHndl, ETH_MIB_BROADCAST_FRAMES_RECEIVED, NULL));
    mvOsPrintf("MulticastFramesReceived     = %u\n", 
              mvEthMibCounterRead(pHndl, ETH_MIB_MULTICAST_FRAMES_RECEIVED, NULL));

    regValue = mvEthMibCounterRead(pHndl, ETH_MIB_GOOD_OCTETS_RECEIVED_LOW, 
                                 &regValHigh);
    mvOsPrintf("GoodOctetsReceived          = 0x%08x%08x\n", 
               regValHigh, regValue);

    mvOsPrintf("\n");
    mvOsPrintf("GoodFramesSent              = %u\n", 
              mvEthMibCounterRead(pHndl, ETH_MIB_GOOD_FRAMES_SENT, NULL));
    mvOsPrintf("BroadcastFramesSent         = %u\n", 
              mvEthMibCounterRead(pHndl, ETH_MIB_BROADCAST_FRAMES_SENT, NULL));
    mvOsPrintf("MulticastFramesSent         = %u\n", 
              mvEthMibCounterRead(pHndl, ETH_MIB_MULTICAST_FRAMES_SENT, NULL));

    regValue = mvEthMibCounterRead(pHndl, ETH_MIB_GOOD_OCTETS_SENT_LOW, 
                                 &regValHigh);
    mvOsPrintf("GoodOctetsSent              = 0x%08x%08x\n", regValHigh, regValue);


    mvOsPrintf("\n\t FC Control Counters\n");

    regValue = mvEthMibCounterRead(pHndl, ETH_MIB_UNREC_MAC_CONTROL_RECEIVED, NULL);
    mvOsPrintf("UnrecogMacControlReceived   = %u\n", regValue);

    regValue = mvEthMibCounterRead(pHndl, ETH_MIB_GOOD_FC_RECEIVED, NULL);
    mvOsPrintf("GoodFCFramesReceived        = %u\n", regValue);

    regValue = mvEthMibCounterRead(pHndl, ETH_MIB_BAD_FC_RECEIVED, NULL);
    mvOsPrintf("BadFCFramesReceived         = %u\n", regValue);

    regValue = mvEthMibCounterRead(pHndl, ETH_MIB_FC_SENT, NULL);
    mvOsPrintf("FCFramesSent                = %u\n", regValue);


    mvOsPrintf("\n\t RX Errors\n");

    regValue = mvEthMibCounterRead(pHndl, ETH_MIB_BAD_OCTETS_RECEIVED, NULL);
    mvOsPrintf("BadOctetsReceived           = %u\n", regValue);

    regValue = mvEthMibCounterRead(pHndl, ETH_MIB_UNDERSIZE_RECEIVED, NULL);
    mvOsPrintf("UndersizeFramesReceived     = %u\n", regValue);

    regValue = mvEthMibCounterRead(pHndl, ETH_MIB_FRAGMENTS_RECEIVED, NULL);
    mvOsPrintf("FragmentsReceived           = %u\n", regValue);

    regValue = mvEthMibCounterRead(pHndl, ETH_MIB_OVERSIZE_RECEIVED, NULL);
    mvOsPrintf("OversizeFramesReceived      = %u\n", regValue);
    
    regValue = mvEthMibCounterRead(pHndl, ETH_MIB_JABBER_RECEIVED, NULL);
    mvOsPrintf("JabbersReceived             = %u\n", regValue);

    regValue = mvEthMibCounterRead(pHndl, ETH_MIB_MAC_RECEIVE_ERROR, NULL);
    mvOsPrintf("MacReceiveErrors            = %u\n", regValue);

    regValue = mvEthMibCounterRead(pHndl, ETH_MIB_BAD_CRC_EVENT, NULL);
    mvOsPrintf("BadCrcReceived              = %u\n", regValue);

    mvOsPrintf("\n\t TX Errors\n");

    regValue = mvEthMibCounterRead(pHndl, ETH_MIB_INTERNAL_MAC_TRANSMIT_ERR, NULL);
    mvOsPrintf("TxMacErrors                 = %u\n", regValue);

    regValue = mvEthMibCounterRead(pHndl, ETH_MIB_EXCESSIVE_COLLISION, NULL);
    mvOsPrintf("TxExcessiveCollisions       = %u\n", regValue);

    regValue = mvEthMibCounterRead(pHndl, ETH_MIB_COLLISION, NULL);
    mvOsPrintf("TxCollisions                = %u\n", regValue);

    regValue = mvEthMibCounterRead(pHndl, ETH_MIB_LATE_COLLISION, NULL);
    mvOsPrintf("TxLateCollisions            = %u\n", regValue);


    mvOsPrintf("\n");
    regValue = MV_REG_READ( ETH_RX_DISCARD_PKTS_CNTR_REG(port));
    mvOsPrintf("Rx Discarded packets counter    = %u\n", regValue);

    regValue = MV_REG_READ(ETH_RX_OVERRUN_PKTS_CNTR_REG(port));
    mvOsPrintf("Rx Overrun packets counter  = %u\n", regValue);
}

/* Print RMON counters of the Ethernet port */
void    ethPortRmonCounters(int port)
{
    void*   pHndl;

    pHndl = mvEthPortHndlGet(port);
    if(pHndl == NULL)
        return;

    mvOsPrintf("\n\t Port #%d RMON MIB Counters\n\n", port);

    mvOsPrintf("64 ByteFramesReceived           = %u\n", 
              mvEthMibCounterRead(pHndl, ETH_MIB_FRAMES_64_OCTETS, NULL));
    mvOsPrintf("65...127 ByteFramesReceived     = %u\n", 
              mvEthMibCounterRead(pHndl, ETH_MIB_FRAMES_65_TO_127_OCTETS, NULL));
    mvOsPrintf("128...255 ByteFramesReceived    = %u\n", 
              mvEthMibCounterRead(pHndl, ETH_MIB_FRAMES_128_TO_255_OCTETS, NULL));
    mvOsPrintf("256...511 ByteFramesReceived    = %u\n", 
              mvEthMibCounterRead(pHndl, ETH_MIB_FRAMES_256_TO_511_OCTETS, NULL));
    mvOsPrintf("512...1023 ByteFramesReceived   = %u\n", 
              mvEthMibCounterRead(pHndl, ETH_MIB_FRAMES_512_TO_1023_OCTETS, NULL));
    mvOsPrintf("1024...Max ByteFramesReceived   = %u\n", 
              mvEthMibCounterRead(pHndl, ETH_MIB_FRAMES_1024_TO_MAX_OCTETS, NULL));
}

/* Print port information */
void    ethPortStatus(int port)
{
    void*   pHndl;

    pHndl = mvEthPortHndlGet(port);
    if(pHndl != NULL)
    {
        mvEthPortShow(pHndl);
    }
}

/* Print port queues information */
void    ethPortQueues(int port, int rxQueue, int txQueue, int mode)  
{
    void*   pHndl;

    pHndl = mvEthPortHndlGet(port);
    if(pHndl != NULL)
    {
        mvEthQueuesShow(pHndl, rxQueue, txQueue, mode);
    }
}

void    ethUcastSet(int port, char* macStr, int queue)
{
    void*   pHndl;
    MV_U8   macAddr[MV_MAC_ADDR_SIZE];

    pHndl = mvEthPortHndlGet(port);
    if(pHndl != NULL)
    {
        mvMacStrToHex(macStr, macAddr);
        mvEthMacAddrSet(pHndl, macAddr, queue);
    }
}

void    ethMcastAdd(int port, char* macStr, int queue)
{
    void*   pHndl;
    MV_U8   macAddr[MV_MAC_ADDR_SIZE];

    pHndl = mvEthPortHndlGet(port);
    if(pHndl != NULL)
    {
        mvMacStrToHex(macStr, macAddr);
        mvEthMcastAddrSet(pHndl, macAddr, queue);
    }
}

void    ethPortMcast(int port)
{
    int     tblIdx, regIdx;
    MV_U32  smcTableReg;

    mvOsPrintf("\n\t Port #%d Special (IP) Multicast table: 01:00:5E:00:00:XX\n\n", 
                port);

    for(tblIdx=0; tblIdx < 256/4; tblIdx++)
    {
        smcTableReg = MV_REG_READ((ETH_DA_FILTER_SPEC_MCAST_BASE(port) + tblIdx*4));
        for(regIdx=0; regIdx < 4; regIdx++)
        {
            if((smcTableReg & (0x01 << (regIdx*8))) != 0)
            {
                mvOsPrintf("%02X: Accepted, rxQ = %d\n", 
                    tblIdx*4+regIdx, (smcTableReg & (0xFF<<(regIdx *8))) >> 1);
            }
        }
    }
/*    mvOsPrintf("\n\t Port #%d Other Multicast table\n\n", port); */
}


/* Print status of Ethernet port */
void    mvEthPortShow(void* pHndl)
{
    MV_U32              regValue, rxCoal, txCoal;
    int                 speed, queue, port;
    ETH_PORT_CTRL*      pPortCtrl = (ETH_PORT_CTRL*)pHndl;

    port = pPortCtrl->portNo;

    regValue = MV_REG_READ( ETH_PORT_STATUS_REG(port) );

    mvOsPrintf("\t ethGiga #%d port Status: 0x%04x = 0x%08x\n\n", 
                port, ETH_PORT_STATUS_REG(port), regValue);

    if(regValue & ETH_GMII_SPEED_1000_MASK)
        speed = 1000;
    else if(regValue & ETH_MII_SPEED_100_MASK)
        speed = 100;
    else
        speed = 10;

    mvEthCoalGet(pPortCtrl, &rxCoal, &txCoal);

    /* Link, Speed, Duplex, FlowControl */
    mvOsPrintf("Link=%s, Speed=%d, Duplex=%s, RxFlowControl=%s",
                (regValue & ETH_LINK_UP_MASK) ? "UP" : "DOWN",
                speed, 
                (regValue & ETH_FULL_DUPLEX_MASK) ? "FULL" : "HALF",
                (regValue & ETH_ENABLE_RCV_FLOW_CTRL_MASK) ? "ENABLE" : "DISABLE");

#ifdef ETH_HALFDUPLEX_ERRATA
    mvOsPrintf(", padLen=%d\n", pPortCtrl->padLen);
#else
    mvOsPrintf("\n");
#endif /* ETH_HALFDUPLEX_ERRATA */ 

    mvOsPrintf("RxCoal = %d usec, TxCoal = %d usec\n", 
                rxCoal, txCoal);

    mvDebugPrintMacAddr(pPortCtrl->portConfig.macAddr);
    mvOsPrintf("\n");
    /* Print all RX and TX queues */
    for(queue=0; queue<MV_ETH_RX_Q_NUM; queue++)
    {
        mvOsPrintf("RX Queue #%d: base=0x%lx, free=%d\n", 
                    queue, (MV_ULONG)pPortCtrl->rxQueue[queue].pFirstDescr,
                    mvEthRxResourceGet(pPortCtrl, queue) );
    }
    mvOsPrintf("\n");
    for(queue=0; queue<MV_ETH_TX_Q_NUM; queue++)
    {
        mvOsPrintf("TX Queue #%d: base=0x%lx, free=%d\n", 
                queue, (MV_ULONG)pPortCtrl->txQueue[queue].pFirstDescr,
                mvEthTxResourceGet(pPortCtrl, queue) );
    }
}

/* Print RX and TX queue of the Ethernet port */
void    mvEthQueuesShow(void* pHndl, int rxQueue, int txQueue, int mode)  
{
    ETH_PORT_CTRL   *pPortCtrl = (ETH_PORT_CTRL*)pHndl;
    ETH_QUEUE_CTRL  *pQueueCtrl;
    MV_U32          regValue;
    ETH_RX_DESC     *pRxDescr;
    ETH_TX_DESC     *pTxDescr;
    int             i, port = pPortCtrl->portNo;

    if( (rxQueue >=0) && (rxQueue < MV_ETH_RX_Q_NUM) )
    {
        pQueueCtrl = &(pPortCtrl->rxQueue[rxQueue]);
        mvOsPrintf("Port #%d, RX Queue #%d\n\n", port, rxQueue);

        mvOsPrintf("CURR_RX_DESC_PTR        : 0x%X = 0x%08x\n", 
            ETH_RX_CUR_DESC_PTR_REG(port, rxQueue), 
            MV_REG_READ( ETH_RX_CUR_DESC_PTR_REG(port, rxQueue)));


        if(pQueueCtrl->pFirstDescr != NULL)
        {
            mvOsPrintf("pFirstDescr=0x%lx, pLastDescr=0x%lx, numOfResources=%d\n",
                (MV_ULONG)pQueueCtrl->pFirstDescr, (MV_ULONG)pQueueCtrl->pLastDescr, 
                pQueueCtrl->resource);
            mvOsPrintf("pCurrDescr: 0x%lx, pUsedDescr: 0x%lx\n",
                (MV_ULONG)pQueueCtrl->pCurrentDescr, 
                (MV_ULONG)pQueueCtrl->pUsedDescr);

            if(mode == 1)
            {
                pRxDescr = (ETH_RX_DESC*)pQueueCtrl->pFirstDescr;
                i = 0; 
                do 
                {
                    mvOsPrintf("%3d. pDescr=0x%lx, cmdSts=0x%08x, byteCnt=%4d, bufSize=%4d, pBuf=0x%08x, rInfo=0x%lx\n", 
                                i, (MV_ULONG)pRxDescr, pRxDescr->cmdSts, pRxDescr->byteCnt, 
                                (MV_U32)pRxDescr->bufSize, (unsigned int)pRxDescr->bufPtr, (MV_ULONG)pRxDescr->returnInfo);

                    ETH_DESCR_INV(pPortCtrl, pRxDescr);
                    pRxDescr = RX_NEXT_DESC_PTR(pRxDescr, pQueueCtrl);
                    i++;
                } while (pRxDescr != pQueueCtrl->pFirstDescr);
            }
        }
        else
            mvOsPrintf("RX Queue #%d is NOT CREATED\n", rxQueue);
    }

    if( (txQueue >=0) && (txQueue < MV_ETH_TX_Q_NUM) )
    {
        pQueueCtrl = &(pPortCtrl->txQueue[txQueue]);
        mvOsPrintf("Port #%d, TX Queue #%d\n\n", port, txQueue);

        regValue = MV_REG_READ( ETH_TX_CUR_DESC_PTR_REG(port, txQueue));
        mvOsPrintf("CURR_TX_DESC_PTR        : 0x%X = 0x%08x\n", 
                    ETH_TX_CUR_DESC_PTR_REG(port, txQueue), regValue);

        if(pQueueCtrl->pFirstDescr != NULL)
        {
            mvOsPrintf("pFirstDescr=0x%lx, pLastDescr=0x%lx, numOfResources=%d\n",
                       (MV_ULONG)pQueueCtrl->pFirstDescr, 
                       (MV_ULONG)pQueueCtrl->pLastDescr, 
                        pQueueCtrl->resource);
            mvOsPrintf("pCurrDescr: 0x%lx, pUsedDescr: 0x%lx\n",
                       (MV_ULONG)pQueueCtrl->pCurrentDescr, 
                       (MV_ULONG)pQueueCtrl->pUsedDescr);

            if(mode == 1)
            {
                pTxDescr = (ETH_TX_DESC*)pQueueCtrl->pFirstDescr;
                i = 0; 
                do 
                {
                    mvOsPrintf("%3d. pDescr=0x%lx, cmdSts=0x%08x, byteCnt=%4d, pBuf=0x%08x, rInfo=0x%lx, pAlign=0x%lx\n", 
                                i, (MV_ULONG)pTxDescr, pTxDescr->cmdSts, pTxDescr->byteCnt, 
                                (MV_U32)pTxDescr->bufPtr, (MV_ULONG)pTxDescr->returnInfo, (MV_ULONG)pTxDescr->alignBufPtr);

                    ETH_DESCR_INV(pPortCtrl, pTxDescr);
                    pTxDescr = TX_NEXT_DESC_PTR(pTxDescr, pQueueCtrl);
                    i++;
                } while (pTxDescr != pQueueCtrl->pFirstDescr);
            }
        }
        else
            mvOsPrintf("TX Queue #%d is NOT CREATED\n", txQueue);
    }
}
