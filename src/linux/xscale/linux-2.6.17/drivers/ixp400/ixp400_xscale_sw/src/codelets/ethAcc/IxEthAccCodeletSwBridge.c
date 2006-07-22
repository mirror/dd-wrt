/**
 * @file IxEthAccCodeletSwBridge.c
 *
 * @date 22 April 2002
 *
 * @brief This file contains the implementation of the Ethernet Access
 * Codelet that implements a simple bridge between two Ethernet ports
 * 
 * @par
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
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
