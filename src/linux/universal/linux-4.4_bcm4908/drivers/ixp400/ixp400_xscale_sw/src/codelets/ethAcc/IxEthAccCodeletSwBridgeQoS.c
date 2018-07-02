/**
 * @file IxEthAccCodeletSwBridgeQoS.c
 *
 * @date 22 April 2004
 *
 * @brief This file contains the implementation of the Ethernet Access
 * Codelet that implements a simple bridge between two Ethernet ports,
 * with VLAN tagged frames being prioritized over untagged frames
 *
 * <pre>
 *
 * The setup is as follows
 *
 *  VLAN tagged network ----- first Port ---+
 *                                          |
 *                                          |
 *                                    Bridge on Xscale
 *                                          |
 *  untagged                                |
 *  10 Mb/s network -------- second Port ---+
 *  link
 * 
 *
 * The first port is configured to accept both VLAN tagged and untagged 
 * frames from the network. All egress non VLAN frames will be filled
 * with the default VLAN tag for this port. VLAN tags are stripped out
 * from ingress frames.
 *
 * The second port disacrds all recieved tagged frames from the network
 * and allows only untagged frames to enter the bridge.
 *
 * Because the second port is configured as 10Mb/s, congestion is 
 * expected to occur when processing traffic from the first port
 * to the second port.
 *
 * </pre>
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
#include "IxEthAcc.h"
#include "IxEthDB.h"
#include "IxEthAccCodelet.h"
#include "IxEthAccCodelet_p.h"

/**
 * Range of VLAN IDs from Min to Max that will be accepted on the VLAN
 * enabled port
 */
#define IX_ETHACC_CODELET_VLANID_MIN (100)
#define IX_ETHACC_CODELET_VLANID_MAX (200)

/* This is the maximum number of transmits allowed before the QoS bridge
 * begins throttling receive.  Thus this value must be less than the
 * rx replenish pool size for one port.
 */
#define IX_ETHACC_CODELET_QOS_BRIDGE_MAX_PENDING_TX (128)

/** 
 * Default VLAN ID for untagged frames received on the VLAN enabled port 
 */
#define IX_ETHACC_CODELET_VLANID_DEFAULT (150)

/* prototype for TxDone callback */
extern void 
ixEthTxFrameDoneQMCallback(UINT32 qId, UINT32 callbackId);

/**
 * UINT32 array of pendingTx counters for each port
 * tells the rx callback how many transmits are pending completion
 */
UINT32 pendingTx[IX_ETHACC_CODELET_MAX_PORT];

/**
 * @fn void ixEthAccCodeletSwBridgeQoSTxCB()
 *
 * Transmit done callback for Bridge Operation. Transmitted frames are put
 * on the rx free queue of the other port.
 *
 */

PRIVATE void ixEthAccCodeletSwBridgeQoSTxCB(UINT32 cbTag, IX_OSAL_MBUF* mBufPtr)
{
    IxEthAccPortId thisPortId = (IxEthAccPortId)(cbTag & 0xffff);
    IxEthAccPortId otherPortId = (IxEthAccPortId)(cbTag>>16);

    /* 
     * Put the mbuf on the rx freeQ of the other port since it was
     * received there initially.
     */
    ixEthAccCodeletStats[thisPortId].txCount++;

    /* one tx that was pending is now complete */
    pendingTx[thisPortId]--;
    
    /* reset the frame length */
    ixEthAccCodeletMbufChainSizeSet(mBufPtr);

    /* replenish the other port */
    if(ixEthAccPortRxFreeReplenish(otherPortId, 
				   mBufPtr)
       !=IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
		  "SwBridgeQos: Error replenishing RX free q for port %u\n", 
		  (UINT32)(1 - cbTag), 0, 0, 0, 0, 0);
    }
}

/**
 * @fn void ixEthAccCodeletSwBridgeQoSTaggedToUntaggedRxCB()
 *
 * Receive callback for Bridge Operation. Received frames are 
 * retransmitted on the other port with the priority of the rx frame.
 * 
 */

PRIVATE void ixEthAccCodeletSwBridgeQoSTaggedToUntaggedRxCB(UINT32 cbTag, 
					    IX_OSAL_MBUF* mBufPtr, 
					    UINT32 destPortId)
{
    IxEthAccPortId thisPortId = (IxEthAccPortId)(cbTag & 0xffff);
    IxEthAccPortId otherPortId = (IxEthAccPortId)(cbTag>>16);

    /* Extract the priority of the incoming buffer header
     */
    IxEthAccTxPriority priority = 
	(IX_ETH_DB_GET_QOS_PRIORITY(IX_ETHACC_NE_VLANTCI(mBufPtr)));

    ixEthAccCodeletStats[thisPortId].rxCount++;

    /* This frame is untagged on Rx, just clear the flags 
     * to bypass VLAN processing on the Tx path
     */
    IX_ETHACC_NE_FLAGS(mBufPtr) = 0;

    /* this while loop is what throttles the receive processing to keep the RxFree queue
     * form becoming empty when we receive faster than transmitting
     */
    while(pendingTx[otherPortId] >= IX_ETHACC_CODELET_QOS_BRIDGE_MAX_PENDING_TX)
    {
        ixEthTxFrameDoneQMCallback(0,0);
    }


    /* Transmit the buffer on the other port with its own priority
     */
    if(ixEthAccPortTxFrameSubmit(otherPortId, 
				 mBufPtr,
				 priority)
       !=IX_ETH_ACC_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
		  "SwBridgeQos: Tx Buffer submission failed on port %u\n", 
		  (UINT32)(1 - cbTag), 0, 0, 0, 0, 0);
    }

    /* incriment pendingTx */
    pendingTx[otherPortId]++;

}

/**
 * @fn void ixEthAccCodeletSwBridgeQoSUntaggedToTaggedRxCB()
 *
 * Receive callback for Bridge Operation. Received frames are 
 * retransmitted on the other port with the priority of the rx frame.
 * 
 */

PRIVATE void ixEthAccCodeletSwBridgeQoSUntaggedToTaggedRxCB(UINT32 cbTag, 
					       IX_OSAL_MBUF* mBufPtr, 
					       UINT32 destPortId)
{
    IxEthAccPortId thisPortId = (IxEthAccPortId)(cbTag & 0xffff);
    IxEthAccPortId otherPortId = (IxEthAccPortId)(cbTag>>16);

    IxEthAccTxPriority priority = 
	(IX_ETH_DB_GET_QOS_PRIORITY(IX_ETHACC_CODELET_VLANID_DEFAULT));

    ixEthAccCodeletStats[thisPortId].rxCount++;

    /* Set the VLAN information for this frame : the outgoing frame
    * is tagged with the default tagging rules for this port
    */
    IX_ETHACC_NE_FLAGS(mBufPtr) = IX_ETHACC_NE_VLANENABLEMASK;
    IX_ETHACC_NE_VLANTCI(mBufPtr) = IX_ETHACC_CODELET_VLANID_DEFAULT;

    /* Transmit the buffer on the other port */
    if(ixEthAccPortTxFrameSubmit(otherPortId, 
				 mBufPtr,
				 priority)
       !=IX_ETH_ACC_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
		  "SwBridgeQos: Tx Buffer submission failed on port %u\n", 
		  (UINT32)(1 - cbTag), 0, 0, 0, 0, 0);
    }
}

/*
 * Function definition: ixEthAccCodeletSwBridgeQoSStart()
 *
 * Configure QoS and Start bridge datapath
 */
IX_STATUS ixEthAccCodeletSwBridgeQoSStart(IxEthAccPortId firstPortId, 
					  IxEthAccPortId secondPortId)
{
    UINT32 firstPortCbTag = firstPortId | (secondPortId << 16);
    UINT32 secondPortCbTag = secondPortId | (firstPortId << 16);

    IxEthDBPriorityTable priorityTable = { 0,1,2,3,4,5,6,7};
    IxEthDBFeature featureSet = 0;

    if (firstPortId == secondPortId)
    {
	printf("SwBridgeQoS: Cannot configure a Bridge Operation between port %u and port %u (ports must be different)\n",
	       firstPortId, 
               secondPortId);
	return (IX_FAIL);
    }

    /* initialize pendingTx for both ports */
    pendingTx[firstPortId] = pendingTx[secondPortId] = 0;

    /* register the rx/tx callback */
    if ( ixEthAccCodeletPortConfigure(firstPortId, 
	      ixEthAccCodeletSwBridgeQoSTaggedToUntaggedRxCB, 
	      (IxEthAccPortMultiBufferRxCallback) NULL,
	      ixEthAccCodeletSwBridgeQoSTxCB,
	      firstPortCbTag) != IX_SUCCESS)
    {
	printf("SwBridgeQoS: Failed to configure the Bridge Operation for port %u\n",
	       firstPortId);
	return (IX_FAIL);
    }

    if ( ixEthAccCodeletPortConfigure(secondPortId, 
	      ixEthAccCodeletSwBridgeQoSUntaggedToTaggedRxCB, 
	      NULL,
	      ixEthAccCodeletSwBridgeQoSTxCB,
	      secondPortCbTag) != IX_SUCCESS)
    {
	printf("SwBridgeQoS: Failed to configure the Bridge Operation for port %u\n",
	       secondPortId);
	return (IX_FAIL);
    }

    /* Enable the VLAN/QoS Feature in EthDB for each port but first 
     * check that the Firmware downloaded to the NPE can support it
     */
    ixEthDBFeatureCapabilityGet((IxEthDBPortId)firstPortId, &featureSet);
    
    if ((featureSet & IX_ETH_DB_VLAN_QOS) == 0)
    {
	printf("SwBridgeQoS: Port %u NPE image not VLAN/QoS capable\n",
	       firstPortId);
	return (IX_FAIL);
    }

    if ( ixEthDBFeatureEnable((IxEthDBPortId)firstPortId, 
			      IX_ETH_DB_VLAN_QOS,
			      TRUE) != IX_ETH_DB_SUCCESS )
    {
	printf("SwBridgeQoS: Failed to enable VLAN/QoS on port %u\n",
	       firstPortId);
	return (IX_FAIL);
    }

    /* Enable the EthDB Port in order to configure and download the
     * VLAN/QoS configuration information
     */ 
    if ((ixEthDBPortEnable(firstPortId)) != IX_ETH_DB_SUCCESS)
    {
        printf("SwBridgeQoS: Cannot enable port %u\n", firstPortId);
        return (IX_FAIL);
    }
    if ((ixEthDBPortEnable(secondPortId)) != IX_ETH_DB_SUCCESS)
    {
        printf("SwBridgeQos: Cannot enable port %u\n", secondPortId);
        return (IX_FAIL);
    }

    /* Configure Xscale QoS : the access layer datapath 
     * prioritizes the different classes of traffic.
     */
    printf("Set Tx Scheduling discipline...\n");
    if (ixEthAccTxSchedulingDisciplineSet(firstPortId, 
					  FIFO_PRIORITY)
	!= IX_ETH_ACC_SUCCESS)
    {
	printf("SwBridgeQos: Failed to set Tx Scheduling for discipline port %u\n",
	       (UINT32)firstPortId);
	return (IX_FAIL);
    }

    if (ixEthAccTxSchedulingDisciplineSet(secondPortId, 
					  FIFO_PRIORITY)
	!= IX_ETH_ACC_SUCCESS)
    {
	printf("SwBridgeQos: Failed to set Tx Scheduling for discipline port %u\n",
	       (UINT32)secondPortId);
	return (IX_FAIL);
    }

    printf("Set Rx Scheduling discipline...\n");
    if (ixEthAccRxSchedulingDisciplineSet(FIFO_PRIORITY)
	!= IX_ETH_ACC_SUCCESS)
    {
	printf("SwBridgeQos: Failed to set Rx Scheduling discipline!\n");
	return (IX_FAIL);
    }

    /* NPE QoS : Configure VLAN traffic for highest priority 
     * on first port. Tagging is enabled on Egress (use default tag
     * for this port) and untagging is enabled on ingress.
     * The traffic running on this bridge will be untagged.
     */
    printf("Set VLAN default tag...\n");
    if (ixEthDBPortVlanTagSet(firstPortId, 
			      IX_ETHACC_CODELET_VLANID_DEFAULT)
	!= IX_ETH_DB_SUCCESS)
    {
	printf("SwBridgeQos: Failed to set the default VLAN ID for port %u\n",
	       firstPortId);
	return (IX_FAIL);
    }

    printf("Enable tagged frames...\n");
    if (ixEthDBAcceptableFrameTypeSet(firstPortId,
				      IX_ETH_DB_VLAN_TAGGED_FRAMES
				      | IX_ETH_DB_UNTAGGED_FRAMES)
	!= IX_ETH_DB_SUCCESS)
    {
	printf("SwBridgeQos: Failed to set the acceptable frame type for port %u\n",
	       firstPortId);
	return (IX_FAIL);
    }

    printf("Setting VLAN membership...\n");

    /* by default the entire VLAN range 0-4094 is included in the
       port VLAN membership table, therefore we need to remove all 
       VLAN IDs but 0 (which is required to accept untagged frames) */
    if (ixEthDBPortVlanMembershipRangeRemove(firstPortId, 
        1, IX_ETH_DB_802_1Q_MAX_VLAN_ID) != IX_ETH_DB_SUCCESS)
    {
        printf("SwBridgeQos: Failed to set VLAN membership for port %u\n", firstPortId);
        return (IX_FAIL);
    }

    /* now add the range used by this codelet */
    if (ixEthDBPortVlanMembershipRangeAdd(firstPortId, 
					  IX_ETHACC_CODELET_VLANID_MIN,
					  IX_ETHACC_CODELET_VLANID_MAX)
	!= IX_ETH_DB_SUCCESS)
    {
	printf("SwBridgeQos: Failed to set VLAN membership for port %u\n",
	       firstPortId);
	return (IX_FAIL);
    }

    printf("Enable Egress VLAN tagging...\n");
    if (ixEthDBEgressVlanRangeTaggingEnabledSet(firstPortId, 
						IX_ETHACC_CODELET_VLANID_MIN,
						IX_ETHACC_CODELET_VLANID_MAX,
						TRUE)
	!= IX_ETH_DB_SUCCESS)
    {
	printf("SwBridgeQos: Failed to enable VLAN Egress tagging for port %u\n",
	       firstPortId);
	return (IX_FAIL);
    }

    printf("Enable Ingress VLAN untagging...\n");
    if (ixEthDBIngressVlanTaggingEnabledSet(firstPortId, 
					    IX_ETH_DB_REMOVE_TAG)
	!= IX_ETH_DB_SUCCESS)
    {
	printf("SwBridgeQos: Failed to enable VLAN Ingress untagging for port %u\n",
	       firstPortId);
	return (IX_FAIL);
    }
    
    printf("Setup priority mapping table...\n");
    if (ixEthDBPriorityMappingTableSet(firstPortId,
				       priorityTable)
	!= IX_ETH_DB_SUCCESS)
    {
	printf("SwBridgeQos: Failed to set the priority mapping Table for port %u\n",
	       firstPortId);
	return (IX_FAIL);	
    }
    
    /* Configure 10 mb/s on second port to create a 
     * traffic congestion on the bridge : the high 
     * priority traffic should pass, the low priority
     * traffic should starve.
     */
    if (ixEthAccCodeletLinkSlowSpeedSet(secondPortId)
	!= IX_SUCCESS)
    {
	printf("SwBridgeQos: Failed to set port %u to 10 Mbit\n", 
	       secondPortId);
	return (IX_FAIL);
    }

    /* Allow RX and TX traffic to run */
    if ( ixEthAccPortEnable(firstPortId) != IX_SUCCESS)
    {
	printf("SwBridgeQos: Failed to start the Bridge Operation for port %u\n",
	       firstPortId);
	return (IX_FAIL);
    }

    if ( ixEthAccPortEnable(secondPortId) != IX_SUCCESS)
    {
	printf("SwBridgeQos: Failed to start the Bridge Operation for port %u\n",
	       secondPortId);
	return (IX_FAIL);
    }

    /* display the default settings for both ports */
    printf("Port %u configuration:\n", 
	   (UINT32)firstPortId);
    printf("- Accept Ingress VLAN-tagged traffic, VLAN tag range is [%u-%u]\n",
	   (UINT32)IX_ETHACC_CODELET_VLANID_MIN,
	   (UINT32)IX_ETHACC_CODELET_VLANID_MAX);
    printf("- Strip tag from ingress traffic\n");
    printf("- Bridge Ingress to port %u without tag\n",
	   (UINT32)secondPortId);
    printf("- Frame priorities may change the frame order (QoS enabled)\n");
    printf("- Insert a default tag [%u] to egress traffic\n",
	   (UINT32)IX_ETHACC_CODELET_VLANID_DEFAULT);
 
    printf("Port %u configuration:\n", 
	   (UINT32)secondPortId);
    printf("- Set as default\n\n");

    return (IX_SUCCESS);
}

/*
 * Function definition: ixEthAccCodeletSwBridgeQoSStop()
 *
 * Unconfigure QoS and Stop bridge datapath
 */
IX_STATUS ixEthAccCodeletSwBridgeQoSStop(IxEthAccPortId firstPortId, 
					 IxEthAccPortId secondPortId)
{

    /* Stop the receive and transmit traffic */
    if ( ixEthAccCodeletPortUnconfigure(firstPortId)
	 != IX_SUCCESS)
    {
	printf("SwBridgeQos: Failed to unconfigure the Bridge Operation for port %u\n",
	       firstPortId);
	return (IX_FAIL);
    }

    if ( ixEthAccCodeletPortUnconfigure(secondPortId)
	 != IX_SUCCESS)
    {
	printf("SwBridgeQos: Failed to unconfigure the Bridge Operation for port %u\n",
	       secondPortId);
	return (IX_FAIL);
    }

    if ( ixEthAccPortDisable(firstPortId)
	 != IX_SUCCESS)
    {
	printf("SwBridgeQos: Failed to stop the Bridge Operation for port %u\n",
	       firstPortId);
	return (IX_FAIL);
    }

    if ( ixEthAccPortDisable(secondPortId)
	 != IX_SUCCESS)
    {
	printf("SwBridgeQos: Failed to stop the Bridge Operation for port %u\n",
	       secondPortId);
	return (IX_FAIL);
    }

    /* Disable Xscale QoS */
    printf("Set Tx Scheduling discipline...\n");
    if (ixEthAccTxSchedulingDisciplineSet(firstPortId, 
					  FIFO_NO_PRIORITY)
	!= IX_ETH_ACC_SUCCESS)
    {
	printf("SwBridgeQos: Failed to set Tx Scheduling discipline port %u\n",
	       (UINT32)firstPortId);
	return (IX_FAIL);
    }
    if (ixEthAccTxSchedulingDisciplineSet(secondPortId, 
					  FIFO_NO_PRIORITY)
	!= IX_ETH_ACC_SUCCESS)
    {
	printf("SwBridgeQos: Failed to set Tx Scheduling discipline port %u\n",
	       (UINT32)secondPortId);
	return (IX_FAIL);
    }

    printf("Set Rx Scheduling discipline...\n");
    if (ixEthAccRxSchedulingDisciplineSet(FIFO_NO_PRIORITY)
	!= IX_ETH_ACC_SUCCESS)
    {
	printf("SwBridgeQos: Failed to set Rx Scheduling discipline\n");
	return (IX_FAIL);
    }

    /* Enable the EDB Port again and disable the VLAN/QoS configuration
     * note that no traffic will pass in this mode. After the restoring the
     * configuration to a known state that will not affect other operation
     * modes the ethDB port will be disabled
     */
    if ((ixEthDBPortEnable(firstPortId)) != IX_ETH_DB_SUCCESS)
    {
        printf("SwBridgeQoS: Cannot enable port %u\n", firstPortId);
        return (IX_FAIL);
    }
    if ((ixEthDBPortEnable(secondPortId)) != IX_ETH_DB_SUCCESS)
    {
        printf("SwBridgeQoS: Cannot enable port %u\n", secondPortId);
        return (IX_FAIL);
    }    

    /* Disable NPE QoS and VLAN setup */
    printf("Disable VLAN tagging...\n");
    if (ixEthDBEgressVlanRangeTaggingEnabledSet(firstPortId, 
						IX_ETHACC_CODELET_VLANID_MIN,
						IX_ETHACC_CODELET_VLANID_MAX,
						FALSE)
	!= IX_ETH_DB_SUCCESS)
    {
	printf("SwBridgeQos: Failed to disable VLAN Egress tagging for port %u\n",
	       firstPortId);
	return (IX_FAIL);
    }

    printf("Disable tagged frames...\n");
    if (ixEthDBAcceptableFrameTypeSet(firstPortId,
				      IX_ETH_DB_UNTAGGED_FRAMES)
	!= IX_ETH_DB_SUCCESS)
    {
	printf("SwBridgeQos: Failed to set the acceptable frame type for port %u\n",
	       firstPortId);
	return (IX_FAIL);
    }

    /* Disable the VLAN/QoS Feature to return to the initial processing
     * capabilities
     */
    if ( ixEthDBFeatureEnable((IxEthDBPortId) firstPortId, 
			      IX_ETH_DB_VLAN_QOS, 
			      FALSE) != IX_ETH_DB_SUCCESS )
    {
	printf("SwBridgeQoS: Failed to disable the VLAN/QoS Feature for port %u\n", 
	       firstPortId);
	return (IX_FAIL);
    }

    /* disable the ethDB port */
    if ((ixEthDBPortDisable(firstPortId)) != IX_ETH_DB_SUCCESS)
    {
        printf("SwBridgeQoS: Cannot disable port %u\n", firstPortId);
        return (IX_FAIL);
    }
    if ((ixEthDBPortDisable(secondPortId)) != IX_ETH_DB_SUCCESS)
    {
        printf("SwBridgeQoS: Cannot disable port %u\n", secondPortId);
        return (IX_FAIL);
    }    

    /* Restore the link speed to 100 Mbit */
    if (ixEthAccCodeletLinkDefaultSpeedSet(secondPortId)
	!= IX_SUCCESS)
    {
	printf("SwBridgeQos: Failed to reset port %u to a 100 Mbit port!\n",
	       secondPortId);
	return (IX_FAIL);
    }

    return (IX_SUCCESS);
}
