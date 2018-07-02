 /**
 * @file IxEthAccCodelet_p.h
 *
 * @date 22 April 2002
 *
 * @brief This file contains some private data structures and
 * defines for the Ethernet Access Codelet.
 *
 * 
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */

#ifndef IXETHACCCODELET_P_H
#define IXETHACCCODELET_P_H

#include "IxOsal.h"
#include "IxEthAcc.h"

#if defined(__wince) && defined(IX_USE_SERCONSOLE)
#include "IxSerConsole.h"
#define printf ixSerPrintf
#define gets ixSerGets
#endif

/**
 *
 * This function is the entry point to the Ethernet Access codelet.
 * It must be called before any of the operations as it initialises the neccessary 
 * components.
 * 
 * The initialisation sequence is as follows:
 * <UL>
 *	<LI> Initialise Queue manager
 *	<LI> Start Queue manager dispatcher loop
 *	<LI> Download NPE microcode
 *	<LI> Start NPE message handler
 *	<LI> Start NPEs
 *	<LI> Initialise Ethernet Access component
 *	<LI> Initialise Ethernet ports
 *	<LI> Initialise Ethernet PHYs
 *	<LI> Program MAC addresses
 *	<LI> Set ports to promiscuous mode
 *	<LI> Initialise MBUF pool
 *	<LI> Set the Tx scheduling priority to FIFO_NO_PRIORITY 
 * </UL>  
 *
 * At the end of the initialisation sequence ports are not enabled.
 * They are enabled as appropriate for each operation that's run.
 *
 * return IX_SUCCESS - Codelet successfully initialised 
 * return IX_FAIL - Error initialising codelet
 */
IX_STATUS
ixEthAccCodeletInit(IxEthAccCodeletOperation operationType, 
                    IxEthAccPortId inPort, IxEthAccPortId outPort, int disableStats);

/**
 *
 * This function unintialize the Ethernet Access codelet.
 * It must be called after any Operation is complete to uninitialise the codelet.
 * 
 * return IX_SUCCESS - Codelet successfully uninitialised 
 * return IX_FAIL - Error uninitialising codelet
 */
IX_STATUS
ixEthAccCodeletUninit(void);

/**
 *
 * Receive Sink operation.
 *
 * This function sinks received packets as fast as possible on both ports.
 * An external source must be connected to both ports transmitting
 * Ethernet packets.  
 *
 * return IX_SUCCESS - Rx Sink Operation successfully started
 * return IX_FAIL - Error starting Rx Sink Operation
 */ 
IX_STATUS
ixEthAccCodeletRxSink(void);

/**
 * Software loopback operation.
 *
 * This function sets up some callbacks so that any frames received are
 * looped back and retransmitted unmodified on the same port.
 * An external Ethernet source must be connected to both ports transmitting
 * packets.
 * 
 * return IX_SUCCESS - Software Loopback Operation successfully started
 * return IX_FAIL - Software started external Loopback Operation
 */ 
IX_STATUS
ixEthAccCodeletSwLoopback(void);


/**
 *
 * Port-to-Port (Tx to Rx Sink) loopback operation.
 *
 * This function performs a software loopback on two ethernet ports.
 * Frames are generated on the XScale and sent from the outPort and received on the
 * inPort via a cross-over cable only. Received frames on the outPort are put back on 
 * the free queue which is shared between the two ports. A verify function ensures 
 * the frames transmitted are the same as those received. A cross-over cable connecting
 * outPort and inPort is needed to perform this operation.
 *
 * return IX_SUCCESS - SW Loopback successfully started
 * return IX_FAIL - Error starting SW loopback Operation
 */
IX_STATUS
ixEthAccCodeletTxGenRxSinkLoopback(IxEthAccPortId inPort, IxEthAccPortId outPort);

/**
 *
 * @brief PHY loopback operation.
 *
 * This function performs a MII-PHY level loopback on each ethernet port.
 * Frames are generated on the XScale and sent from each port.At PHY level,
 * frames will be loopbacked to the same port and then received on port.  
 *
 * @return IX_SUCCESS - PHY Loopback successfully started
 * @return IX_FAIL - Error starting PHY loopback Operation
 */
IX_STATUS
ixEthAccCodeletPhyLoopback(void);

/**
 *
 * Bridge Operation.
 *
 * This Operation transmits any frames received on one port through the other one,
 * as a bridge would do. An external Ethernet source must be connected to
 * both ports transmitting frames.
 * 
 * return IX_SUCCESS - Bridge Operation successfully started.
 * return IX_FAIL - Error starting bridge operation.
 */
IX_STATUS
ixEthAccCodeletSwBridge(IxEthAccPortId inPort, IxEthAccPortId outPort);


/**
 *
 * Bridge + QoS Operation.
 *
 * This Operation transmits any frames received on one port through the other one,
 * as a bridge would do. An external Ethernet source must be connected to
 * both ports transmitting frames.
 * One port is configured to be 10 Mb, the second port is configured at 100 mb.
 * and as a result of congestion and QoS being enabled , low priority frames 
 * will starve. 
 * 
 * return IX_SUCCESS - Bridge Operation successfully started.
 * return IX_FAIL - Error starting bridge operation.
 */
IX_STATUS
ixEthAccCodeletSwBridgeQoS(IxEthAccPortId inPort, IxEthAccPortId outPort);


/**
 *
 * Bridge + Firewall Operation.
 *
 * This Operation transmits any frames received on one port through the other one,
 * as a bridge would do. An external Ethernet source must be connected to
 * both ports transmitting frames.
 * A black list of MAC addresses is configured on both ports. Only the
 * frame with the appropriate source MAC address are allowed to
 * pass thru the bridge. Other frames are dropped at the NPE level.
 * 
 * return IX_SUCCESS - Bridge Operation successfully started.
 * return IX_FAIL - Error starting bridge operation.
 */
IX_STATUS
ixEthAccCodeletSwBridgeFirewall(IxEthAccPortId inPort, IxEthAccPortId outPort);

/**
 *
 * Bridge + WiFi header conversion operation.
 *
 * This Operation transmits any frames received on one port through the other one,
 * as a bridge would do. An external Ethernet source must be connected to
 * both ports transmitting frames.
 * 802.3 frames received on port 1 with certain destination MAC addresses,
 * as described when running the test, are converted to 802.11 format and
 * vice-versa (802.11 frames received on port 2 are converted to 802.3
 * format when bridged over port 1).
 * 
 * return IX_SUCCESS - Bridge Operation successfully started.
 * return IX_FAIL - Error starting bridge operation.
 */
IX_STATUS
ixEthAccCodeletSwBridgeWiFi(IxEthAccPortId inPort, IxEthAccPortId outPort);

/**
 *
 * Ethernet MAC address learning Operation.
 *
 * This function demonstrates the use of the Ethernet MAC learning facility.
 * It adds some static and dynamic entries. Dynamic entries are then aged and
 * verified that they no longer appear in the database.
 *
 * return IX_SUCCESS - DB Learning operation successfully started
 * return IX_FAIL - Error starting DB Learning operation.
 */ 
IX_STATUS
ixEthAccCodeletDBLearning(void);


/**
 *
 * This function is called at the end of each ethernet operation.
 * It displays the MIB statistics for each port.
 *
 * return void
 */
void
ixEthAccCodeletShow(void);



/** Recommended priority of queue manager dispatch loop */
#define IX_ETHACC_CODELET_QMR_PRIORITY 150 

/** Recommended priority of Ethernet DB Maintenance task */
#define IX_ETHACC_CODELET_DB_PRIORITY 91 

#if defined (__wince)
/* use interrupts for performances */
#define IX_ETH_CODELET_QMGR_DISPATCH_MODE TRUE
#else
/* use polled mode for performances */
#define IX_ETH_CODELET_QMGR_DISPATCH_MODE FALSE
#endif

#ifdef __linux
#define IX_ETHACC_CODELET_USER_INSTRUCTION \
    "Unload the codelet module to stop the operation ... \n"
#elif defined(__wince)
#define IX_ETHACC_CODELET_USER_INSTRUCTION \
    "Type 'q'<enter> at any time to stop the operation ... \n"
#else
#define IX_ETHACC_CODELET_USER_INSTRUCTION \
    "Press ESC at any time to stop the operation ... \n"
#endif

/*
 * Internal functions declarations.
 */

/* start and stop all tests */
IX_STATUS ixEthAccCodeletRxSinkStart(IxEthAccPortId portId);
IX_STATUS ixEthAccCodeletRxSinkStop(IxEthAccPortId portId);
IX_STATUS ixEthAccCodeletTxGenRxSinkLoopbackStart(IxEthAccPortId sinkPortId, IxEthAccPortId loopPortId);
IX_STATUS ixEthAccCodeletTxGenRxSinkLoopbackStop(IxEthAccPortId sinkPortId, IxEthAccPortId loopPortId);
IX_STATUS ixEthAccCodeletSwLoopbackStart(IxEthAccPortId portId);
IX_STATUS ixEthAccCodeletSwLoopbackStop(IxEthAccPortId portId);
IX_STATUS ixEthAccCodeletPhyLoopbackStart(IxEthAccPortId portId);
IX_STATUS ixEthAccCodeletPhyLoopbackStop(IxEthAccPortId portId);
IX_STATUS ixEthAccCodeletSwBridgeStart(IxEthAccPortId firstPortId1,
				       IxEthAccPortId secondPortId2);
IX_STATUS ixEthAccCodeletSwBridgeStop(IxEthAccPortId firstPortId1, 
				      IxEthAccPortId secondPortId2);
IX_STATUS ixEthAccCodeletSwBridgeQoSStart(IxEthAccPortId firstPortId1,
					  IxEthAccPortId secondPortId2);
IX_STATUS ixEthAccCodeletSwBridgeQoSStop(IxEthAccPortId firstPortId1, 
					 IxEthAccPortId secondPortId2);
IX_STATUS ixEthAccCodeletSwBridgeFirewallStart(IxEthAccPortId firstPortId1,
					       IxEthAccPortId secondPortId2);
IX_STATUS ixEthAccCodeletSwBridgeFirewallStop(IxEthAccPortId firstPortId1, 
					      IxEthAccPortId secondPortId2);
IX_STATUS ixEthAccCodeletDBLearningRun(BOOL validPorts[]);
IX_STATUS ixEthAccCodeletSwBridgeWiFiStart(IxEthAccPortId firstPortId, 
                                           IxEthAccPortId secondPortId);
IX_STATUS ixEthAccCodeletSwBridgeWiFiStop(IxEthAccPortId firstPortId, 
                                          IxEthAccPortId secondPortId);

/* buffer pool access */
IX_STATUS ixEthAccCodeletMemPoolInit(void);
IX_STATUS ixEthAccCodeletMemPoolFree(void);
void ixEthAccCodeletMbufChainSizeSet(IX_OSAL_MBUF *mBufPtr);
IX_STATUS ixEthAccCodeletReplenishBuffers(IxEthAccPortId portNo,
					  UINT32 num);
void ixEthAccCodeletMemPoolFreeRxCB(UINT32 cbTag, 
				    IX_OSAL_MBUF* mBufPtr, 
				    UINT32 reserved);
void ixEthAccCodeletMultiBufferMemPoolFreeRxCB(UINT32 cbTag, 
				    IX_OSAL_MBUF** mBufPtr);
void ixEthAccCodeletMemPoolFreeTxCB(UINT32 cbTag, IX_OSAL_MBUF* mBufPtr);
IX_STATUS ixEthAccCodeletRecoverBuffers(void);

/* tasks start and stop */
IX_STATUS ixEthAccCodeletDispatcherStart(BOOL useInterrupt);
IX_STATUS ixEthAccCodeletDispatcherStop(BOOL useInterrupt);
IX_STATUS ixEthAccCodeletDBMaintenanceStart(void);
IX_STATUS ixEthAccCodeletDBMaintenanceStop(void);

/* Port setup */
IX_STATUS ixEthAccCodeletPortConfigure(IxEthAccPortId portId,
	       IxEthAccPortRxCallback portRxCB,
	       IxEthAccPortMultiBufferRxCallback portMultiBufferRxCB,
	       IxEthAccPortTxDoneCallback portTxDoneCB,
	       UINT32 callbackTag);
IX_STATUS ixEthAccCodeletPortInit(IxEthAccPortId portId);
IX_STATUS ixEthAccCodeletPortUnconfigure(IxEthAccPortId portId);
IX_STATUS ixEthAccCodeletPortMultiBufferUnconfigure(IxEthAccPortId portId);

/* PHY setup */
IX_STATUS ixEthAccCodeletLinkUpCheck(IxEthAccPortId portId);
IX_STATUS ixEthAccCodeletPhyInit(void);
IX_STATUS ixEthAccCodeletPhyUninit(void);
IX_STATUS ixEthAccCodeletLinkLoopbackEnable(IxEthAccPortId portId);
IX_STATUS ixEthAccCodeletLinkLoopbackDisable(IxEthAccPortId portId);
IX_STATUS ixEthAccCodeletLinkSlowSpeedSet(IxEthAccPortId portId);
IX_STATUS ixEthAccCodeletLinkDefaultSpeedSet(IxEthAccPortId portId);

/* bridge callbacks */
void ixEthAccCodeletBridgeTxCB(UINT32 cbTag, IX_OSAL_MBUF* mBufPtr);
void ixEthAccCodeletBridgeRxCB(UINT32 cbTag, 
			       IX_OSAL_MBUF* mBufPtr, 
			       UINT32 reserved);

/* wait for test completion */
#ifdef __linux
void ixEthAccCodelet_wait(void);
#endif

/*
 * Variable declarations.
 */
typedef struct
{
    UINT32 rxCount;
    UINT32 txCount;
} IxEthAccCodeletStats;
  
#ifndef EXTERN
#define EXTERN extern
#endif

EXTERN IX_OSAL_MBUF *ixEthAccCodeletFreeBufQ;

EXTERN volatile UINT32 ixEthAccCodeletFreeCount;

EXTERN IxEthAccCodeletStats ixEthAccCodeletStats[IX_ETHACC_CODELET_MAX_PORT];

/* Buffer management utilities */

/** Macro to check if the free buffer queue is empty */
#define IX_ETHACC_CODELET_IS_Q_EMPTY(mbuf_list) (mbuf_list == NULL)


/** Macro to add an MBUF to the queue. The queue uses the m_nextpkt pointer 
 * which is not used by the Ethernet Access component */ 
#define IX_ETHACC_CODELET_ADD_MBUF_TO_Q_HEAD(mbuf_list,mbuf_to_add)   	\
  {									\
    int lockVal = ixOsalIrqLock();					\
    IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR((mbuf_to_add)) = mbuf_list;		\
    (mbuf_list) = (mbuf_to_add);					\
    ixEthAccCodeletFreeCount++;                                         \
    ixOsalIrqUnlock(lockVal);						\
  }


/** Macro to remove and MBUF from the queue */
#define IX_ETHACC_CODELET_REMOVE_MBUF_FROM_Q_HEAD(mbuf_list,mbuf_to_rem)\
  {									\
    int lockVal = ixOsalIrqLock();					\
    if ((mbuf_list) != NULL ) 						\
    {									\
        (mbuf_to_rem) = (mbuf_list);					\
        (mbuf_list) = (IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR((mbuf_to_rem)));	\
        IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(mbuf_to_rem) = NULL;         \
        ixEthAccCodeletFreeCount--;                                     \
    } 									\
    else								\
    {									\
        (mbuf_to_rem) = NULL;						\
    } 									\
    ixOsalIrqUnlock(lockVal);						\
  }


/** Macro to check if the codelet has been initialised */
#define IX_ETHACC_IS_CODELET_INITIALISED()				\
  {									\
    if(!ixEthAccCodeletInitialised)					\
    {									\
	printf("Ethernet Access Codelet not yet initialised!\n");	\
	return (IX_FAIL);						\
    }									\
  }

#endif /* IXETHACCCODELET_P_H */
