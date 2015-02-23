/**
 * @file IxHssAccPCM.c
 *
 * @author Intel Corporation
 * @date 10 Jan 2002
 *
 * @brief This file contains the implementation of the private API for the 
 * Packetised Connection Manager.
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

#ifndef IXHSSACCPCM_P_H
#   define IXHSSACCPCM_C
#else
#   error "Error: IxHssAccPCM_p.h should not be included before this definition."
#endif

/**
 * Put the system defined include files required.
 */

/**
 * Put the user defined include files required.
 */
#include "IxOsal.h"
#include "IxHssAcc.h"
#include "IxHssAccPCM_p.h"
#include "IxHssAccError_p.h"
#include "IxHssAccCommon_p.h"
#include "IxHssAccNpeA_p.h"

/**
 * #defines and macros used in this file.
 */
#define IX_HSSACC_PKT_MAX_NUM_FRAME_FLAGS     3
#define IX_HSSACC_PKT_NUM_FRAME_FLAGS_OFFSET  3
#define IX_HSSACC_PKT_HDLC_CRC_TYPE_OFFSET    1
#define IX_HSSACC_PKT_HDLC_BIG_ENDIAN_OFFSET  2
#define IX_HSSACC_PKT_HSS_PORT_OFFSET         16
#define IX_HSSACC_PKT_HDLC_PORT_OFFSET        24
#define IX_HSSACC_PKT_NUM_NPE_REGADDRS        6
#define IX_HSSACC_PKT_NUM_MILLISECS_TO_WAIT   100

#define IX_HSSACC_PKT_HDLC_CRC_32_MASK             0x2
#define IX_HSSACC_PKT_HDLC_CRC_16_MASK             0x0
#define IX_HSSACC_PKT_HDLC_IDLE_ONES_ENABLED_MASK  0x1
#define IX_HSSACC_PKT_HDLC_IDLE_FLAGS_ENABLED_MASK 0x0
#define IX_HSSACC_PKT_HDLC_MSB_ENDIAN_MASK         0x4 
#define IX_HSSACC_PKT_HDLC_LSB_ENDIAN_MASK         0x0
#define IX_HSSACC_PKT_HDLC_56KMODE_CAS_POLARITY0   0
#define IX_HSSACC_PKT_HDLC_56KMODE_CAS_POLARITY1   1
#define IX_HSSACC_PKT_RXFREE_Q_WATERMARK           IX_QMGR_Q_WM_LEVEL4
#define IX_HSSACC_PKT_CRC_32_SIZE_IN_BYTES         4
#define IX_HSSACC_PKT_CRC_16_SIZE_IN_BYTES         2

/*
 * Typedefs global to this file
 */
typedef struct 
{
    BOOL hdlcFraming;                                 
    IxHssAccHdlcMode hdlcMode;
    BOOL hdlcBitInvert;
    unsigned blockSizeInWords;
    UINT32 rawIdleBlockPattern;
    IxHssAccPktHdlcFraming hdlcTxFraming;            
    IxHssAccPktHdlcFraming hdlcRxFraming;            
    IxHssAccPktHdlcIdleType hdlcIdleType; 
    unsigned frmFlagStart;
    IxHssAccPktRxCallback rxCallback;  
    IxHssAccPktRxCallback rxDisconnectingCallback;
    IxHssAccPktRxFreeLowCallback rxFreeLowCallback;  
    IxHssAccPktTxDoneCallback txDoneCallback;
    IxHssAccPktTxDoneCallback txDoneDisconnectingCallback;
    IxHssAccPktUserId rxUserId;
    IxHssAccPktUserId txDoneUserId;
    IxHssAccPktUserId disconnectingTxDoneUserId;
    IxHssAccPktUserId disconnectingRxUserId;
    IxHssAccPktUserId rxFreeLowUserId;
    BOOL thisIsConnected;
    BOOL thisIsEnabled;
    UINT32 numBuffersInUse;
} IxHssAccPCMInfo;


typedef struct 
{
    unsigned hdlc_rxcfg;
    unsigned hdlc_txcfg;
    unsigned idle_pattern;
    unsigned mode;
    unsigned rx_sizeb;
    unsigned rx_sizew;
} IxHssAccPktNpeRegAddrs;


typedef struct 
{
    unsigned connections;
    unsigned disconnections;
    unsigned rawConnections;
    unsigned hdlcConnections;
    unsigned hdlc64kConnections;
    unsigned hdlc56kConnections;
    unsigned enables;
    unsigned disables;
    unsigned connectionRollbacks;
    unsigned rxCallbackRuns;
    unsigned txDoneCallbackRuns;
    unsigned rxFreeCallbackRuns;
    unsigned rxDummyCallbackRuns;
    unsigned rxFreeDummyCallbackRuns;
    unsigned txDoneDummyCallbackRuns;
} IxHssAccPCMStats;

/**
 * Variable declarations global to this file only. 
 * Externs are followed by static variables.
 */

IxQMgrQId ixHssAccPCMRxFreeQId[IX_HSSACC_HSS_PORT_MAX]
[IX_HSSACC_HDLC_PORT_MAX] = 
{
   {IX_NPE_A_QMQ_HSS0_PKT_RX_FREE0,      
    IX_NPE_A_QMQ_HSS0_PKT_RX_FREE1,      
    IX_NPE_A_QMQ_HSS0_PKT_RX_FREE2,       
    IX_NPE_A_QMQ_HSS0_PKT_RX_FREE3}, 
   {IX_NPE_A_QMQ_HSS1_PKT_RX_FREE0,      
    IX_NPE_A_QMQ_HSS1_PKT_RX_FREE1,      
    IX_NPE_A_QMQ_HSS1_PKT_RX_FREE2,       
    IX_NPE_A_QMQ_HSS1_PKT_RX_FREE3}
};

IxQMgrQId ixHssAccPCMTxQId[IX_HSSACC_HSS_PORT_MAX]
[IX_HSSACC_HDLC_PORT_MAX] = 
{
    {IX_NPE_A_QMQ_HSS0_PKT_TX0,       
     IX_NPE_A_QMQ_HSS0_PKT_TX1,      
     IX_NPE_A_QMQ_HSS0_PKT_TX2,       
     IX_NPE_A_QMQ_HSS0_PKT_TX3}, 
    {IX_NPE_A_QMQ_HSS1_PKT_TX0,       
     IX_NPE_A_QMQ_HSS1_PKT_TX1,      
     IX_NPE_A_QMQ_HSS1_PKT_TX2,       
     IX_NPE_A_QMQ_HSS1_PKT_TX3}
};  

IxQMgrQId ixHssAccPCMRxQId[IX_HSSACC_HSS_PORT_MAX] = 
{    IX_NPE_A_QMQ_HSS0_PKT_RX,       
     IX_NPE_A_QMQ_HSS1_PKT_RX
};

IxQMgrQId ixHssAccPCMTxDoneQId[IX_HSSACC_HSS_PORT_MAX] = 
{    IX_NPE_A_QMQ_HSS0_PKT_TX_DONE,       
     IX_NPE_A_QMQ_HSS1_PKT_TX_DONE
};

static IxHssAccPCMInfo ixHssAccPCMClientInfo[IX_HSSACC_HSS_PORT_MAX]
[IX_HSSACC_HDLC_PORT_MAX];

static IxHssAccPCMStats ixHssAccPCMStats;

/*
 * Static function prototypes
 */
PRIVATE IX_STATUS 
ixHssAccPCMRxCfgCreate (IxHssAccPktHdlcFraming hdlcRxFraming, UINT8 *rxCfg);

PRIVATE IX_STATUS 
ixHssAccPCMTxCfgCreate (IxHssAccPktHdlcFraming hdlcTxFraming,
			unsigned frmFlagStart, UINT8 *cfg);

PRIVATE IX_STATUS
ixHssAccPCMComCfgCreate (IxHssAccPktHdlcFraming hdlcComFraming, UINT8 *cfg);

PRIVATE IX_STATUS
ixHssAccPCMPktPipeModeCreate (IxHssAccHdlcMode hdlcMode, 
			      BOOL hdlcBitInvert, UINT32 *mode,
			      UINT32 *invMask, UINT32 *orMask);

PRIVATE void
ixHssAccPCMClientInfoReset (IxHssAccHssPort hssPortId, IxHssAccHdlcPort hdlcPortId);

PRIVATE IX_STATUS
ixHssAccPCMQFlush (IxQMgrQId readQId, IxQMgrQId writeQId);

PRIVATE void
ixHssAccPCMRxFreeBufLowEmptyCallback (IxHssAccPktUserId rxFreeLowUserId);

PRIVATE void 
ixHssAccPktTxDoneDisconnectCallback (IX_OSAL_MBUF *buffer, 
				     unsigned numHssErrs, 
				     IxHssAccPktStatus pktStatus,
				     IxHssAccPktUserId 
				     txDoneUserId);

PRIVATE void 
ixHssAccPktRxDisconnectCallback (IX_OSAL_MBUF *buffer, 
				 unsigned numHssErrs, 
				 IxHssAccPktStatus pktStatus,
				 IxHssAccPktUserId 
				 rxUserId);

PRIVATE void
ixHssAccPCMTxDoneEmptyCallback (IX_OSAL_MBUF *buffer, 
				unsigned numHssErrs, 
				IxHssAccPktStatus pktStatus,
				IxHssAccPktUserId txDoneUserId);
PRIVATE void
ixHssAccPCMRxEmptyCallback (IX_OSAL_MBUF *buffer, 
			    unsigned numHssErrs, 
			    IxHssAccPktStatus pktStatus,
			    IxHssAccPktUserId rxUserId);


/*
 *Function :ixHssAccPCMConnect
 */
IX_STATUS 
ixHssAccPCMConnect (IxHssAccHssPort hssPortId, 
		    IxHssAccHdlcPort hdlcPortId, 
		    BOOL hdlcFraming,
		    IxHssAccHdlcMode hdlcMode,
		    BOOL hdlcBitInvert,
		    unsigned blockSizeInWords,
		    UINT32 rawIdleBlockPattern,
		    IxHssAccPktHdlcFraming hdlcTxFraming, 
		    IxHssAccPktHdlcFraming hdlcRxFraming, 
		    unsigned frmFlagStart,
		    IxHssAccPktRxCallback rxCallback,
		    IxHssAccPktUserId rxUserId,
		    IxHssAccPktRxFreeLowCallback rxFreeLowCallback,
		    IxHssAccPktUserId rxFreeLowUserId,
		    IxHssAccPktTxDoneCallback txDoneCallback,
		    IxHssAccPktUserId txDoneUserId)

{
    IX_STATUS status = IX_SUCCESS;
    UINT8 rxCfg  = 0;
    UINT8 txCfg  = 0;
    UINT32 data = 0;
    UINT32 mode = 0;
    UINT32 invMask = 0;
    UINT32 orMask = 0;
    unsigned crcSizeInBytes;    
    unsigned msgData;
    IxNpeMhMessage message;
    
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering "
		      "ixHssAccPCMConnect\n");
    

    /* Is this Hss/Hdlc port combination already in use? */
    if (ixHssAccPCMClientInfo[hssPortId][hdlcPortId].thisIsConnected)
    {
	IX_HSSACC_REPORT_ERROR ("ixHssAccPCMConnect:"
				"This HSS/HDLC port combination "
				"already in use\n");
	return IX_FAIL;
    }

    if (hdlcFraming)
    {
	/* if HDLC framing is needed for this HDLC port*/
	status = ixHssAccPCMRxCfgCreate (hdlcRxFraming, &rxCfg);
	if (status == IX_SUCCESS)
	{	
	    status = ixHssAccPCMTxCfgCreate (hdlcTxFraming, 
					     frmFlagStart, &txCfg);
	}

	if (status == IX_SUCCESS)
	{	
	    data = rxCfg << IX_HSSACC_NPE_PKT_RXCFG_OFFSET |
		txCfg << IX_HSSACC_NPE_PKT_TXCFG_OFFSET;

	    /* create the NpeMh message - NPE_A message format */
	    ixHssAccComNpeCmdMsgCreate (
		IX_NPE_A_MSSG_HSS_PKT_PIPE_HDLC_CFG_WRITE, 
		0, hssPortId, hdlcPortId, data, &message);

	    /* send the message */
	    status = ixHssAccComNpeCmdMsgSend (
		message, 
		FALSE, /* no resp expected */
		IX_NPE_A_MSSG_HSS_PKT_PIPE_HDLC_CFG_WRITE);

	}

	mode = IX_HSSACC_NPE_PKT_MODE_HDLC;
    }
    else
    {
	/* Write the idle pattern to the NPE memory address specified */
	ixHssAccComNpeCmdMsgCreate (
	    IX_NPE_A_MSSG_HSS_PKT_PIPE_IDLE_PATTERN_WRITE, 
		0, hssPortId, hdlcPortId, rawIdleBlockPattern, &message);

	    /* send the message */
	    status = ixHssAccComNpeCmdMsgSend (
		message, 
		FALSE, /* no resp expected */
		IX_NPE_A_MSSG_HSS_PKT_PIPE_IDLE_PATTERN_WRITE);

	mode = IX_HSSACC_NPE_PKT_MODE_RAW;
    }

    if (status == IX_SUCCESS)
    {
	/* create packetised pipe mode settings */
	status = ixHssAccPCMPktPipeModeCreate (hdlcMode, hdlcBitInvert,
					       &mode, &invMask, &orMask);
        if (status == IX_SUCCESS)
        {
	    /* create the NpeMh message - NPE_A message format */
	    msgData = mode    << IX_HSSACC_NPE_PKT_MODE_OFFSET    |
	              invMask << IX_HSSACC_NPE_PKT_INVMASK_OFFSET |
	              orMask  << IX_HSSACC_NPE_PKT_ORMASK_OFFSET;
	
	    ixHssAccComNpeCmdMsgCreate (
	        IX_NPE_A_MSSG_HSS_PKT_PIPE_MODE_WRITE, 
	        0, hssPortId, hdlcPortId, msgData, &message);
	
	    /* send the message */
	    status = ixHssAccComNpeCmdMsgSend (
	        message, 
	        FALSE, /* no resp expected */
	        IX_NPE_A_MSSG_HSS_PKT_PIPE_MODE_WRITE);
        }
    }

    if (status == IX_SUCCESS)
    {
	/* Configuring the client max tx/rx block size */
	/* rx_sizeb and rx_sizew are the size in bytes and the same size*/
	/* expressed in words*/
	if (hdlcFraming)
	{
	    /* In the case of HDLC framing the crcSizeInBytes needs to be */
	    /* added to the max packet size the NPE is expecting.         */
	    if (hdlcRxFraming.crcType == IX_HSSACC_PKT_32_BIT_CRC) 
	    {
		crcSizeInBytes = IX_HSSACC_PKT_CRC_32_SIZE_IN_BYTES;
	    }
	    else
	    {
		crcSizeInBytes = IX_HSSACC_PKT_CRC_16_SIZE_IN_BYTES;
	    }
	    data = 
		(((blockSizeInWords * IX_HSSACC_BYTES_PER_WORD) +
		  crcSizeInBytes) << IX_HSSACC_NPE_PKT_RXSIZEB_OFFSET);
	}
	else
	{
	    data = 
		((blockSizeInWords * IX_HSSACC_BYTES_PER_WORD) << 
		 IX_HSSACC_NPE_PKT_RXSIZEB_OFFSET);
	}
	    /* create the NpeMh message - NPE_A message format */
	ixHssAccComNpeCmdMsgCreate (
	    IX_NPE_A_MSSG_HSS_PKT_PIPE_RX_SIZE_WRITE, 
	    0, hssPortId, hdlcPortId, data, &message);
	
	/* send the message */
	status = ixHssAccComNpeCmdMsgSend (
	    message, 
	    FALSE, /* no resp expected */
	    IX_NPE_A_MSSG_HSS_PKT_PIPE_RX_SIZE_WRITE);
    }

    if (status == IX_SUCCESS)
    {
	/* Only setup interrupts and callbacks if an ixHssAccPktRxFreeLowCallback*/
	/* has been supplied */
	if (rxFreeLowCallback != NULL)
	{
	    /* Set the Watermark for the RxFree Q for this client */
	    status = ixQMgrWatermarkSet (
		ixHssAccPCMRxFreeQId[hssPortId][hdlcPortId],
		IX_HSSACC_PKT_RXFREE_Q_WATERMARK/*NE flag*/,
		0/*NF flag*/);

	    if (status != IX_SUCCESS)
	    {
		IX_HSSACC_REPORT_ERROR("ixHssAccPCMConnect:"
				       "ixQMgrWatermarkSet failed for this"
				       " client\n");
		ixHssAccPCMClientInfoReset (hssPortId, hdlcPortId);
		/* Increment the ConnectionRollBacks stat */	
		ixHssAccPCMStats.connectionRollbacks++;
		return IX_FAIL;
	    }
	
	    /* set the client callback before enabling notifications */
	    ixHssAccPCMClientInfo[hssPortId][hdlcPortId].rxFreeLowCallback =
		rxFreeLowCallback; 

	    /* Enable notification for the RxFree Q for this client */
	    status = ixQMgrNotificationEnable (
		ixHssAccPCMRxFreeQId[hssPortId][hdlcPortId],
		IX_QMGR_Q_SOURCE_ID_NE);
	    if (status != IX_SUCCESS)
	    {
		IX_HSSACC_REPORT_ERROR("ixHssAccPCMConnect:"
				       "Call to ixQMgrNotificationEnable"
				       " failed\n");
		ixHssAccPCMClientInfoReset (hssPortId, hdlcPortId);
		ixHssAccPCMStats.connectionRollbacks++; 
		/* Increment the ConnectionRollBacks stat */
		return IX_FAIL;
	    } 
	}
    }    

    if (status == IX_SUCCESS)
    { 
	/* 
	 * Save the values supplied by the client into the 
	 * ixHssAccPCMClientInfo struct for later reference
	 */
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].hdlcFraming = hdlcFraming;
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].hdlcMode = hdlcMode;
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].hdlcBitInvert = hdlcBitInvert;
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].blockSizeInWords = blockSizeInWords;
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].rawIdleBlockPattern = rawIdleBlockPattern;
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].hdlcTxFraming = hdlcTxFraming;
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].hdlcRxFraming = hdlcRxFraming;
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].frmFlagStart = frmFlagStart;
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].rxCallback =
	    rxCallback;    
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].
	    rxDisconnectingCallback = rxCallback;    
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].txDoneCallback =
	    txDoneCallback;
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].
	    txDoneDisconnectingCallback = txDoneCallback;
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].thisIsEnabled = FALSE;
	/* 
	 * rxUserId is a client specified value which gets passed back 
	 * to the client through the clients Rx Callback
	 */
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].rxUserId = rxUserId;
	/* 
	 * disconnectingRxUserId is a copy of the clients specified rxUserId
	 * While disconnecting, rxUserId will be overwritten by the hssAcc to 
	 * pass other info to the rxDisconnectCallback. disconnectingRxUserId
	 * will be the passed back to the client in the rxDisconnectCallback.
	 */  
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].disconnectingRxUserId =
	    rxUserId;
	/* 
	 * txDoneUserId is a client specified value which gets passed back 
	 * to the client through the clients TxDone Callback
	 */
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].txDoneUserId = txDoneUserId;
	/* 
	 * disconnectingTxDoneUserId is a copy of the clients specified txDoneUserId
	 * While disconnecting, txDoneUserId will be overwritten by the hssAcc to 
	 * pass other info to the txDoneDisconnectCallback. disconnectingTxDoneUserId
	 * will be the passed back to the client in the txDoneDisconnectCallback.
	 */
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].disconnectingTxDoneUserId =
	    txDoneUserId;
        /* 
	 * rxFreeLowUserId is a client specified value which gets passed back 
	 * to the client through the clients rxFreeLow Callback
	 */
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].rxFreeLowUserId = rxFreeLowUserId;
	/* Set the IsConnected flag for this Hss/Hdlc Port combination*/
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].thisIsConnected = TRUE;
	ixHssAccPCMStats.connections++;

    /* Set the number of IX_OSAL_BUF buffers in use counter to zero for this client*/
    ixHssAccPCMClientInfo[hssPortId][hdlcPortId].numBuffersInUse = 0;
	
	if (hdlcFraming)
	{
	    ixHssAccPCMStats.hdlcConnections++;
	}
	else
	{
	    ixHssAccPCMStats.rawConnections++;
	}
	
	if (!hdlcMode.hdlc56kMode)
	{
	    ixHssAccPCMStats.hdlc64kConnections++;
	}
	else
	{
	    ixHssAccPCMStats.hdlc56kConnections++;
	}
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting ixHssAccPCMConnect\n");
    return status;
}

/*
 * Function : ixHssAccPCMRxCfgCreate
 */
PRIVATE IX_STATUS 
ixHssAccPCMRxCfgCreate (IxHssAccPktHdlcFraming hdlcRxFraming, UINT8 *rxCfg)
{
    IX_STATUS status = IX_SUCCESS;
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering "
		      "ixHssAccPCMRxCfgCreate\n");
    /* Call the ixHssAccPCMComCfgCreate to create the Rx config register */
    status = ixHssAccPCMComCfgCreate (hdlcRxFraming, rxCfg);
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting "
		      "ixHssAccPCMRxCfgCreate\n");
    return status;
}

/*
 *Function :ixHssAccPCMTxCfgCreate
 */
PRIVATE IX_STATUS 
ixHssAccPCMTxCfgCreate (IxHssAccPktHdlcFraming hdlcTxFraming,
			unsigned frmFlagStart, 
			UINT8 *txCfg)
{
    IX_STATUS status = IX_SUCCESS;
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering "
		      "ixHssAccPCMTxCfgCreate\n");

    /* The Tx config register is the same as the Rx config register */
    /* except for the frmFlagStart parameter, */
    /* so this is set and then ixHssAccPCMComCfgCreate is called */   
    *txCfg |= frmFlagStart << IX_HSSACC_PKT_NUM_FRAME_FLAGS_OFFSET;
    status = ixHssAccPCMComCfgCreate (hdlcTxFraming, txCfg);

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting "
		      "ixHssAccPCMTxCfgCreate\n");
    return status;
}

/*
 *Function :ixHssAccPCMComCfgCreate
 */
PRIVATE IX_STATUS
ixHssAccPCMComCfgCreate (IxHssAccPktHdlcFraming hdlcComFraming, UINT8 *cfg)
{
    IX_STATUS status = IX_SUCCESS;
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering"
		      " ixHssAccPCMComCfgCreate \n");

    /* Firstly, set the Hdlc Idle Type on the register*/
    if (hdlcComFraming.hdlcIdleType == IX_HSSACC_HDLC_IDLE_ONES)
    {
	*cfg |= IX_HSSACC_PKT_HDLC_IDLE_ONES_ENABLED_MASK;
    }
    else if (hdlcComFraming.hdlcIdleType == IX_HSSACC_HDLC_IDLE_FLAGS)
    {
	*cfg |= IX_HSSACC_PKT_HDLC_IDLE_FLAGS_ENABLED_MASK;
    }
    else
    {
	IX_HSSACC_REPORT_ERROR ("ixHssAccPCMComCfgCreate:"
				"hdlcComFraming.hdlcIdleType was invalid\n");
	return IX_HSSACC_PARAM_ERR;
    }

    /* Next, set the Hdlc CRC-Type on the register*/
    if (hdlcComFraming.crcType == IX_HSSACC_PKT_32_BIT_CRC)
    {
	*cfg |= IX_HSSACC_PKT_HDLC_CRC_32_MASK;
    }
    else if (hdlcComFraming.crcType == IX_HSSACC_PKT_16_BIT_CRC)
    {
	*cfg |= IX_HSSACC_PKT_HDLC_CRC_16_MASK;
    }
    else
    {
	IX_HSSACC_REPORT_ERROR("ixHssAccPCMComCfgCreate:"
			       "hdlcComFraming.crcType was invalid\n");
	return IX_HSSACC_PARAM_ERR;
    }
    
    /* Finally, set the Data Endianness on the register*/
    if (hdlcComFraming.dataEndian == IX_HSSACC_MSB_ENDIAN)
    {
	*cfg |= IX_HSSACC_PKT_HDLC_MSB_ENDIAN_MASK;
    }
    else if (hdlcComFraming.dataEndian == IX_HSSACC_LSB_ENDIAN)
    {
	*cfg |= IX_HSSACC_PKT_HDLC_LSB_ENDIAN_MASK;
    }
    else
    {
	IX_HSSACC_REPORT_ERROR("ixHssAccPCMComCfgCreate:"
			       "hdlcComFraming.dataEndian was invalid\n");
	return IX_HSSACC_PARAM_ERR;
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting "
		      "ixHssAccPCMComCfgCreate\n");
    return status;
}

/*
 *Function : ixHssAccPCMPktPipeModeCreate
 */
PRIVATE IX_STATUS
ixHssAccPCMPktPipeModeCreate (IxHssAccHdlcMode hdlcMode, 
			      BOOL hdlcBitInvert,
			      UINT32 *mode,
			      UINT32 *invMask,
			      UINT32 *orMask)
{
    IX_STATUS status = IX_SUCCESS;
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering"
		      " ixHssAccPCMPktPipeModeCreate \n");

    /* if this HDLC port is going to run in 56k mode */
    if (hdlcMode.hdlc56kMode)
    {
	/* Error check the hdlcMode parameter */
        if (IX_HSSACC_ENUM_INVALID (hdlcMode.hdlc56kEndian, IX_HSSACC_56KE_MAX))
        {
      	    /* report the error */
	    IX_HSSACC_REPORT_ERROR ("ixHssAccPCMPktPipeModeCreate: invalid parameter\n");
	    /* return error */
	    status = IX_HSSACC_PARAM_ERR;
    	}
        else
        {
            *mode |= IX_HSSACC_NPE_PKT_MODE_56KMODE;
            
            if (hdlcMode.hdlc56kEndian == IX_HSSACC_56KE_BIT_7_UNUSED)
            {
            	/* if CAS bit is at MSB position */
            	*mode |= IX_HSSACC_NPE_PKT_MODE_56KENDIAN_MSB;

            	if (!hdlcMode.hdlc56kUnusedBitPolarity0)
            	{
            	    /* if the polarity of the unused bit (CAS bit) is 1 */
            	    *orMask = IX_HSSACC_NPE_PKT_ORMASK_MSB_POLARITY1;
		}
	    }
	    else if (hdlcMode.hdlc56kEndian == IX_HSSACC_56KE_BIT_0_UNUSED)
	    {
            	/* if CAS bit is at LSB position */
            	*mode |= IX_HSSACC_NPE_PKT_MODE_56KENDIAN_LSB;	    
            	
            	if (!hdlcMode.hdlc56kUnusedBitPolarity0)
            	{
            	    /* if the polarity of the unused bit (CAS bit) is 1 */
            	    *orMask = IX_HSSACC_NPE_PKT_ORMASK_LSB_POLARITY1;
		}
	    }
        }
    }	

    if (status == IX_SUCCESS)
    {
	/* if bit inversion is required between HDLC and HSS co-processors */
	if (hdlcBitInvert)
	{
	    *invMask = IX_HSSACC_NPE_PKT_INVMASK;
	}
    }
    	
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting "
		      "ixHssAccPCMPktPipeModeCreate\n");
    return status;
}

/*
 *Function :ixHssAccPCMPortEnable
 */
IX_STATUS 
ixHssAccPCMPortEnable (IxHssAccHssPort hssPortId, IxHssAccHdlcPort hdlcPortId)
{
    IX_STATUS status = IX_SUCCESS; 
    IxNpeMhMessage message;
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering ixHssAccPCMPortEnable\n");
    
    /* Ensure that the Hdlc Port is connected but not started */
    if (ixHssAccPCMClientInfo[hssPortId][hdlcPortId].thisIsConnected && 
	!(ixHssAccPCMClientInfo[hssPortId][hdlcPortId].thisIsEnabled))
    {
	/* create the message to enable the NPE flow */
	ixHssAccComNpeCmdMsgCreate (IX_NPE_A_MSSG_HSS_PKT_PIPE_FLOW_ENABLE,
				       0, hssPortId, hdlcPortId, 0, &message);
	/* send the message - don't block for a response */
	/* As the NPE does not send any*/
	status = ixHssAccComNpeCmdMsgSend (message, FALSE, 
					   IX_NPE_A_MSSG_HSS_PKT_PIPE_FLOW_ENABLE); 
	if (status != IX_SUCCESS) 
	{
	    /* report the error */
	    IX_HSSACC_REPORT_ERROR ("ixHssAccPCMPortEnable: NpeMh failed "
				    "to send flow enable\n");
	    return IX_FAIL;
	}

	/* Mark the port enabled if it enabled successfully */
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].thisIsEnabled = TRUE;
	ixHssAccPCMStats.enables++; /* Increment the num of Enables stat */
    }
    else
    {
	IX_HSSACC_REPORT_ERROR ("ixHssAccPCMEnable:"
				"Tried to start a hdlc Port that isnt "
				"connected or is already enabled\n");

	status = IX_FAIL; 
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting ixHssAccPCMPortEnable\n");

    return status;
}

/*
 *Function :ixHssAccPCMPortDisable
 */
IX_STATUS 
ixHssAccPCMPortDisable (IxHssAccHssPort hssPortId, IxHssAccHdlcPort hdlcPortId)
{
    IX_STATUS status = IX_SUCCESS; 
    IxNpeMhMessage message;
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering ixHssAccPCMPortDisable\n");

    /* Ensure that the the Hdlc Port is connected and started */
    if (ixHssAccPCMClientInfo[hssPortId][hdlcPortId].thisIsConnected &&
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].thisIsEnabled)
    {	
        /* create the message to disable the NPE flow */
	ixHssAccComNpeCmdMsgCreate (IX_NPE_A_MSSG_HSS_PKT_PIPE_FLOW_DISABLE,
				       0, hssPortId, hdlcPortId, 0, &message);
	/* send the message - don't block for a response as*/
        /* the NPE doesent send one */
	status = ixHssAccComNpeCmdMsgSend (message, FALSE,
					   IX_NPE_A_MSSG_HSS_PKT_PIPE_FLOW_DISABLE);
	if (status != IX_SUCCESS) 
	{
	    /* report the error */
	    IX_HSSACC_REPORT_ERROR ("ixHssAccPCMDisable:"
				    "NpeMh failed to send flow disable\n");
	    return IX_FAIL;
	}
	/* Mark the port disabled if it disabled successfully */
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].thisIsEnabled = FALSE;
	ixHssAccPCMStats.disables++; /* Increment the num of Disables stat */
    }
    else
    {
	IX_HSSACC_REPORT_ERROR("ixHssAccPCMDisable:"
			       "Tried to stop a hdlc Port that isnt"
			       " connected or isnt started\n");
	status = IX_FAIL;
    }
    
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting ixHssAccPCMPortDisable\n");
    return status;   
}

/*
 *Function :ixHssAccPCMDisconnect
 */
IX_STATUS 
ixHssAccPCMDisconnect (IxHssAccHssPort hssPortId, IxHssAccHdlcPort hdlcPortId)
{
    IX_STATUS status = IX_SUCCESS;
    IxHssAccPktRxFreeLowCallback tmpRxFreeLowCallback;
    UINT32 lockKey;
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering "
		      "ixHssAccPCMDisconnect\n");

    /* Ensure the port is connected */
    if (!ixHssAccPCMClientInfo[hssPortId][hdlcPortId].thisIsConnected)
    {
       IX_HSSACC_REPORT_ERROR("ixHssAccPCMDisconnect:Attempted to disconnect"
			      " a port that was not connected\n");
       return IX_FAIL;
    }
    
    tmpRxFreeLowCallback = ixHssAccPCMClientInfo[hssPortId][hdlcPortId].
	rxFreeLowCallback;
    ixHssAccPCMClientInfo[hssPortId][hdlcPortId].rxFreeLowCallback =
	ixHssAccPCMRxFreeBufLowEmptyCallback;
   
    status = ixHssAccPCMQFlush (ixHssAccPCMRxFreeQId[hssPortId][hdlcPortId],
                                ixHssAccPCMRxQId[hssPortId]);
    if (status != IX_SUCCESS)
    {
	IX_HSSACC_REPORT_ERROR("ixHssAccPCMDisconnect:"
			       "flushing the RxFree Q failed for this "
			       "client\n");
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].rxFreeLowCallback =
	    tmpRxFreeLowCallback;
	return IX_FAIL;
    }
    
    /* Disable Interrupts - if TxDone is called during these instructions */
    /* the txDoneUserId passed back to the client will be incorrect */
    lockKey = ixOsalIrqLock();
    /* setup the new txDoneUserId - combination of hss and hdlc port Ids. These */
    /* will be used in the ixHssAccPktTxDoneDisconnectCallback registered here */
    /* to enable an extra check. This will see if all descriptors have been returned */
    /* for this client. If so, the thisIsConnected state is updated */
    ixHssAccPCMClientInfo[hssPortId][hdlcPortId].txDoneUserId = 
	(hssPortId << IX_HSSACC_PKT_CBID_HSS_OFFSET)
	| (unsigned)hdlcPortId;
    ixHssAccPCMClientInfo[hssPortId][hdlcPortId].txDoneCallback =
	ixHssAccPktTxDoneDisconnectCallback;

    /* modify rx path similar to the txdone while disconnecting */
    ixHssAccPCMClientInfo[hssPortId][hdlcPortId].rxUserId = 
	(hssPortId << IX_HSSACC_PKT_CBID_HSS_OFFSET)
	| (unsigned)hdlcPortId;
    ixHssAccPCMClientInfo[hssPortId][hdlcPortId].rxCallback =
	ixHssAccPktRxDisconnectCallback;
    /* Enable Interrupts again */
    /* ixOsalIrqUnlock(lockKey);*/

    /* if started then stop */
    if (ixHssAccPCMClientInfo[hssPortId][hdlcPortId].thisIsEnabled)
    {
	status = ixHssAccPCMPortDisable (hssPortId, hdlcPortId);
	ixOsalIrqUnlock(lockKey);
	if (status != IX_SUCCESS) 
	{
	    /* report the error */
	    IX_HSSACC_REPORT_ERROR ("ixHssAccPCMDisconnect:"
				    "Unable to disable this port on the NPE\n");

	    /* Disable Interrupts */
	    lockKey = ixOsalIrqLock();
	    /* rollback userIds */
	    ixHssAccPCMClientInfo[hssPortId][hdlcPortId].txDoneUserId = 
		ixHssAccPCMClientInfo[hssPortId][hdlcPortId].disconnectingTxDoneUserId;
	    ixHssAccPCMClientInfo[hssPortId][hdlcPortId].rxUserId = 
		ixHssAccPCMClientInfo[hssPortId][hdlcPortId].disconnectingRxUserId;
	    /* rollback the client callbacks */
	    ixHssAccPCMClientInfo[hssPortId][hdlcPortId].rxFreeLowCallback =
		tmpRxFreeLowCallback;
	    ixHssAccPCMClientInfo[hssPortId][hdlcPortId].txDoneCallback =
		ixHssAccPCMClientInfo[hssPortId][hdlcPortId].txDoneDisconnectingCallback;
	    ixHssAccPCMClientInfo[hssPortId][hdlcPortId].rxCallback =
		ixHssAccPCMClientInfo[hssPortId][hdlcPortId].rxDisconnectingCallback;
	    /* Enable Interrupts again */
	    ixOsalIrqUnlock(lockKey);
	    return IX_FAIL;
	}
    }
    else
    {
      ixOsalIrqUnlock(lockKey);
    }

    status = ixHssAccPCMQFlush (ixHssAccPCMTxQId[hssPortId][hdlcPortId],
                                ixHssAccPCMTxDoneQId[hssPortId]);
    if (status != IX_SUCCESS)
    {
    IX_HSSACC_REPORT_ERROR("ixHssAccPCMDisconnect:"
                   "flushing the Tx Q failed for this "
                   "client - packetised service in an unknown state.\n");
    return IX_FAIL;
    }

    /* Check whether there are any outstanding buffers in use for this client */
    if (ixHssAccPCMClientInfo[hssPortId][hdlcPortId].numBuffersInUse == 0)
    {
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].thisIsConnected = FALSE;
	ixHssAccPCMClientInfoReset (hssPortId, hdlcPortId);
    }
    else
    {
	status = IX_HSSACC_PKT_DISCONNECTING;
    }
    
    ixHssAccPCMStats.disconnections++; 
    /* Increment the num of disconnections for stats */
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting "
		      "ixHssAccPCMDisconnect\n");
    
    return status; 
}

/*
 *Function :ixHssAccPCMIsDisconnectComplete
 */
BOOL 
ixHssAccPCMIsDisconnectComplete (IxHssAccHssPort hssPortId, 
				 IxHssAccHdlcPort hdlcPortId)
{
    return !(ixHssAccPCMClientInfo[hssPortId][hdlcPortId].thisIsConnected);
}

/*
 *Function :ixHssAccPCMClientInfoReset
 */
void 
ixHssAccPCMClientInfoReset (IxHssAccHssPort hssPortId, 
			     IxHssAccHdlcPort hdlcPortId)
{
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccPCMClientInfoReset\n");
    /* ixOsalMemSet the entire ixHssAccPCMClientInfo entry for this client to 0*/
    ixOsalMemSet (&(ixHssAccPCMClientInfo[hssPortId][hdlcPortId]), 
		  0, /*value to fill the struct with*/
		  sizeof (IxHssAccPCMInfo));
    ixHssAccPCMClientInfo[hssPortId][hdlcPortId].
	rxCallback = ixHssAccPCMRxEmptyCallback;
    ixHssAccPCMClientInfo[hssPortId][hdlcPortId].
	txDoneCallback = ixHssAccPCMTxDoneEmptyCallback;
    ixHssAccPCMClientInfo[hssPortId][hdlcPortId].
      rxFreeLowCallback = ixHssAccPCMRxFreeBufLowEmptyCallback;
	    
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting"
		      " ixHssAccPCMClientInfoReset \n");  
}

/*
 *Function :ixHssAccPCMRxCallbackRun
 */
void 
ixHssAccPCMRxCallbackRun (IxHssAccHssPort hssPortId, 
			  IxHssAccHdlcPort hdlcPortId,
			  IX_OSAL_MBUF *buffer, 
			  unsigned numHssErrs,
			  IxHssAccPktStatus pktStatus,
			  UINT32 packetLength)
{
    /* This function is called from within an ISR */
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering "
					   "ixHssAccPCMRxCallbackRun \n"));
    
    IX_OSAL_MBUF_PKT_LEN(buffer) = packetLength;
    /* Increment the rxCallbackRuns stat */
    ixHssAccPCMStats.rxCallbackRuns++; 
    /* Execute the clients Rx Callback function from the */
    /*ixHssAccPCMClientInfo struct */
    ixHssAccPCMClientInfo[hssPortId][hdlcPortId].
	rxCallback (buffer, numHssErrs, pktStatus,
			   ixHssAccPCMClientInfo[hssPortId][hdlcPortId].
			   rxUserId);

    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting"
					   " ixHssAccPCMCallbackRun\n"));
}


/*
 *Function :ixHssAccPCMTxDoneCallbackRun
 */
void 
ixHssAccPCMTxDoneCallbackRun (IxHssAccHssPort hssPortId,
			      IxHssAccHdlcPort hdlcPortId, 
			      IX_OSAL_MBUF *buffer, unsigned numHssErrs, 
			      IxHssAccPktStatus pktStatus)
{
    /* This function is called from within an ISR */
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering"
					   " ixHssAccPCMTxDoneCallbackRun \n"));
    
    /* Execute the clients TxDone Callback function from the*/
    /* ixHssAccPCMClientInfo struct */
    ixHssAccPCMClientInfo[hssPortId][hdlcPortId].
	txDoneCallback (buffer, numHssErrs, pktStatus, 
			ixHssAccPCMClientInfo[hssPortId][hdlcPortId].
			txDoneUserId);
    /* Increment the txDoneCallbackRuns stat */
    ixHssAccPCMStats.txDoneCallbackRuns++; 
    
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting "
					   "ixHssAccPCMTxDoneCallbackRun\n"));
}


/*
 *Function :ixHssAccPCMRxFreeLowCallbackRun
 */
void 
ixHssAccPCMRxFreeLowCallbackRun (IxHssAccHssPort hssPortId, 
				 IxHssAccHdlcPort hdlcPortId)
{
    /* This function is called from within an ISR */
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering "
					   "ixHssAccPCMFreeLowCallbackRun \n"));
    /* Increment the FreeLowCallbackRuns stat */
    ixHssAccPCMStats.rxFreeCallbackRuns++;  
 
    /* Execute the clients FreeLow Callback function from*/
    /* the ixHssAccPCMClientInfo struct */
    ixHssAccPCMClientInfo[hssPortId][hdlcPortId].rxFreeLowCallback (
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].rxFreeLowUserId);
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting "
					   "ixHssAccPCMRxFreeLowCallbackRun \n"));
}

/*
 * Function :ixHssAccPCMInit
 */
IX_STATUS ixHssAccPCMInit ()
{
    IX_STATUS status = IX_SUCCESS;
    IxHssAccHdlcPort hdlcPortIndex;
    IxHssAccHssPort hssPortIndex;
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering "
		      "ixHssAccPCMInit \n");

    /* Initialise the stats */ 
    ixHssAccPCMStatsInit ();  
   
    /* Set up the common Qs, ie Rx and TxDone... */
    for (hssPortIndex = IX_HSSACC_HSS_PORT_0; 
	 hssPortIndex < IX_HSSACC_HSS_PORT_MAX; 
	 hssPortIndex++)
    {
	for (hdlcPortIndex = IX_HSSACC_HDLC_PORT_0; 
	     hdlcPortIndex < IX_HSSACC_HDLC_PORT_MAX; 
	     hdlcPortIndex++)
	{
	    /*Set the thisIsConnected flag of every Hss/Hdlc port */
            /*combination to be FALSE*/
	    ixHssAccPCMClientInfo[hssPortIndex][hdlcPortIndex].
		thisIsConnected = FALSE;
	    ixHssAccPCMClientInfo[hssPortIndex][hdlcPortIndex].
		thisIsEnabled = FALSE;
	    ixHssAccPCMClientInfo[hssPortIndex][hdlcPortIndex].
		rxCallback = ixHssAccPCMRxEmptyCallback;
	    ixHssAccPCMClientInfo[hssPortIndex][hdlcPortIndex].
		txDoneCallback = ixHssAccPCMTxDoneEmptyCallback;
	    ixHssAccPCMClientInfo[hssPortIndex][hdlcPortIndex].
		rxFreeLowCallback = ixHssAccPCMRxFreeBufLowEmptyCallback;
	}
    }
        
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting "
		      "ixHssAccPCMInit\n");
    return status;
}


/*
 * Function :ixHssAccPCMCheckTxOk
 */
BOOL
ixHssAccPCMCheckTxOk (IxHssAccHssPort hssPortId, IxHssAccHdlcPort hdlcPortId)
{
    /* This function is called from within an ISR */
    BOOL status = FALSE;
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering "
					   "ixHssAccPCMCheckTxOk\n"));
    /* Is this port connected and started, if yes, then ok to Tx so return*/
    /* success */
    if (ixHssAccPCMClientInfo[hssPortId][hdlcPortId].thisIsConnected && 
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].thisIsEnabled)
    {
       status = TRUE;
    }
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting "
		      "ixHssAccPCMCheckTxOk\n");
    return status;
}


/*
 * Function :ixHssAccPCMCheckReplenishOk
 */
BOOL
ixHssAccPCMCheckReplenishOk (IxHssAccHssPort hssPortId, 
			     IxHssAccHdlcPort hdlcPortId)
{
    /* This function may be called from within an ISR */
    BOOL status = FALSE;
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering "
					   "ixHssAccPCMCheckReplenishOk\n"));
    /* Is this port connected, if yes, then ok to replenish so return*/
    /* success */
    if (ixHssAccPCMClientInfo[hssPortId][hdlcPortId].thisIsConnected)
    {
       status = TRUE;
    }
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting "
					   "ixHssAccPCMCheckReplenishOk\n"));
    return status;
}


/*
 * Function :ixHssAccPCMShow
 */
void ixHssAccPCMShow (void)
{  
    IxHssAccHssPort hssPortIndex;
    IxHssAccHdlcPort hdlcPortIndex;
    IxHssAccPCMInfo *pcmInfo;
    BOOL hdlc56kCasMsb = TRUE;

    printf ("\nixHssAccPCMShow:\n");
    printf ("\t   connects: %d ([H]%d,[R]%d)    \t enables: %d\n"
	    "       HDLC64K conn: %d  HDLC56K conn: %d\n",
	    ixHssAccPCMStats.connections,
	    ixHssAccPCMStats.hdlcConnections,
	    ixHssAccPCMStats.rawConnections, 
	    ixHssAccPCMStats.enables,
	    ixHssAccPCMStats.hdlc64kConnections,
	    ixHssAccPCMStats.hdlc56kConnections);
    printf ("\tdisconnects: %d connRollBacks: %d\tdisables: %d\n\n", 
	    ixHssAccPCMStats.disconnections,
	    ixHssAccPCMStats.connectionRollbacks,
	    ixHssAccPCMStats.disables);
    printf ("\t    rxCallbackRuns: %d    rxDummyCallbackRuns : %d\n",
	    ixHssAccPCMStats.rxCallbackRuns,
	    ixHssAccPCMStats.rxDummyCallbackRuns);
    printf ("\ttxDoneCallbackRuns: %d txDoneDummyCallbackRuns: %d\n",
	    ixHssAccPCMStats.txDoneCallbackRuns,
	    ixHssAccPCMStats.txDoneDummyCallbackRuns);
    printf ("\trxFreeCallbackRuns: %d rxFreeDummyCallbackRuns: %d\n\n",
	    ixHssAccPCMStats.rxFreeCallbackRuns,
	    ixHssAccPCMStats.rxFreeDummyCallbackRuns);

    for (hssPortIndex = IX_HSSACC_HSS_PORT_0;
	 hssPortIndex < hssPortMax;
	 hssPortIndex++)
    {
	for (hdlcPortIndex = IX_HSSACC_HDLC_PORT_0;
	     hdlcPortIndex < IX_HSSACC_HDLC_PORT_MAX;
	     hdlcPortIndex++)
	{
	    pcmInfo = &ixHssAccPCMClientInfo[hssPortIndex][hdlcPortIndex];
	    printf ("\tHSS[%d] HDLC[%d]: %s, %s\n"
		    "\t\t\tHDLC%s Mode  - Selected\n",
		    hssPortIndex,
		    hdlcPortIndex,
		    (pcmInfo->thisIsConnected ? "Connected" : "Disconnected"),
		    (pcmInfo->thisIsEnabled ? "Enabled" : "Disabled"),
		    (pcmInfo->hdlcMode.hdlc56kMode ? "56K" : "64K"));
		    
	    if (pcmInfo->hdlcMode.hdlc56kMode)
	    {
	        if (pcmInfo->hdlcMode.hdlc56kEndian == 
	            IX_HSSACC_56KE_BIT_7_UNUSED)
	        {    
	            hdlc56kCasMsb = TRUE;
	        }
	        else if (pcmInfo->hdlcMode.hdlc56kEndian == 
	                 IX_HSSACC_56KE_BIT_0_UNUSED)
	        {
	            hdlc56kCasMsb = FALSE;
	        }
	        printf ("\t\t\t\t      - CAS at %s\n"
			"\t\t\t\t      - CAS Polarity %d\n",
			hdlc56kCasMsb ? "MSB" : "LSB",
			pcmInfo->hdlcMode.hdlc56kUnusedBitPolarity0 ? 
			IX_HSSACC_PKT_HDLC_56KMODE_CAS_POLARITY0 : 
			IX_HSSACC_PKT_HDLC_56KMODE_CAS_POLARITY1);
	    }
            
            printf ("\t\t\tBit Inversion - %s\n",
		    (pcmInfo->hdlcBitInvert ? "Enabled" : "Disabled"));
	}
    }
}

/*
 * Function :ixHssAccPCMStatsInit
 */
void ixHssAccPCMStatsInit (void)
{
    ixHssAccPCMStats.connections          = 0;
    ixHssAccPCMStats.disconnections       = 0;
    ixHssAccPCMStats.rawConnections       = 0;
    ixHssAccPCMStats.hdlcConnections      = 0;
    ixHssAccPCMStats.enables              = 0;
    ixHssAccPCMStats.disables             = 0;
    ixHssAccPCMStats.connectionRollbacks  = 0;
    ixHssAccPCMStats.rxCallbackRuns       = 0;
    ixHssAccPCMStats.txDoneCallbackRuns   = 0;
    ixHssAccPCMStats.rxFreeCallbackRuns   = 0;
    ixHssAccPCMStats.rxDummyCallbackRuns  = 0;
    ixHssAccPCMStats.rxFreeDummyCallbackRuns = 0;
    ixHssAccPCMStats.txDoneDummyCallbackRuns = 0;
}


/*
 * Function :ixHssAccPktRxDisconnectCallback
 * 
 * The following callback replaces the client callback during a packetised
 * disconnect.  Having passed the received data to the client, this
 * function will check to see if all resources have been returned to
 * hssAccPkt before declaring the connection disconnected
 * 
 */
PRIVATE void 
ixHssAccPktRxDisconnectCallback (IX_OSAL_MBUF *buffer, 
				 unsigned numHssErrs, 
				 IxHssAccPktStatus pktStatus,
				 IxHssAccPktUserId 
				 rxUserId)
{
    /* This function is called from within an ISR */
    IxHssAccHdlcPort hdlcPortId = IX_HSSACC_PKT_CBID_HDLC_MASK & 
	(unsigned)rxUserId;
    IxHssAccHssPort hssPortId = (rxUserId >> IX_HSSACC_PKT_CBID_HSS_OFFSET);
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering "
					   "ixHssAccPktRxDisconnectCallback\n"));
    /* execute client registered callback */
    ixHssAccPCMClientInfo[hssPortId][hdlcPortId].
    rxDisconnectingCallback (
        buffer, 
        numHssErrs, 
        pktStatus,
        ixHssAccPCMClientInfo[hssPortId][hdlcPortId].
        disconnectingRxUserId);

    /* Check whether there are any outstanding buffers in use for this client */
    if (ixHssAccPCMClientInfo[hssPortId][hdlcPortId].numBuffersInUse == 0)
    {
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].thisIsConnected = FALSE;
	ixHssAccPCMClientInfoReset (hssPortId, hdlcPortId);
    }
    
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting "
					   "ixHssAccPktRxDisconnectCallback\n"));
}


PRIVATE void 
ixHssAccPktTxDoneDisconnectCallback (IX_OSAL_MBUF *buffer, 
				     unsigned numHssErrs, 
				     IxHssAccPktStatus pktStatus,
				     IxHssAccPktUserId 
				     txDoneUserId)
{
    /* This function is called from within an ISR */
    IxHssAccHdlcPort hdlcPortId = IX_HSSACC_PKT_CBID_HDLC_MASK &
	(unsigned)txDoneUserId;
    IxHssAccHssPort hssPortId = (txDoneUserId >> IX_HSSACC_PKT_CBID_HSS_OFFSET);
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering "
					   "ixHssAccPktTxDoneDisconnectCallback\n"));

    /* execute client registered callback */
    ixHssAccPCMClientInfo[hssPortId][hdlcPortId].
    txDoneDisconnectingCallback (buffer, 
                     numHssErrs, 
                     pktStatus, 
                     ixHssAccPCMClientInfo
                     [hssPortId][hdlcPortId].
                     disconnectingTxDoneUserId);

    /* Check whether the number of IX_OSAL_BUF buffers in use count is equal to zero, if so no more
     * data is outstanding for Tx or Rx for this client
     */ 
    if (ixHssAccPCMClientInfo[hssPortId][hdlcPortId].numBuffersInUse == 0)
    {
	ixHssAccPCMClientInfo[hssPortId][hdlcPortId].thisIsConnected = FALSE;
	ixHssAccPCMClientInfoReset (hssPortId, hdlcPortId);
    }
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting "
					   "ixHssAccPktTxDoneDisconnectCallback\n"));
}

PRIVATE void
ixHssAccPCMRxEmptyCallback (IX_OSAL_MBUF *buffer, 
			    unsigned numHssErrs, 
			    IxHssAccPktStatus pktStatus,
			    IxHssAccPktUserId rxUserId)
{
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "In "
					   "ixHssAccPCMRxEmptyCallback\n"));
    ixHssAccPCMStats.rxDummyCallbackRuns++;
}


PRIVATE void
ixHssAccPCMTxDoneEmptyCallback (IX_OSAL_MBUF *buffer, 
				unsigned numHssErrs, 
				IxHssAccPktStatus pktStatus,
				IxHssAccPktUserId txDoneUserId)
{
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "In "
					   "ixHssAccPCMTxDoneEmptyCallback\n"));    
    ixHssAccPCMStats.txDoneDummyCallbackRuns++;
}

PRIVATE void
ixHssAccPCMRxFreeBufLowEmptyCallback (IxHssAccPktUserId rxFreeLowPktUserId)
{
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "In "
					   "ixHssAccPCMRxFreeBufLowEmptyCallback\n")); 
    ixHssAccPCMStats.rxFreeDummyCallbackRuns++;
    /*Dummy function for Rx Free Low Callback*/  
}

PRIVATE IX_STATUS
ixHssAccPCMQFlush (IxQMgrQId readQId, IxQMgrQId writeQId)
{
    IX_STATUS status;
    IX_STATUS writeStatus = IX_SUCCESS;
    IxHssAccNpeBuffer *pNpeShared = NULL;
    IxHssAccNpeBuffer *pPhyNpeShared = NULL;
    UINT32 pNpeShrQEntry = 0; 
    IX_OSAL_MBUF *pBuffer = NULL;
    IxHssAccHdlcPort hdlcPortId = 0;
    BOOL flushQIdIsReadQ;

    if ((writeQId == IX_NPE_A_QMQ_HSS0_PKT_RX) || (writeQId == IX_NPE_A_QMQ_HSS1_PKT_RX))
    {
       flushQIdIsReadQ = TRUE;
    }
    else 
    {
       flushQIdIsReadQ = FALSE;
    }
    
    IX_HSSACC_TRACE1 (IX_HSSACC_FN_ENTRY_EXIT, 
                      "Entering ixHssAccPCMQFlush for %s\n",
                      flushQIdIsReadQ ? (int) "RxFreeQ flush" : (int) "TxQ flush");
    
    do
    {
        /* Read the queue entry */
        status = ixQMgrQRead (readQId, &pNpeShrQEntry);

        if ((status == IX_SUCCESS) && (pNpeShrQEntry != 0))
        {
            /* 
             * Extract the HDLC port id and the physical address of the NPE shared region of the
             * IX_OSAL_BUF buffer from the queue entry.
             */
            hdlcPortId = (IxHssAccHdlcPort)(pNpeShrQEntry & IX_HSSACC_QM_Q_CHAN_NUM_MASK);
            pPhyNpeShared = (IxHssAccNpeBuffer *)(pNpeShrQEntry & IX_HSSACC_QM_Q_ADDR_MASK);
        
            /*
             * Convert the physical address to virtual address. Get the address of the
             * OS dependant region of the IX_OSAL_BUF buffer from this address.
             */
            pNpeShared = (IxHssAccNpeBuffer *) IX_HSSACC_PKT_MMU_PHY_TO_VIRT(pPhyNpeShared);
            pBuffer = IX_HSSACC_IX_OSAL_MBUF_FROM_IX_NE(pNpeShared);

            /* Invalidate the cache for the IX_OSAL_BUF buffer */
            IX_HSSACC_IX_NE_SHARED_CACHE_INVALIDATE(pBuffer);

            /* Set the status field of the NPE shared region to IX_HSSACC_DISCONNECT_IN_PROGRESS */
            IX_HSSACC_IX_NE_SHARED_STATUS(pBuffer) = (UINT8) IX_HSSACC_DISCONNECT_IN_PROGRESS;

            /* Flush the cache for the IX_OSAL_BUF buffer */
            IX_HSSACC_IX_NE_SHARED_CACHE_FLUSH(pBuffer);

            /* Write the queue entry to the write queue */
            writeStatus = ixQMgrQWrite (writeQId, &pNpeShrQEntry);

            if (writeStatus != IX_SUCCESS)
            {
                    IX_HSSACC_REPORT_ERROR ("ixHssAccPCMQFlush:"
                                "Writing Buffer Pointers to the write "
                                "Q failed - Pool in a "
                                "depleted state; client mbufs lost also\n");
                    /* 
                    * Lets not return here - We still need to completely flush
                    * the readQ queue if possible 
                    */
                    writeStatus = IX_FAIL;
            }

        }
        else if (status == IX_FAIL)
        {
                    IX_HSSACC_REPORT_ERROR ("ixHssAccPCMQFlush:"
                                "Reading an entry from the read "
                                "Q failed while trying to flush it\n");
                    return status;
        } /* end of else if (status == IX_FAIL)*/
    
    } while (status != IX_QMGR_Q_UNDERFLOW);
    IX_HSSACC_TRACE1 (IX_HSSACC_FN_ENTRY_EXIT, 
              "Exiting ixHssAccPCMQFlush for %s\n",
              (flushQIdIsReadQ == TRUE ? 
               (int) "RxFreeQ flush" : (int) "TxQ flush"));


    return writeStatus;
}

IX_STATUS 
ixHssAccPCMnoBuffersInUseCountInc(
    IxHssAccHssPort hssPortId, 
    IxHssAccHdlcPort hdlcPortId, 
    UINT32 count)
{
    UINT32 lockKey;
    lockKey = ixOsalIrqLock();
    
    /* Increase the number of buffers in use count for the specified client. */
    ixHssAccPCMClientInfo[hssPortId][hdlcPortId].numBuffersInUse += count;
    
    /* Release the lock and return success */
    ixOsalIrqUnlock(lockKey);
    return IX_SUCCESS;      

}

IX_STATUS 
ixHssAccPCMnoBuffersInUseCountDec(
    IxHssAccHssPort hssPortId,
    IxHssAccHdlcPort hdlcPortId,
    UINT32 count)
{
    UINT32 lockKey;
    lockKey = ixOsalIrqLock();
    
    if((INT32)(ixHssAccPCMClientInfo[hssPortId][hdlcPortId].numBuffersInUse - count) < 0)
    {
        /*
         * If the count is greater than the number of buffers in use 
         * for this client, Release the lock and return IX_FAIL 
         */
        ixOsalIrqUnlock(lockKey);
        return IX_FAIL;
    }
    else
    {
        /* Decrement the number of buffers count */
        ixHssAccPCMClientInfo[hssPortId][hdlcPortId].numBuffersInUse -= count;
    }/* end of if-else */

    /* Release the lock and return success */
    ixOsalIrqUnlock(lockKey);
    return IX_SUCCESS;      
    
}
