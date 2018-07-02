/**
 * @file IxHssAccCCM.c
 *
 * @author Intel Corporation
 * @date 30-Jan-02
 *
 * @brief HssAccess Channelised Connection Manager Interface
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

/**
 * Put the system defined include files required.
 */


/**
 * Put the user defined include files required.
 */

#include "IxOsal.h"
#include "IxHssAccError_p.h"
#include "IxQMgr.h"
#include "IxNpeA.h"

#include "IxHssAccCCM_p.h"

#include "IxHssAccCommon_p.h"
#include "IxHssAccNpeA_p.h"

/**
 * #defines and macros used in this file.
 */

#define IX_HSSACC_BYTE_SIZE 8
#define IX_HSSACC_CHAN_GCT_BYTE0_POS 0
#define IX_HSSACC_CHAN_GCT_BYTE1_POS 1
#define IX_HSSACC_CHAN_GCT_BYTE2_POS 2
#define IX_HSSACC_CHAN_GCT_BYTE3_POS 3

/* 
 * the value indicating timeslot switching channel not available 
 */
#define IX_HSSACC_CHAN_TSLOTSWITCH_BYPASS_NOT_AVAIL (0xFFFFFFFF)

/**
 * Typedefs whose scope is limited to this file.
 */

typedef struct
{
    unsigned blk1;
    unsigned blk2;
} IxHssAccCCMTxBlkSize;

typedef struct
{
    unsigned connects;
    unsigned disconnects;
    unsigned enables;
    unsigned disables;
    unsigned rxCallbacks;
    unsigned emptyRxCallbacks;
    unsigned bypassChanEnables;
    unsigned bypassChanDisables;
    unsigned bypassChanGctDl;
} IxHssAccCCMStats;

/**
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */

static IxHssAccCCMParams ixHssAccCCMParams[IX_HSSACC_HSS_PORT_MAX];
static IxQMgrQId ixHssAccCCMQids[IX_HSSACC_HSS_PORT_MAX] = { IX_NPE_A_QMQ_HSS0_CHL_RX_TRIG,
                                                             IX_NPE_A_QMQ_HSS1_CHL_RX_TRIG};
static BOOL ixHssAccCCMConnectedState[IX_HSSACC_HSS_PORT_MAX] = { FALSE, FALSE};
static BOOL ixHssAccCCMEnabledState[IX_HSSACC_HSS_PORT_MAX] = { FALSE, FALSE};

/*
 * The following array holds the blk1/blk2 combinations for various block sizes, as
 * required by the NPE
 */
static IxHssAccCCMTxBlkSize ixHssAccCCMTxBlkSize[] = {
    {  8,  8 }, /* 16 byte blk size */
    { 12,  8 }, /* 20 byte blk size */
    { 12, 12 }, /* 24 byte blk size */
    { 16, 12 }, /* 28 byte blk size */
    { 16, 16 }, /* 32 byte blk size */
    { 20, 16 }, /* 36 byte blk size */
    { 20, 20 }, /* 40 byte blk size */
    { 24, 20 }, /* 44 byte blk size */
    { 24, 24 }  /* 48 byte blk size */
};

static IxHssAccCCMStats ixHssAccCCMStats;

/**
 * Extern function prototypes.
 */

/**
 * Static function prototypes.
 */

PRIVATE void
ixHssAccCCMEmptyRxCallback (IxHssAccHssPort hssPortId,
			    unsigned rxOffset, 
			    unsigned txOffset, 
			    unsigned numHssErrs);

/**
 * Function definition: ixHssAccCCMConnect
 */
IX_STATUS 
ixHssAccCCMConnect (IxHssAccHssPort hssPortId, 
		    unsigned bytesPerTSTrigger, 
		    UINT8 *rxCircular, 
		    unsigned numRxBytesPerTS, 
		    UINT32 *txPtrList, 
		    unsigned numTxPtrLists, 
		    unsigned numTxBytesPerBlk, 
		    IxHssAccChanRxCallback rxCallback)
{
    IX_STATUS status = IX_SUCCESS;
    unsigned index1;
    unsigned txBlk1Bytes;
    unsigned txBlk2Bytes;
    unsigned msgData;
    UINT32 data = 0;
    UINT32 bypassNum;
    IxNpeMhMessage npeMhMsg;
    IxHssAccChanTsSwitchConf *pConfig = &(ixHssAccCCMParams[hssPortId].tsSwitchConf[0]);

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering ixHssAccCCMConnect\n");

    /* check to see if already connected - one connection per client per port */
    if (ixHssAccCCMConnectedState[hssPortId] == TRUE)
    {
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccCCMConnect - already connected\n");
	/* return error */
	status = IX_FAIL;
	
    }

    /* save the parameters for reference */
    ixHssAccCCMParams[hssPortId].bytesPerTSTrigger = bytesPerTSTrigger;
    ixHssAccCCMParams[hssPortId].rxCircular = rxCircular;
    ixHssAccCCMParams[hssPortId].numRxBytesPerTS = numRxBytesPerTS;
    ixHssAccCCMParams[hssPortId].txPtrList = txPtrList;
    ixHssAccCCMParams[hssPortId].numTxPtrLists = numTxPtrLists;
    ixHssAccCCMParams[hssPortId].numTxBytesPerBlk = numTxBytesPerBlk;

    /* initialising timeslot switching channels' configuration */
    for (bypassNum = 0; 
         bypassNum < IX_HSSACC_CHAN_TSLOTSWITCH_NUM_BYPASS_MAX; 
         bypassNum++)
    {
	pConfig[bypassNum].bypassEnabledState = FALSE;
    	pConfig[bypassNum].srcTimeslot = 0;
    	pConfig[bypassNum].destTimeslot = 0;
    }    

    /* ensure tmpRxCallback is not registered to any client callback */
    ixHssAccCCMParams[hssPortId].tmpRxCallback = NULL;

    if (rxCallback != NULL)
    {
	ixHssAccCCMParams[hssPortId].rxCallback = rxCallback;
	if (status == IX_SUCCESS)
	{
	    /* enable QMgr notifications on the specified QMQ */
	    status =  ixQMgrNotificationEnable (ixHssAccCCMQids[hssPortId], 
						IX_QMGR_Q_SOURCE_ID_NOT_E);
	    if (status != IX_SUCCESS)
	    {
		/* report the error */
		IX_HSSACC_REPORT_ERROR_WITH_ARG ("ixHssAccCCMConnect - "
		    "unexpected behaviour on QMQ notification "
		    "enable: return status = %d\n",
		    status,
		    0, 0, 0, 0, 0);
 		/* return error */
		status = IX_FAIL;
	    }
	}
    }	

    /* write the connection configurables to the NPE */
    if (status == IX_SUCCESS)
    {
	/* the rx buffer address */
	ixHssAccComNpeCmdMsgCreate (
	    IX_NPE_A_MSSG_HSS_CHAN_RX_BUF_ADDR_WRITE,
	    0, hssPortId, 0,
	    (UINT32) IX_HSSACC_PKT_MMU_VIRT_TO_PHY(rxCircular),
	    &npeMhMsg);
    
	status = ixHssAccComNpeCmdMsgSend (
	    npeMhMsg, 
	    FALSE, /* no response expected */
	    IX_NPE_A_MSSG_HSS_CHAN_RX_BUF_ADDR_WRITE);
    }

    if (status == IX_SUCCESS)
    {
	/* the rx buffer size and trigger period */
	data = (numRxBytesPerTS << IX_HSSACC_NPE_CHAN_RXSIZEB_OFFSET) | 
	    ((bytesPerTSTrigger/IX_HSSACC_BYTE_SIZE) << IX_HSSACC_NPE_CHAN_TRIG_OFFSET);
	ixHssAccComNpeCmdMsgCreate (
	    IX_NPE_A_MSSG_HSS_CHAN_RX_BUF_CFG_WRITE,
	    0, hssPortId, 0, data, &npeMhMsg);
    
	status = ixHssAccComNpeCmdMsgSend (
	    npeMhMsg, 
	    FALSE, /* no response expected */
	    IX_NPE_A_MSSG_HSS_CHAN_RX_BUF_CFG_WRITE);
    }

    /*
     * The NPE expects the TX Blk size to be written in blk1/blk2 combination
     * values, as specified by the NPE. A global array containing the 
     * blk1/blk2 values is indexed through the numTxBytesPerBlk value
     */
    if (status == IX_SUCCESS)
    {
	index1 = (numTxBytesPerBlk - IX_HSSACC_CHAN_TXBYTES_PER_BLK_MIN) / 
	    IX_HSSACC_CHAN_TXBYTES_PER_BLK_DIV;

	
	txBlk1Bytes = ixHssAccCCMTxBlkSize[index1].blk1;
	txBlk2Bytes = ixHssAccCCMTxBlkSize[index1].blk2;
	data = (txBlk1Bytes << IX_HSSACC_NPE_CHAN_TXBLK1B_OFFSET) |
	    ((txBlk1Bytes/IX_HSSACC_BYTES_PER_WORD) << IX_HSSACC_NPE_CHAN_TXBLK1W_OFFSET) |
	    (txBlk2Bytes << IX_HSSACC_NPE_CHAN_TXBLK2B_OFFSET) |
	    ((txBlk2Bytes/IX_HSSACC_BYTES_PER_WORD) << IX_HSSACC_NPE_CHAN_TXBLK2W_OFFSET);
	ixHssAccComNpeCmdMsgCreate (
	    IX_NPE_A_MSSG_HSS_CHAN_TX_BLK_CFG_WRITE,
	    0, hssPortId, 0, data, &npeMhMsg);
    
	status = ixHssAccComNpeCmdMsgSend (
	    npeMhMsg, 
	    FALSE, /* no response expected */
	    IX_NPE_A_MSSG_HSS_CHAN_TX_BLK_CFG_WRITE);
    }

    if (status == IX_SUCCESS)
    {
	/* the tx ptrList address */
	ixHssAccComNpeCmdMsgCreate (
	    IX_NPE_A_MSSG_HSS_CHAN_TX_BUF_ADDR_WRITE,
	    0, hssPortId, 0,
	    (UINT32) IX_HSSACC_PKT_MMU_VIRT_TO_PHY(txPtrList),
	    &npeMhMsg);
    
	status = ixHssAccComNpeCmdMsgSend (
	    npeMhMsg, 
	    FALSE, /* no response expected */
	    IX_NPE_A_MSSG_HSS_CHAN_TX_BUF_ADDR_WRITE);
    }

    if (status == IX_SUCCESS)
    {
	msgData = numTxPtrLists << IX_HSSACC_NPE_CHAN_TXBUFSIZE_OFFSET;
	/* the tx buf size */
	ixHssAccComNpeCmdMsgCreate (
	    IX_NPE_A_MSSG_HSS_CHAN_TX_BUF_SIZE_WRITE,
	    0, hssPortId, 0, msgData, &npeMhMsg);
    
	status = ixHssAccComNpeCmdMsgSend (
	    npeMhMsg, 
	    FALSE, /* no response expected */
	    IX_NPE_A_MSSG_HSS_CHAN_TX_BUF_SIZE_WRITE);
    }

    if (status == IX_SUCCESS)
    {
	/* note the active connection */
	ixHssAccCCMConnectedState[hssPortId] = TRUE;
	ixHssAccCCMStats.connects++;
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting ixHssAccCCMConnect\n");
    return status;
}

/**
 * Function definition: ixHssAccCCMPortEnable
 */
IX_STATUS 
ixHssAccCCMPortEnable (IxHssAccHssPort hssPortId)
{
    IX_STATUS status = IX_SUCCESS;
    IxNpeMhMessage message;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering ixHssAccCCMPortEnable\n");

    /* check to see if already enabled */
    if (ixHssAccCCMEnabledState[hssPortId] == TRUE)
    {
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccCCMPortEnable - already enabled\n");
	/* return error */
	status = IX_FAIL;
	
    }

    /* check to see if there is a valid connection on the hss port */
    if (ixHssAccCCMConnectedState[hssPortId] == FALSE)
    {
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccCCMPortEnable - no valid connection on specified hssPortId\n");
	/* return error */
	status = IX_FAIL;
	
    }

    /* enable the npe flow */
    if (status == IX_SUCCESS)
    {
	/* 
	 * check if tmpRxCallback contains any client callback. If any,
	 * this means the client is trying to reenable the service for
	 * the second (or third, fourth, ...) time without disconnecting
	 * the service. Hence, use back the client callback that has been
	 * supplied during ixHssAccCCMConnect
	 */
	if (ixHssAccCCMParams[hssPortId].tmpRxCallback != NULL)
	{
	    ixHssAccCCMParams[hssPortId].rxCallback = 
		ixHssAccCCMParams[hssPortId].tmpRxCallback;
	}

	/* create the NpeMh message - NPE_A message format */
	ixHssAccComNpeCmdMsgCreate (IX_NPE_A_MSSG_HSS_CHAN_FLOW_ENABLE, 0, 
				       hssPortId, 0, 0, &message);
	/* send the message */
	status = ixHssAccComNpeCmdMsgSend (message, 
					   FALSE,
					   IX_NPE_A_MSSG_HSS_CHAN_FLOW_ENABLE);

	if (status == IX_SUCCESS)
	{
	    ixHssAccCCMEnabledState[hssPortId] = TRUE;
	    ixHssAccCCMStats.enables++;
	}
	else
	{
	    /* report the error */
	    IX_HSSACC_REPORT_ERROR ("ixHssAccCCMPortEnable - npe flow enable failed\n");
	    /* ensure not passing another components fail return type */
	    status = IX_FAIL;
	}
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting ixHssAccCCMPortEnable\n");
    return status;
}

/**
 * Function definition: ixHssAccCCMPortDisable
 */
IX_STATUS 
ixHssAccCCMPortDisable (IxHssAccHssPort hssPortId)
{
    IX_STATUS status = IX_SUCCESS;
    IxNpeMhMessage message;
    IxHssAccChanRxCallback currentClientCallback = 
	ixHssAccCCMParams[hssPortId].rxCallback;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccCCMPortDisable\n");

    /* check to see if enabled */
    if (ixHssAccCCMEnabledState[hssPortId] == FALSE)
    {
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccCCMPortDisable - not enabled\n");
	/* return error */
	status = IX_FAIL;
	
    }

    /* disable the npe flow */
    if (status == IX_SUCCESS)
    {
	/* 
	 * temporary save client callback just in case client would like to
	 * reenable the service after disabling it for a while
	 */
	ixHssAccCCMParams[hssPortId].tmpRxCallback = 
	    ixHssAccCCMParams[hssPortId].rxCallback;

	/* stop client notifications from the sync QMQ - clear client callback */
	ixHssAccCCMParams[hssPortId].rxCallback = ixHssAccCCMEmptyRxCallback;

	/* create the NpeMh message - NPE_A message format */
	ixHssAccComNpeCmdMsgCreate (IX_NPE_A_MSSG_HSS_CHAN_FLOW_DISABLE, 0, 
				       hssPortId, 0, 0, &message);
	/* send the message */
	status = ixHssAccComNpeCmdMsgSend (message, 
					   FALSE,
					   IX_NPE_A_MSSG_HSS_CHAN_FLOW_DISABLE);

	if (status == IX_SUCCESS)
	{
	    ixHssAccCCMEnabledState[hssPortId] = FALSE;
	    ixHssAccCCMStats.disables++;
	}
	else
	{
	    /* report the error */
	    IX_HSSACC_REPORT_ERROR ("ixHssAccCCMPortDisable - npe flow disable failed\n");
	    /* re-assign the client callback */
	    ixHssAccCCMParams[hssPortId].rxCallback = currentClientCallback;
	    status = IX_FAIL;
	}
    }

    /* 
     * Note: QMQ notifications will stay enabled. This will help the QMQ
     * flushing - if an entry gets pushed on the queue during disabling of
     * the npe flow, the entry will need to be read from the queue. The
     * hssAccess trigger callback will stay attached.  The client callback
     * will just not be called. 
     */

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting ixHssAccCCMPortDisable\n");
    return status;
}

/**
 * Function definition: ixHssAccCCMIsPortEnabled
 *
 * NOTE: hssPortId should be validated before calling this function 
 */
BOOL 
ixHssAccCCMIsPortEnabled (IxHssAccHssPort hssPortId)
{
    return ixHssAccCCMEnabledState[hssPortId];
}

/**
 * Function definition: ixHssAccCCMDisconnect
 */
IX_STATUS 
ixHssAccCCMDisconnect (IxHssAccHssPort hssPortId)
{
    IX_STATUS status = IX_SUCCESS;
    IxHssAccChanTsSwitchConf *pConfig = &(ixHssAccCCMParams[hssPortId].tsSwitchConf[0]);
    UINT32 bypassNum;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccCCMDisconnect\n");

    /* check to see if connected */
    if (ixHssAccCCMConnectedState[hssPortId] == FALSE)
    {
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccCCMDisconnect - not connected\n");
	/* return error */
	status = IX_FAIL;
    }

    /* check to see if disabled yet */
    if ((status == IX_SUCCESS) && (ixHssAccCCMEnabledState[hssPortId] == TRUE))
    {
	/* stop the service */
	status = ixHssAccCCMPortDisable (hssPortId);
    }

    /* 
     * check to see if there is any timeslot switching channels enabled. 
     * If there are, disable all of them.
     */
    if (status == IX_SUCCESS)
    {
        for (bypassNum = 0; 
             bypassNum < IX_HSSACC_CHAN_TSLOTSWITCH_NUM_BYPASS_MAX; 
             bypassNum++)
        {
	    /* if timeslot switching channel exists, disable it */
	    if (pConfig[bypassNum].bypassEnabledState == TRUE)
	    {
		status = ixHssAccCCMTslotSwitchDisable (hssPortId, bypassNum);
	    }
	    
	    /* if disable fail, exit the for loop */
	    if (status != IX_SUCCESS)
	    {
	    	break;
	    }
        }
    }

    if (status == IX_SUCCESS)
    {      
	/* 
	 * clear the client connection parameters saved for reference
	 * NOTE: don't want to zero whole structure. Want to leave the
	 * rxCallback set to the dummy callback for debug purposes
	 */
	ixHssAccCCMParams[hssPortId].bytesPerTSTrigger = 0;
	ixHssAccCCMParams[hssPortId].rxCircular = 0;
	ixHssAccCCMParams[hssPortId].numRxBytesPerTS = 0;
	ixHssAccCCMParams[hssPortId].txPtrList = 0;
	ixHssAccCCMParams[hssPortId].numTxPtrLists = 0;
	ixHssAccCCMParams[hssPortId].numTxBytesPerBlk = 0;
	ixHssAccCCMParams[hssPortId].tmpRxCallback = NULL;

	ixHssAccCCMConnectedState[hssPortId] = FALSE;
	ixHssAccCCMStats.disconnects++;
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccCCMDisconnect\n");
    return status;
}

/**
 * Function definition: ixHssAccCCMTslotSwitchEnable
 */
IX_STATUS 
ixHssAccCCMTslotSwitchEnable (IxHssAccHssPort hssPortId,
			      UINT32 sTimeslot, 
	 	 	      UINT32 dTimeslot, 
	 		      UINT32 *tsSwitchHandle)
{
    IX_STATUS status = IX_SUCCESS;
    IxNpeMhMessage message;
    UINT32 bypassNum;
    UINT32 availBypassChan = IX_HSSACC_CHAN_TSLOTSWITCH_BYPASS_NOT_AVAIL;
    UINT32 bypassChanConf = 0;
    UINT32 srcNpeVoiceChanId;
    UINT32 destNpeVoiceChanId;
    IxHssAccChanTsSwitchConf *pConfig = &(ixHssAccCCMParams[hssPortId].tsSwitchConf[0]);

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering ixHssAccCCMTslotSwitchEnable\n");

    /* check to see if there is a valid connection on the hss port */
    if (ixHssAccCCMConnectedState[hssPortId] == FALSE)
    {
	/* report the error */
        IX_HSSACC_TRACE0 (IX_HSSACC_DEBUG, 
			  "ixHssAccCCMTslotSwitchEnable - no valid channelised "
			  "connection on specified hssPortId\n");	
	/* return error */
	status = IX_FAIL;
    }

    if (status == IX_SUCCESS)
    {
	/* scan through each timeslot switching channel associated with pConfig[bypassNum] */
	for (bypassNum = 0; 
	     bypassNum < IX_HSSACC_CHAN_TSLOTSWITCH_NUM_BYPASS_MAX; 
	     bypassNum++)
    	{
	    /* check if the timeslot switching channel has been enabled */
	    if (pConfig[bypassNum].bypassEnabledState == TRUE)
	    {
	    	/* 
	    	 * check whether the timeslot switching channel with same source and
	    	 * destination timeslots already exist 
	    	 */
	    	if ((sTimeslot == pConfig[bypassNum].srcTimeslot) &&
	    	    (dTimeslot == pConfig[bypassNum].destTimeslot))
	    	{
	    	    /* report the error */
		    IX_HSSACC_TRACE0 (IX_HSSACC_DEBUG, 
				      "ixHssAccCCMTslotSwitchEnable - this bypass"
		    		      " channel already existed on specified"
		    		      " hssPortId\n");	
		    /* return error */
		    status = IX_FAIL;
		    break;
		}

	    	/* 
	    	 * check whether the destination timeslot has already been used in
	    	 * other timeslot switching channel 
	    	 */
		if (dTimeslot == pConfig[bypassNum].destTimeslot)
		{
	    	    /* report the error */
		    IX_HSSACC_TRACE0 (IX_HSSACC_DEBUG, 
				      "ixHssAccCCMTslotSwitchEnable - this"
		    		      " destination timeslot has been used"
		    		      " in other timeslot switching channel"
		    		      " on the specified hssPortId\n");	
		    /* return error */
		    status = IX_FAIL;
		    break;
		}
	    }
	    else
	    {
	    	/* 
	    	 * if the timeslot switching channel associated with pConfig[bypassNum] 
	    	 * is available, assign availBypassChan to it
	    	 */
	    	if (availBypassChan == IX_HSSACC_CHAN_TSLOTSWITCH_BYPASS_NOT_AVAIL)
	    	{
	    	    availBypassChan = bypassNum;
	        }
	    }	
    	}
    }

    if (status == IX_SUCCESS)
    {
    	/* if no more timeslot switching channel available */
    	if (availBypassChan == IX_HSSACC_CHAN_TSLOTSWITCH_BYPASS_NOT_AVAIL)
    	{
	    /* report the error */
	    IX_HSSACC_TRACE0 (IX_HSSACC_DEBUG, 
			      "ixHssAccCCMTslotSwitchEnable - only up to 2 bypass"
	    		      " channels are allowed on one HSS Port at any one"
	    		      " time\n");	
	    /* return error */
	    status = IX_FAIL;
    	}
    	else 
    	{
	    /* enable the timeslot switching channel */
	    /* translate both source & destination timeslot Id to NPE voice channel Id */
	    srcNpeVoiceChanId  = ixHssAccComTdmToNpeVoiceChanTranslate (hssPortId, 
	    			    sTimeslot);
	    destNpeVoiceChanId = ixHssAccComTdmToNpeVoiceChanTranslate (hssPortId, 
	    			    dTimeslot);
	    	    
	    /* send both source & destination timeslot Id to NPE via the message handler */
	    bypassChanConf = (srcNpeVoiceChanId << IX_HSSACC_NPE_CHAN_SRCTSLOT_OFFSET) |
	      		     (destNpeVoiceChanId << IX_HSSACC_NPE_CHAN_DESTTSLOT_OFFSET);
    	
	    /* create the NpeMh message - NPE_A message format */
	    ixHssAccComNpeCmdMsgCreate (IX_NPE_A_MSSG_HSS_CHAN_TSLOTSWITCH_ENABLE, 0, 
				        hssPortId, availBypassChan, bypassChanConf, 
				        &message);
	    /* send the message */
	    status = ixHssAccComNpeCmdMsgSend (message, 
		  			       TRUE, /* block for response */
					       IX_NPE_A_MSSG_HSS_CHAN_TSLOTSWITCH_ENABLE);

	    if (status == IX_SUCCESS)
	    {
	    	/* save for reference */
	    	pConfig[availBypassChan].bypassEnabledState = TRUE;
    	    	pConfig[availBypassChan].srcTimeslot = sTimeslot;
    	    	pConfig[availBypassChan].destTimeslot = dTimeslot;
	    	*tsSwitchHandle = availBypassChan;
	    	ixHssAccCCMStats.bypassChanEnables++;
	    }
	    else
	    {
	    	/* report the error */
		IX_HSSACC_TRACE0 (IX_HSSACC_DEBUG, 
				  "ixHssAccCCMTslotSwitchEnable - bypass channel"
	    			  " enable failed\n");	
	    	/* ensure not passing another components fail return type */
	    	status = IX_FAIL;
	    }
    	}
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting ixHssAccCCMTslotSwitchEnable\n");
    return status;
}

/**
 * Function definition: ixHssAccCCMTslotSwitchDisable
 */
IX_STATUS 
ixHssAccCCMTslotSwitchDisable (IxHssAccHssPort hssPortId,
	 		       UINT32 tsSwitchHandle)
{
    IX_STATUS status = IX_SUCCESS;
    IxNpeMhMessage message;
    IxHssAccChanTsSwitchConf *pConfig = &(ixHssAccCCMParams[hssPortId].tsSwitchConf[0]);

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering ixHssAccCCMTslotSwitchDisable\n");

    /* check to see if tsSwitchHandle hooks to a valid bypass channel on the hss port */
    if (pConfig[tsSwitchHandle].bypassEnabledState == FALSE)
    {
	/* report the error */
	IX_HSSACC_TRACE0 (IX_HSSACC_DEBUG, 
			  "ixHssAccCCMTslotSwitchDisable - this timeslot switching"
			  " channel is not enabled\n");
	/* return error */
	status = IX_FAIL;
    }
    else
    {
	/* create the NpeMh message - NPE_A message format */
	ixHssAccComNpeCmdMsgCreate (IX_NPE_A_MSSG_HSS_CHAN_TSLOTSWITCH_DISABLE, 0, 
				    hssPortId, tsSwitchHandle, 0, &message);
	/* send the message */
	status = ixHssAccComNpeCmdMsgSend (message, 
					   TRUE, /* block for response */
					   IX_NPE_A_MSSG_HSS_CHAN_TSLOTSWITCH_DISABLE);

	if (status == IX_SUCCESS)
	{
	    /* save for reference */
	    pConfig[tsSwitchHandle].bypassEnabledState = FALSE;
	    ixHssAccCCMStats.bypassChanDisables++;
	}
	else
	{
	    /* report the error */
	    IX_HSSACC_TRACE0 (IX_HSSACC_DEBUG, 
			      "ixHssAccCCMTslotSwitchDisable - timeslot switching"
	    		      " channel disable failed\n");
	    /* ensure not passing another components fail return type */
	    status = IX_FAIL;
	}    	
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting ixHssAccCCMTslotSwitchDisable\n");
    return status;
}

/**
 * Function definition: ixHssAccCCMTslotSwitchGctDownload
 */
IX_STATUS 
ixHssAccCCMTslotSwitchGctDownload (IxHssAccHssPort hssPortId,
				   UINT8 *gainCtrlTable,
	 		      	   UINT32 tsSwitchHandle)
{
    IX_STATUS status = IX_SUCCESS;
    IxNpeMhMessage message;
    UINT32 gctEntryWord;
    UINT32 index1;
    IxHssAccChanTsSwitchConf *pConfig = &(ixHssAccCCMParams[hssPortId].tsSwitchConf[0]);

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering ixHssAccCCMTslotSwitchGctDownload\n");

    /* check to see if tsSwitchHandle hooks to a valid bypass channel on the hss port */
    if (pConfig[tsSwitchHandle].bypassEnabledState == FALSE)
    {
	/* report the error */
	IX_HSSACC_TRACE0 (IX_HSSACC_DEBUG, 
			  "ixHssAccCCMTslotSwitchGctDownload - this timeslot switching"
			  " channel is not enabled\n");
	/* return error */
	status = IX_FAIL;
    }
    else
    {
    	/* download gain control table to NPE */
    	for (index1 = 0; 
	     (index1 < IX_HSSACC_CHAN_TSLOTSWITCH_NUM_GCT_ENTRY_MAX) && (status == IX_SUCCESS);
	     index1+=4)
    	{
	    /* 
	     * create gain control table word (4 entries/word) from the entries taken from
	     * the table passed by client
	     */
	    gctEntryWord = 
	        (gainCtrlTable[index1 + IX_HSSACC_CHAN_GCT_BYTE0_POS] 
	            << IX_HSSACC_NPE_CHAN_GCT_BYTE0_OFFSET) |
	    	(gainCtrlTable[index1 + IX_HSSACC_CHAN_GCT_BYTE1_POS] 
	    	    << IX_HSSACC_NPE_CHAN_GCT_BYTE1_OFFSET) |
	    	(gainCtrlTable[index1 + IX_HSSACC_CHAN_GCT_BYTE2_POS] 
	    	    << IX_HSSACC_NPE_CHAN_GCT_BYTE2_OFFSET) |
	    	(gainCtrlTable[index1 + IX_HSSACC_CHAN_GCT_BYTE3_POS] 
	    	    << IX_HSSACC_NPE_CHAN_GCT_BYTE3_OFFSET);

	    /* create the NpeMh message - NPE_A message format */
	    ixHssAccComNpeCmdMsgCreate (IX_NPE_A_MSSG_HSS_CHAN_TSLOTSWITCH_GCT_DOWNLOAD, index1, 
				  	hssPortId, tsSwitchHandle, gctEntryWord, &message);
	    /* send the message */
	    status = ixHssAccComNpeCmdMsgSend (message, 
					       TRUE, /* block for response */
					       IX_NPE_A_MSSG_HSS_CHAN_TSLOTSWITCH_GCT_DOWNLOAD);
	}
	
	if (status == IX_SUCCESS)
	{
	    /* save for reference */
	    pConfig[tsSwitchHandle].gctDownloadedState = TRUE;
	    ixHssAccCCMStats.bypassChanGctDl++;
	}
	else
	{
	    /* report the error */
	    IX_HSSACC_TRACE0 (IX_HSSACC_DEBUG, 
			      "ixHssAccCCMTslotSwitchGctDownload - gain control"
	    		      " table downloading failed\n");
	    /* ensure not passing another components fail return type */
	    status = IX_FAIL;
	}    	
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting ixHssAccCCMTslotSwitchGctDownload\n");
    return status;
}

/**
 * Function definition: ixHssAccCCMRxCallbackRun
 */
void 
ixHssAccCCMRxCallbackRun (IxHssAccHssPort hssPortId, 
			  unsigned rxOffset, 
			  unsigned txOffset, 
			  unsigned numHssErrs)
{
    /* 
     * call the client issued callback. NOTE: no need for error
     * check here. When disconnected a private callback will be
     * registered to gather statistics
     */
    ixHssAccCCMParams[hssPortId].rxCallback (hssPortId,
					     rxOffset, 
					     txOffset, 
					     numHssErrs);
    ixHssAccCCMStats.rxCallbacks++;
}

/**
 * Function definition: ixHssAccCCMEmptyRxCallback
 */
PRIVATE void
ixHssAccCCMEmptyRxCallback (IxHssAccHssPort hssPortId,
			    unsigned rxOffset, 
			    unsigned txOffset, 
			    unsigned numHssErrs)
{
    ixHssAccCCMStats.emptyRxCallbacks++;
}

/**
 * Function definition: ixHssAccCCMQidGet
 */
IxQMgrQId
ixHssAccCCMQidGet (IxHssAccHssPort hssPortId)
{
    /* parameters error checked before use */
    return ixHssAccCCMQids[hssPortId];
}

/**
 * Function definition: ixHssAccCCMShow
 */
void 
ixHssAccCCMShow (void)
{
    UINT32 bypassNum;
    IxHssAccChanTsSwitchConf *pConfig = &(ixHssAccCCMParams[0].tsSwitchConf[0]);
    IxHssAccHssPort hssPortIndex;
    
    printf ("\nixHssAccCCMShow:\n");
    printf ("\t   connects: %d \t enables: %d \t     rxCallbacks: %d\n", 
	    ixHssAccCCMStats.connects,
	    ixHssAccCCMStats.enables,
	    ixHssAccCCMStats.rxCallbacks);
    printf ("\tdisconnects: %d \tdisables: %d \temptyRxCallbacks: %d\n\n", 
	    ixHssAccCCMStats.disconnects,
	    ixHssAccCCMStats.disables,
	    ixHssAccCCMStats.emptyRxCallbacks);

    for (hssPortIndex = IX_HSSACC_HSS_PORT_0; 
         hssPortIndex < hssPortMax; 
         hssPortIndex++)
    {
        printf ("\thssPort[%d]: %s, %s\n", 
		hssPortIndex,
		(ixHssAccCCMConnectedState[hssPortIndex] ? "Connected" : "Disconnected"),
		(ixHssAccCCMEnabledState[hssPortIndex] ? "Enabled" : "Disabled"));

        if (ixHssAccCCMConnectedState[hssPortIndex])
        {
	    printf ("\t\tbytesPerTSTrigger: %d\n", 
		    ixHssAccCCMParams[hssPortIndex].bytesPerTSTrigger);
	    printf ("\t\t       rxCircular: 0x%p\n", 
		    ixHssAccCCMParams[hssPortIndex].rxCircular);
	    printf ("\t\t  numRxBytesPerTS: %d\n", 
		    ixHssAccCCMParams[hssPortIndex].numRxBytesPerTS);
	    printf ("\t\t        txPtrList: 0x%p\n", 
		    ixHssAccCCMParams[hssPortIndex].txPtrList);
	    printf ("\t\t    numTxPtrLists: %d\n", 
		    ixHssAccCCMParams[hssPortIndex].numTxPtrLists);
	    printf ("\t\t numTxBytesPerBlk: %d\n", 
		    ixHssAccCCMParams[hssPortIndex].numTxBytesPerBlk);
	    printf ("\t\t       rxCallback: %p\n", 
		    ixHssAccCCMParams[hssPortIndex].rxCallback);
        }
    }

 
    printf ("\n\nChannelised timeslot switching (for HSS Port 0 only):\n"); 
    printf ("\t    enables: %d \tdisables: %d \t   GCT downloads: %d\n\n", 
	    ixHssAccCCMStats.bypassChanEnables,
	    ixHssAccCCMStats.bypassChanDisables,
	    ixHssAccCCMStats.bypassChanGctDl);

    /* scan through each timeslot switching channel associated with pConfig[bypassNum] */
    for (bypassNum = 0; 
	 bypassNum < IX_HSSACC_CHAN_TSLOTSWITCH_NUM_BYPASS_MAX; 
	 bypassNum++)
    {
        printf ("\tTimeslot switching channel #%d: %s\n",
      	    	bypassNum, 
	    	pConfig[bypassNum].bypassEnabledState ? "Enabled" : "Disabled");
	    	
    	if (pConfig[bypassNum].bypassEnabledState)
    	{
	    printf ("\t\t       srcTimeslot: %d\n", 
		    pConfig[bypassNum].srcTimeslot);
	    printf ("\t\t      destTimeslot: %d\n", 
		    pConfig[bypassNum].destTimeslot);
	    printf ("\t\tgctDownloadedState: %s\n", 
		    pConfig[bypassNum].gctDownloadedState ? 
		    "Downloaded" : "Not downloaded");
    	}
    }
}

/**
 * Function definition: ixHssAccCCMStatsInit
 */
void 
ixHssAccCCMStatsInit (void)
{
    ixHssAccCCMStats.connects = 0;
    ixHssAccCCMStats.disconnects = 0;
    ixHssAccCCMStats.enables = 0;
    ixHssAccCCMStats.disables = 0;
    ixHssAccCCMStats.rxCallbacks = 0;
    ixHssAccCCMStats.emptyRxCallbacks = 0;
    ixHssAccCCMStats.bypassChanEnables = 0;
    ixHssAccCCMStats.bypassChanDisables = 0;
    ixHssAccCCMStats.bypassChanGctDl = 0;
}

/**
 * Function definition: ixHssAccCCMInit
 */
IX_STATUS 
ixHssAccCCMInit (void)
{
    IxHssAccHssPort hssPortIndex;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccCCMInit\n");

    /* initialise the stats */
    ixHssAccCCMStatsInit ();

    for (hssPortIndex = IX_HSSACC_HSS_PORT_0; 
	 hssPortIndex < IX_HSSACC_HSS_PORT_MAX;
	 hssPortIndex++)
    {
	ixHssAccCCMConnectedState[hssPortIndex] = FALSE;
	ixHssAccCCMEnabledState[hssPortIndex] = FALSE;
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccCCMInit\n");
    return IX_SUCCESS;
}
