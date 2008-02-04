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
* mvEth.c - Marvell's Gigabit Ethernet controller low level driver
*
* DESCRIPTION:
*       This file introduce OS independent APIs to Marvell's Gigabit Ethernet
*       controller. This Gigabit Ethernet Controller driver API controls
*       1) Operations (i.e. port Init, Finish, Up, Down, PhyReset etc').
*       2) Data flow (i.e. port Send, Receive etc').
*       3) MAC Filtering functions (ethSetMcastAddr, ethSetRxFilterMode, etc.)
*       4) MIB counters support (ethReadMibCounter)
*       5) Debug functions (ethPortRegs, ethPortCounters, ethPortQueues, etc.)
*       Each Gigabit Ethernet port is controlled via ETH_PORT_CTRL struct.
*       This struct includes configuration information as well as driver
*       internal data needed for its operations.
*
*       Supported Features:
*       - OS independent. All required OS services are implemented via external 
*       OS dependent components (like osLayer or ethOsg)
*       - The user is free from Rx/Tx queue managing.
*       - Simple Gigabit Ethernet port operation API.
*       - Simple Gigabit Ethernet port data flow API.
*       - Data flow and operation API support per queue functionality.
*       - Support cached descriptors for better performance.
*       - PHY access and control API.
*       - Port Configuration API.
*       - Full control over Special and Other Multicast MAC tables.
*
*******************************************************************************/
/* includes */
#include "mvTypes.h"
#include "mv802_3.h"
#include "mvDebug.h"
#include "mvCommon.h"
#include "mvOs.h"
#include "mvCtrlEnvLib.h"
#include "mvEthPhy.h"
#include "mvEth.h"
#include "mvCpu.h"

#ifdef INCLUDE_SYNC_BARR
#include "mvCpuIf.h"
#endif

#ifdef MV_RT_DEBUG
#   define ETH_DEBUG
#endif


/* locals */
MV_BOOL         ethDescInSram;
MV_BOOL         ethDescSwCoher;

/* This array holds the control structure of each port */
ETH_PORT_CTRL* ethPortCtrl[BOARD_ETH_PORT_NUM];

#ifdef ETH_HALFDUPLEX_ERRATA
int ethPortHdPadLen[BOARD_ETH_PORT_NUM];
#endif /* ETH_HALFDUPLEX_ERRATA */

/* Ethernet Port Local routines */

static void    ethInitRxDescRing(ETH_PORT_CTRL* pPortCtrl, int queue);

static void    ethInitTxDescRing(ETH_PORT_CTRL* pPortCtrl, int queue);

static void    ethSetUcastTable(int portNo, int queue);

static MV_BOOL ethSetUcastAddr (int ethPortNum, MV_U8 lastNibble, int queue);
static MV_BOOL ethSetSpecialMcastAddr(int ethPortNum, MV_U8 lastByte, int queue);
static MV_BOOL ethSetOtherMcastAddr(int ethPortNum, MV_U8 crc8, int queue);

static void    ethBCopy(char* srcAddr, char* dstAddr, int byteCount);

static void    ethFreeDescrMemory(ETH_PORT_CTRL* pEthPortCtrl, MV_BUF_INFO* pDescBuf);
static MV_U8*  ethAllocDescrMemory(ETH_PORT_CTRL* pEthPortCtrl, int size, 
                                   MV_ULONG* pPhysAddr);

static MV_U32 mvEthMruGet(MV_U32 maxRxPktSize);



static INLINE MV_ULONG  ethDescVirtToPhy(ETH_PORT_CTRL* pPortCtrl, MV_U8* pDesc)
{
#if defined (ETH_DESCR_IN_SRAM)
    if( ethDescInSram )
        return mvSramVirtToPhy(pDesc);
    else
#endif /* ETH_DESCR_IN_SRAM */
        return mvOsIoVirtToPhy(NULL, pDesc);
}







/******************************************************************************/
/*                      EthDrv Initialization functions                       */
/******************************************************************************/

/*******************************************************************************
* mvEthInit - Initialize the Giga Ethernet unit
*
* DESCRIPTION:
*       This function initialize the Giga Ethernet unit.
*       1) Configure Address decode windows of the unit
*       2) Set registers to HW default values. 
*       3) Clear and Disable interrupts
*
* INPUT:  NONE
*
* RETURN: NONE
*
* NOTE: this function is called once in the boot process.
*******************************************************************************/
void    mvEthInit(void)
{
    int port;

    /* Init static data structures */
    for(port=0; port<BOARD_ETH_PORT_NUM; port++)
    {
        ethPortCtrl[port] = NULL;
 
#ifdef ETH_HALFDUPLEX_ERRATA
        if(port == 0)
            ethPortHdPadLen[port] = MV_ETH0_HD_PKT_SIZE;
        else if(port == 1)
            ethPortHdPadLen[port] = MV_ETH1_HD_PKT_SIZE;
        else if(port == 2)
            ethPortHdPadLen[port] = MV_ETH2_HD_PKT_SIZE;
        else
        {
            mvOsPrintf("Undefined HD workaround for port %d\n", port);
        }
#endif /* ETH_HALFDUPLEX_ERRATA */
    }

    /* Disable Giga Ethernet Unit interrupts */
    MV_REG_WRITE(ETH_UNIT_INTERRUPT_MASK_REG, 0);

    mvEthWinInit();

    /* Clear ETH_UNIT_INTERRUPT_CAUSE_REG register */
    MV_REG_WRITE(ETH_UNIT_INTERRUPT_CAUSE_REG, 0);

    mvEthMemAttrGet(&ethDescInSram, &ethDescSwCoher);

#if defined(ETH_DESCR_IN_SRAM)
        if(ethDescInSram == MV_FALSE)
        {
            mvOsPrintf("ethDrv: WARNING! Descriptors will be allocated in DRAM instead of SRAM.\n");
        }
#endif /* ETH_DESCR_IN_SRAM */
}

/*******************************************************************************
* mvEthMemAttrGet - Define properties (SRAM/DRAM, SW_COHER / HW_COHER / UNCACHED) 
*                       of of memory location for RX and TX descriptors.
*
* DESCRIPTION:
*       This function allocates memory for RX and TX descriptors.
*       - If ETH_DESCR_IN_SRAM defined, allocate from SRAM memory.
*       - If ETH_DESCR_IN_SDRAM defined, allocate from SDRAM memory.
*
* INPUT:
*   MV_BOOL* pIsSram - place of descriptors: 
*                      MV_TRUE  - in SRAM
*                      MV_FALSE - in DRAM
*   MV_BOOL* pIsSwCoher - cache coherency of descriptors:
*                      MV_TRUE  - driver is responsible for cache coherency
*                      MV_FALSE - driver is not responsible for cache coherency
*
* RETURN:
*
*******************************************************************************/
void   mvEthMemAttrGet(MV_BOOL* pIsSram, MV_BOOL* pIsSwCoher)
{
    MV_BOOL isSram, isSwCoher;

    isSram = MV_FALSE;
#if (ETHER_DRAM_COHER == MV_CACHE_COHER_SW) 
    isSwCoher = MV_TRUE;
#else 
    isSwCoher = MV_FALSE;
#endif

#if defined(ETH_DESCR_IN_SRAM)
    if( mvCtrlSramSizeGet() > 0)
    {
        isSram = MV_TRUE;
        #if (INTEG_SRAM_COHER == MV_CACHE_COHER_SW) 
            isSwCoher = MV_TRUE;
        #else 
            isSwCoher = MV_FALSE;
        #endif
    }
#endif /* ETH_DESCR_IN_SRAM */

    if(pIsSram != NULL)
        *pIsSram = isSram;

    if(pIsSwCoher != NULL)
        *pIsSwCoher = isSwCoher;
}








/******************************************************************************/
/*                      Port Initialization functions                         */
/******************************************************************************/

/*******************************************************************************
* mvEthPortInit - Initialize the Ethernet port driver
*
* DESCRIPTION:
*       This function initialize the ethernet port.
*       1) Allocate and initialize internal port Control structure.
*       2) Create RX and TX descriptor rings for default RX and TX queues
*       3) Disable RX and TX operations, clear cause registers and 
*          mask all interrupts.
*       4) Set all registers to default values and clean all MAC tables. 
*
* INPUT:
*       int             portNo          - Ethernet port number
*       ETH_PORT_INIT   *pEthPortInit   - Ethernet port init structure
*
* RETURN:
*       void* - ethernet port handler, that should be passed to the most other
*               functions dealing with this port.
*
* NOTE: This function is called once per port when loading the eth module.
*******************************************************************************/
void*   mvEthPortInit(int portNo, MV_ETH_PORT_INIT *pEthPortInit)
{
    int             queue, descSize;
    ETH_PORT_CTRL*  pPortCtrl;

    /* Check validity of parameters */
    if( (portNo >= mvCtrlEthMaxPortGet()) || 
        (pEthPortInit->rxDefQ   >= MV_ETH_RX_Q_NUM)  ||
        (pEthPortInit->maxRxPktSize < 1518) )
    {
        mvOsPrintf("EthPort #%d: Bad initialization parameters\n", portNo);
        return NULL;
    }
    if( (pEthPortInit->rxDescrNum[pEthPortInit->rxDefQ]) == 0)
    {
        mvOsPrintf("EthPort #%d: rxDefQ (%d) must be created\n", 
                    portNo, pEthPortInit->rxDefQ);
        return NULL;

    }
        
    pPortCtrl = (ETH_PORT_CTRL*)mvOsMalloc( sizeof(ETH_PORT_CTRL) );
    if(pPortCtrl == NULL)   
    {
       mvOsPrintf("EthDrv: Can't allocate %dB for port #%d control structure!\n",
                   (int)sizeof(ETH_PORT_CTRL), portNo);
       return NULL;
    }

    memset(pPortCtrl, 0, sizeof(ETH_PORT_CTRL) );
    ethPortCtrl[portNo] = pPortCtrl;

    pPortCtrl->portState = MV_UNDEFINED_STATE;

    pPortCtrl->portNo = portNo;

    /* Copy Configuration parameters */
    pPortCtrl->portConfig.maxRxPktSize = pEthPortInit->maxRxPktSize;
    pPortCtrl->portConfig.rxDefQ = pEthPortInit->rxDefQ;

    for( queue=0; queue<MV_ETH_RX_Q_NUM; queue++ )
    {
    pPortCtrl->rxQueueConfig[queue].descrNum = pEthPortInit->rxDescrNum[queue];
    }
    for( queue=0; queue<MV_ETH_TX_Q_NUM; queue++ )
    {
    pPortCtrl->txQueueConfig[queue].descrNum = pEthPortInit->txDescrNum[queue];
    }

    mvEthPortDisable(pPortCtrl);

    /* Set the board information regarding PHY address */
    mvEthPhyAddrSet(pPortCtrl, mvBoardPhyAddrGet(portNo) );

    /* Create all requested RX queues */
    for(queue=0; queue<MV_ETH_RX_Q_NUM; queue++)
    {
        if(pPortCtrl->rxQueueConfig[queue].descrNum == 0)
            continue;

        /* Allocate memory for RX descriptors */
        descSize = ((pPortCtrl->rxQueueConfig[queue].descrNum * ETH_RX_DESC_ALIGNED_SIZE) +
                                                        CPU_D_CACHE_LINE_SIZE);
 
        pPortCtrl->rxQueue[queue].descBuf.bufVirtPtr = 
                        ethAllocDescrMemory(pPortCtrl, descSize, 
                                    &pPortCtrl->rxQueue[queue].descBuf.bufPhysAddr);
        pPortCtrl->rxQueue[queue].descBuf.bufSize = descSize;
        if(pPortCtrl->rxQueue[queue].descBuf.bufVirtPtr == NULL)
        {
            mvOsPrintf("EthPort #%d, rxQ=%d: Can't allocate %d bytes in %s for %d RX descr\n", 
                        pPortCtrl->portNo, queue, descSize, 
                        ethDescInSram ? "SRAM" : "DRAM",
                        pPortCtrl->rxQueueConfig[queue].descrNum);
            return NULL;
        }

        ethInitRxDescRing(pPortCtrl, queue);
    }
    /* Create TX queues */
    for(queue=0; queue<MV_ETH_TX_Q_NUM; queue++)
    {
        if(pPortCtrl->txQueueConfig[queue].descrNum == 0)
            continue;

        /* Allocate memory for RX descriptors */
        descSize = ((pPortCtrl->txQueueConfig[queue].descrNum * ETH_TX_DESC_ALIGNED_SIZE) +
                                                        CPU_D_CACHE_LINE_SIZE);
 
        pPortCtrl->txQueue[queue].descBuf.bufVirtPtr = ethAllocDescrMemory(pPortCtrl, 
                             descSize, &pPortCtrl->txQueue[queue].descBuf.bufPhysAddr);
        pPortCtrl->txQueue[queue].descBuf.bufSize = descSize;
        if(pPortCtrl->txQueue[queue].descBuf.bufVirtPtr == NULL)
        {
            mvOsPrintf("EthPort #%d, txQ=%d: Can't allocate %d bytes in %s for %d TX descr\n", 
                        pPortCtrl->portNo, queue, descSize, ethDescInSram ? "SRAM" : "DRAM",
                        pPortCtrl->txQueueConfig[queue].descrNum);
            return NULL;
        }

#ifdef ETH_HALFDUPLEX_ERRATA
        if( MV64460B_DEV_ID == mvCtrlModelGet() )
        {
            pPortCtrl->txAlignedBuf[queue].bufSize = 0;
            pPortCtrl->txAlignedBuf[queue].bufVirtPtr = NULL;
            pPortCtrl->txAlignedBuf[queue].bufPhysAddr = 0;
        }
        else
        {
            pPortCtrl->txAlignedBuf[queue].bufSize = 
                (pPortCtrl->txQueueConfig[queue].descrNum *
                MV_ALIGN_UP(ethPortHdPadLen[portNo], CPU_D_CACHE_LINE_SIZE)) 
                + CPU_D_CACHE_LINE_SIZE;

            pPortCtrl->txAlignedBuf[queue].bufVirtPtr = 
                (char*)mvOsIoCachedMalloc(NULL, pPortCtrl->txAlignedBuf[queue].bufSize, 
                                        &pPortCtrl->txAlignedBuf[queue].bufPhysAddr);
            if(pPortCtrl->txAlignedBuf[queue].bufVirtPtr == NULL)
            {
            mvOsPrintf("EthPort #%d, txQ=%d: Can't allocate %d bytes for HD workaround\n", 
                pPortCtrl->portNo, queue, pPortCtrl->txAlignedBuf[queue].bufSize);
            return NULL;
            }
            memset(pPortCtrl->txAlignedBuf[queue].bufVirtPtr, 0, 
            pPortCtrl->txAlignedBuf[queue].bufSize);
        }
#endif /* ETH_HALFDUPLEX_ERRATA */

        ethInitTxDescRing(pPortCtrl, queue);
    }
    mvEthDefaultsSet(pPortCtrl);

    pPortCtrl->portState = MV_IDLE;
    return pPortCtrl;
}

/*******************************************************************************
* ethPortFinish - Finish the Ethernet port driver
*
* DESCRIPTION:
*       This function finish the ethernet port.
*       1) Down ethernet port if needed.
*       2) Delete RX and TX descriptor rings for all created RX and TX queues
*       3) Free internal port Control structure.
*
* INPUT:
*       void*   pEthPortHndl  - Ethernet port handler
*
* RETURN:   NONE.
*
*******************************************************************************/
void    mvEthPortFinish(void* pPortHndl)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHndl;
    int             queue, portNo  = pPortCtrl->portNo;

    if(pPortCtrl->portState == MV_ACTIVE)
    {
        mvOsPrintf("ethPort #%d: Warning !!! Finish port in Active state\n",
                 portNo);
        mvEthPortDisable(pPortHndl);
    }

    /* Free all allocated RX queues */
    for(queue=0; queue<MV_ETH_RX_Q_NUM; queue++)
    {
        ethFreeDescrMemory(pPortCtrl, &pPortCtrl->rxQueue[queue].descBuf);
    }

    /* Free all allocated TX queues */
    for(queue=0; queue<MV_ETH_TX_Q_NUM; queue++)
    {
        ethFreeDescrMemory(pPortCtrl, &pPortCtrl->txQueue[queue].descBuf);

#ifdef ETH_HALFDUPLEX_ERRATA
        if(pPortCtrl->txAlignedBuf[queue].bufVirtPtr != NULL) 
        {
            mvOsIoCachedFree(NULL, pPortCtrl->txAlignedBuf[queue].bufSize, 
                                     pPortCtrl->txAlignedBuf[queue].bufPhysAddr,
                                     pPortCtrl->txAlignedBuf[queue].bufVirtPtr);
        }
#endif /* ETH_HALFDUPLEX_ERRATA */
    }

    /* Free port control structure */
    mvOsFree(pPortCtrl);

    ethPortCtrl[portNo] = NULL;
}

/*******************************************************************************
* mvEthDefaultsSet - Set defaults to the ethernet port
*
* DESCRIPTION:
*       This function set default values to the ethernet port.
*       1) Clear Cause registers and Mask all interrupts
*       2) Clear all MAC tables
*       3) Set defaults to all registers
*       4) Reset all created RX and TX descriptors ring
*       5) Reset PHY
*
* INPUT:
*       void*   pEthPortHndl  - Ethernet port handler
*
* RETURN:   MV_STATUS  
*               MV_OK - Success, Others - Failure
* NOTE:
*   This function update all the port configuration except those set
*   Initialy by the OsGlue by MV_ETH_PORT_INIT.
*   This function can be called after portDown to return the port setting 
*   to defaults.
*******************************************************************************/
MV_STATUS   mvEthDefaultsSet(void* pPortHndl)
{
    int             ethPortNo, queue;
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHndl;
    ETH_QUEUE_CTRL* pQueueCtrl;
    MV_U32          txPrio;
    MV_U32      portCfgReg;
    MV_U32      portCfgExtReg;
    MV_U32      portSerialCtrlReg;
    MV_U32      portSdmaCfgReg;

    ethPortNo = pPortCtrl->portNo;

    /* Clear Cause registers */
    MV_REG_WRITE(ETH_INTR_CAUSE_REG(ethPortNo),0);
    MV_REG_WRITE(ETH_INTR_CAUSE_EXT_REG(ethPortNo),0);

    /* Mask all interrupts */
    MV_REG_WRITE(ETH_INTR_MASK_REG(ethPortNo),0);
    MV_REG_WRITE(ETH_INTR_MASK_EXT_REG(ethPortNo),0);

    portCfgReg  =   PORT_CONFIG_VALUE;
    portCfgExtReg  =  PORT_CONFIG_EXTEND_VALUE;

    portSerialCtrlReg =  PORT_SERIAL_CONTROL_VALUE;

#ifdef MV_88F5181
    if ((DB_88F5181_DDR1_PRPMC == mvBoardIdGet())||
        (RD_88F5181_VOIP == mvBoardIdGet())||
		(RD_88F5181L_VOIP_FE == mvBoardIdGet())
		)
    {
        portSerialCtrlReg =  PORT_SERIAL_CONTROL_100MB_FORCE_VALUE;
    }
	else if (RD_88F5181L_VOIP_GE == mvBoardIdGet())
	{        
        portSerialCtrlReg =  PORT_SERIAL_CONTROL_1000MB_FORCE_VALUE;       
	}
#endif /* MV_88F5181 */

    /* build PORT_SDMA_CONFIG_REG */
    portSdmaCfgReg = ETH_RX_INTR_COAL_MASK(0);
    portSdmaCfgReg |= ETH_TX_BURST_SIZE_MASK(ETH_BURST_SIZE_16_64BIT_VALUE);

#if defined(MV644xx) && ( (ETHER_DRAM_COHER == MV_CACHE_COHER_HW_WB) ||  \
                          (ETHER_DRAM_COHER == MV_CACHE_COHER_HW_WT) )
    /* some devices have restricted RX burst size when using HW coherency */
    portSdmaCfgReg |= ETH_RX_BURST_SIZE_MASK(ETH_BURST_SIZE_4_64BIT_VALUE);
#else
    portSdmaCfgReg |= ETH_RX_BURST_SIZE_MASK(ETH_BURST_SIZE_16_64BIT_VALUE);
#endif

#if defined(MV_CPU_BE)
    /* big endian */
# if defined(MV_88F5181)
    portSdmaCfgReg |= (ETH_RX_NO_DATA_SWAP_MASK |
                       ETH_TX_NO_DATA_SWAP_MASK |
                       ETH_DESC_SWAP_MASK);
# elif defined(MV644xx) 
    portSdmaCfgReg |= (ETH_RX_DATA_SWAP_MASK |
                       ETH_TX_DATA_SWAP_MASK |
                       ETH_NO_DESC_SWAP_MASK);
# endif
#else
    /* little endian */
    portSdmaCfgReg |= (ETH_RX_NO_DATA_SWAP_MASK | 
                       ETH_TX_NO_DATA_SWAP_MASK | 
                       ETH_NO_DESC_SWAP_MASK);
#endif

    pPortCtrl->portRxQueueCmdReg = 0;
    pPortCtrl->portTxQueueCmdReg = 0;

    ethSetUcastTable(ethPortNo, -1);
    mvEthSetSpecialMcastTable(ethPortNo, -1);
    mvEthSetOtherMcastTable(ethPortNo, -1);

    portSerialCtrlReg &= ~ETH_MAX_RX_PACKET_SIZE_MASK;

    portSerialCtrlReg |= mvEthMruGet(pPortCtrl->portConfig.maxRxPktSize);

    MV_REG_WRITE(ETH_PORT_SERIAL_CTRL_REG(ethPortNo), portSerialCtrlReg);

    /* Update value of PortConfig register accordingly with all RxQueue types */
    pPortCtrl->portConfig.rxArpQ = pPortCtrl->portConfig.rxDefQ;
    pPortCtrl->portConfig.rxBpduQ = pPortCtrl->portConfig.rxDefQ; 
    pPortCtrl->portConfig.rxTcpQ = pPortCtrl->portConfig.rxDefQ; 
    pPortCtrl->portConfig.rxUdpQ = pPortCtrl->portConfig.rxDefQ; 

    portCfgReg &= ~ETH_DEF_RX_QUEUE_ALL_MASK;
    portCfgReg |= ETH_DEF_RX_QUEUE_MASK(pPortCtrl->portConfig.rxDefQ);
    
    portCfgReg &= ~ETH_DEF_RX_ARP_QUEUE_ALL_MASK;
    portCfgReg |= ETH_DEF_RX_ARP_QUEUE_MASK(pPortCtrl->portConfig.rxArpQ); 

    portCfgReg &= ~ETH_DEF_RX_BPDU_QUEUE_ALL_MASK;
    portCfgReg |= ETH_DEF_RX_BPDU_QUEUE_MASK(pPortCtrl->portConfig.rxBpduQ);

    portCfgReg &= ~ETH_DEF_RX_TCP_QUEUE_ALL_MASK;
    portCfgReg |= ETH_DEF_RX_TCP_QUEUE_MASK(pPortCtrl->portConfig.rxTcpQ);

    portCfgReg &= ~ETH_DEF_RX_UDP_QUEUE_ALL_MASK;
    portCfgReg |= ETH_DEF_RX_UDP_QUEUE_MASK(pPortCtrl->portConfig.rxUdpQ);

    /* Assignment of Tx CTRP of given queue */
    txPrio = 0;
    for(queue=0; queue<MV_ETH_TX_Q_NUM; queue++)
    {
        pQueueCtrl = &pPortCtrl->txQueue[queue];

        if(pQueueCtrl->pFirstDescr != NULL)
        {
            ethResetTxDescRing(pPortCtrl, queue);

            MV_REG_WRITE(ETH_TX_TOKEN_COUNT_REG(ethPortNo, queue),
                         0x3fffffff);
            MV_REG_WRITE(ETH_TX_TOKEN_CFG_REG(ethPortNo, queue), 
                         0x03fffcff);

            mvEthTxQueueConfig(pPortCtrl, queue, MV_ETH_PRIO_FIXED, 0);

        }
        else
        {
            MV_REG_WRITE(ETH_TX_TOKEN_COUNT_REG(ethPortNo, queue),  0x0);
            MV_REG_WRITE(ETH_TX_TOKEN_CFG_REG(ethPortNo, queue), 0x0);
        }
    }

    /* Assignment of Rx CRDP of given queue */
    for(queue=0; queue<MV_ETH_RX_Q_NUM; queue++)
    {
        ethResetRxDescRing(pPortCtrl, queue);
    }

    /* Assign port configuration and command. */
    MV_REG_WRITE(ETH_PORT_CONFIG_REG(ethPortNo), portCfgReg);

    MV_REG_WRITE(ETH_PORT_CONFIG_EXTEND_REG(ethPortNo), portCfgExtReg);

    /* Assign port SDMA configuration */
    MV_REG_WRITE(ETH_SDMA_CONFIG_REG(ethPortNo), portSdmaCfgReg);
    
    /* Turn off the port/queue bandwidth limitation */
    MV_REG_WRITE(ETH_MAX_TRANSMIT_UNIT_REG(ethPortNo), 0x0);

    return MV_OK;
}

/*******************************************************************************
* ethPortUp - Start the Ethernet port RX and TX activity.
*
* DESCRIPTION:
*       This routine start Rx and Tx activity:
*
*       Note: Each Rx and Tx queue descriptor's list must be initialized prior
*       to calling this function (use etherInitTxDescRing for Tx queues and
*       etherInitRxDescRing for Rx queues).
*
* INPUT:
*       void*   pEthPortHndl  - Ethernet port handler
*
* RETURN:   MV_STATUS
*           MV_OK - Success, Others - Failure.
*
* NOTE : used for port link up.
*******************************************************************************/
MV_STATUS   mvEthPortUp(void* pEthPortHndl)
{
    int             ethPortNo;
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pEthPortHndl;

    ethPortNo = pPortCtrl->portNo;
    
    if( (pPortCtrl->portState != MV_ACTIVE) && 
        (pPortCtrl->portState != MV_PAUSED) )
    {
        mvOsPrintf("ethDrv port%d: Unexpected port state %d\n", 
                        ethPortNo, pPortCtrl->portState);
        return MV_BAD_STATE;
    }
    
    ethPortNo = pPortCtrl->portNo;

    /* Enable port RX. */
    MV_REG_WRITE(ETH_RX_QUEUE_COMMAND_REG(ethPortNo), pPortCtrl->portRxQueueCmdReg);

    /* Enable port TX. */
    MV_REG_VALUE(ETH_TX_QUEUE_COMMAND_REG(ethPortNo)) = pPortCtrl->portTxQueueCmdReg;

#ifdef ETH_HALFDUPLEX_ERRATA
    if( MV64460B_DEV_ID == mvCtrlModelGet() ) /* Errata fixed */
    {
        pPortCtrl->padLen = 0;
    }
    else
    {
        if( ETH_FULL_DUPLEX_MASK & MV_REG_READ(ETH_PORT_STATUS_REG(ethPortNo)) )
            pPortCtrl->padLen = 0;
        else
            pPortCtrl->padLen = ethPortHdPadLen[ethPortNo];
    }
#endif /* ETH_HALFDUPLEX_ERRATA */

    pPortCtrl->portState = MV_ACTIVE;

    return MV_OK;
}

/*******************************************************************************
* ethPortDown - Stop the Ethernet port activity.
*
* DESCRIPTION:
*
* INPUT:
*       void*   pEthPortHndl  - Ethernet port handler
*
* RETURN:   MV_STATUS
*               MV_OK - Success, Others - Failure.
*
* NOTE : used for port link down.
*******************************************************************************/
MV_STATUS   mvEthPortDown(void* pEthPortHndl)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pEthPortHndl;
    int             ethPortNum = pPortCtrl->portNo;
    unsigned int    regData;
    volatile int    rxTimeout, txTimeout, txFifoTimeout, delay;

    /* Stop Rx port activity. Check port Rx activity. */
    regData = (MV_REG_READ(ETH_RX_QUEUE_COMMAND_REG(ethPortNum))) & 0xFF;
    if(regData != 0)
    {
        /* Issue stop command for active channels only */
        MV_REG_WRITE(ETH_RX_QUEUE_COMMAND_REG(ethPortNum), (regData << 8));
    }

    /* Force link down */
    regData = MV_REG_READ(ETH_PORT_SERIAL_CTRL_REG(ethPortNum));
    regData &= ~(ETH_DO_NOT_FORCE_LINK_FAIL_MASK);
    MV_REG_WRITE(ETH_PORT_SERIAL_CTRL_REG(ethPortNum), regData);

    /* Wait for all Rx activity to terminate. */
    rxTimeout = 0;
    do
    {
        if(rxTimeout >= RX_DISABLE_TIMEOUT)
        {
            mvOsPrintf("ethPort_%d: TIMEOUT for RX stopped !!! rxQueueCmd - 0x08%x\n", 
                        ethPortNum, regData);
            break;
        }
        rxTimeout++;
        
        /* Check port RX Command register that all Rx queues are stopped */
        regData = MV_REG_READ(ETH_RX_QUEUE_COMMAND_REG(ethPortNum));
    }
    while(regData & 0xFF);

    /* Wait for all Tx activity to terminate. */
    txTimeout = 0;  
    do
    {
        if(txTimeout >= TX_DISABLE_TIMEOUT)
        {
            mvOsPrintf("ethPort_%d: TIMEOUT for TX stoped !!! txQueueCmd - 0x08%x\n", 
                        ethPortNum, regData);
            break;
        }
        txTimeout++;

        /* Check port TX Command register that all Tx queues are stopped */
        regData = MV_REG_READ(ETH_TX_QUEUE_COMMAND_REG(ethPortNum));
    }
    while(regData & 0xFF);
    
    /* Double check to Verify that TX FIFO is Empty */
    txFifoTimeout = 0;
    while(MV_TRUE)
    {
        do
        {
            if(txFifoTimeout >= TX_FIFO_EMPTY_TIMEOUT)
            {
                mvOsPrintf("\n ethPort_%d: TIMEOUT for TX FIFO empty !!! portStatus - 0x08%x\n", 
                            ethPortNum, regData);
                break;
            }
            txFifoTimeout++;

            regData = MV_REG_READ(ETH_PORT_STATUS_REG(ethPortNum));
        }
        while( ((regData & ETH_TX_FIFO_EMPTY_MASK) == 0) || 
               ((regData & ETH_TX_IN_PROGRESS_MASK) != 0) );

        if(txFifoTimeout >= TX_FIFO_EMPTY_TIMEOUT)
            break;

        /* Double check */
        regData = MV_REG_READ(ETH_PORT_STATUS_REG(ethPortNum));
        if( ((regData & ETH_TX_FIFO_EMPTY_MASK) != 0) && 
            ((regData & ETH_TX_IN_PROGRESS_MASK) == 0) )
        {
            break;
        }
        else
            mvOsPrintf("ethPort_%d: TX FIFO Empty double check failed. %d loops, portStatus=0x%x\n",
                                ethPortNum, txFifoTimeout, regData);     
    }

    /* Do NOT force link down */
    regData = MV_REG_READ(ETH_PORT_SERIAL_CTRL_REG(ethPortNum));
    regData |= (ETH_DO_NOT_FORCE_LINK_FAIL_MASK);
    MV_REG_WRITE(ETH_PORT_SERIAL_CTRL_REG(ethPortNum), regData);

    delay = (PORT_DISABLE_WAIT_TCLOCKS*(mvCpuPclkGet()/mvBoardTclkGet()));

    /* Wait about 2500 tclk cycles */
    for(delay; delay>0; delay--);

    pPortCtrl->portState = MV_PAUSED;

    return MV_OK;   
}


/*******************************************************************************
* ethPortEnable - Enable the Ethernet port and Start RX and TX.
*
* DESCRIPTION:
*       This routine enable the Ethernet port and Rx and Tx activity:
*
*       Note: Each Rx and Tx queue descriptor's list must be initialized prior
*       to calling this function (use etherInitTxDescRing for Tx queues and
*       etherInitRxDescRing for Rx queues).
*
* INPUT:
*       void*   pEthPortHndl  - Ethernet port handler
*
* RETURN:   MV_STATUS
*               MV_OK - Success, Others - Failure.
*
* NOTE: main usage is to enable the port after ifconfig up.
*******************************************************************************/
MV_STATUS   mvEthPortEnable(void* pEthPortHndl)
{
    int             ethPortNo;
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pEthPortHndl;
    MV_U32      portSerialCtrlReg;


    ethPortNo = pPortCtrl->portNo;

    /* Enable port */
    portSerialCtrlReg = MV_REG_READ(ETH_PORT_SERIAL_CTRL_REG(ethPortNo));
    MV_REG_WRITE(ETH_PORT_SERIAL_CTRL_REG(ethPortNo),
                 portSerialCtrlReg | ETH_PORT_ENABLE_MASK);

    mvEthMibCountersClear(pEthPortHndl);

    pPortCtrl->portState = MV_PAUSED;

    /* If Link is UP, Start RX and TX traffic */
    if( MV_REG_READ( ETH_PORT_STATUS_REG(ethPortNo) ) & ETH_LINK_UP_MASK)
        return( mvEthPortUp(pEthPortHndl) );
    
    return MV_NOT_READY;
}


/*******************************************************************************
* mvEthPortDisable - Stop RX and TX activities and Disable the Ethernet port.
*
* DESCRIPTION:
*
* INPUT:
*       void*   pEthPortHndl  - Ethernet port handler
*
* RETURN:   MV_STATUS
*               MV_OK - Success, Others - Failure.
*
* NOTE: main usage is to disable the port after ifconfig down.
*******************************************************************************/
MV_STATUS   mvEthPortDisable(void* pEthPortHndl)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pEthPortHndl;
    int             ethPortNum = pPortCtrl->portNo;
    unsigned int    regData;
    volatile int    delay;

    if(pPortCtrl->portState == MV_ACTIVE)           
    {    
        /* Stop RX and TX activities */
        mvEthPortDown(pEthPortHndl);
    }

    /* Reset the Enable bit in the Serial Control Register */
    regData = MV_REG_READ(ETH_PORT_SERIAL_CTRL_REG(ethPortNum));
    regData &= ~(ETH_PORT_ENABLE_MASK);
    MV_REG_WRITE(ETH_PORT_SERIAL_CTRL_REG(ethPortNum), regData);

    /* Wait about 2500 tclk cycles */
    delay = (PORT_DISABLE_WAIT_TCLOCKS*(mvCpuPclkGet()/mvBoardTclkGet()));
    for(delay; delay>0; delay--);

    pPortCtrl->portState = MV_IDLE;
    return MV_OK;
}








/******************************************************************************/
/*                          Data Flow functions                               */
/******************************************************************************/

/*******************************************************************************
* mvEthPortTx - Send an Ethernet packet
*
* DESCRIPTION:
*       This routine send a given packet described by pBufInfo parameter. It
*       supports transmitting of a packet spaned over multiple buffers. 
*
* INPUT:
*       void*       pEthPortHndl  - Ethernet Port handler.
*       int         txQueue       - Number of Tx queue.
*       MV_PKT_INFO *pPktInfo     - User packet to send.
*
* RETURN:
*       MV_NO_RESOURCE  - No enough resources to send this packet.
*       MV_ERROR        - Unexpected Fatal error.
*       MV_OK           - Packet send successfully.
*
*******************************************************************************/
MV_STATUS   mvEthPortTx(void* pEthPortHndl, int txQueue, MV_PKT_INFO* pPktInfo)
{
    ETH_TX_DESC*    pTxFirstDesc;
    ETH_TX_DESC*    pTxCurrDesc;
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pEthPortHndl;
    ETH_QUEUE_CTRL* pQueueCtrl;
    int             portNo, bufCount;
    MV_BUF_INFO*    pBufInfo = pPktInfo->pFrags;
    MV_U8*          pTxBuf;

#ifdef ETH_DEBUG
    if(pPortCtrl->portState != MV_ACTIVE)
        return MV_BAD_STATE;
#endif /* ETH_DEBUG */

    portNo = pPortCtrl->portNo;
    pQueueCtrl = &pPortCtrl->txQueue[txQueue];

    /* Get the Tx Desc ring indexes */
    pTxCurrDesc = pQueueCtrl->pCurrentDescr;

#ifdef ETH_DEBUG
    if(pTxCurrDesc == NULL)
        return MV_ERROR;
#endif /* ETH_DEBUG */

#ifdef ETH_HALFDUPLEX_ERRATA
    if(pPktInfo->pktSize < pPortCtrl->padLen)
    {
        pTxBuf = pTxCurrDesc->alignBufPtr;
        for(bufCount=0; bufCount<pPktInfo->numFrags; bufCount++)
        {
            memcpy(pTxBuf, pBufInfo[bufCount].bufVirtPtr, 
                           pBufInfo[bufCount].bufSize);
            pTxBuf += pBufInfo[bufCount].bufSize;
        }
        if(pPktInfo->pktSize < 60)
        {
            /* Aligned data size up to 8 bytes */
            int newSize = MV_ALIGN_UP(pPktInfo->pktSize, 8);

            memset(pTxBuf, 0, newSize - pPktInfo->pktSize);
            pPktInfo->pktSize = newSize;
        }
        /* Change PktInfo structure */
        pPktInfo->numFrags = 1;
        pBufInfo[0].bufVirtPtr = pTxCurrDesc->alignBufPtr;
        pBufInfo[0].bufSize = pPktInfo->pktSize;
    }
#endif /* ETH_HALFDUPLEX_ERRATA */

    /* Check if there is enough resources to send the packet */
    if(pQueueCtrl->resource < pPktInfo->numFrags)
        return MV_NO_RESOURCE;

    /* Remember first desc */
    pTxFirstDesc  = pTxCurrDesc;

    bufCount = 0;
    while(MV_TRUE)
    {   
        if(pBufInfo[bufCount].bufSize <= MIN_TX_BUFF_LOAD)
        {
            /* Buffers with a payload smaller than MIN_TX_BUFF_LOAD (8 bytes) must be aligned    */
            /* to 64-bit boundary. Two options here:                                             */
            /* 1) Usually, copy the payload to the reserved 8 bytes inside descriptor.           */
            /* 2) In the Half duplex workaround, the reserved 8 bytes inside descriptor are used */ 
            /*    as a pointer to the aligned buffer, copy the small payload to this buffer.     */
#ifdef ETH_HALFDUPLEX_ERRATA
            /* For MV64460B - half duplex errata is fixed, but image is common for all MV644xx */
            /* ETH_HALFDUPLEX_ERRATA is defined, but txAlignedBuf is not allocated            */
            /* so buffers less than MIN_TX_BUFF_LOAD will be copied to the descriptor         */
            if(pPortCtrl->txAlignedBuf[txQueue].bufVirtPtr != NULL)
            {
                pTxBuf = pTxCurrDesc->alignBufPtr;
                ethBCopy(pBufInfo[bufCount].bufVirtPtr, pTxBuf, pBufInfo[bufCount].bufSize);
                pTxCurrDesc->bufPtr = mvOsIoVirtToPhy(NULL, pTxBuf);
                ETH_PACKET_CACHE_FLUSH(pTxBuf, pBufInfo[bufCount].bufSize);
            }
            else
#endif /* ETH_HALFDUPLEX_ERRATA */
            {
                pTxBuf = ((MV_U8*)pTxCurrDesc)+TX_BUF_OFFSET_IN_DESC;
                ethBCopy(pBufInfo[bufCount].bufVirtPtr, pTxBuf, pBufInfo[bufCount].bufSize);
                pTxCurrDesc->bufPtr = mvOsIoVirtToPhy(NULL, pTxBuf);
            }
        }
        else
        {
            pTxCurrDesc->bufPtr = mvOsIoVirtToPhy(NULL, pBufInfo[bufCount].bufVirtPtr);
            /* Flush Buffer */
            ETH_PACKET_CACHE_FLUSH(pBufInfo[bufCount].bufVirtPtr, pBufInfo[bufCount].bufSize);
        }

        pTxCurrDesc->byteCnt = pBufInfo[bufCount].bufSize;
        bufCount++;

        if(bufCount >= pPktInfo->numFrags)
            break;

        if(bufCount > 1)
        {
            /* There is middle buffer of the packet Not First and Not Last */
            pTxCurrDesc->cmdSts = ETH_BUFFER_OWNED_BY_DMA;
            ETH_DESCR_FLUSH_INV(pPortCtrl, pTxCurrDesc);
        }
        /* Go to next descriptor and next buffer */
        pTxCurrDesc = TX_NEXT_DESC_PTR(pTxCurrDesc, pQueueCtrl);
    }
    /* Set last desc with DMA ownership and interrupt enable. */
    pTxCurrDesc->returnInfo = pPktInfo->osInfo;
    if(bufCount == 1) 
    {
        /* There is only one buffer in the packet */
        /* The OSG might set some bits for checksum offload, so add them to first descriptor */
        pTxCurrDesc->cmdSts = pPktInfo->status              |
                              ETH_BUFFER_OWNED_BY_DMA       |
                              ETH_TX_GENERATE_CRC_MASK      |
                              ETH_TX_ENABLE_INTERRUPT_MASK  |
                              ETH_TX_ZERO_PADDING_MASK      |
                              ETH_TX_FIRST_DESC_MASK        |
                              ETH_TX_LAST_DESC_MASK;

        ETH_DESCR_FLUSH_INV(pPortCtrl, pTxCurrDesc);
    }
    else
    {
        /* Last but not First */
        pTxCurrDesc->cmdSts = ETH_BUFFER_OWNED_BY_DMA       |
                              ETH_TX_ENABLE_INTERRUPT_MASK  |
                              ETH_TX_ZERO_PADDING_MASK      |
                              ETH_TX_LAST_DESC_MASK;

        ETH_DESCR_FLUSH_INV(pPortCtrl, pTxCurrDesc);

        /* Update First when more than one buffer in the packet */
        /* The OSG might set some bits for checksum offload, so add them to first descriptor */
        pTxFirstDesc->cmdSts = pPktInfo->status             |
                               ETH_BUFFER_OWNED_BY_DMA      |
                               ETH_TX_GENERATE_CRC_MASK     |
                               ETH_TX_FIRST_DESC_MASK;

        ETH_DESCR_FLUSH_INV(pPortCtrl, pTxFirstDesc);
    }

    /* Apply send command */
    MV_REG_VALUE(ETH_TX_QUEUE_COMMAND_REG(portNo)) = pPortCtrl->portTxQueueCmdReg;

    /* Update txQueue state */
    pQueueCtrl->resource -= bufCount;
    pQueueCtrl->pCurrentDescr = TX_NEXT_DESC_PTR(pTxCurrDesc, pQueueCtrl);

    return MV_OK;
}

/*******************************************************************************
* mvEthPortTxDone - Free all used Tx descriptors and mBlks.
*
* DESCRIPTION:
*       This routine returns the transmitted packet information to the caller.
*
* INPUT:
*       void*       pEthPortHndl    - Ethernet Port handler.
*       int         txQueue         - Number of Tx queue.
*
* OUTPUT:
*       MV_PKT_INFO *pPktInfo       - Pointer to packet was sent.
*
* RETURN:
*       MV_NOT_FOUND    - No transmitted packets to return. Transmit in progress.
*       MV_EMPTY        - No transmitted packets to return. TX Queue is empty.
*       MV_ERROR        - Unexpected Fatal error.
*       MV_OK           - There is transmitted packet in the queue, 
*                       'pPktInfo' filled with relevant information.
*
*******************************************************************************/
MV_STATUS   mvEthPortTxDone(void* pEthPortHndl, int txQueue, MV_PKT_INFO *pPktInfo)
{
    ETH_TX_DESC*    pTxCurrDesc;
    ETH_TX_DESC*    pTxUsedDesc;
    ETH_QUEUE_CTRL* pQueueCtrl;
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pEthPortHndl;
    MV_U32          commandStatus;

    pQueueCtrl = &pPortCtrl->txQueue[txQueue];

    pTxUsedDesc = pQueueCtrl->pUsedDescr;
    pTxCurrDesc = pQueueCtrl->pCurrentDescr;

    /* Sanity check */
#ifdef ETH_DEBUG
    if(pTxUsedDesc == 0)
        return MV_ERROR;
#endif /* ETH_DEBUG */

    while(MV_TRUE)
    {
        /* No more used descriptors */
        commandStatus = pTxUsedDesc->cmdSts;
        if (commandStatus  & (ETH_BUFFER_OWNED_BY_DMA))
        {           
            ETH_DESCR_INV(pPortCtrl, pTxUsedDesc);
            return MV_NOT_FOUND;
        }
        if( (pTxUsedDesc == pTxCurrDesc) &&
            (pQueueCtrl->resource != 0) )
        {
            return MV_EMPTY;
        }
        pQueueCtrl->resource++;
        pQueueCtrl->pUsedDescr = TX_NEXT_DESC_PTR(pTxUsedDesc, pQueueCtrl);
        if(commandStatus & (ETH_TX_LAST_DESC_MASK)) 
        {
            pPktInfo->status  = commandStatus;
            pPktInfo->osInfo  = pTxUsedDesc->returnInfo;
            
            return MV_OK;
        }
        pTxUsedDesc = pQueueCtrl->pUsedDescr;
    }
}

/*******************************************************************************
* mvEthPortForceTxDone - Get next buffer from TX queue in spite of buffer ownership.
*
* DESCRIPTION:
*       This routine used to free buffers attached to the Tx ring and should
*       be called only when Giga Ethernet port is Down
*
* INPUT:
*       void*       pEthPortHndl    - Ethernet Port handler.
*       int         txQueue         - Number of TX queue.
*
* OUTPUT:
*       MV_PKT_INFO *pPktInfo       - Pointer to packet was sent.
*
* RETURN:
*       MV_EMPTY    - There is no more buffers in this queue.
*       MV_OK       - Buffer detached from the queue and pPktInfo structure 
*                   filled with relevant information.
*
*******************************************************************************/
MV_STATUS   mvEthPortForceTxDone(void* pEthPortHndl, int txQueue, MV_PKT_INFO *pPktInfo)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pEthPortHndl;
    ETH_QUEUE_CTRL* pQueueCtrl;
    int             port = pPortCtrl->portNo;
       
    pQueueCtrl = &pPortCtrl->txQueue[txQueue];

    while( (pQueueCtrl->pUsedDescr != pQueueCtrl->pCurrentDescr) ||
           (pQueueCtrl->resource == 0) )
    {
        /* Free next descriptor */
        pQueueCtrl->resource++;
        pPktInfo->status = ((ETH_TX_DESC*)pQueueCtrl->pUsedDescr)->cmdSts;
        pPktInfo->osInfo  = ((ETH_TX_DESC*)pQueueCtrl->pUsedDescr)->returnInfo;
        ((ETH_TX_DESC*)pQueueCtrl->pUsedDescr)->cmdSts = 0x0;
        ((ETH_TX_DESC*)pQueueCtrl->pUsedDescr)->returnInfo = 0x0;
        ETH_DESCR_FLUSH_INV(pPortCtrl, pQueueCtrl->pUsedDescr);

        pQueueCtrl->pUsedDescr = TX_NEXT_DESC_PTR(pQueueCtrl->pUsedDescr, pQueueCtrl);

        if(pPktInfo->status  & ETH_TX_LAST_DESC_MASK) 
            return MV_OK;
    }   
    MV_REG_WRITE( ETH_TX_CUR_DESC_PTR_REG(port, txQueue), 
                    (MV_U32)ethDescVirtToPhy(pPortCtrl, pQueueCtrl->pCurrentDescr) );
    return MV_EMPTY;
}

        
/*******************************************************************************
* mvEthPortRx - Get new received packets from Rx queue.
*
* DESCRIPTION:
*       This routine returns the received data to the caller. There is no
*       data copying during routine operation. All information is returned
*       using pointer to packet information struct passed from the caller.
*
* INPUT:
*       void*       pEthPortHndl    - Ethernet Port handler.
*       int         rxQueue         - Number of Rx queue.
*
* OUTPUT:
*       MV_PKT_INFO *pPktInfo       - Pointer to received packet.
*
* RETURN:
*       MV_NO_RESOURCE  - No free resources in RX queue.
*       MV_ERROR        - Unexpected Fatal error.
*       MV_OK           - New packet received and 'pBufInfo' structure filled
*                       with relevant information.
*
*******************************************************************************/
MV_STATUS   mvEthPortRx(void* pEthPortHndl, int rxQueue, MV_PKT_INFO *pPktInfo)
{
    ETH_RX_DESC     *pRxCurrDesc;
    MV_U32          commandStatus;
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pEthPortHndl;
    ETH_QUEUE_CTRL* pQueueCtrl;

    pQueueCtrl = &(pPortCtrl->rxQueue[rxQueue]);

    /* Check resources */
    if(pQueueCtrl->resource == 0)
        return MV_NO_RESOURCE;
    
    while(MV_TRUE)
    {
        /* Get the Rx Desc ring 'curr and 'used' indexes */
        pRxCurrDesc = pQueueCtrl->pCurrentDescr;
    
        /* Sanity check */
#ifdef ETH_DEBUG
        if (pRxCurrDesc == 0)
            return MV_ERROR;
#endif  /* ETH_DEBUG */

        commandStatus   = pRxCurrDesc->cmdSts;
        if (commandStatus & (ETH_BUFFER_OWNED_BY_DMA))
        {
            /* Nothing to receive... */
            ETH_DESCR_INV(pPortCtrl, pRxCurrDesc);
            return MV_NO_MORE;
        }

        /* Valid RX only if FIRST and LAST bits are set */
        if( (commandStatus & (ETH_RX_LAST_DESC_MASK | ETH_RX_FIRST_DESC_MASK)) == 
                             (ETH_RX_LAST_DESC_MASK | ETH_RX_FIRST_DESC_MASK) )
        {
            pPktInfo->pktSize    = pRxCurrDesc->byteCnt;
            pPktInfo->status     = commandStatus;
            pPktInfo->osInfo     = pRxCurrDesc->returnInfo;
            pPktInfo->fragIP     = pRxCurrDesc->bufSize & ETH_RX_IP_FRAGMENTED_FRAME_MASK;

            pQueueCtrl->resource--;
            /* Update 'curr' in data structure */
            pQueueCtrl->pCurrentDescr = RX_NEXT_DESC_PTR(pRxCurrDesc, pQueueCtrl);

#ifdef INCLUDE_SYNC_BARR
            mvCpuIfSyncBarr(DRAM_TARGET);
#endif
            return MV_OK;
        }
        else
        {
            ETH_RX_DESC*    pRxUsedDesc = pQueueCtrl->pUsedDescr;

#ifdef ETH_DEBUG
            mvOsPrintf("ethDrv: Unexpected Jumbo frame: "
                       "status=0x%08x, byteCnt=%d, pData=0x%x\n", 
                 commandStatus, pRxCurrDesc->byteCnt, pRxCurrDesc->bufPtr);
#endif /* ETH_DEBUG */
            /* move buffer from pCurrentDescr position to pUsedDescr position */
            pRxUsedDesc->bufPtr     = pRxCurrDesc->bufPtr;
            pRxUsedDesc->returnInfo = pRxCurrDesc->returnInfo;
            pRxUsedDesc->bufSize    = pRxCurrDesc->bufSize & ETH_RX_BUFFER_MASK;

            /* Return the descriptor to DMA ownership */
            pRxUsedDesc->cmdSts = ETH_BUFFER_OWNED_BY_DMA | 
                                  ETH_RX_ENABLE_INTERRUPT_MASK;

            /* Flush descriptor and CPU pipe */
            ETH_DESCR_FLUSH_INV(pPortCtrl, pRxUsedDesc);

            /* Move the used descriptor pointer to the next descriptor */
            pQueueCtrl->pUsedDescr = RX_NEXT_DESC_PTR(pRxUsedDesc, pQueueCtrl);
            pQueueCtrl->pCurrentDescr = RX_NEXT_DESC_PTR(pRxCurrDesc, pQueueCtrl);            
        }
    }
}

/*******************************************************************************
* mvEthPortRxDone - Returns a Rx buffer back to the Rx ring.
*
* DESCRIPTION:
*       This routine returns a Rx buffer back to the Rx ring. 
*
* INPUT:
*       void*       pEthPortHndl    - Ethernet Port handler.
*       int         rxQueue         - Number of Rx queue.
*       MV_PKT_INFO *pPktInfo       - Pointer to received packet.
*
* RETURN:
*       MV_ERROR        - Unexpected Fatal error.
*       MV_OUT_OF_RANGE - RX queue is already FULL, so this buffer can't be 
*                       returned to this queue.
*       MV_FULL         - Buffer returned successfully and RX queue became full.
*                       More buffers should not be returned at the time.
*       MV_OK           - Buffer returned successfully and there are more free 
*                       places in the queue.
*
*******************************************************************************/
MV_STATUS   mvEthPortRxDone(void* pEthPortHndl, int rxQueue, MV_PKT_INFO *pPktInfo)
{
    ETH_RX_DESC*    pRxUsedDesc;
    ETH_QUEUE_CTRL* pQueueCtrl;
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pEthPortHndl;
            
    pQueueCtrl = &pPortCtrl->rxQueue[rxQueue];

    /* Get 'used' Rx descriptor */
    pRxUsedDesc = pQueueCtrl->pUsedDescr;

#ifdef ETH_DEBUG
    /* Sanity check */
    if (pRxUsedDesc == NULL)
        return MV_ERROR;
#endif /* ETH_DEBUG */

    /* Check that ring is not FULL */
    if( (pQueueCtrl->pUsedDescr == pQueueCtrl->pCurrentDescr) && 
        (pQueueCtrl->resource != 0) )
    {
        mvOsPrintf("%s %d: out of range Error resource=%d, curr=%p, used=%p\n", 
                    __FUNCTION__, pPortCtrl->portNo, pQueueCtrl->resource, 
                    pQueueCtrl->pCurrentDescr, pQueueCtrl->pUsedDescr);
        return MV_OUT_OF_RANGE;
    }

    pRxUsedDesc->bufPtr     = mvOsIoVirtToPhy(pPortCtrl, pPktInfo->pFrags->bufVirtPtr);
    pRxUsedDesc->returnInfo = pPktInfo->osInfo;
    pRxUsedDesc->bufSize    = pPktInfo->pFrags->bufSize & ETH_RX_BUFFER_MASK;

    /* Invalidate data buffer accordingly with pktSize */
    ETH_PACKET_CACHE_INVALIDATE(pPktInfo->pFrags->bufVirtPtr, pPktInfo->pktSize);

    /* Return the descriptor to DMA ownership */
    pRxUsedDesc->cmdSts = ETH_BUFFER_OWNED_BY_DMA | ETH_RX_ENABLE_INTERRUPT_MASK;

    /* Flush descriptor and CPU pipe */
    ETH_DESCR_FLUSH_INV(pPortCtrl, pRxUsedDesc);

    pQueueCtrl->resource++;

    /* Move the used descriptor pointer to the next descriptor */
    pQueueCtrl->pUsedDescr = RX_NEXT_DESC_PTR(pRxUsedDesc, pQueueCtrl);
    
    /* If ring became Full return MV_FULL */
    if(pQueueCtrl->pUsedDescr == pQueueCtrl->pCurrentDescr) 
        return MV_FULL;

    return MV_OK;
}

/*******************************************************************************
* mvEthPortForceRx - Get next buffer from RX queue in spite of buffer ownership.
*
* DESCRIPTION:
*       This routine used to free buffers attached to the Rx ring and should
*       be called only when Giga Ethernet port is Down
*
* INPUT:
*       void*       pEthPortHndl    - Ethernet Port handler.
*       int         rxQueue         - Number of Rx queue.
*
* OUTPUT:
*       MV_PKT_INFO *pPktInfo       - Pointer to received packet.
*
* RETURN:
*       MV_EMPTY    - There is no more buffers in this queue.
*       MV_OK       - Buffer detached from the queue and pBufInfo structure 
*                   filled with relevant information.
*
*******************************************************************************/
MV_STATUS   mvEthPortForceRx(void* pEthPortHndl, int rxQueue, MV_PKT_INFO *pPktInfo)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pEthPortHndl;
    ETH_QUEUE_CTRL* pQueueCtrl;
    int             port = pPortCtrl->portNo;
       
    pQueueCtrl = &pPortCtrl->rxQueue[rxQueue];

    if(pQueueCtrl->resource == 0)
    {
        MV_REG_WRITE( ETH_RX_CUR_DESC_PTR_REG(port, rxQueue), 
                    (MV_U32)ethDescVirtToPhy(pPortCtrl, pQueueCtrl->pCurrentDescr) );

        return MV_EMPTY;
    }
    /* Free next descriptor */
    pQueueCtrl->resource--;
    pPktInfo->status  = ((ETH_RX_DESC*)pQueueCtrl->pCurrentDescr)->cmdSts;
    pPktInfo->osInfo  = ((ETH_RX_DESC*)pQueueCtrl->pCurrentDescr)->returnInfo;
    ((ETH_RX_DESC*)pQueueCtrl->pCurrentDescr)->cmdSts = 0x0;
    ((ETH_RX_DESC*)pQueueCtrl->pCurrentDescr)->returnInfo = 0x0;
    ETH_DESCR_FLUSH_INV(pPortCtrl, pQueueCtrl->pCurrentDescr);

    pQueueCtrl->pCurrentDescr = RX_NEXT_DESC_PTR(pQueueCtrl->pCurrentDescr, pQueueCtrl);
    return MV_OK;    
}

/******************************************************************************/
/*                          Port Status functions                             */
/******************************************************************************/
/* Get number of Free resources in specific TX queue */
int     mvEthTxResourceGet(void* pPortHndl, int txQueue)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHndl;

    return (pPortCtrl->txQueue[txQueue].resource);      
}

/* Get number of Free resources in specific RX queue */
int     mvEthRxResourceGet(void* pPortHndl, int rxQueue)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHndl;

    return (pPortCtrl->rxQueue[rxQueue].resource);      
}







/******************************************************************************/
/*                          Port Configuration functions                      */
/******************************************************************************/
/*******************************************************************************
* mvEthMruGet - Get MRU configuration for Max Rx packet size.
*
* INPUT:
*           MV_U32 maxRxPktSize - max  packet size.
*
* RETURN:   MV_U32 - MRU configuration.
*
*******************************************************************************/
static MV_U32 mvEthMruGet(MV_U32 maxRxPktSize)
{
    MV_U32 portSerialCtrlReg = 0;

    if(maxRxPktSize > 9192)
        portSerialCtrlReg |= ETH_MAX_RX_PACKET_9700BYTE;
    else if(maxRxPktSize > 9022)
        portSerialCtrlReg |= ETH_MAX_RX_PACKET_9192BYTE;
    else if(maxRxPktSize > 1552)
        portSerialCtrlReg |= ETH_MAX_RX_PACKET_9022BYTE;
    else if(maxRxPktSize > 1522)
        portSerialCtrlReg |= ETH_MAX_RX_PACKET_1552BYTE;
    else if(maxRxPktSize > 1518)
        portSerialCtrlReg |= ETH_MAX_RX_PACKET_1522BYTE;
    else
        portSerialCtrlReg |= ETH_MAX_RX_PACKET_1518BYTE;

    return portSerialCtrlReg;
}

/*******************************************************************************
* mvEthRxCoalSet  - Sets coalescing interrupt mechanism on RX path
*
* DESCRIPTION:
*       This routine sets the RX coalescing interrupt mechanism parameter.
*       This parameter is a timeout counter, that counts in 64 tClk
*       chunks, that when timeout event occurs a maskable interrupt occurs.
*       The parameter is calculated using the tCLK frequency of the
*       MV-64xxx chip, and the required number is in micro seconds.
*
* INPUT:
*       void*           pPortHndl   - Ethernet Port handler.
*       MV_U32          uSec        - Number of micro seconds between 
*                                   RX interrupts
*
* RETURN:
*       None.
*
* COMMENT:     
*   1 sec           - TCLK_RATE clocks
*   1 uSec          - TCLK_RATE / 1,000,000 clocks
*
*   Register Value for N micro seconds -  ((N * ( (TCLK_RATE / 1,000,000)) / 64)
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_U32    mvEthRxCoalSet (void* pPortHndl, MV_U32 uSec) 
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHndl;
    MV_U32          coal = ((uSec * (mvBoardTclkGet() / 1000000)) / 64);
    MV_U32          portSdmaCfgReg;

    portSdmaCfgReg =  MV_REG_READ(ETH_SDMA_CONFIG_REG(pPortCtrl->portNo));
    portSdmaCfgReg &= ~ETH_RX_INTR_COAL_ALL_MASK;
    portSdmaCfgReg |= (ETH_RX_INTR_COAL_MASK(coal));
    MV_REG_WRITE (ETH_SDMA_CONFIG_REG(pPortCtrl->portNo), portSdmaCfgReg);
    return coal;
}

/*******************************************************************************
* mvEthTxCoalSet - Sets coalescing interrupt mechanism on TX path
*
* DESCRIPTION:
*       This routine sets the TX coalescing interrupt mechanism parameter.
*       This parameter is a timeout counter, that counts in 64 tClk
*       chunks, that when timeout event occurs a maskable interrupt
*       occurs.
*       The parameter is calculated using the tCLK frequency of the
*       MV-64xxx chip, and the required number is in micro seconds.
*
* INPUT:
*       void*           pPortHndl    - Ethernet Port handler.
*       MV_U32          uSec        - Number of micro seconds between 
*                                   RX interrupts
*
* RETURN:
*       None.
*
* COMMENT:     
*   1 sec           - TCLK_RATE clocks
*   1 uSec          - TCLK_RATE / 1,000,000 clocks
*
*   Register Value for N micro seconds -  ((N * ( (TCLK_RATE / 1,000,000)) / 64)
*
*******************************************************************************/
MV_U32    mvEthTxCoalSet(void* pPortHndl, MV_U32 uSec) 
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHndl;
    MV_U32          coal = ((uSec * (mvBoardTclkGet() / 1000000)) / 64);

    /* Set TX Coalescing mechanism */
    MV_REG_WRITE (ETH_TX_FIFO_URGENT_THRESH_REG(pPortCtrl->portNo), 
                  (coal << ETH_TX_INTR_COAL_OFFSET));
    return coal;
}

/*******************************************************************************
* mvEthCoalGet - Gets RX and TX coalescing values in micro seconds
*
* DESCRIPTION:
*       This routine gets the RX and TX coalescing interrupt values.
*       The parameter is calculated using the tCLK frequency of the
*       MV-64xxx chip, and the returned numbers are in micro seconds.
*
* INPUTs:
*       void*   pPortHndl   - Ethernet Port handler.
*
* OUTPUTs:
*       MV_U32* pRxCoal     - Number of micro seconds between RX interrupts
*       MV_U32* pTxCoal     - Number of micro seconds between TX interrupts
*
* RETURN:
*       MV_STATUS   MV_OK  - success
*                   Others - failure.
*
* COMMENT:     
*   1 sec           - TCLK_RATE clocks
*   1 uSec          - TCLK_RATE / 1,000,000 clocks
*
*   Register Value for N micro seconds -  ((N * ( (TCLK_RATE / 1,000,000)) / 64)
*
*******************************************************************************/
MV_STATUS   mvEthCoalGet(void* pPortHndl, MV_U32* pRxCoal, MV_U32* pTxCoal)
{
    MV_U32  coal, usec;

    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHndl;

    /* get TX Coalescing */
    coal = MV_REG_READ (ETH_TX_FIFO_URGENT_THRESH_REG(pPortCtrl->portNo));
    coal = ((coal & ETH_TX_INTR_COAL_ALL_MASK) >> ETH_TX_INTR_COAL_OFFSET);

    usec = (coal * 64) / (mvBoardTclkGet() / 1000000);
    if(pTxCoal != NULL)
        *pTxCoal = usec;

    /* Get RX Coalescing */
    coal =  MV_REG_READ(ETH_SDMA_CONFIG_REG(pPortCtrl->portNo));
    coal = ((coal & ETH_RX_INTR_COAL_ALL_MASK) >> ETH_RX_INTR_COAL_OFFSET);

    usec = (coal * 64) / (mvBoardTclkGet() / 1000000);
    if(pRxCoal != NULL)
        *pRxCoal = usec;

    return MV_OK;
}

/*******************************************************************************
* mvEthMaxRxSizeSet - 
*
* DESCRIPTION:
*       Change maximum receive size of the port. This configuration will take place 
*       after next call of ethPortSetDefaults() function.
*
* INPUT:
*
* RETURN:
*******************************************************************************/
MV_STATUS   mvEthMaxRxSizeSet(void* pPortHndl, int maxRxSize)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHndl;
    MV_U32      portSerialCtrlReg;

    if((maxRxSize < 1518) || (maxRxSize & ~ETH_RX_BUFFER_MASK))
       return MV_BAD_PARAM;
    
    pPortCtrl->portConfig.maxRxPktSize = maxRxSize;

    portSerialCtrlReg =  MV_REG_READ(ETH_PORT_SERIAL_CTRL_REG(pPortCtrl->portNo));
    portSerialCtrlReg &= ~ETH_MAX_RX_PACKET_SIZE_MASK;
    portSerialCtrlReg |= mvEthMruGet(pPortCtrl->portConfig.maxRxPktSize);
    MV_REG_WRITE(ETH_PORT_SERIAL_CTRL_REG(pPortCtrl->portNo), portSerialCtrlReg);

    return MV_OK;
}


/******************************************************************************/
/*                      MAC Filtering functions                               */
/******************************************************************************/

/*******************************************************************************
* mvEthRxFilterModeSet - Configure Fitering mode of Ethernet port
*
* DESCRIPTION:
*       This routine used to free buffers attached to the Rx ring and should
*       be called only when Giga Ethernet port is Down
*
* INPUT:
*       void*       pEthPortHndl    - Ethernet Port handler.
*       MV_BOOL     isPromisc       - Promiscous mode
*                                   MV_TRUE  - accept all Broadcast, Multicast 
*                                              and Unicast packets
*                                   MV_FALSE - accept all Broadcast, 
*                                              specially added Multicast and
*                                              single Unicast packets
*
* RETURN:   MV_STATUS   MV_OK - Success, Other - Failure
*
*******************************************************************************/
MV_STATUS   mvEthRxFilterModeSet(void* pEthPortHndl, MV_BOOL isPromisc)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pEthPortHndl;
    int             queue;
    MV_U32      portCfgReg;

    portCfgReg = MV_REG_READ(ETH_PORT_CONFIG_REG(pPortCtrl->portNo));
    /* Set / Clear UPM bit in port configuration register */
    if(isPromisc)
    {
        /* Accept all multicast packets to RX default queue */
        queue = pPortCtrl->portConfig.rxDefQ;
        portCfgReg |= ETH_UNICAST_PROMISCUOUS_MODE_MASK;
        memset(pPortCtrl->mcastCount, 1, sizeof(pPortCtrl->mcastCount));
    }
    else
    {
        /* Reject all Multicast addresses */
        queue = -1;
        portCfgReg &= ~ETH_UNICAST_PROMISCUOUS_MODE_MASK;
        /* Clear all mcastCount */
        memset(pPortCtrl->mcastCount, 0, sizeof(pPortCtrl->mcastCount));
    }
    MV_REG_WRITE(ETH_PORT_CONFIG_REG(pPortCtrl->portNo), portCfgReg);        

    /* Set Special Multicast and Other Multicast tables */
    mvEthSetSpecialMcastTable(pPortCtrl->portNo, queue);
    mvEthSetOtherMcastTable(pPortCtrl->portNo, queue);

    ethSetUcastTable(pPortCtrl->portNo, queue);
    
    return MV_OK;
}

/*******************************************************************************
* mvEthMacAddrSet - This function Set the port Unicast address.
*
* DESCRIPTION:
*       This function Set the port Ethernet MAC address. This address
*       will be used to send Pause frames if enabled. Packets with this
*       address will be accepted and dispatched to default RX queue
*
* INPUT:
*       void*   pEthPortHndl    - Ethernet port handler.
*       char*   pAddr           - Address to be set
*
* RETURN:   MV_STATUS
*               MV_OK - Success,  Other - Faulure
*
*******************************************************************************/
MV_STATUS   mvEthMacAddrSet(void* pPortHndl, unsigned char *pAddr, int queue)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHndl;
    unsigned int    macH;
    unsigned int    macL;

    if(queue >= MV_ETH_RX_Q_NUM)
    {
        mvOsPrintf("ethDrv: RX queue #%d is out of range\n", queue);
        return MV_BAD_PARAM;
    }

    if(queue != -1)
    {
        macL =  (pAddr[4] << 8) | (pAddr[5]);
        macH =  (pAddr[0] << 24)| (pAddr[1] << 16) |
                (pAddr[2] << 8) | (pAddr[3] << 0);

        MV_REG_WRITE(ETH_MAC_ADDR_LOW_REG(pPortCtrl->portNo),  macL);
        MV_REG_WRITE(ETH_MAC_ADDR_HIGH_REG(pPortCtrl->portNo), macH);

        memcpy(pPortCtrl->portConfig.macAddr, pAddr, MV_MAC_ADDR_SIZE);
    }

    /* Accept frames of this address */
    ethSetUcastAddr(pPortCtrl->portNo, pAddr[5], queue);

    return MV_OK;
}

/*******************************************************************************
* mvEthMacAddrGet - This function returns the port Unicast address.
*
* DESCRIPTION:
*       This function returns the port Ethernet MAC address.
*
* INPUT:
*       int     portNo          - Ethernet port number.
*       char*   pAddr           - Pointer where address will be written to
*
* RETURN:   MV_STATUS
*               MV_OK - Success,  Other - Faulure
*
*******************************************************************************/
MV_STATUS   mvEthMacAddrGet(int portNo, unsigned char *pAddr)
{
    unsigned int    macH;
    unsigned int    macL;

    if(pAddr == NULL)
    {
        mvOsPrintf("mvEthMacAddrGet: NULL pointer.\n");
        return MV_BAD_PARAM;
    }

    macH = MV_REG_READ(ETH_MAC_ADDR_HIGH_REG(portNo));
    macL = MV_REG_READ(ETH_MAC_ADDR_LOW_REG(portNo));
    pAddr[0] = (macH >> 24) & 0xff;
    pAddr[1] = (macH >> 16) & 0xff;
    pAddr[2] = (macH >> 8) & 0xff;
    pAddr[3] = macH  & 0xff;
    pAddr[4] = (macL >> 8) & 0xff;
    pAddr[5] = macL  & 0xff;

    return MV_OK;
}

/*******************************************************************************
* mvEthMcastCrc8Get - Calculate CRC8 of MAC address.
*
* DESCRIPTION:
*
* INPUT:
*       MV_U8*  pAddr           - Address to calculate CRC-8
*
* RETURN: MV_U8 - CRC-8 of this MAC address
*
*******************************************************************************/
MV_U8   mvEthMcastCrc8Get(MV_U8* pAddr)
{
    unsigned int    macH;
    unsigned int    macL;
    int             macArray[48];
    int             crc[8];
    int             i;
    unsigned char   crcResult = 0;

        /* Calculate CRC-8 out of the given address */
    macH =  (pAddr[0] << 8) | (pAddr[1]);
    macL =  (pAddr[2] << 24)| (pAddr[3] << 16) |
            (pAddr[4] << 8) | (pAddr[5] << 0);

    for(i=0; i<32; i++)
        macArray[i] = (macL >> i) & 0x1;

    for(i=32; i<48; i++)
        macArray[i] = (macH >> (i - 32)) & 0x1;

    crc[0] = macArray[45] ^ macArray[43] ^ macArray[40] ^ macArray[39] ^
             macArray[35] ^ macArray[34] ^ macArray[31] ^ macArray[30] ^
             macArray[28] ^ macArray[23] ^ macArray[21] ^ macArray[19] ^
             macArray[18] ^ macArray[16] ^ macArray[14] ^ macArray[12] ^
             macArray[8]  ^ macArray[7]  ^ macArray[6]  ^ macArray[0];

    crc[1] = macArray[46] ^ macArray[45] ^ macArray[44] ^ macArray[43] ^
             macArray[41] ^ macArray[39] ^ macArray[36] ^ macArray[34] ^
             macArray[32] ^ macArray[30] ^ macArray[29] ^ macArray[28] ^
             macArray[24] ^ macArray[23] ^ macArray[22] ^ macArray[21] ^
             macArray[20] ^ macArray[18] ^ macArray[17] ^ macArray[16] ^
             macArray[15] ^ macArray[14] ^ macArray[13] ^ macArray[12] ^
             macArray[9]  ^ macArray[6]  ^ macArray[1]  ^ macArray[0];

    crc[2] = macArray[47] ^ macArray[46] ^ macArray[44] ^ macArray[43] ^
             macArray[42] ^ macArray[39] ^ macArray[37] ^ macArray[34] ^
             macArray[33] ^ macArray[29] ^ macArray[28] ^ macArray[25] ^
             macArray[24] ^ macArray[22] ^ macArray[17] ^ macArray[15] ^
             macArray[13] ^ macArray[12] ^ macArray[10] ^ macArray[8]  ^
             macArray[6]  ^ macArray[2]  ^ macArray[1]  ^ macArray[0];

    crc[3] = macArray[47] ^ macArray[45] ^ macArray[44] ^ macArray[43] ^
             macArray[40] ^ macArray[38] ^ macArray[35] ^ macArray[34] ^
             macArray[30] ^ macArray[29] ^ macArray[26] ^ macArray[25] ^
             macArray[23] ^ macArray[18] ^ macArray[16] ^ macArray[14] ^
             macArray[13] ^ macArray[11] ^ macArray[9]  ^ macArray[7]  ^
             macArray[3]  ^ macArray[2]  ^ macArray[1];

    crc[4] = macArray[46] ^ macArray[45] ^ macArray[44] ^ macArray[41] ^
             macArray[39] ^ macArray[36] ^ macArray[35] ^ macArray[31] ^
             macArray[30] ^ macArray[27] ^ macArray[26] ^ macArray[24] ^
             macArray[19] ^ macArray[17] ^ macArray[15] ^ macArray[14] ^
             macArray[12] ^ macArray[10] ^ macArray[8]  ^ macArray[4]  ^
             macArray[3]  ^ macArray[2];

    crc[5] = macArray[47] ^ macArray[46] ^ macArray[45] ^ macArray[42] ^
             macArray[40] ^ macArray[37] ^ macArray[36] ^ macArray[32] ^
             macArray[31] ^ macArray[28] ^ macArray[27] ^ macArray[25] ^
             macArray[20] ^ macArray[18] ^ macArray[16] ^ macArray[15] ^
             macArray[13] ^ macArray[11] ^ macArray[9]  ^ macArray[5]  ^
             macArray[4]  ^ macArray[3];

    crc[6] = macArray[47] ^ macArray[46] ^ macArray[43] ^ macArray[41] ^
             macArray[38] ^ macArray[37] ^ macArray[33] ^ macArray[32] ^
             macArray[29] ^ macArray[28] ^ macArray[26] ^ macArray[21] ^
             macArray[19] ^ macArray[17] ^ macArray[16] ^ macArray[14] ^
             macArray[12] ^ macArray[10] ^ macArray[6]  ^ macArray[5]  ^
             macArray[4];

    crc[7] = macArray[47] ^ macArray[44] ^ macArray[42] ^ macArray[39] ^
             macArray[38] ^ macArray[34] ^ macArray[33] ^ macArray[30] ^
             macArray[29] ^ macArray[27] ^ macArray[22] ^ macArray[20] ^
             macArray[18] ^ macArray[17] ^ macArray[15] ^ macArray[13] ^
             macArray[11] ^ macArray[7]  ^ macArray[6]  ^ macArray[5];

    for(i=0; i<8; i++)
        crcResult = crcResult | (crc[i] << i);

    return crcResult;
}
/*******************************************************************************
* mvEthMcastAddrSet - Multicast address settings.
*
* DESCRIPTION:
*       This API controls the MV device MAC multicast support.
*       The MV device supports multicast using two tables:
*       1) Special Multicast Table for MAC addresses of the form
*          0x01-00-5E-00-00-XX (where XX is between 0x00 and 0xFF).
*          The MAC DA[7:0] bits are used as a pointer to the Special Multicast
*          Table entries in the DA-Filter table.
*          In this case, the function calls ethPortSmcAddr() routine to set the
*          Special Multicast Table.
*       2) Other Multicast Table for multicast of another type. A CRC-8bit
*          is used as an index to the Other Multicast Table entries in the
*          DA-Filter table.
*          In this case, the function calculates the CRC-8bit value and calls
*          ethPortOmcAddr() routine to set the Other Multicast Table.
*
* INPUT:
*       void*   pEthPortHndl    - Ethernet port handler.
*       MV_U8*  pAddr           - Address to be set
*       int     queue           - RX queue to capture all packets with this 
*                               Multicast MAC address.
*                               -1 means delete this Multicast address.
*
* RETURN: MV_STATUS
*       MV_TRUE - Success, Other - Failure
*
*******************************************************************************/
MV_STATUS   mvEthMcastAddrSet(void* pPortHndl, MV_U8 *pAddr, int queue)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHndl;
    unsigned char   crcResult = 0;

    if(queue >= MV_ETH_RX_Q_NUM)
    {
        mvOsPrintf("ethDrv: RX queue #%d is out of range\n", queue);
        return MV_BAD_PARAM;
    }

    if((pAddr[0] == 0x01) &&
       (pAddr[1] == 0x00) &&
       (pAddr[2] == 0x5E) &&
       (pAddr[3] == 0x00) &&
       (pAddr[4] == 0x00))
    {
        ethSetSpecialMcastAddr(pPortCtrl->portNo, pAddr[5], queue);
    }
    else
    {
        crcResult = mvEthMcastCrc8Get(pAddr);

        /* Check Add counter for this CRC value */
        if(queue == -1)
        {
            if(pPortCtrl->mcastCount[crcResult] == 0)
                return MV_NO_SUCH;

            pPortCtrl->mcastCount[crcResult]--;
            if(pPortCtrl->mcastCount[crcResult] != 0)
                return MV_NO_CHANGE;
        }
        else
        {
            pPortCtrl->mcastCount[crcResult]++;
            if(pPortCtrl->mcastCount[crcResult] > 1)
                return MV_NO_CHANGE;
        }
        ethSetOtherMcastAddr(pPortCtrl->portNo, crcResult, queue);
    }
    return MV_OK;
}

/*******************************************************************************
* ethSetUcastTable - Unicast address settings.
*
* DESCRIPTION:
*      Set all entries in the Unicast MAC Table queue==-1 means reject all 
* INPUT:
*
* RETURN: 
*
*******************************************************************************/
static void    ethSetUcastTable(int portNo, int queue)
{
    int     offset;
    MV_U32  regValue;

    if(queue == -1)
    {
        regValue = 0;
    }
    else
    {
        regValue = (((0x01 | (queue<<1)) << 0)  |
                    ((0x01 | (queue<<1)) << 8)  |
                    ((0x01 | (queue<<1)) << 16) |
                    ((0x01 | (queue<<1)) << 24));
    }

    for (offset=0; offset<=0xC; offset+=4)
        MV_REG_WRITE((ETH_DA_FILTER_UCAST_BASE(portNo) + offset), regValue);
}

/*******************************************************************************
* mvEthSetSpecialMcastTable - Special Multicast address settings.
*
* DESCRIPTION:
*   Set all entries to the Special Multicast MAC Table. queue==-1 means reject all
* INPUT:
*
* RETURN: 
*
*******************************************************************************/
MV_VOID    mvEthSetSpecialMcastTable(int portNo, int queue)
{
    int     offset;
    MV_U32  regValue;

    if(queue == -1)
    {
        regValue = 0;
    }
    else
    {
        regValue = (((0x01 | (queue<<1)) << 0)  |
                    ((0x01 | (queue<<1)) << 8)  |
                    ((0x01 | (queue<<1)) << 16) |
                    ((0x01 | (queue<<1)) << 24));
    }

    for (offset=0; offset<=0xFC; offset+=4)
    {
        MV_REG_WRITE((ETH_DA_FILTER_SPEC_MCAST_BASE(portNo) + 
                      offset), regValue);
    }
}

/*******************************************************************************
* mvEthSetOtherMcastTable - Other Multicast address settings.
*
* DESCRIPTION:
*   Set all entries to the Other Multicast MAC Table. queue==-1 means reject all
* INPUT:
*
* RETURN: 
*
*******************************************************************************/
MV_VOID    mvEthSetOtherMcastTable(int portNo, int queue)
{
    int     offset;
    MV_U32  regValue;

    if(queue == -1)
    {
        regValue = 0;
    }
    else
    {
        regValue = (((0x01 | (queue<<1)) << 0)  |
                    ((0x01 | (queue<<1)) << 8)  |
                    ((0x01 | (queue<<1)) << 16) |
                    ((0x01 | (queue<<1)) << 24));
    }

    for (offset=0; offset<=0xFC; offset+=4)
    {
        MV_REG_WRITE((ETH_DA_FILTER_OTH_MCAST_BASE(portNo) + 
                      offset), regValue);
    }
}

/*******************************************************************************
* ethSetUcastAddr - This function Set the port unicast address table
*
* DESCRIPTION:
*       This function locates the proper entry in the Unicast table for the
*       specified MAC nibble and sets its properties according to function
*       parameters.
*
* INPUT:
*       int     ethPortNum  - Port number.
*       MV_U8   lastNibble  - Unicast MAC Address last nibble.
*       int     queue       - Rx queue number for this MAC address.
*                           value "-1" means remove address
*
* OUTPUT:
*       This function add/removes MAC addresses from the port unicast address
*       table.
*
* RETURN:
*       MV_TRUE is output succeeded.
*       MV_FALSE if option parameter is invalid.
*
*******************************************************************************/
static MV_BOOL ethSetUcastAddr(int portNo, MV_U8 lastNibble, int queue)
{
    unsigned int unicastReg;
    unsigned int tblOffset;
    unsigned int regOffset;

    /* Locate the Unicast table entry */
    lastNibble  = (0xf & lastNibble);
    tblOffset = (lastNibble / 4) * 4; /* Register offset from unicast table base*/
    regOffset = lastNibble % 4;     /* Entry offset within the above register */


    unicastReg = MV_REG_READ( (ETH_DA_FILTER_UCAST_BASE(portNo) + 
                               tblOffset));
                 

    if(queue == -1)
    {
        /* Clear accepts frame bit at specified unicast DA table entry */
        unicastReg &= ~(0xFF << (8*regOffset));
    }
    else
    {
        unicastReg &= ~(0xFF << (8*regOffset));
        unicastReg |= ((0x01 | (queue<<1)) << (8*regOffset));
    }
    MV_REG_WRITE( (ETH_DA_FILTER_UCAST_BASE(portNo) + tblOffset),
                  unicastReg);

    return MV_TRUE;
}

/*******************************************************************************
* ethSetSpecialMcastAddr - Special Multicast address settings.
*
* DESCRIPTION:
*       This routine controls the MV device special MAC multicast support.
*       The Special Multicast Table for MAC addresses supports MAC of the form
*       0x01-00-5E-00-00-XX (where XX is between 0x00 and 0xFF).
*       The MAC DA[7:0] bits are used as a pointer to the Special Multicast
*       Table entries in the DA-Filter table.
*       This function set the Special Multicast Table appropriate entry
*       according to the argument given.
*
* INPUT:
*       int     ethPortNum      Port number.
*       unsigned char   mcByte      Multicast addr last byte (MAC DA[7:0] bits).
*       int          queue      Rx queue number for this MAC address.
*       int             option      0 = Add, 1 = remove address.
*
* OUTPUT:
*       See description.
*
* RETURN:
*       MV_TRUE is output succeeded.
*       MV_FALSE if option parameter is invalid.
*
*******************************************************************************/
static MV_BOOL ethSetSpecialMcastAddr(int ethPortNum, MV_U8 lastByte, int queue)
{
    unsigned int smcTableReg;
    unsigned int tblOffset;
    unsigned int regOffset;

    /* Locate the SMC table entry */
    tblOffset = (lastByte / 4);     /* Register offset from SMC table base    */
    regOffset = lastByte % 4;       /* Entry offset within the above register */

    smcTableReg = MV_REG_READ((ETH_DA_FILTER_SPEC_MCAST_BASE(ethPortNum) + tblOffset*4));
    
    if(queue == -1)
    {
        /* Clear accepts frame bit at specified Special DA table entry */
        smcTableReg &= ~(0xFF << (8 * regOffset));
    }
    else
    {
        smcTableReg &= ~(0xFF << (8 * regOffset));
        smcTableReg |= ((0x01 | (queue<<1)) << (8 * regOffset));
    }
    MV_REG_WRITE((ETH_DA_FILTER_SPEC_MCAST_BASE(ethPortNum) + 
                  tblOffset*4), smcTableReg);

    return MV_TRUE;
}

/*******************************************************************************
* ethSetOtherMcastAddr - Multicast address settings.
*
* DESCRIPTION:
*       This routine controls the MV device Other MAC multicast support.
*       The Other Multicast Table is used for multicast of another type.
*       A CRC-8bit is used as an index to the Other Multicast Table entries
*       in the DA-Filter table.
*       The function gets the CRC-8bit value from the calling routine and
*       set the Other Multicast Table appropriate entry according to the
*       CRC-8 argument given.
*
* INPUT:
*       int     ethPortNum  Port number.
*       MV_U8   crc8        A CRC-8bit (Polynomial: x^8+x^2+x^1+1).
*       int     queue       Rx queue number for this MAC address.
*
* OUTPUT:
*       See description.
*
* RETURN:
*       MV_TRUE is output succeeded.
*       MV_FALSE if option parameter is invalid.
*
*******************************************************************************/
static MV_BOOL ethSetOtherMcastAddr(int ethPortNum, MV_U8 crc8, int queue)
{
    unsigned int omcTableReg;
    unsigned int tblOffset;
    unsigned int regOffset;

    /* Locate the OMC table entry */
    tblOffset = (crc8 / 4) * 4;     /* Register offset from OMC table base    */
    regOffset = crc8 % 4;           /* Entry offset within the above register */

    omcTableReg = MV_REG_READ(
        (ETH_DA_FILTER_OTH_MCAST_BASE(ethPortNum) + tblOffset));

    if(queue == -1)
    {
        /* Clear accepts frame bit at specified Other DA table entry */
        omcTableReg &= ~(0xFF << (8 * regOffset));
    }
    else
    {
        omcTableReg &= ~(0xFF << (8 * regOffset));
        omcTableReg |= ((0x01 | (queue<<1)) << (8 * regOffset));
    }
    MV_REG_WRITE((ETH_DA_FILTER_OTH_MCAST_BASE(ethPortNum) + 
                  tblOffset), 
                 omcTableReg);

    return MV_TRUE;
}


/******************************************************************************/
/*                      MIB Counters functions                                */
/******************************************************************************/


/*******************************************************************************
* mvEthMibCounterRead - Read a MIB counter
*
* DESCRIPTION:
*       This function reads a MIB counter of a specific ethernet port.
*       NOTE - Read from ETH_MIB_GOOD_OCTETS_RECEIVED_LOW or 
*              ETH_MIB_GOOD_OCTETS_SENT_LOW counters will return 64 bits value,
*              so pHigh32 pointer should not be NULL in this case.
*
* INPUT:
*       int           ethPortNum  - Ethernet Port number.
*       unsigned int  mibOffset   - MIB counter offset.
*
* OUTPUT:
*       MV_U32*       pHigh32 - pointer to place where 32 most significant bits
*                             of the counter will be stored.
*
* RETURN:
*       32 low sgnificant bits of MIB counter value.
*
*******************************************************************************/
MV_U32  mvEthMibCounterRead(void* pPortHandle, unsigned int mibOffset, 
                            MV_U32* pHigh32)
{
    int             portNo;
    MV_U32          valLow32, valHigh32;
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHandle;

    portNo = pPortCtrl->portNo;

    valLow32 = MV_REG_READ(ETH_MIB_COUNTERS_BASE(portNo) + mibOffset);
    if( (mibOffset == ETH_MIB_GOOD_OCTETS_RECEIVED_LOW) || 
        (mibOffset == ETH_MIB_GOOD_OCTETS_SENT_LOW) )
    {
        valHigh32 = MV_REG_READ(ETH_MIB_COUNTERS_BASE(portNo) + mibOffset + 4);
        if(pHigh32 != NULL)
            *pHigh32 = valHigh32;
    }
    return valLow32;
}

/*******************************************************************************
* mvEthMibCountersClear - Clear all MIB counters
*
* DESCRIPTION:
*       This function clears all MIB counters
*
* INPUT:
*       int           ethPortNum  - Ethernet Port number.
*
*
* RETURN:   void
*
*******************************************************************************/
void  mvEthMibCountersClear(void* pPortHandle)
{
    int             i, portNo;
    unsigned int    dummy;
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHandle;

    portNo = pPortCtrl->portNo;

    /* Perform dummy reads from MIB counters */
    for(i=ETH_MIB_GOOD_OCTETS_RECEIVED_LOW; i<ETH_MIB_LATE_COLLISION; i+=4)
        dummy = MV_REG_READ((ETH_MIB_COUNTERS_BASE(portNo) + i));
}


/******************************************************************************/
/*                        TX Scheduling configuration routines                */
/******************************************************************************/

/*******************************************************************************
* mvEthTxQueueConfig - Configure Transmit (TX) queue.
*
* DESCRIPTION:
*       This function configures parameters of specific TX queue.
*
* INPUT:
*   void*               pPortHandle - Pointer to port specific handler;
*   int                 txQueue     - Number of TX queue to configure
*   MV_ETH_PRIO_MODE    prioMode    - Scheduling priority mode (FIXED or WRR)
*   int                 txQuota     - TX queue quota (number of bytes n*256) for 
*                                   weighted round-robin priority scheduling. 
*                                   Value 0 means that transmit from the queue - disabled.
*                                   Maximum value is 255 (64 Kbytes)
*
* RETURN:   MV_STATUS
*       MV_OK       - Success
*       MV_FAIL     - Failed. 
*
*******************************************************************************/
MV_STATUS   mvEthTxQueueConfig(void* pPortHandle, int txQueue,                          
                               MV_ETH_PRIO_MODE txPrioMode, int txQuota)
{
    int             portNo;
    MV_U32          txPrio;
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHandle;

    if(txQueue >= MV_ETH_TX_Q_NUM)
    {
        mvOsPrintf("ethDrv: TX queue #%d is out of range\n", txQueue);
        return MV_BAD_PARAM;
    }

    portNo = pPortCtrl->portNo;
    txPrio = MV_REG_READ( ETH_TX_QUEUE_FIXED_PRIO_REG(portNo));

    pPortCtrl->txQueueConfig[txQueue].prioMode = txPrioMode;

    if(pPortCtrl->txQueueConfig[txQueue].prioMode == MV_ETH_PRIO_FIXED)
    {
        txPrio |= (1<<txQueue);
    }
    else
    {
        txPrio &= ~(1<<txQueue);
        pPortCtrl->txQueueConfig[txQueue].quota = txQuota;
        MV_REG_WRITE( ETH_TX_ARBITER_CFG_REG(portNo, txQueue), 
                        pPortCtrl->txQueueConfig[txQueue].quota);
    }
    MV_REG_WRITE( ETH_TX_QUEUE_FIXED_PRIO_REG(portNo), txPrio);

    return MV_OK;
}
    

/******************************************************************************/
/*                        RX Dispatching configuration routines               */
/******************************************************************************/

/*******************************************************************************
* mvEthVlanPrioRxQueue - Configure RX queue to capture VLAN tagged packets with 
*                        special priority bits [0-2]
*
* DESCRIPTION:
*
* INPUT:
*       void*   pPortHandle - Pointer to port specific handler;
*       int     bpduQueue   - Special queue to capture VLAN tagged packets with special
*                           priority.
*                           Negative value (-1) means no special processing for these packets, 
*                           so they will be processed as regular packets.
*
* RETURN:   MV_STATUS
*       MV_OK       - Success
*       MV_FAIL     - Failed. 
*
*******************************************************************************/
MV_STATUS   mvEthVlanPrioRxQueue(void* pPortHandle, int vlanPrio, int vlanPrioQueue)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHandle;
    MV_U32          vlanPrioReg;

    if(vlanPrioQueue >= MV_ETH_RX_Q_NUM)
    {
        mvOsPrintf("ethDrv: RX queue #%d is out of range\n", vlanPrioQueue);
        return MV_BAD_PARAM;
    }
    if(vlanPrio >= 8)
    {
        mvOsPrintf("ethDrv: vlanPrio=%d is out of range\n", vlanPrio);
        return MV_BAD_PARAM;
    }
  
    vlanPrioReg = MV_REG_READ(ETH_VLAN_TAG_TO_PRIO_REG(pPortCtrl->portNo));
    vlanPrioReg &= ~(0x7 << (vlanPrio*3));
    vlanPrioReg |= (vlanPrioQueue << (vlanPrio*3));
    MV_REG_WRITE(ETH_VLAN_TAG_TO_PRIO_REG(pPortCtrl->portNo), vlanPrioReg);

    return MV_OK;
}


/*******************************************************************************
* mvEthBpduRxQueue - Configure RX queue to capture BPDU packets.
*
* DESCRIPTION:
*       This function defines processing of BPDU packets. 
*   BPDU packets can be accepted and captured to one of RX queues 
*   or can be processing as regular Multicast packets. 
*
* INPUT:
*       void*   pPortHandle - Pointer to port specific handler;
*       int     bpduQueue   - Special queue to capture BPDU packets (DA is equal to 
*                           01-80-C2-00-00-00 through 01-80-C2-00-00-FF, 
*                           except for the Flow-Control Pause packets). 
*                           Negative value (-1) means no special processing for BPDU, 
*                           packets so they will be processed as regular Multicast packets.
*
* RETURN:   MV_STATUS
*       MV_OK       - Success
*       MV_FAIL     - Failed. 
*
*******************************************************************************/
MV_STATUS   mvEthBpduRxQueue(void* pPortHandle, int bpduQueue)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHandle;
    MV_U32      portCfgReg;
    MV_U32      portCfgExtReg;

    if(bpduQueue >= MV_ETH_RX_Q_NUM)
    {
        mvOsPrintf("ethDrv: RX queue #%d is out of range\n", bpduQueue);
        return MV_BAD_PARAM;
    }
  
    portCfgExtReg = MV_REG_READ(ETH_PORT_CONFIG_EXTEND_REG(pPortCtrl->portNo));

    portCfgReg = MV_REG_READ(ETH_PORT_CONFIG_REG(pPortCtrl->portNo));
    if(bpduQueue >= 0)
    {
        pPortCtrl->portConfig.rxBpduQ = bpduQueue;

        portCfgReg &= ~ETH_DEF_RX_BPDU_QUEUE_ALL_MASK;
        portCfgReg |= ETH_DEF_RX_BPDU_QUEUE_MASK(pPortCtrl->portConfig.rxBpduQ);

        MV_REG_WRITE(ETH_PORT_CONFIG_REG(pPortCtrl->portNo), portCfgReg);

        portCfgExtReg |= ETH_CAPTURE_SPAN_BPDU_ENABLE_MASK;
    }
    else
    {
        pPortCtrl->portConfig.rxBpduQ = -1;
        /* no special processing for BPDU packets */
        portCfgExtReg &= (~ETH_CAPTURE_SPAN_BPDU_ENABLE_MASK);
    }

    MV_REG_WRITE(ETH_PORT_CONFIG_EXTEND_REG(pPortCtrl->portNo),  portCfgExtReg);

    return MV_OK;
}


/*******************************************************************************
* mvEthArpRxQueue - Configure RX queue to capture ARP packets.
*
* DESCRIPTION:
*       This function defines processing of ARP (type=0x0806) packets. 
*   ARP packets can be accepted and captured to one of RX queues 
*   or can be processed as other Broadcast packets. 
*
* INPUT:
*       void*   pPortHandle - Pointer to port specific handler;
*       int     arpQueue    - Special queue to capture ARP packets (type=0x806). 
*                           Negative value (-1) means discard ARP packets
*
* RETURN:   MV_STATUS
*       MV_OK       - Success
*       MV_FAIL     - Failed. 
*
*******************************************************************************/
MV_STATUS   mvEthArpRxQueue(void* pPortHandle, int arpQueue)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHandle;
    MV_U32      portCfgReg;

    if(arpQueue >= MV_ETH_RX_Q_NUM)
    {
        mvOsPrintf("ethDrv: RX queue #%d is out of range\n", arpQueue);
        return MV_BAD_PARAM;
    }

    portCfgReg = MV_REG_READ(ETH_PORT_CONFIG_REG(pPortCtrl->portNo));

    if(arpQueue >= 0)
    {
        pPortCtrl->portConfig.rxArpQ = arpQueue;
        portCfgReg &= ~ETH_DEF_RX_ARP_QUEUE_ALL_MASK;
        portCfgReg |= ETH_DEF_RX_ARP_QUEUE_MASK(pPortCtrl->portConfig.rxArpQ);

        portCfgReg &= (~ETH_REJECT_ARP_BCAST_MASK);
    }
    else
    {
        pPortCtrl->portConfig.rxArpQ = -1;
        portCfgReg |= ETH_REJECT_ARP_BCAST_MASK;
    }

    MV_REG_WRITE(ETH_PORT_CONFIG_REG(pPortCtrl->portNo), portCfgReg);

    return MV_OK;
}


/*******************************************************************************
* mvEthTcpRxQueue - Configure RX queue to capture TCP packets.
*
* DESCRIPTION:
*       This function defines processing of TCP packets. 
*   TCP packets can be accepted and captured to one of RX queues 
*   or can be processed as regular Unicast packets. 
*
* INPUT:
*       void*   pPortHandle - Pointer to port specific handler;
*       int     tcpQueue    - Special queue to capture TCP packets. Value "-1" 
*                           means no special processing for TCP packets, 
*                           so they will be processed as regular
*
* RETURN:   MV_STATUS
*       MV_OK       - Success
*       MV_FAIL     - Failed. 
*
*******************************************************************************/
MV_STATUS   mvEthTcpRxQueue(void* pPortHandle, int tcpQueue)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHandle;
    MV_U32      portCfgReg;

    if(tcpQueue >= MV_ETH_RX_Q_NUM)
    {
        mvOsPrintf("ethDrv: RX queue #%d is out of range\n", tcpQueue);
        return MV_BAD_PARAM;
    }
    portCfgReg = MV_REG_READ(ETH_PORT_CONFIG_REG(pPortCtrl->portNo));

    if(tcpQueue >= 0)
    {
        pPortCtrl->portConfig.rxTcpQ = tcpQueue;
        portCfgReg &= ~ETH_DEF_RX_TCP_QUEUE_ALL_MASK;
        portCfgReg |= ETH_DEF_RX_TCP_QUEUE_MASK(pPortCtrl->portConfig.rxTcpQ);

        portCfgReg |= ETH_CAPTURE_TCP_FRAMES_ENABLE_MASK;
    }
    else
    {
        pPortCtrl->portConfig.rxTcpQ = -1;
        portCfgReg &= (~ETH_CAPTURE_TCP_FRAMES_ENABLE_MASK);
    }

    MV_REG_WRITE(ETH_PORT_CONFIG_REG(pPortCtrl->portNo), portCfgReg);

    return MV_OK;
}


/*******************************************************************************
* mvEthUdpRxQueue - Configure RX queue to capture UDP packets.
*
* DESCRIPTION:
*       This function defines processing of UDP packets. 
*   TCP packets can be accepted and captured to one of RX queues 
*   or can be processed as regular Unicast packets. 
*
* INPUT:
*       void*   pPortHandle - Pointer to port specific handler;
*       int     udpQueue    - Special queue to capture UDP packets. Value "-1" 
*                           means no special processing for UDP packets, 
*                           so they will be processed as regular
*
* RETURN:   MV_STATUS
*       MV_OK       - Success
*       MV_FAIL     - Failed. 
*
*******************************************************************************/
MV_STATUS   mvEthUdpRxQueue(void* pPortHandle, int udpQueue)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHandle;
    MV_U32          portCfgReg;

    if(udpQueue >= MV_ETH_RX_Q_NUM)
    {
        mvOsPrintf("ethDrv: RX queue #%d is out of range\n", udpQueue);
        return MV_BAD_PARAM;
    }

    portCfgReg = MV_REG_READ(ETH_PORT_CONFIG_REG(pPortCtrl->portNo));

    if(udpQueue >= 0)
    {
        pPortCtrl->portConfig.rxUdpQ = udpQueue;
        portCfgReg &= ~ETH_DEF_RX_UDP_QUEUE_ALL_MASK;
        portCfgReg |= ETH_DEF_RX_UDP_QUEUE_MASK(pPortCtrl->portConfig.rxUdpQ);

        portCfgReg |= ETH_CAPTURE_UDP_FRAMES_ENABLE_MASK;        
    }
    else
    {
        pPortCtrl->portConfig.rxUdpQ = -1;
        portCfgReg &= ~ETH_CAPTURE_UDP_FRAMES_ENABLE_MASK;
    }

    MV_REG_WRITE(ETH_PORT_CONFIG_REG(pPortCtrl->portNo), portCfgReg);

    return MV_OK;
}


/******************************************************************************/
/*                          Speed, Duplex, FlowControl routines               */
/******************************************************************************/

/*******************************************************************************
* mvEthSpeedDuplexSet - Set Speed and Duplex of the port.
*
* DESCRIPTION:
*       This function configure the port to work with desirable Duplex and Speed.
*       Changing of these parameters are allowed only when port is disabled.
*       This function disable the port if was enabled, change duplex and speed 
*       and, enable the port back if needed.
*
* INPUT:
*       void*           pPortHandle - Pointer to port specific handler;
*       ETH_PORT_SPEED  speed       - Speed of the port.
*       ETH_PORT_SPEED  duplex      - Duplex of the port.
*
* RETURN:   MV_STATUS
*       MV_OK           - Success
*       MV_OUT_OF_RANGE - Failed. Port is out of valid range
*       MV_NOT_FOUND    - Failed. Port is not initialized.
*       MV_BAD_PARAM    - Input parameters (speed/duplex) in conflict.
*       MV_BAD_VALUE    - Value of one of input parameters (speed, duplex) 
*                       is not valid
*
*******************************************************************************/
MV_STATUS   mvEthSpeedDuplexSet(void* pPortHandle, MV_ETH_PORT_SPEED speed, 
                                MV_ETH_PORT_DUPLEX duplex)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHandle;
    int             port = pPortCtrl->portNo;
    MV_U32      portSerialCtrlReg;
    
    if( (port < 0) || (port >= mvCtrlEthMaxPortGet()) )
        return MV_OUT_OF_RANGE;

    pPortCtrl = ethPortCtrl[port];
    if(pPortCtrl == NULL)
        return MV_NOT_FOUND;

    /* Check validity */
    if( (speed == MV_ETH_SPEED_1000) && (duplex == MV_ETH_DUPLEX_HALF) )
        return MV_BAD_PARAM;

    portSerialCtrlReg = MV_REG_READ(ETH_PORT_SERIAL_CTRL_REG(port));
    /* Set Speed */
    switch(speed)
    {
        case MV_ETH_SPEED_AN:
            portSerialCtrlReg &= ~ETH_DISABLE_SPEED_AUTO_NEG_MASK;
            break;

        case MV_ETH_SPEED_10:
            portSerialCtrlReg |= ETH_DISABLE_SPEED_AUTO_NEG_MASK;
            portSerialCtrlReg &= ~ETH_SET_GMII_SPEED_1000_MASK;
            portSerialCtrlReg &= ~ETH_SET_MII_SPEED_100_MASK;
            break;

        case MV_ETH_SPEED_100:
            portSerialCtrlReg |= ETH_DISABLE_SPEED_AUTO_NEG_MASK;
            portSerialCtrlReg &= ~ETH_SET_GMII_SPEED_1000_MASK;
            portSerialCtrlReg |= ETH_SET_MII_SPEED_100_MASK;
            break;

        case MV_ETH_SPEED_1000:
            portSerialCtrlReg |= ETH_DISABLE_SPEED_AUTO_NEG_MASK;
            portSerialCtrlReg |= ETH_SET_GMII_SPEED_1000_MASK;
            break;

        default:
            mvOsPrintf("ethDrv: Unexpected Speed value %d\n", speed);
            return MV_BAD_VALUE;
    }
    /* Set duplex */
    switch(duplex)
    {
        case MV_ETH_DUPLEX_AN:
            portSerialCtrlReg &= ~ETH_DISABLE_DUPLEX_AUTO_NEG_MASK;
            break;

        case MV_ETH_DUPLEX_HALF:
            portSerialCtrlReg |= ETH_DISABLE_DUPLEX_AUTO_NEG_MASK;
            portSerialCtrlReg &= ~ETH_SET_FULL_DUPLEX_MASK;
            break;

        case MV_ETH_DUPLEX_FULL:
            portSerialCtrlReg |= ETH_DISABLE_DUPLEX_AUTO_NEG_MASK;
            portSerialCtrlReg |= ETH_SET_FULL_DUPLEX_MASK;
            break;

        default:
            mvOsPrintf("ethDrv: Unexpected Duplex value %d\n", duplex);
            return MV_BAD_VALUE;
    }
    MV_REG_WRITE(ETH_PORT_SERIAL_CTRL_REG(port), portSerialCtrlReg);

    return MV_OK;
}

/*******************************************************************************
* mvEthFlowCtrlSet - Set Flow Control of the port.
*
* DESCRIPTION:
*       This function configure the port to work with desirable Duplex and 
*       Speed. Changing of these parameters are allowed only when port is 
*       disabled. This function disable the port if was enabled, change 
*       duplex and speed and, enable the port back if needed.
*
* INPUT:
*       void*           pPortHandle - Pointer to port specific handler;
*       MV_ETH_PORT_FC  flowControl - Flow control of the port.
*
* RETURN:   MV_STATUS
*       MV_OK           - Success
*       MV_OUT_OF_RANGE - Failed. Port is out of valid range
*       MV_NOT_FOUND    - Failed. Port is not initialized.
*       MV_BAD_VALUE    - Value flowControl parameters is not valid
*
*******************************************************************************/
MV_STATUS   mvEthFlowCtrlSet(void* pPortHandle, MV_ETH_PORT_FC flowControl)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHandle;
    int             port = pPortCtrl->portNo;
    MV_U32      portSerialCtrlReg;
    
    if( (port < 0) || (port >= mvCtrlEthMaxPortGet() ) )
        return MV_OUT_OF_RANGE;

    pPortCtrl = ethPortCtrl[port];
    if(pPortCtrl == NULL)
        return MV_NOT_FOUND;

    portSerialCtrlReg = MV_REG_READ(ETH_PORT_SERIAL_CTRL_REG(port));
    switch(flowControl)
    {
        case MV_ETH_FC_AN_ADV_DIS:
            portSerialCtrlReg &= ~ETH_DISABLE_FC_AUTO_NEG_MASK;
            portSerialCtrlReg &= ~ETH_ADVERTISE_SYM_FC_MASK;
            break;

        case MV_ETH_FC_AN_ADV_SYM:
            portSerialCtrlReg &= ~ETH_DISABLE_FC_AUTO_NEG_MASK;
            portSerialCtrlReg |= ETH_ADVERTISE_SYM_FC_MASK;
            break;

        case MV_ETH_FC_DISABLE:
            portSerialCtrlReg |= ETH_DISABLE_FC_AUTO_NEG_MASK;
            portSerialCtrlReg &= ~ETH_SET_FLOW_CTRL_MASK;
            break;

        case MV_ETH_FC_ENABLE:
            portSerialCtrlReg |= ETH_DISABLE_FC_AUTO_NEG_MASK;
            portSerialCtrlReg |= ETH_SET_FLOW_CTRL_MASK;
            break;

        default:
            mvOsPrintf("ethDrv: Unexpected FlowControl value %d\n", flowControl);
            return MV_BAD_VALUE;
    }
    MV_REG_WRITE(ETH_PORT_SERIAL_CTRL_REG(port), portSerialCtrlReg);

    return MV_OK;
}

/*******************************************************************************
* mvEthStatusGet - Get major properties of the port .
*
* DESCRIPTION:
*       This function get major properties of the port (link, speed, duplex, 
*       flowControl, etc) and return them using the single structure.
*
* INPUT:
*       void*           pPortHandle - Pointer to port specific handler;
*
* OUTPUT:
*       MV_ETH_PORT_STATUS* pStatus - Pointer to structure, were port status 
*                                   will be placed.
*
* RETURN:   None.
*
*******************************************************************************/
void    mvEthStatusGet(void* pPortHandle, MV_ETH_PORT_STATUS* pStatus)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHandle;
    int             port = pPortCtrl->portNo;

    MV_U32  regValue;

    regValue = MV_REG_READ( ETH_PORT_STATUS_REG(port) );

    if(regValue & ETH_GMII_SPEED_1000_MASK)
        pStatus->speed = MV_ETH_SPEED_1000;
    else if(regValue & ETH_MII_SPEED_100_MASK)
        pStatus->speed = MV_ETH_SPEED_100;
    else
        pStatus->speed = MV_ETH_SPEED_10;

    if(regValue & ETH_LINK_UP_MASK) 
        pStatus->isLinkUp = MV_TRUE;
    else
        pStatus->isLinkUp = MV_FALSE;

    if(regValue & ETH_FULL_DUPLEX_MASK) 
        pStatus->duplex = MV_ETH_DUPLEX_FULL; 
    else
        pStatus->duplex = MV_ETH_DUPLEX_HALF; 


    if(regValue & ETH_ENABLE_RCV_FLOW_CTRL_MASK) 
        pStatus->flowControl = MV_ETH_FC_ENABLE;
    else
        pStatus->flowControl = MV_ETH_FC_DISABLE;
}


/******************************************************************************/
/*                         PHY Control Functions                              */
/******************************************************************************/


/*******************************************************************************
* mvEthPhyAddrSet - Set the ethernet port PHY address.
*
* DESCRIPTION:
*       This routine set the ethernet port PHY address according to given
*       parameter.
*
* INPUT:
*       void*   pPortHandle     - Pointer to port specific handler;
*       int     phyAddr         - PHY address       
*
* RETURN:
*       None.
*
*******************************************************************************/
void    mvEthPhyAddrSet(void* pPortHandle, int phyAddr)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHandle;
    int             port = pPortCtrl->portNo;
    unsigned int    regData;

    regData = MV_REG_READ(ETH_PHY_ADDR_REG);

    regData &= ~(0x1F << (5 * port));
    regData |=  (phyAddr << (5 * port));

    MV_REG_WRITE(ETH_PHY_ADDR_REG, regData);

    return;
}

/*******************************************************************************
* mvEthPhyAddrGet - Get the ethernet port PHY address.
*
* DESCRIPTION:
*       This routine returns the given ethernet port PHY address.
*
* INPUT:
*       void*   pPortHandle - Pointer to port specific handler;
*
*
* RETURN: int - PHY address.
*
*******************************************************************************/
int     mvEthPhyAddrGet(void* pPortHandle)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHandle;
    int             port = pPortCtrl->portNo;
    unsigned int    regData;

    regData = MV_REG_READ(ETH_PHY_ADDR_REG);

    return ((regData >> (5 * port)) & 0x1f);
}

/******************************************************************************/
/*                Descriptor handling Functions                               */
/******************************************************************************/

/*******************************************************************************
* etherInitRxDescRing - Curve a Rx chain desc list and buffer in memory.
*
* DESCRIPTION:
*       This function prepares a Rx chained list of descriptors and packet
*       buffers in a form of a ring. The routine must be called after port
*       initialization routine and before port start routine.
*       The Ethernet SDMA engine uses CPU bus addresses to access the various
*       devices in the system (i.e. DRAM). This function uses the ethernet
*       struct 'virtual to physical' routine (set by the user) to set the ring
*       with physical addresses.
*
* INPUT:
*       ETH_QUEUE_CTRL  *pEthPortCtrl   Ethernet Port Control srtuct.
*       int             rxQueue         Number of Rx queue.
*       int             rxDescNum       Number of Rx descriptors
*       MV_U8*          rxDescBaseAddr  Rx descriptors memory area base addr.
*
* OUTPUT:
*       The routine updates the Ethernet port control struct with information
*       regarding the Rx descriptors and buffers.
*
* RETURN: None
*
*******************************************************************************/
static void ethInitRxDescRing(ETH_PORT_CTRL* pPortCtrl, int queue)
{
    ETH_RX_DESC     *pRxDescBase, *pRxDesc, *pRxPrevDesc; 
    int             ix, rxDescNum = pPortCtrl->rxQueueConfig[queue].descrNum;
    ETH_QUEUE_CTRL  *pQueueCtrl = &pPortCtrl->rxQueue[queue];

    /* Make sure descriptor address is cache line size aligned  */        
    pRxDescBase = (ETH_RX_DESC*)MV_ALIGN_UP((MV_ULONG)pQueueCtrl->descBuf.bufVirtPtr,
                                     CPU_D_CACHE_LINE_SIZE);

    pRxDesc      = (ETH_RX_DESC*)pRxDescBase;
    pRxPrevDesc  = pRxDesc;

    /* initialize the Rx descriptors ring */
    for (ix=0; ix<rxDescNum; ix++)
    {
        pRxDesc->bufSize     = 0x0;
        pRxDesc->byteCnt     = 0x0;
        pRxDesc->cmdSts      = ETH_BUFFER_OWNED_BY_HOST;
        pRxDesc->bufPtr      = 0x0;
        pRxDesc->returnInfo  = 0x0;
        pRxPrevDesc = pRxDesc;
        if(ix == (rxDescNum-1))
        {
            /* Closing Rx descriptors ring */
            pRxPrevDesc->nextDescPtr = (MV_U32)ethDescVirtToPhy(pPortCtrl, (void*)pRxDescBase);
        }
        else
        {
            pRxDesc = (ETH_RX_DESC*)((MV_ULONG)pRxDesc + ETH_RX_DESC_ALIGNED_SIZE);
            pRxPrevDesc->nextDescPtr = (MV_U32)ethDescVirtToPhy(pPortCtrl, (void*)pRxDesc);
        }
        ETH_DESCR_FLUSH_INV(pPortCtrl, pRxPrevDesc);
    }

    pQueueCtrl->pCurrentDescr = pRxDescBase;
    pQueueCtrl->pUsedDescr = pRxDescBase;
    
    pQueueCtrl->pFirstDescr = pRxDescBase;
    pQueueCtrl->pLastDescr = pRxDesc;
    pQueueCtrl->resource = 0;
}

void ethResetRxDescRing(void* pPortHndl, int queue)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHndl;
    ETH_QUEUE_CTRL* pQueueCtrl = &pPortCtrl->rxQueue[queue];
    ETH_RX_DESC*    pRxDesc = (ETH_RX_DESC*)pQueueCtrl->pFirstDescr;

    pQueueCtrl->resource = 0;
    if(pQueueCtrl->pFirstDescr != NULL)
    {
        while(MV_TRUE)
        {
            pRxDesc->bufSize     = 0x0;
            pRxDesc->byteCnt     = 0x0;
            pRxDesc->cmdSts      = ETH_BUFFER_OWNED_BY_HOST;
            pRxDesc->bufPtr      = 0x0;
            pRxDesc->returnInfo  = 0x0;
            ETH_DESCR_FLUSH_INV(pPortCtrl, pRxDesc);
            if( (void*)pRxDesc == pQueueCtrl->pLastDescr)
                    break;
            pRxDesc = RX_NEXT_DESC_PTR(pRxDesc, pQueueCtrl);
        }
        pQueueCtrl->pCurrentDescr = pQueueCtrl->pFirstDescr;
        pQueueCtrl->pUsedDescr = pQueueCtrl->pFirstDescr;

        /* Update RX Command register */
    pPortCtrl->portRxQueueCmdReg |= (1 << queue);

        /* update HW */
        MV_REG_WRITE( ETH_RX_CUR_DESC_PTR_REG(pPortCtrl->portNo, queue),
                 (MV_U32)ethDescVirtToPhy(pPortCtrl, pQueueCtrl->pCurrentDescr) );
    }
    else
    {
        /* Update RX Command register */
    pPortCtrl->portRxQueueCmdReg &= ~(1 << queue);

    /* update HW */
        MV_REG_WRITE( ETH_RX_CUR_DESC_PTR_REG(pPortCtrl->portNo, queue), 0);
    }
}

/*******************************************************************************
* etherInitTxDescRing - Curve a Tx chain desc list and buffer in memory.
*
* DESCRIPTION:
*       This function prepares a Tx chained list of descriptors and packet
*       buffers in a form of a ring. The routine must be called after port
*       initialization routine and before port start routine.
*       The Ethernet SDMA engine uses CPU bus addresses to access the various
*       devices in the system (i.e. DRAM). This function uses the ethernet
*       struct 'virtual to physical' routine (set by the user) to set the ring
*       with physical addresses.
*
* INPUT:
*       ETH_PORT_CTRL   *pEthPortCtrl   Ethernet Port Control srtuct.
*       int             txQueue         Number of Tx queue.
*       int             txDescNum       Number of Tx descriptors
*       int             txBuffSize      Size of Tx buffer
*       MV_U8*          pTxDescBase     Tx descriptors memory area base addr.
*
* OUTPUT:
*       The routine updates the Ethernet port control struct with information
*       regarding the Tx descriptors and buffers.
*
* RETURN:   None.
*
*******************************************************************************/
static void ethInitTxDescRing(ETH_PORT_CTRL* pPortCtrl, int queue)
{
    ETH_TX_DESC     *pTxDescBase, *pTxDesc, *pTxPrevDesc; 
    int             ix, txDescNum = pPortCtrl->txQueueConfig[queue].descrNum;
    ETH_QUEUE_CTRL  *pQueueCtrl = &pPortCtrl->txQueue[queue];
#ifdef ETH_HALFDUPLEX_ERRATA
    MV_U8* pTxBuf = pPortCtrl->txAlignedBuf[queue].bufVirtPtr;
#endif /* ETH_HALFDUPLEX_ERRATA */

    /* Make sure descriptor address is cache line size aligned  */
    pTxDescBase = (ETH_TX_DESC*)MV_ALIGN_UP((MV_ULONG)pQueueCtrl->descBuf.bufVirtPtr, 
                                     CPU_D_CACHE_LINE_SIZE);

    pTxDesc      = (ETH_TX_DESC*)pTxDescBase;
    pTxPrevDesc  = pTxDesc;

    /* initialize the Tx descriptors ring */
    for (ix=0; ix<txDescNum; ix++)
    {
        pTxDesc->byteCnt     = 0x0000;
        pTxDesc->L4iChk      = 0x0000;
        pTxDesc->cmdSts      = ETH_BUFFER_OWNED_BY_HOST;
        pTxDesc->bufPtr      = 0x0;
        pTxDesc->returnInfo  = 0x0;

#ifdef ETH_HALFDUPLEX_ERRATA
        if( pTxBuf != NULL )
        {
            pTxBuf = (MV_U8*)MV_ALIGN_UP( (MV_ULONG)pTxBuf, CPU_D_CACHE_LINE_SIZE);
            pTxDesc->alignBufPtr = pTxBuf;
            pTxBuf += ethPortHdPadLen[pPortCtrl->portNo];
        }
        else
        {
            pTxDesc->alignBufPtr = NULL;
        }
#endif /* ETH_HALFDUPLEX_ERRATA */

        pTxPrevDesc = pTxDesc;

        if(ix == (txDescNum-1))
        {
            /* Closing Tx descriptors ring */
            pTxPrevDesc->nextDescPtr = (MV_U32)ethDescVirtToPhy(pPortCtrl, (void*)pTxDescBase);
        }
        else
        {
            pTxDesc = (ETH_TX_DESC*)((MV_ULONG)pTxDesc + ETH_TX_DESC_ALIGNED_SIZE);
            pTxPrevDesc->nextDescPtr = (MV_U32)ethDescVirtToPhy(pPortCtrl, (void*)pTxDesc);
        }
        ETH_DESCR_FLUSH_INV(pPortCtrl, pTxPrevDesc);
    }

    pQueueCtrl->pCurrentDescr = pTxDescBase;
    pQueueCtrl->pUsedDescr = pTxDescBase;
    
    pQueueCtrl->pFirstDescr = pTxDescBase;
    pQueueCtrl->pLastDescr = pTxDesc;
    pQueueCtrl->resource = txDescNum;
}

void ethResetTxDescRing(void* pPortHndl, int queue)
{
    ETH_PORT_CTRL*  pPortCtrl = (ETH_PORT_CTRL*)pPortHndl;
    ETH_QUEUE_CTRL* pQueueCtrl = &pPortCtrl->txQueue[queue];
    ETH_TX_DESC*    pTxDesc = (ETH_TX_DESC*)pQueueCtrl->pFirstDescr;
    
    pQueueCtrl->resource = 0;
    if(pQueueCtrl->pFirstDescr != NULL)
    {
        while(MV_TRUE)
        {
            pTxDesc->byteCnt     = 0x0000;
            pTxDesc->L4iChk      = 0x0000;
            pTxDesc->cmdSts      = ETH_BUFFER_OWNED_BY_HOST;
            pTxDesc->bufPtr      = 0x0;
            pTxDesc->returnInfo  = 0x0;
            ETH_DESCR_FLUSH_INV(pPortCtrl, pTxDesc);
            pQueueCtrl->resource++;
            if( (void*)pTxDesc == pQueueCtrl->pLastDescr)
                    break;
            pTxDesc = TX_NEXT_DESC_PTR(pTxDesc, pQueueCtrl);
        }
        pQueueCtrl->pCurrentDescr = pQueueCtrl->pFirstDescr;
        pQueueCtrl->pUsedDescr = pQueueCtrl->pFirstDescr;

        /* Update TX Command register */
        pPortCtrl->portTxQueueCmdReg |= MV_32BIT_LE_FAST(1 << queue);
        /* update HW */
        MV_REG_WRITE( ETH_TX_CUR_DESC_PTR_REG(pPortCtrl->portNo, queue),
        (MV_U32)ethDescVirtToPhy(pPortCtrl, pQueueCtrl->pCurrentDescr) );
    }
    else
    {
        /* Update TX Command register */
        pPortCtrl->portTxQueueCmdReg &=  MV_32BIT_LE_FAST(~(1 << queue));
        /* update HW */
        MV_REG_WRITE( ETH_TX_CUR_DESC_PTR_REG(pPortCtrl->portNo, queue), 0 );    
    }
}

/*******************************************************************************
* ethAllocDescrMemory - Free memory allocated for RX and TX descriptors.
*
* DESCRIPTION:
*       This function allocates memory for RX and TX descriptors.
*       - If ETH_DESCR_IN_SRAM defined, allocate memory from SRAM.
*       - If ETH_DESCR_IN_SDRAM defined, allocate memory in SDRAM.
*
* INPUT:
*       int size - size of memory should be allocated.
*
* RETURN: None
*
*******************************************************************************/
MV_U8*  ethAllocDescrMemory(ETH_PORT_CTRL* pPortCtrl, int descSize, 
                            MV_ULONG* pPhysAddr)
{
    MV_U8*  pVirt;

#if defined(ETH_DESCR_IN_SRAM)
    if(ethDescInSram == MV_TRUE)
        pVirt = (char*)mvSramMalloc(descSize, pPhysAddr);
    else
#endif /* ETH_DESCR_IN_SRAM */
        pVirt = (char*)mvOsIoCachedMalloc(NULL, descSize, pPhysAddr);

    memset(pVirt, 0, descSize);

    return pVirt;
}

/*******************************************************************************
* ethFreeDescrMemory - Free memory allocated for RX and TX descriptors.
*
* DESCRIPTION:
*       This function frees memory allocated for RX and TX descriptors.
*       - If ETH_DESCR_IN_SRAM defined, free memory using gtSramFree() function.
*       - If ETH_DESCR_IN_SDRAM defined, free memory using mvOsFree() function.
*
* INPUT:
*       void* pVirtAddr - virtual pointer to memory allocated for RX and TX
*                       desriptors.        
*
* RETURN: None
*
*******************************************************************************/
void    ethFreeDescrMemory(ETH_PORT_CTRL* pPortCtrl, MV_BUF_INFO* pDescBuf)
{
    if( (pDescBuf == NULL) || (pDescBuf->bufVirtPtr == NULL) )
        return;

#if defined(ETH_DESCR_IN_SRAM)
    if( ethDescInSram )
    {
        mvSramFree(pDescBuf->bufSize, pDescBuf->bufPhysAddr, pDescBuf->bufVirtPtr);
        return;
    }
#endif /* ETH_DESCR_IN_SRAM */

    mvOsIoCachedFree(NULL, pDescBuf->bufSize, pDescBuf->bufPhysAddr, 
                     pDescBuf->bufVirtPtr);
}
                                                                                                                             
/******************************************************************************/
/*                Other Functions                                         */
/******************************************************************************/
/*******************************************************************************
* ethBCopy - Copy bytes from source to destination
*
* DESCRIPTION:
*       This function supports the eight bytes limitation on Tx buffer size.
*       The routine will zero eight bytes starting from the destination address
*       followed by copying bytes from the source address to the destination.
*
* INPUT:
*       unsigned int srcAddr    32 bit source address.
*       unsigned int dstAddr    32 bit destination address.
*       int        byteCount    Number of bytes to copy.
*
* OUTPUT:
*       See description.
*
* RETURN:
*       None.
*
*******************************************************************************/
static void ethBCopy(char* srcAddr, char* dstAddr, int byteCount)
{
    while(byteCount != 0)
    {
        *dstAddr = *srcAddr;
        dstAddr++;
        srcAddr++;
        byteCount--;
    }
}

/* Return port handler */
void*   mvEthPortHndlGet(int port)
{
    if( (port < 0) || (port >= BOARD_ETH_PORT_NUM) )
    {
        mvOsPrintf("eth port #%d is not exist\n", port);
        return NULL;
    }
    return  ethPortCtrl[port];
}










