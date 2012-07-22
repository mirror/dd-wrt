/**
 * @file IxEthAccCodeletSwBridge.c
 *
 * @date 22 April 2002
 *
 * @brief This file contains the implementation of the Ethernet Access
 * Codelet that implements a simple bridge between two Ethernet ports
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

/*
 * Put the user defined include files required.
 */
#include "IxEthAccCodelet.h"
#include "IxEthAccCodelet_p.h"

/**
 * Function definition: ixEthAccCodeletBridgeTxCB()
 *
 * Transmit callback for Bridge Operation. Transmitted frames are put on
 * the receive queue of the other port.
 */

void ixEthAccCodeletBridgeTxCB(UINT32 cbTag, IX_OSAL_MBUF* mBufPtr)
{
    IxEthAccPortId thisPortId = (IxEthAccPortId)(cbTag & 0xffff);
    IxEthAccPortId otherPortId = (IxEthAccPortId)(cbTag>>16);

    /* 
     * Put the mbuf on the Rx free q of the other port since it was
     * received there initially.
     */
    ixEthAccCodeletStats[thisPortId].txCount++;

    ixEthAccCodeletMbufChainSizeSet(mBufPtr);

    if(ixEthAccPortRxFreeReplenish(otherPortId, mBufPtr)!=IX_SUCCESS)
    {
	    ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
		      "Error replenishing RX free q for port %d\n", 
		      cbTag, 0, 0, 0, 0, 0);
    }    
}

/**
 * Function definition: ixEthAccCodeletBridgeRxCB()
 *
 * Receive callback for Bridge Operation. Received frames are retransmitted
 * on the other port.
 * 
 */

void ixEthAccCodeletBridgeRxCB(UINT32 cbTag, 
			       IX_OSAL_MBUF* mBufPtr, 
			       UINT32 destPortId)
{
    IxEthAccPortId thisPortId = (IxEthAccPortId)(cbTag & 0xffff);
    IxEthAccPortId otherPortId = (IxEthAccPortId)(cbTag>>16);

    /* Transmit the buffer on the other port */
    ixEthAccCodeletStats[thisPortId].rxCount++;

    if(ixEthAccPortTxFrameSubmit(otherPortId, 
				     mBufPtr,
				     IX_ETH_ACC_TX_DEFAULT_PRIORITY)
	   !=IX_ETH_ACC_SUCCESS)
    {
        ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT, 
		      "Tx Buffer submission failed on port 2\n", 
		      0, 0, 0, 0, 0, 0);
    }
}

/*
 * Function definition: ixEthAccCodeletSwBridgeStart()
 *
 * Start bridge datapath
 */
IX_STATUS ixEthAccCodeletSwBridgeStart(IxEthAccPortId firstPortId, 
				       IxEthAccPortId secondPortId)
{
    UINT32 firstPortCbTag = firstPortId | (secondPortId << 16);
    UINT32 secondPortCbTag = secondPortId | (firstPortId << 16);

    if (firstPortId == secondPortId)
    {
	printf("SwBridge: Cannot configure a bridge between Port %u and Port %u (ports must be different)\n", 
                firstPortId, 
                secondPortId);
	return (IX_FAIL);
    }

    /* Configure and register the traffic callbacks for both ports */
    if ( ixEthAccCodeletPortConfigure(firstPortId, 
				      ixEthAccCodeletBridgeRxCB, 
				      (IxEthAccPortMultiBufferRxCallback) NULL,
				      ixEthAccCodeletBridgeTxCB,
				      firstPortCbTag) != IX_SUCCESS)
    {
	printf("SwBridge: Failed to configure Port %u\n", firstPortId);
	return (IX_FAIL);
    }

    if ( ixEthAccCodeletPortConfigure(secondPortId, 
				      ixEthAccCodeletBridgeRxCB, 
				      NULL,
				      ixEthAccCodeletBridgeTxCB,
				      secondPortCbTag) != IX_SUCCESS)
    {
	printf("SwBridge: Failed to configure Port %u\n", secondPortId);
	return (IX_FAIL);
    }

    /* Enable the traffic over both ports */
    if ( ixEthAccPortEnable(firstPortId) != IX_SUCCESS)
    {
	printf("SwBridge: Failed to enable Port %u\n", firstPortId);
	return (IX_FAIL);
    }

    if ( ixEthAccPortEnable(secondPortId) != IX_SUCCESS)
    {
	printf("SwBridge: Failed to enable Port %u\n", secondPortId);
	return (IX_FAIL);
    }

    return (IX_SUCCESS);
}

/*
 * Function definition: ixEthAccCodeletSwBridgeStop()
 *
 * Stop bridge datapath
 */
IX_STATUS ixEthAccCodeletSwBridgeStop(IxEthAccPortId firstPortId, 
				      IxEthAccPortId secondPortId)
{
    /* Unconfigure the traffic callbacks for both ports */
    if ( ixEthAccCodeletPortUnconfigure(firstPortId)
	 != IX_SUCCESS)
    {
	printf("SwBridge: Failed to unconfigure Port %u\n", firstPortId);
	return (IX_FAIL);
    }

    if ( ixEthAccCodeletPortUnconfigure(secondPortId)
	 != IX_SUCCESS)
    {
	printf("SwBridge: Failed to unconfigure Port %u\n", secondPortId);
	return (IX_FAIL);
    }

    /* Disable the traffic over both ports */
    if ( ixEthAccPortDisable(firstPortId)
	 != IX_SUCCESS)
    {
	printf("SwBridge: Failed to disable Port %u\n", firstPortId);
	return (IX_FAIL);
    }

    if ( ixEthAccPortDisable(secondPortId)
	 != IX_SUCCESS)
    {
	printf("SwBridge: Failed to disable Port %u\n", secondPortId);
	return (IX_FAIL);
    }

    return (IX_SUCCESS);
}
