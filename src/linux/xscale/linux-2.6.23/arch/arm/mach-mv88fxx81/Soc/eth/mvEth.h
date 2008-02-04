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
* mvEth.h - Header File for : MV-643xx network interface header
*
* DESCRIPTION:
*       This header file contains macros typedefs and function declaration for
*       the Marvell Gig Bit Ethernet Controller.
*
* DEPENDENCIES:
*       None.
*
*******************************************************************************/

#ifndef __mvEth_h__
#define __mvEth_h__

/* includes */
#include "mvTypes.h"
#include "mv802_3.h"
#include "mvCtrlEnvLib.h"           
#include "mvCtrlEnvAddrDec.h"
#include "mvEthRegs.h"
#include "mvSysHwConfig.h"

/* defines  */

#define MV_ETH_EXTRA_FRAGS_NUM      2
#define MV_ETH_TX_Q_NUM             1

#ifdef INCLUDE_MULTI_QUEUE
#   define MV_ETH_RX_Q_NUM   8
#else
#   define MV_ETH_RX_Q_NUM   1
#endif /* INCLUDE_MULTI_QUEUE */

#define MV_ETH_RX_READY_ISR_MASK                                    \
            (((1<<MV_ETH_RX_Q_NUM)-1)<<ETH_CAUSE_RX_READY_OFFSET)

#define MV_ETH_TX_DONE_ISR_MASK                                    \
            (((1<<MV_ETH_TX_Q_NUM)-1)<<ETH_CAUSE_TX_BUF_OFFSET) 


typedef enum
{
    MV_ETH_SPEED_AN,
    MV_ETH_SPEED_10,
    MV_ETH_SPEED_100,
    MV_ETH_SPEED_1000
    
}MV_ETH_PORT_SPEED;

typedef enum
{
    MV_ETH_DUPLEX_AN,
    MV_ETH_DUPLEX_HALF,
    MV_ETH_DUPLEX_FULL
    
}MV_ETH_PORT_DUPLEX;

typedef enum
{
    MV_ETH_FC_AN_ADV_DIS,
    MV_ETH_FC_AN_ADV_SYM,
    MV_ETH_FC_DISABLE,
    MV_ETH_FC_ENABLE

}MV_ETH_PORT_FC;

typedef enum
{
    MV_ETH_PRIO_FIXED = 0,  /* Fixed priority mode */
    MV_ETH_PRIO_WRR   = 1   /* Weighted round robin priority mode */    
} MV_ETH_PRIO_MODE;

/* Ethernet port specific infomation */
typedef struct
{
    MV_U8   macAddr[MV_MAC_ADDR_SIZE];
    int     maxRxPktSize;
    int     rxDefQ;
    int     rxBpduQ;
    int     rxArpQ;
    int     rxTcpQ;
    int     rxUdpQ;
} MV_ETH_PORT_CFG;

typedef struct
{
    int     descrNum;
} MV_ETH_RX_Q_CFG;

typedef struct
{
    int         descrNum;
    MV_ETH_PRIO_MODE    prioMode;
    int         quota;
} MV_ETH_TX_Q_CFG;

typedef struct
{
    int     maxRxPktSize;
    int     rxDefQ;
    int     txDescrNum[MV_ETH_TX_Q_NUM];
    int     rxDescrNum[MV_ETH_RX_Q_NUM];
} MV_ETH_PORT_INIT;

typedef struct
{
    MV_BOOL             isLinkUp;
    MV_ETH_PORT_SPEED   speed;
    MV_ETH_PORT_DUPLEX  duplex;
    MV_ETH_PORT_FC      flowControl;

} MV_ETH_PORT_STATUS;

typedef struct _mvEthDecWin
{
        MV_TARGET     target;
        MV_ADDR_WIN   addrWin;  /* An address window*/
        MV_BOOL       enable;   /* Address decode window is enabled/disabled */
  
}MV_ETH_DEC_WIN;



/* ethernet.h API list */
void        mvEthInit(void);
void        mvEthMemAttrGet(MV_BOOL* pIsSram, MV_BOOL* pIsSwCoher);

void    ethResetTxDescRing(void* pPortHndl, int queue);
void    ethResetRxDescRing(void* pPortHndl, int queue);

/* Port Initalization routines */
void*       mvEthPortInit (int port, MV_ETH_PORT_INIT *pPortInit);
void*       mvEthPortHndlGet(int port);

void        mvEthPortFinish(void* pEthPortHndl);
MV_STATUS   mvEthPortDown(void* pEthPortHndl);
MV_STATUS   mvEthPortDisable(void* pEthPortHndl);
MV_STATUS   mvEthPortUp(void* pEthPortHndl);
MV_STATUS   mvEthPortEnable(void* pEthPortHndl);

/* Port data flow routines */
MV_STATUS   mvEthPortTx(void* pEthPortHndl, int txQueue, MV_PKT_INFO *pPktInfo);
MV_STATUS   mvEthPortTxDone(void* pEthPortHndl, int txQueue, MV_PKT_INFO *pPktInfo);
MV_STATUS   mvEthPortForceTxDone(void* pEthPortHndl, int txQueue, MV_PKT_INFO *pPktInfo);

MV_STATUS   mvEthPortRx(void* pEthPortHndl, int rxQueue, MV_PKT_INFO *pPktInfo);
MV_STATUS   mvEthPortRxDone(void* pEthPortHndl, int rxQueue, MV_PKT_INFO *pPktInfo);
MV_STATUS   mvEthPortForceRx(void* pEthPortHndl, int rxQueue, MV_PKT_INFO *pPktInfo);

/* Port Configuration routines */
MV_STATUS   mvEthDefaultsSet(void* pEthPortHndl);
MV_STATUS   mvEthMaxRxSizeSet(void* pPortHndl, int maxRxSize);

/* Port RX MAC Filtering control routines */
MV_U8       mvEthMcastCrc8Get(MV_U8* pAddr);
MV_STATUS   mvEthMacAddrSet(void* pPortHandle, MV_U8* pMacAddr, int queue);
MV_STATUS   mvEthMacAddrGet(int portNo, unsigned char *pAddr);
MV_STATUS   mvEthMcastAddrSet(void* pPortHandle, MV_U8 *pAddr, int queue);
MV_STATUS   mvEthRxFilterModeSet(void* pPortHndl, MV_BOOL isPromisc);
MV_VOID     mvEthSetSpecialMcastTable(int portNo, int queue);
MV_VOID     mvEthSetOtherMcastTable(int portNo, int queue);

/* Interrupt Coalesting functions */
MV_U32        mvEthRxCoalSet (void* pPortHndl, MV_U32 uSec);
MV_U32        mvEthTxCoalSet (void* pPortHndl, MV_U32 uSec);
MV_STATUS     mvEthCoalGet(void* pPortHndl, MV_U32* pRxCoal, MV_U32* pTxCoal);

/* Port status APIs */ 
int         mvEthRxResourceGet(void* pPortHndl, int rxQueue);
int         mvEthTxResourceGet(void* pPortHndl, int txQueue);

/* MIB Counters APIs */
MV_U32      mvEthMibCounterRead(void* pPortHndl, unsigned int mibOffset, 
                               MV_U32* pHigh32);
void        mvEthMibCountersClear(void* pPortHandle);

/* TX Scheduling configuration routines */
MV_STATUS   mvEthTxQueueConfig(void* pPortHandle, int txQueue,                          
                               MV_ETH_PRIO_MODE txPrioMode, int txQuota);

/* RX Dispatching configuration routines */
MV_STATUS   mvEthBpduRxQueue(void* pPortHandle, int bpduQueue);
MV_STATUS   mvEthArpRxQueue(void* pPortHandle, int arpQueue);
MV_STATUS   mvEthTcpRxQueue(void* pPortHandle, int tcpQueue);
MV_STATUS   mvEthUdpRxQueue(void* pPortHandle, int udpQueue);
MV_STATUS   mvEthVlanPrioRxQueue(void* pPortHandle, int vlanPrio, int vlanPrioQueue);

/* Speed, Duplex, FlowControl routines */
MV_STATUS   mvEthSpeedDuplexSet(void* pPortHandle, MV_ETH_PORT_SPEED speed, 
                                                   MV_ETH_PORT_DUPLEX duplex);

MV_STATUS   mvEthFlowCtrlSet(void* pPortHandle, MV_ETH_PORT_FC flowControl);

void        mvEthStatusGet(void* pPortHandle, MV_ETH_PORT_STATUS* pStatus);

/* PHY routines */
void       mvEthPhyAddrSet(void* pPortHandle, int phyAddr);
int        mvEthPhyAddrGet(void* pPortHandle);


/* addr decode */
MV_STATUS   mvEthWinInit (void);
MV_STATUS   mvEthWinSet(MV_U32 winNum, MV_ETH_DEC_WIN *pAddrDecWin);
MV_STATUS   mvEthWinGet(MV_U32 winNum, MV_ETH_DEC_WIN *pAddrDecWin);
MV_STATUS   mvEthWinEnable(MV_U32 winNum,MV_BOOL enable);
MV_U32      mvEthWinTargetGet(MV_TARGET target);
MV_VOID     mvEthAddrDecShow(MV_VOID);


/******************** ETH PRIVATE ************************/

/*#define UNCACHED_TX_BUFFERS*/
/*#define UNCACHED_RX_BUFFERS*/


/* Port attributes */
/* Size of a Tx/Rx descriptor used in chain list data structure */
#define ETH_RX_DESC_ALIGNED_SIZE        32
#define ETH_TX_DESC_ALIGNED_SIZE        32

/* Default port configuration value */
#define PORT_CONFIG_VALUE                       \
             ETH_DEF_RX_QUEUE_MASK(0)       |   \
             ETH_DEF_RX_ARP_QUEUE_MASK(0)   |   \
             ETH_DEF_RX_TCP_QUEUE_MASK(0)   |   \
             ETH_DEF_RX_UDP_QUEUE_MASK(0)   |   \
             ETH_DEF_RX_BPDU_QUEUE_MASK(0)  |   \
             ETH_RX_CHECKSUM_WITH_PSEUDO_HDR

/* Default port extend configuration value */
#define PORT_CONFIG_EXTEND_VALUE            0

#define PORT_SERIAL_CONTROL_VALUE                           \
            ETH_DISABLE_FC_AUTO_NEG_MASK                |   \
            BIT9                                        |   \
            ETH_DO_NOT_FORCE_LINK_FAIL_MASK             |   \
            ETH_MAX_RX_PACKET_1552BYTE                  |   \
            ETH_SET_FULL_DUPLEX_MASK

#define PORT_SERIAL_CONTROL_100MB_FORCE_VALUE               \
            ETH_FORCE_LINK_PASS_MASK                    |   \
            ETH_DISABLE_DUPLEX_AUTO_NEG_MASK            |   \
            ETH_DISABLE_FC_AUTO_NEG_MASK                |   \
            BIT9                                        |   \
            ETH_DO_NOT_FORCE_LINK_FAIL_MASK             |   \
            ETH_DISABLE_SPEED_AUTO_NEG_MASK             |   \
            ETH_SET_FULL_DUPLEX_MASK                    |   \
            ETH_SET_MII_SPEED_100_MASK                  |   \
            ETH_MAX_RX_PACKET_1552BYTE


#define PORT_SERIAL_CONTROL_1000MB_FORCE_VALUE              \
            ETH_FORCE_LINK_PASS_MASK                    |   \
            ETH_DISABLE_DUPLEX_AUTO_NEG_MASK            |   \
            ETH_DISABLE_FC_AUTO_NEG_MASK                |   \
            BIT9                                        |   \
            ETH_DO_NOT_FORCE_LINK_FAIL_MASK             |   \
            ETH_DISABLE_SPEED_AUTO_NEG_MASK             |   \
            ETH_SET_FULL_DUPLEX_MASK                    |   \
            ETH_SET_GMII_SPEED_1000_MASK                |   \
            ETH_MAX_RX_PACKET_1552BYTE

/* An offest in Tx descriptors to store data for buffers less than 8 Bytes */
#define MIN_TX_BUFF_LOAD            8
#define TX_BUF_OFFSET_IN_DESC       (ETH_TX_DESC_ALIGNED_SIZE - MIN_TX_BUFF_LOAD)

#define TX_DISABLE_TIMEOUT          0x1000000
#define RX_DISABLE_TIMEOUT          0x1000000
#define TX_FIFO_EMPTY_TIMEOUT       0x1000000
#define PORT_DISABLE_WAIT_TCLOCKS   5000

/* Macros that save access to desc in order to find next desc pointer  */
#define RX_NEXT_DESC_PTR(pRxDescr, pQueueCtrl)                              \
        ((pRxDescr) == (pQueueCtrl)->pLastDescr) ?                          \
               (ETH_RX_DESC*)((pQueueCtrl)->pFirstDescr) :                 \
               (ETH_RX_DESC*)(((MV_ULONG)(pRxDescr)) + ETH_RX_DESC_ALIGNED_SIZE)

#define TX_NEXT_DESC_PTR(pTxDescr, pQueueCtrl)                        \
        ((pTxDescr) == (pQueueCtrl)->pLastDescr) ?                          \
               (ETH_TX_DESC*)((pQueueCtrl)->pFirstDescr) :                 \
               (ETH_TX_DESC*)(((MV_ULONG)(pTxDescr)) + ETH_TX_DESC_ALIGNED_SIZE)


/* Queue specific information */
typedef struct 
{
    void*       pFirstDescr;
    void*       pLastDescr;
    void*       pCurrentDescr;
    void*       pUsedDescr;
    int         resource;
    MV_BUF_INFO descBuf;
} ETH_QUEUE_CTRL;

/* Ethernet port specific infomation */
typedef struct _ethPortCtrl
{
    int             portNo;
    ETH_QUEUE_CTRL  rxQueue[MV_ETH_RX_Q_NUM]; /* Rx ring resource  */
    ETH_QUEUE_CTRL  txQueue[MV_ETH_TX_Q_NUM]; /* Tx ring resource  */

    MV_ETH_PORT_CFG portConfig;
    MV_ETH_RX_Q_CFG rxQueueConfig[MV_ETH_RX_Q_NUM];
    MV_ETH_TX_Q_CFG txQueueConfig[MV_ETH_TX_Q_NUM];

    /* Register images - For DP */
    MV_U32          portTxQueueCmdReg;   /* Port active Tx queues summary    */
    MV_U32          portRxQueueCmdReg;   /* Port active Rx queues summary    */

    MV_STATE        portState;

#ifdef ETH_HALFDUPLEX_ERRATA
    int             padLen;
    MV_BUF_INFO     txAlignedBuf[MV_ETH_TX_Q_NUM];
#endif /* ETH_HALFDUPLEX_ERRATA */

    MV_U8           mcastCount[256];
} ETH_PORT_CTRL; 

/************** MACROs ****************/

/* MACROs to Flush / Invalidate TX / RX Buffers */
#if (ETHER_DRAM_COHER == MV_CACHE_COHER_SW) && !defined(UNCACHED_TX_BUFFERS)
#   define ETH_PACKET_CACHE_FLUSH(pAddr, size)                                          \
        mvOsCacheFlush(NULL, (pAddr), (size));                                    \
        /*CPU_PIPE_FLUSH;*/
#else
#   define ETH_PACKET_CACHE_FLUSH(pAddr, size)        
#endif /* ETHER_DRAM_COHER == MV_CACHE_COHER_SW */

#if ( (ETHER_DRAM_COHER == MV_CACHE_COHER_SW) && !defined(UNCACHED_RX_BUFFERS) )
#   define ETH_PACKET_CACHE_INVALIDATE(pAddr, size)                             \
        mvOsCacheInvalidate (NULL, (pAddr), (size));                       \
        /*CPU_PIPE_FLUSH;*/
#else
#   define ETH_PACKET_CACHE_INVALIDATE(pAddr, size)    
#endif /* ETHER_DRAM_COHER == MV_CACHE_COHER_SW && !UNCACHED_RX_BUFFERS */


#define ETH_DESCR_FLUSH_INV(pPortCtrl, pDescr)  \
    if(ethDescSwCoher == MV_TRUE)                  \
        mvOsCacheLineFlushInv((MV_ULONG)(pDescr))

#define ETH_DESCR_INV(pPortCtrl, pDescr)  \
    if(ethDescSwCoher == MV_TRUE)                  \
        mvOsCacheLineInv((MV_ULONG)(pDescr))


#endif /* __mvEth_h__ */

