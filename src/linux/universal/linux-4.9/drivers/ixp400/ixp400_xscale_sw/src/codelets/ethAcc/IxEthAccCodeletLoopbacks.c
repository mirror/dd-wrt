/**
 * @file IxEthAccCodeletLoopbacks.c
 *
 * @date 01 July 2005
 *
 * @brief This file contains the implementation of the Ethernet Access 
 * Codelet that implements the different loopback scenarios
 *
 * @li RxSink
 * @li Sw Loopback
 * @li TxGen RxSink
 * @li Phy Loopback
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

/*
 * Put the system defined include files required.
 */

#include "IxOsal.h"
#include "IxEthDB.h"

/*
 * Put the user defined include files required.
 */
#include "IxEthAccCodelet.h"
#include "IxEthAccCodelet_p.h"

/*
 * Private data declarations.
 */

/* Generated frames payload data */
PRIVATE UINT8 compData[IX_ETHACC_CODELET_PCK_LEN];

/** Macro to generate random payload data 
 * (multicast|broadcast bits are masked in both source 
 * and destination MAC addresses, typelength is set to IP)
 */
#define IX_ETHACC_CODELET_DATAGEN(compPtr)				\
  {									\
	int i = 0;							\
        UINT8 random = (UINT8)ixOsalTimestampGet();                     \
	for(i=0; i<IX_ETHACC_CODELET_TXGEN_PCK_LEN; i++)		\
	 {								\
	     compPtr[i] = random++;					\
	 }								\
        compPtr[0] &= 0xfe;                                             \
        compPtr[IX_IEEE803_MAC_ADDRESS_SIZE] &= 0xfe;                   \
        compPtr[2 * IX_IEEE803_MAC_ADDRESS_SIZE] = 0x08;                \
        compPtr[2 * IX_IEEE803_MAC_ADDRESS_SIZE + 1] &= 0x00;           \
  }


/** Macro to verify payload */
#define IX_ETHACC_CODELET_DATA_VERIFY(m_data, compPtr)			     \
  {									     \
      if(memcmp(&compPtr[0], m_data, IX_ETHACC_CODELET_TXGEN_PCK_LEN) != 0)  \
	{								     \
	    ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,                     \
		      "Mbuf data verification failed\n",                     \
		      0, 0, 0, 0, 0, 0);                                     \
	}								     \
  }

/*
 * SECTION I : Datapath callbacks
 *
 * The following callbacks are supposed to run under the QMgr dispatcher
 * context and are able to process the following options
 * - re-transmit received frames (Loopback)
 * - drop incoming traffic (RxSink)
 * - retransmit transmitted frames again (TxGen)
 *
 */

/**
 * @fn void ixEthAccCodeletRxSinkRxCB()
 * 
 * Recieve callback for Rx sink
 * 
 * @return void
 */
PRIVATE void 
ixEthAccCodeletRxSinkRxCB(UINT32 cbTag, IX_OSAL_MBUF** mBufPtr)
{
    while (NULL != *mBufPtr)
    {
	ixEthAccCodeletStats[cbTag].rxCount++;
	
#ifdef IX_ETHACC_CODELET_TXGENRXSINK_VERIFY
	IX_ETHACC_CODELET_DATA_VERIFY(IX_OSAL_MBUF_MDATA(*mBufPtr), compData);
	
	/* After reading the payload, invalidate it before replenish */
	IX_OSAL_CACHE_INVALIDATE(IX_OSAL_MBUF_MDATA(*mBufPtr), 
				 IX_OSAL_MBUF_MLEN(*mBufPtr));
#endif
	
	ixEthAccCodeletMbufChainSizeSet(*mBufPtr);

	/* Return mBuf to free queue on same port */
	if(ixEthAccPortRxFreeReplenish(cbTag, *mBufPtr) != IX_SUCCESS)
	{
	    ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
		      "Error replenishing RX free q in TxGen-RxSink\n", 
		      0, 0, 0, 0, 0, 0);
	}
	/* move to next buffer received */
	mBufPtr++;
    }
}

/**
 * @fn void ixEthAccCodeletTxGenTxCB()
 *
 * Transmit callback for TxGen operation. Transmitted frames are re-
 * transmitted.
 * 
 * @return void
 */
PRIVATE void 
ixEthAccCodeletTxGenTxCB(UINT32 cbTag, 
			 IX_OSAL_MBUF* mBufPtr)
{
    ixEthAccCodeletStats[cbTag].txCount++;

    /* Re-transmit the frame on same port */
    if(ixEthAccPortTxFrameSubmit(cbTag, mBufPtr,
				 IX_ETH_ACC_TX_DEFAULT_PRIORITY) != IX_ETH_ACC_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
		  "Tx Buffer submission failure in TxGen-RxSink\n", 
		  0, 0, 0, 0, 0, 0);
    }
}

/**
 * @fn void ixEthAccCodeletLoopbackTxCB()
 *
 * Transmit callback for the Sw Loopback Operation. Transmitted frames
 * are placed in the receive free pool.
 * 
 * @return void
 */

PRIVATE void 
ixEthAccCodeletLoopbackTxCB(UINT32 cbTag, 
			    IX_OSAL_MBUF* mBufPtr)
{
    /* Put the mbuf back in the rx free pool */
    if(IX_ETHNPE_PHYSICAL_ID_TO_LOGICAL_ID(cbTag) < IX_ETHNPE_UNKNOWN_PORT)
    {
	ixEthAccCodeletStats[cbTag].txCount++;

	/* reset the frame length to the length at pool setup */
	ixEthAccCodeletMbufChainSizeSet(mBufPtr);

	if(ixEthAccPortRxFreeReplenish(cbTag, mBufPtr) != IX_SUCCESS)
	{
	    ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
		      "Error replenishing RX free queue on port %d\n", 
		      cbTag, 0, 0, 0, 0, 0);
	}    
    }
    else
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
		  "Invalid Tx callback tag %d in RxSink Operation\n", 
		  cbTag, 0, 0, 0, 0, 0);
    }
}


/**
 * @fn void ixEthAccCodeletLoopbackRxCB()
 *
 * Receive callback for the Sw Loopback Operation. Any frames received
 * are transmitted back out on the same port.
 * 
 * @return void
 */

PRIVATE void
ixEthAccCodeletLoopbackRxCB(UINT32 cbTag, IX_OSAL_MBUF** mBufPtr)
{
    /* Transmit the buffer back on the port it was received on*/
    if(IX_ETHNPE_PHYSICAL_ID_TO_LOGICAL_ID(cbTag) < IX_ETHNPE_UNKNOWN_PORT )
    {
	while (NULL != *mBufPtr)
	{
	    ixEthAccCodeletStats[cbTag].rxCount++;

	    /* transmit the frame on the same port */
	    if(ixEthAccPortTxFrameSubmit(cbTag, 
					 *mBufPtr,
					 IX_ETH_ACC_TX_DEFAULT_PRIORITY) 
	       != IX_ETH_ACC_SUCCESS)
	    {
		ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
			  "Tx Buffer submission failure port %d\n", 
			  cbTag, 0, 0, 0, 0, 0);
	    }
	    
	    /* move to next buffer received */
	    mBufPtr++;
	}
    }
    else
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
		  "Invalid Rx callback tag %d in Sw Loopback Operation\n", 
		  cbTag, 0, 0, 0, 0, 0);
    }
}

/*
 * SECTION II : Control path
 *
 * the following functions register the different callbacks 
 * to enable the following features
 * - Sink all rx traffic, silent on Tx
 * - sw loopback on one port (Rx frames are transmitted on the 
 *   same port)
 * - transmit as fast as possible on one port, sw loopback on 
 *   an other port, allows full traffic to be seen with a
 *   crossover cable.
 *
 */

/*
 * Function definition: ixEthAccCodeletTxGenRxSinkStart()
 *
 * Start datapath traffic for TxGen-RxSink demo.
 */

PRIVATE IX_STATUS 
ixEthAccCodeletTxGenRxSinkStart(IxEthAccPortId portId)
{
    IX_OSAL_MBUF *mBufPtr;
    UINT32 numBufs;
    IxEthDBStatus status;

    /* port transmits as fast as possible and drops rx traffic */
    if (ixEthAccCodeletPortConfigure(portId,
				     NULL,
				     ixEthAccCodeletRxSinkRxCB, 
				     ixEthAccCodeletTxGenTxCB,
				     portId)
	!= IX_ETH_ACC_SUCCESS)
    {
	printf("Loopbacks: Failed to start the Tx-Gen Rx-Sink Operation port %u\n",
	       (UINT32)portId);
	return IX_FAIL;
    }

    /* Disable MAC learning and filtering at the port that sinks the frame. 
       With learning enabled at both ports, TxGenRxSink will report buffer underrun and overruns 
       as this mode generates a heavy load of MAC address migrations in the learning/filtering 
       database. 
     */
    status = ixEthDBFeatureEnable (portId, IX_ETH_DB_FILTERING, FALSE);
    if (IX_ETH_DB_SUCCESS == status)
    {
        status = ixEthDBFeatureEnable (portId, IX_ETH_DB_LEARNING, FALSE); 
    }

    if (IX_ETH_DB_SUCCESS == status)
    {
        printf("\nMAC learning & filtering are disabled at port %d\n", portId);
        printf("This is to prohibit the MAC address from being migrated back and forth\n");
        printf("between two connected ports in the learning/filtering database.\n\n");
    }
    else
    {
        printf("\nFailed to disable MAC learning & filtering at port %d.\n", portId);
        printf("TxGenRxSink will report buffer underrun and overruns as this mode generates \n");
        printf("a heavy load of MAC address migrations in the learning/filtering database. \n");
        printf("With learning enabled at both ports, buffer underrun and overruns are expected.\n");
    }

    /* Generate our random data for the payload */
    IX_ETHACC_CODELET_DATAGEN(compData);
	
    /* Now start the loopback by transmitting the first few frames */
    for (numBufs=0; numBufs<IX_ETHACC_CODELET_TXGEN_PCKS; numBufs++)
    {
	IX_ETHACC_CODELET_REMOVE_MBUF_FROM_Q_HEAD(ixEthAccCodeletFreeBufQ,
						  mBufPtr);
	
	if (mBufPtr == NULL)
	{
	    printf("Loopbacks: Buffer queue empty. Not enough free buffers to transmit in TxGen-RxSink Loopback!\n");
	    return (IX_FAIL);
	}
	
	IX_OSAL_MBUF_MLEN(mBufPtr) = IX_ETHACC_CODELET_TXGEN_PCK_LEN;
	IX_OSAL_MBUF_PKT_LEN(mBufPtr) = IX_ETHACC_CODELET_TXGEN_PCK_LEN;
	
	ixOsalMemCopy(IX_OSAL_MBUF_MDATA(mBufPtr), 
	       &compData[0], 
	       IX_ETHACC_CODELET_TXGEN_PCK_LEN);
	
	IX_OSAL_CACHE_FLUSH(IX_OSAL_MBUF_MDATA(mBufPtr), 
				IX_OSAL_MBUF_MLEN(mBufPtr));
	
	if(ixEthAccPortTxFrameSubmit(portId, 
				     mBufPtr,
				     IX_ETH_ACC_TX_DEFAULT_PRIORITY)
	   != IX_ETH_ACC_SUCCESS)
	{
	    printf("Loopbacks: Error Submitting frame for transmission on port %u\n",
		   portId);
	    return (IX_FAIL);
	}
    }
    printf("Port %d Tx pool has %d buffers\n", portId, numBufs);

    /* enable traffic */
    if(ixEthAccPortEnable(portId) 
       != IX_ETH_ACC_SUCCESS)
    {
	printf("Loopbacks: Error Enabling port %u\n",
	       (UINT32)portId);
	return (IX_FAIL);
    }

    return (IX_SUCCESS);
}

/*
 * Function definition: ixEthAccCodeletStop()
 *
 * Stop datapath traffic for the demo. This works assuming nobody will
 * continue to transmit or replenish for this port. This function
 * will first change the registered callbacks to callbacks which will 
 * drop all traffic. After this portDisable is called to flush all 
 * pending traffic inside the NPEs.
 */
PRIVATE IX_STATUS 
ixEthAccCodeletPortLoopbackStop(IxEthAccPortId portId)
{
    
    if (ixEthAccCodeletPortMultiBufferUnconfigure(portId) 
	!= IX_ETH_ACC_SUCCESS)
    {
	printf("Loopbacks: Failed to unconfigure the port %u\n",
	       (UINT32)portId);
	return IX_FAIL;
    }

    /* disable traffic */
    if(ixEthAccPortDisable(portId) 
       != IX_ETH_ACC_SUCCESS)
    {   
	printf("Loopbacks: Error Disabling port %u\n",
	       (UINT32)portId);
	return (IX_FAIL);
    }

    return (IX_SUCCESS);
}

/*
 * Function definition: ixEthAccCodeletSwLoopbackStart()
 *
 * Start datapath traffic for Sw Loopback demo.
 */
IX_STATUS 
ixEthAccCodeletSwLoopbackStart(IxEthAccPortId portId)
{

    if (ixEthAccCodeletPortConfigure(portId,
				     NULL,
				     ixEthAccCodeletLoopbackRxCB,
				     ixEthAccCodeletLoopbackTxCB, 
				     portId) 
	!= IX_ETH_ACC_SUCCESS)
    {
	printf("Loopbacks: Failed to configure the port %u\n",
	       (UINT32)portId);
	return IX_FAIL;
    }

    /* enable traffic */
    if(ixEthAccPortEnable(portId) 
       != IX_ETH_ACC_SUCCESS)
    {
	printf("Loopbacks: Error enabling port %u\n",
	       (UINT32)portId);
	return (IX_FAIL);
    }

    return (IX_SUCCESS);
}

/*
 * Function definition: ixEthAccCodeletSwLoopbackStop()
 *
 * Stop datapath traffic for Sw Loopback demo.
 */
IX_STATUS 
ixEthAccCodeletSwLoopbackStop(IxEthAccPortId portId)
{
    return ixEthAccCodeletPortLoopbackStop(portId);
}

/*
 * Function definition: ixEthAccCodeletTxGenRxSinkLoopbackStart()
 *
 * Start datapath traffic for TxGen-RxSink 
 *   - port 1 : txGen + rxSink
 *   - port 2 : sw loopback
 * This setup is compatible with a crossover cable. However, the same 
 * traffic is seen on both ports and a huge amount of MAC address 
 * migrations is expected.
 *
 */

IX_STATUS 
ixEthAccCodeletTxGenRxSinkLoopbackStart(IxEthAccPortId sinkPortId,
					IxEthAccPortId loopPortId)
{
    IX_STATUS ret = IX_SUCCESS;

    if (sinkPortId == loopPortId)
    {
	printf("TxGenRxSinkLoopback: Cannot configure a Port %u in both txGenRxSink and Loopback (Ports must be different)\n", sinkPortId);
	return (IX_FAIL);
    }

    ret = ixEthAccCodeletTxGenRxSinkStart(sinkPortId);

    if (ret != IX_SUCCESS)
	return ret;
    else
	return ixEthAccCodeletSwLoopbackStart(loopPortId);
}

/*
 * Function definition:  ixEthAccCodeletTxGenRxSinkLoopbackStop()
 *
 * Stop datapath traffic for TxGen-RxSink demo.
 */
IX_STATUS 
ixEthAccCodeletTxGenRxSinkLoopbackStop(IxEthAccPortId sinkPortId, IxEthAccPortId loopPortId)
{
    IX_STATUS ret = IX_SUCCESS;

    ret = ixEthAccCodeletPortLoopbackStop(sinkPortId);

    if (ret != IX_SUCCESS)
	return ret;
    else
        return ixEthAccCodeletPortLoopbackStop(loopPortId);
}

/*
 * Function definition: ixEthAccCodeletRxSinkStart()
 *
 * Start datapath traffic for RX Sink demo.
 */
IX_STATUS ixEthAccCodeletRxSinkStart(IxEthAccPortId portId)
{
    
    if (ixEthAccCodeletPortConfigure(portId,
				     NULL,
				     ixEthAccCodeletRxSinkRxCB,
				     ixEthAccCodeletMemPoolFreeTxCB, 
				     portId) 
	!= IX_ETH_ACC_SUCCESS)
    {
	printf("Loopbacks: Failed to configure the port %u\n",
	       (UINT32)portId);
	return IX_FAIL;
    }

    /* ready to get traffic */
    if(ixEthAccPortEnable(portId) 
       != IX_ETH_ACC_SUCCESS)
    {
	printf("Loopbacks: Error enabling port %u\n",
	       (UINT32)portId);
	return (IX_FAIL);
    }

    return (IX_SUCCESS);
}

/*
 * Function definition: ixEthAccCodeletRxSinkStop()
 *
 * Stop datapath traffic for RX Sink demo.
 */
IX_STATUS ixEthAccCodeletRxSinkStop(IxEthAccPortId portId)
{
    return ixEthAccCodeletPortLoopbackStop(portId);
}

/*
 * Function definition:  ixEthAccCodeletPhyLoopbackStart()
 *
 * Start datapath traffic for PHY loopback demo.
 */
IX_STATUS 
ixEthAccCodeletPhyLoopbackStart(IxEthAccPortId portId)
{
    if (ixEthAccCodeletLinkLoopbackEnable(portId)
	!= IX_SUCCESS)
    {
	printf("Loopbacks: Failed to enable a phy loopback on port %u\n",
	       (UINT32)portId);
	return IX_FAIL;
    }
    
    return ixEthAccCodeletTxGenRxSinkStart(portId);
}

/*
 * Function definition:  ixEthAccCodeletPhyLoopbackStop()
 *
 * Stop datapath traffic for PHY loopback demo.
 */
IX_STATUS 
ixEthAccCodeletPhyLoopbackStop(IxEthAccPortId portId)
{
    /* register the datapath TxDone callbacks */
    if(ixEthAccPortTxDoneCallbackRegister(portId,
					  ixEthAccCodeletMemPoolFreeTxCB,
					  portId) != IX_ETH_ACC_SUCCESS)
    {
	printf("PortSetup: Failed to register Tx done callback for port %u\n",
	       (UINT32)portId);
	return (IX_FAIL);
    }

    if (ixEthAccCodeletLinkLoopbackDisable(portId)
	!= IX_SUCCESS)
    {
	printf("Loopbacks: Failed to disable a phy loopback on port %u\n",
	       (UINT32)portId);
	return IX_FAIL;
    }

    if (ixEthAccCodeletPortLoopbackStop(portId)
	!= IX_SUCCESS)
    {
	printf("Loopbacks: Failed to disable port %u\n",
	       (UINT32)portId);
	return IX_FAIL;
    }

    return IX_SUCCESS;
}


