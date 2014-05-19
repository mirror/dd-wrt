/**
 * @file IxHssAccCommon.c
 *
 * @author Intel Corporation
 * @date 30-Jan-02
 *
 * @brief HssAccess Common Interface
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

#include "IxHssAccError_p.h"
#include "IxNpeA.h"
#include "IxOsal.h"

#include "IxHssAccCommon_p.h"
#include "IxHssAccNpeA_p.h"
#include "IxAccCommon.h"
#include "IxFeatureCtrl.h"

/*
 * Global variables
 */
IxHssAccHssPort hssPortMax = IX_HSSACC_SINGLE_HSS_PORT; /**< Max no. of HSS ports available */

/**
 * #defines and macros used in this file.
 */

#define IX_HSSACC_LUT_TS_PER_WORD     (IX_HSSACC_LUT_BITS_PER_WORD / IX_HSSACC_LUT_BITS_PER_TS)
#define IX_HSSACC_LUT_BYTES_PER_TRUNK 2
#define IX_HSSACC_NUM_PORT_CONFIG_WORDS 22

#define IX_HSSACC_COM_BYTE0_OFFSET 24
#define IX_HSSACC_COM_BYTE1_OFFSET 16
#define IX_HSSACC_COM_BYTE2_OFFSET  8
#define IX_HSSACC_COM_BYTE3_OFFSET  0

#define IX_HSSACC_PKT_PIPES_1 1
#define IX_HSSACC_PKT_PIPES_2 2
#define IX_HSSACC_PKT_PIPES_4 4

/* maximum number of attempts to send messages to NpeMh */
#define IX_HSSACC_NPEMH_SEND_RETRIES 16

/* macros for mutex control for IxNpeMh responses */
#define IX_HSSACC_COM_MUT_UNLOCK() ixOsalMutexUnlock (&ixHssAccComMutex)
#define IX_HSSACC_COM_MUT_LOCK() ixOsalMutexLock (&ixHssAccComMutex, IX_OSAL_WAIT_FOREVER)



/**
 * @def IX_HSSACC_TSLOTUSAGE_GET (HSSPORTID, TSLOTID, TSLOTUSAGE)
 *
 * @brief To extract the timeslot usage info for TSLOTID from HSS LUT words. 
 * There are total 16 HSS LUT Words per HSS port. They are equally shared by 
 * both tx and rx directions (i.e. one direction uses 8 HSS LUT Words). In 
 * order to find which LUT Word does TDM slot Id locate in, we divide the 
 * TDM slot Id with IX_HSSACC_LUT_TS_PER_WORD. In order to locate which bit 
 * position in the LUT Word that TDM slot Id is in, TDM slot Id is modulus 
 * with IX_HSSACC_LUT_TS_PER_WORD. Lastly, we use these Word and Bit 
 * positions that we calculated above to extract the timeslot usage for the 
 * specified TDM timeslot.
 *
 * @note Example of usage: IX_HSSACC_TSLOTUSAGE_GET(hssPortId, 
 * tdmSlot, tslotUsage)
 *
 * @param IxHssAccHssPort HSSPORTID (in) - The HSS port Id. There are two
 * identical ports (0-1). Only port 0 will be supported.
 * @param UINT32 TSLOTID (in) - TDM slot that is configured as
 * channelised timeslot (0-127).
 * @param IxHssAccTdmSlotUsage TSLOTUSAGE (out) - Timeslot usage of the 
 * specified TDM slot.
 *
 * @return 
 *         - IxHssAccTdmSlotUsage Timeslot usage for the specified TDM slot.
 *
 */
#define IX_HSSACC_TSLOTUSAGE_GET(HSSPORTID, TSLOTID, TSLOTUSAGE) \
{ \
    UINT32 lutWordIndex; \
    UINT32 lutTslotIndex; \
    \
    lutWordIndex  = (TSLOTID / IX_HSSACC_LUT_TS_PER_WORD); \
    lutTslotIndex = (TSLOTID % IX_HSSACC_LUT_TS_PER_WORD); \
    TSLOTUSAGE = ((ixHssAccComConfiguration[HSSPORTID].hssTxLUT[lutWordIndex]) >> \
	(lutTslotIndex * IX_HSSACC_LUT_BITS_PER_TS)) & IX_HSSACC_LUT_TS_MASK; \
}

/**
 * Typedefs whose scope is limited to this file.
 */

typedef enum
{
    IX_HSSACC_TX_PCR,
    IX_HSSACC_RX_PCR
} IxHssAccPcrType;

/* IxHssAccComSysClk is used to program the HSS Co-p HSSCLKCR */
typedef struct
{
    unsigned main;
    unsigned num;
    unsigned denom;
} IxHssAccComSysClk;

typedef struct
{
    unsigned validConfigs;
    unsigned invalidConfigs;
    unsigned emptyLastErrCbs;
    unsigned npeCmdSends;
    unsigned npeCmdResps;
    unsigned npeReadResps;
    unsigned npeCmdInvResps;
    unsigned npeReadInvResps;
} IxHssAccComStats;

/*
 * The following structure will store the HSS configuration details as
 * written to the NPE during configuration for reference. The format of the
 * members map directly to that defined by the HSS co-processor. 
 */
typedef struct
{
    UINT32 txPCR;         /* HSSTXPCR - tx port configuration register */
    UINT32 rxPCR;         /* HSSRXRCR - rx port configuration register */
    UINT32 cCR;           /* HSSCCR - core configuration register */
    UINT32 clkCR;         /* HSSCLKCR - clock configuration register */
    UINT32 txFCR;         /* HSSTXFCR - tx frame configuration register */
    UINT32 rxFCR;         /* HSSRXFCR - rx frame configuration register */
    UINT32 hssTxLUT[IX_HSSACC_LUT_WORDS_PER_LUT]; /* tx look-up-table for TDM slot 
						   assignments */
    UINT32 hssRxLUT[IX_HSSACC_LUT_WORDS_PER_LUT]; /* rx look-up-table for TDM slot 
						   assignments */
    unsigned numChannelised; /* The number of clients of the channelised 
				service */
    unsigned numPacketised;  /* The number of clients of the packetised 
				service */
    IxHssAccLastErrorCallback lastErrorCallback;  /* Client callback for 
						     last error presentation */
} IxHssAccComConfiguration;

/**
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */

/* the following mutex is used to block for NpeMh responses */
static IxOsalMutex ixHssAccComMutex;

static IxHssAccComStats ixHssAccComStats;
static IxHssAccComConfiguration ixHssAccComConfiguration[IX_HSSACC_HSS_PORT_MAX];

/* HSS Co-p clock divider for a 133MHz system clk */

/* 
 * We are defining two sets of clock divider values here 
 * - one for 425 Silicon and one for 465 silicon
 */

static IxHssAccComSysClk ixHssAcc42XComSysClk133M[IX_HSSACC_CLK_SPEED_MAX] = {
    { 130,   2,  15  }, /* 512KHz */
    {  43,  18,  47  }, /* 1.536MHz */
    {  43,  33, 192  }, /* 1.544MHz */
    {  32,  34,  63  }, /* 2.048MHz */
    {  16,  34, 127  }, /* 4.096MHz */
    {   8,  34, 255  }  /* 8.192MHz */
};



static IxHssAccComSysClk ixHssAcc46XComSysClk133M[IX_HSSACC_CLK_SPEED_MAX] = {
    { 130,   24,  127 }, /* 512KHz */
    {  43,  152,  383 }, /* 1.536MHz */
    {  43,   66,  385 }, /* 1.544MHz */
    {  32,  280,  511 }, /* 2.048MHz */
    {  16,  280, 1023 }, /* 4.096MHz */
    {   8,  280, 2047 }  /* 8.192MHz */
};

/* HSSCCR HFIFO values in relation to number of packetised clients */
static unsigned ixHssAccComHfifoValues[] = {
    IX_HSSACC_NPE_HFIFO_ONE_BUFFER,   /* one packetised client */
    IX_HSSACC_NPE_HFIFO_TWO_BUFFERS,  /* two packetised clients */
    IX_HSSACC_NPE_HFIFO_FOUR_BUFFERS, /* three packetised clients */
    IX_HSSACC_NPE_HFIFO_FOUR_BUFFERS  /* four packetised clients */
};

/* num pktPipes in relation to number of packetised clients */
static unsigned ixHssAccComPktPipeValues[] = {
    IX_HSSACC_PKT_PIPES_1, /* one packetised client */
    IX_HSSACC_PKT_PIPES_2, /* two packetised clients */
    IX_HSSACC_PKT_PIPES_4, /* three packetised clients */
    IX_HSSACC_PKT_PIPES_4  /* four packetised clients */
};

static unsigned ixHssAccComPipeFifoSizew[][IX_HSSACC_HDLC_PORT_MAX] = {
    { 4, 0, 0, 0 }, /* pipe0, pipe1, pipe2, pipe3 */
    { 2, 2, 0, 0 },
    { 1, 1, 1, 1 }
};


/*
 * This is required for un-init
 */
static BOOL ixHssAccMutexInitialised = FALSE;


/**
 * Extern function prototypes.
 */

/**
 * Static function prototypes.
 */

PRIVATE IX_STATUS 
ixHssAccComPortConfigWrite (IxHssAccHssPort hssPortId, 
			    IxHssAccConfigParams *configParams, 
			    IxHssAccTdmSlotUsage *tdmMap,
			    unsigned *maxPktTrunkInTdmMap);

PRIVATE IX_STATUS 
ixHssAccComPipeInfoWrite (IxHssAccHssPort hssPortId, 
			  unsigned hfifoValue,
			  unsigned packetizedIdlePattern);

PRIVATE IX_STATUS 
ixHssAccComPCRCreate (IxHssAccPcrType type, 
		      IxHssAccPortConfig *txPortConfig, 
		      unsigned *txPCR);

PRIVATE IX_STATUS 
ixHssAccComCCRCreate (unsigned hdlcPortCount, 
		      BOOL loopback, 
		      IxHssAccHssPort hssPortId, 
		      unsigned *ccr);

PRIVATE IX_STATUS 
ixHssAccComClkCRCreate (IxHssAccClkSpeed clkRate, unsigned *clkCR);

PRIVATE IX_STATUS 
ixHssAccComFCRCreate (unsigned offset, unsigned size, unsigned *fcr);

PRIVATE IX_STATUS 
ixHssAccComHssLUTCreate (IxHssAccTdmSlotUsage *tdmMap, 
			 unsigned numHdlcClients,
			 unsigned numChans,
			 IxHssAccClkSpeed clkSpeed,
			 unsigned *pHssLUT,
			 unsigned *maxPktTrunkInTdmMap);

PRIVATE IX_STATUS 
ixHssAccComPortConfigLoad (IxHssAccHssPort hssPortId);

PRIVATE void
ixHssAccComNpeErrorTranslate (unsigned lastNpeError, unsigned *lastError, 
			      unsigned *servicePort);

PRIVATE BOOL
ixHssAccComPCRCreateComParamsInvalid (IxHssAccPortConfig *portConfig);

PRIVATE BOOL 
ixHssAccComPCRCreateTxParamsInvalid (IxHssAccPortConfig *portConfig);

PRIVATE void
ixHssAccComEmptyLastErrorCallback (unsigned lastHssError, 
				   unsigned servicePort);

PRIVATE void
ixHssAccComClkSpeedToNumTrunks (IxHssAccClkSpeed clkSpeed, 
                                unsigned *numTrunks);

void 
ixHssAccComNpeCmdRespCallback (IxNpeMhNpeId npeId, IxNpeMhMessage msg);

void 
ixHssAccComNpeReadRespCallback (IxNpeMhNpeId npeId, IxNpeMhMessage msg);

UINT32 PortConfigFlag = 0;

/**
 * Function definition: ixHssAccComPortInit
 */
IX_STATUS 
ixHssAccComPortInit (IxHssAccHssPort hssPortId, 
		     IxHssAccConfigParams *configParams, 
		     IxHssAccTdmSlotUsage *tdmMap, 
		     IxHssAccLastErrorCallback lastErrorCallback)
{
    IX_STATUS status = IX_SUCCESS;
    unsigned chanIdlePattern;
    unsigned hssPktChannelCount = configParams->hssPktChannelCount;
    unsigned hfifoValue;
    unsigned maxPktTrunkInTdmMap;
    unsigned msgData;
    IxNpeMhMessage npeMhMsg;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering ixHssAccComPortInit\n");

    /* Parameters error checked in the service interface already */

    /* 
     * the following functionality creates HSS Co-p register formats and writes 
     * them to the NPE via the Message Handler. When all have been written, a
     * config cmd is sent.
     */

    if (status == IX_SUCCESS)
    {
	/* create and send the port configuration details */
	status = ixHssAccComPortConfigWrite (hssPortId, configParams, tdmMap,
					     &maxPktTrunkInTdmMap);
    }

    if (status == IX_SUCCESS)
    {
	/* Create the channelised idle pattern (1word) from the byte passed */
	chanIdlePattern = 
	    (configParams->channelisedIdlePattern << IX_HSSACC_COM_BYTE0_OFFSET) |
	    (configParams->channelisedIdlePattern << IX_HSSACC_COM_BYTE1_OFFSET) |
	    (configParams->channelisedIdlePattern << IX_HSSACC_COM_BYTE2_OFFSET) |
	    (configParams->channelisedIdlePattern << IX_HSSACC_COM_BYTE3_OFFSET);

	/* Send the channelised idle pattern to the NPE via the message handler */
	ixHssAccComNpeCmdMsgCreate (IX_NPE_A_MSSG_HSS_CHAN_IDLE_PATTERN_WRITE,
				       0, hssPortId, 0, chanIdlePattern, &npeMhMsg);
				       
	status = ixHssAccComNpeCmdMsgSend (npeMhMsg, 
					   FALSE, /* no response expected */
					   IX_NPE_A_MSSG_HSS_CHAN_IDLE_PATTERN_WRITE);
    }

    if (status == IX_SUCCESS)
    {
	/* Send the num channelised clients to the NPE via the message handler */
	/* save for reference */
	ixHssAccComConfiguration[hssPortId].numChannelised = 
	    configParams->numChannelised;
	msgData = configParams->numChannelised <<
	    IX_HSSACC_NPE_CHAN_NUMCHANS_OFFSET;
	ixHssAccComNpeCmdMsgCreate (IX_NPE_A_MSSG_HSS_CHAN_NUM_CHANS_WRITE,
				    0, hssPortId, 0, 
				    msgData, &npeMhMsg);
				       
	status = ixHssAccComNpeCmdMsgSend (npeMhMsg, 
					   FALSE, /* no response expected */
					   IX_NPE_A_MSSG_HSS_CHAN_NUM_CHANS_WRITE);
    }

    if( ixEthHssAccCoexistEnable != TRUE )
    {

    if (status == IX_SUCCESS)
    {
	/* Send the num packetised clients to the NPE via the message handler */
	/* save for reference */
	ixHssAccComConfiguration[hssPortId].numPacketised = hssPktChannelCount;
	msgData = ixHssAccComPktPipeValues[maxPktTrunkInTdmMap] <<
	    IX_HSSACC_NPE_PKT_NUMPIPES_OFFSET;
	ixHssAccComNpeCmdMsgCreate (IX_NPE_A_MSSG_HSS_PKT_NUM_PIPES_WRITE,
				    0, hssPortId, 0, 
				    msgData, 
				    &npeMhMsg);
				       
	status = ixHssAccComNpeCmdMsgSend (npeMhMsg, 
					   FALSE, /* no response expected */
					   IX_NPE_A_MSSG_HSS_PKT_NUM_PIPES_WRITE);
    }

    /* write the pipe info values to the NPE via the message handler */
    if (status == IX_SUCCESS)
    {
	hfifoValue = ixHssAccComHfifoValues[maxPktTrunkInTdmMap];
	status = ixHssAccComPipeInfoWrite (hssPortId, hfifoValue,
					   configParams->packetizedIdlePattern);
    }

    } /* if ( ixEthHssAccCoexistEnable != TRUE ) */


    if (status == IX_SUCCESS)
    {
	/* inform the NPE that all config params have been loaded */
	status = ixHssAccComPortConfigLoad (hssPortId);
    }

    /* save for reference */
    if (lastErrorCallback != NULL)
    {
	ixHssAccComConfiguration[hssPortId].lastErrorCallback = lastErrorCallback;
    }
    else
    {
	/* ensure the empty callback is called during this configuration */
	ixHssAccComConfiguration[hssPortId].lastErrorCallback = 
	    ixHssAccComEmptyLastErrorCallback;
    }

    if (status == IX_SUCCESS)
    {
	ixHssAccComStats.validConfigs++;
    }
    else
    {
	ixHssAccComStats.invalidConfigs++;
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccComPortInit - configuration failed\n");
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting ixHssAccComPortInit\n");
    return status;
}


/**
 * Function definition: ixHssAccComPortUninit
 */
IX_STATUS
ixHssAccComPortUninit (IxHssAccHssPort hssPortId)
{
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering ixHssAccComPortUninit\n");

    /* ensure the empty callback is called during this configuration */
    ixHssAccComConfiguration[hssPortId].lastErrorCallback = ixHssAccComEmptyLastErrorCallback;


    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting ixHssAccComPortUninit\n");
    return IX_SUCCESS;
}



/**
 * Function definition: ixHssAccComPortConfigWrite
 */
PRIVATE IX_STATUS 
ixHssAccComPortConfigWrite (IxHssAccHssPort hssPortId, 
			    IxHssAccConfigParams *configParams, 
			    IxHssAccTdmSlotUsage *tdmMap,
			    unsigned *maxPktTrunkInTdmMap)
{
    IX_STATUS status = IX_SUCCESS;
    int i;
    unsigned hssPktChannelCount = configParams->hssPktChannelCount;
    unsigned offset = 0;
    unsigned *configWord = (unsigned *) &(ixHssAccComConfiguration[hssPortId]);
    IxNpeMhMessage npeMhMsg;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccComPortConfigWrite\n");

    /* Create TxPCR */
    status = ixHssAccComPCRCreate (
	IX_HSSACC_TX_PCR, 
	&(configParams->txPortConfig), 
	&(ixHssAccComConfiguration[hssPortId].txPCR));
    if (status == IX_SUCCESS)
    {
	/* Create RxPCR */
	status = ixHssAccComPCRCreate (
	    IX_HSSACC_RX_PCR, 
	    &(configParams->rxPortConfig), 
	    &(ixHssAccComConfiguration[hssPortId].rxPCR));
    }

    if (status == IX_SUCCESS)
    {
	/* Create clkCR */
	status = ixHssAccComClkCRCreate (
	    configParams->clkSpeed, 
	    &(ixHssAccComConfiguration[hssPortId].clkCR));
    }

    if (status == IX_SUCCESS)
    {
	/* Create TxFCR */
	status = ixHssAccComFCRCreate (
	    configParams->txPortConfig.frmOffset, 
	    configParams->txPortConfig.maxFrmSize, 				       
	    &(ixHssAccComConfiguration[hssPortId].txFCR));
    }

    if (status == IX_SUCCESS)
    {
	/* Create RxFCR */
	status = ixHssAccComFCRCreate (
	    configParams->rxPortConfig.frmOffset, 
	    configParams->rxPortConfig.maxFrmSize, 				       
	    &(ixHssAccComConfiguration[hssPortId].rxFCR));
    }

    if (status == IX_SUCCESS)
    {
	/* Create HssTxLUT */
	status = ixHssAccComHssLUTCreate (
	    tdmMap, 
	    hssPktChannelCount, 	
	    configParams->numChannelised,
	    configParams->clkSpeed,
	    &(ixHssAccComConfiguration[hssPortId].hssTxLUT[0]),
	    maxPktTrunkInTdmMap);
    }

    if (status == IX_SUCCESS)
    {
	/* HssRxLUT will be the same as the HssTxLUT*/
	ixOsalMemCopy (&(ixHssAccComConfiguration[hssPortId].hssRxLUT[0]),
		       &(ixHssAccComConfiguration[hssPortId].hssTxLUT[0]),
		       sizeof (UINT32) * IX_HSSACC_LUT_WORDS_PER_LUT);
    }

    if (status == IX_SUCCESS)
    {
	/* Create CCR */
	status = ixHssAccComCCRCreate (
	    *maxPktTrunkInTdmMap, 
	    configParams->loopback, hssPortId, 
	    &(ixHssAccComConfiguration[hssPortId].cCR));
    }

    for (i = 0; 
	 (i < IX_HSSACC_NUM_PORT_CONFIG_WORDS) && (status == IX_SUCCESS); 
	 i++)
    {
	ixHssAccComNpeCmdMsgCreate (
	    IX_NPE_A_MSSG_HSS_PORT_CONFIG_WRITE,
	    0, hssPortId, offset, 
	    configWord[i], &npeMhMsg);
	status = ixHssAccComNpeCmdMsgSend (
	    npeMhMsg, 
	    FALSE, /* no response expected */
	    IX_NPE_A_MSSG_HSS_PORT_CONFIG_WRITE);
	offset += sizeof (UINT32);
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccComPortConfigWrite\n");
    return status;
}

/**
 * Function definition: ixHssAccComPipeInfoWrite
 */
PRIVATE IX_STATUS 
ixHssAccComPipeInfoWrite (IxHssAccHssPort hssPortId, 
			  unsigned hfifoValue,
			  unsigned packetizedIdlePattern)
{
    IX_STATUS status = IX_SUCCESS;
    unsigned msgData;
    IxHssAccHdlcPort hdlcPortIndex;
    IxNpeMhMessage npeMhMsg;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccComPipeInfoWrite\n");

    for (hdlcPortIndex = IX_HSSACC_HDLC_PORT_0;
	 (hdlcPortIndex < IX_HSSACC_HDLC_PORT_MAX) && (status == IX_SUCCESS);
	 hdlcPortIndex++)
    {
	msgData = ixHssAccComPipeFifoSizew[hfifoValue][hdlcPortIndex] <<
	    IX_HSSACC_NPE_PKT_FIFOSIZEW_OFFSET;
	ixHssAccComNpeCmdMsgCreate (
	    IX_NPE_A_MSSG_HSS_PKT_PIPE_FIFO_SIZEW_WRITE,
	    0, hssPortId, hdlcPortIndex, 
	    msgData, 
	    &npeMhMsg);
    
	status = ixHssAccComNpeCmdMsgSend (
	    npeMhMsg, 
	    FALSE, /* no response expected */
	    IX_NPE_A_MSSG_HSS_PKT_PIPE_FIFO_SIZEW_WRITE);
	if (status == IX_SUCCESS)
	{
	    ixHssAccComNpeCmdMsgCreate (
		IX_NPE_A_MSSG_HSS_PKT_PIPE_IDLE_PATTERN_WRITE,
		0, hssPortId, hdlcPortIndex, 
		packetizedIdlePattern, 
		&npeMhMsg);
    
	    status = ixHssAccComNpeCmdMsgSend (
		npeMhMsg, 
		FALSE, /* no response expected */
		IX_NPE_A_MSSG_HSS_PKT_PIPE_IDLE_PATTERN_WRITE);
	}
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccComPipeInfoWrite\n");
    return status;
}

/**
 * Function definition: ixHssAccComPCRCreateComParamsInvalid
 */
PRIVATE BOOL 
ixHssAccComPCRCreateComParamsInvalid (IxHssAccPortConfig *portConfig)
{
    BOOL invalid = FALSE;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccComPCRCreateComParamsInvalid\n");

    if (IX_HSSACC_ENUM_INVALID (portConfig->frmSyncType, IX_HSSACC_FRM_SYNC_TYPE_MAX)  ||
	IX_HSSACC_ENUM_INVALID (portConfig->frmSyncIO, IX_HSSACC_FRM_SYNC_ENABLE_MAX)  ||
	portConfig->frmSyncIO == IX_HSSACC_FRM_SYNC_INVALID_VALUE                      ||
	IX_HSSACC_ENUM_INVALID (portConfig->frmSyncClkEdge, IX_HSSACC_CLK_EDGE_MAX)    ||
	IX_HSSACC_ENUM_INVALID (portConfig->dataClkEdge, IX_HSSACC_CLK_EDGE_MAX)       ||
	IX_HSSACC_ENUM_INVALID (portConfig->clkDirection, IX_HSSACC_SYNC_CLK_DIR_MAX)  ||
	IX_HSSACC_ENUM_INVALID (portConfig->frmPulseUsage, IX_HSSACC_FRM_PULSE_MAX)    ||
	IX_HSSACC_ENUM_INVALID (portConfig->dataRate, IX_HSSACC_DATA_RATE_MAX)         ||
	IX_HSSACC_ENUM_INVALID (portConfig->dataPolarity, IX_HSSACC_DATA_POLARITY_MAX) ||
	IX_HSSACC_ENUM_INVALID (portConfig->dataEndianness, IX_HSSACC_ENDIAN_MAX)      ||
	IX_HSSACC_ENUM_INVALID (portConfig->fBitUsage, IX_HSSACC_SOF_MAX)
	)
    {
	invalid = TRUE;
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccComPCRCreateComParamsInvalid\n");
    return invalid;
}

/**
 * Function definition: ixHssAccComPCRCreateTxParamsInvalid
 */
PRIVATE BOOL 
ixHssAccComPCRCreateTxParamsInvalid (IxHssAccPortConfig *portConfig)
{
    BOOL invalid = FALSE;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccComPCRCreateTxParamsInvalid\n");

    if (IX_HSSACC_ENUM_INVALID (portConfig->drainMode, IX_HSSACC_TX_PINS_MAX)    ||
	IX_HSSACC_ENUM_INVALID (portConfig->dataEnable, IX_HSSACC_DE_MAX)        ||
	IX_HSSACC_ENUM_INVALID (portConfig->voice56kType, IX_HSSACC_TXSIG_MAX)   ||
	IX_HSSACC_ENUM_INVALID (portConfig->unassignedType, IX_HSSACC_TXSIG_MAX) ||
	IX_HSSACC_ENUM_INVALID (portConfig->fBitType, IX_HSSACC_FB_MAX)          ||
	IX_HSSACC_ENUM_INVALID (portConfig->voice56kEndian, IX_HSSACC_56KE_MAX)  ||
	IX_HSSACC_ENUM_INVALID (portConfig->voice56kSel, IX_HSSACC_56KS_MAX)
	)
    {
	invalid = TRUE;
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccComPCRCreateTxParamsInvalid\n");
    return invalid;
}

/**
 * Function definition: ixHssAccComPCRCreate
 */
PRIVATE IX_STATUS 
ixHssAccComPCRCreate (IxHssAccPcrType type, IxHssAccPortConfig *portConfig, unsigned *pcr)
{
    IX_STATUS status = IX_SUCCESS;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccComPCRCreate\n");

    /* Error check the parameters */
    /* first those common to tx and rx */
    if (ixHssAccComPCRCreateComParamsInvalid (portConfig))
    {
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccComPCRCreate - invalid parameter\n");
	/* return error */
	status = IX_HSSACC_PARAM_ERR;
    }
    else
    {
	/* create the common parts of the HSS co-p pcr register */
	*pcr = 
	    portConfig->frmSyncType    << IX_HSSACC_COM_HSSPCR_FTYPE_OFFSET   |
	    portConfig->frmSyncIO      << IX_HSSACC_COM_HSSPCR_FENABLE_OFFSET |
	    portConfig->frmSyncClkEdge << IX_HSSACC_COM_HSSPCR_FEDGE_OFFSET   |
	    portConfig->dataClkEdge    << IX_HSSACC_COM_HSSPCR_DEDGE_OFFSET   |
	    portConfig->clkDirection   << IX_HSSACC_COM_HSSPCR_CLKDIR_OFFSET  |
	    portConfig->frmPulseUsage  << IX_HSSACC_COM_HSSPCR_FRAME_OFFSET   |
	    portConfig->dataRate       << IX_HSSACC_COM_HSSPCR_HALF_OFFSET    |
	    portConfig->dataPolarity   << IX_HSSACC_COM_HSSPCR_DPOL_OFFSET    |
	    portConfig->dataEndianness << IX_HSSACC_COM_HSSPCR_BITEND_OFFSET  |
	    portConfig->fBitUsage      << IX_HSSACC_COM_HSSPCR_FBIT_OFFSET;
    }

    if (status == IX_SUCCESS)
    {
	/* check the tx specific parameters */
	if (type == IX_HSSACC_TX_PCR)
	{
	    if (ixHssAccComPCRCreateTxParamsInvalid (portConfig))
	    {
		/* report the error */
		IX_HSSACC_REPORT_ERROR ("ixHssAccComPCRCreate - invalid parameter\n");
		/* return error */
		status = IX_HSSACC_PARAM_ERR;
	    }
	    else
	    {
		/* create the tx specific parts of the HSS co-p pcr register */
		*pcr |= 
		    portConfig->drainMode      << IX_HSSACC_COM_HSSPCR_ODRAIN_OFFSET  |
		    portConfig->dataEnable     << IX_HSSACC_COM_HSSPCR_ENABLE_OFFSET  |
		    portConfig->voice56kType   << IX_HSSACC_COM_HSSPCR_56KTYPE_OFFSET |
		    portConfig->unassignedType << IX_HSSACC_COM_HSSPCR_UTYPE_OFFSET   |
		    portConfig->fBitType       << IX_HSSACC_COM_HSSPCR_FBTYPE_OFFSET  |
		    portConfig->voice56kEndian << IX_HSSACC_COM_HSSPCR_56KEND_OFFSET  |
		    portConfig->voice56kSel    << IX_HSSACC_COM_HSSPCR_56KSEL_OFFSET;
		
		IX_HSSACC_TRACE1 (IX_HSSACC_DEBUG, "txPCR = 0x%X\n", *pcr);
	    }
	}
	else
	{
	    IX_HSSACC_TRACE1 (IX_HSSACC_DEBUG, "rxPCR = 0x%X\n", *pcr);
	}
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccComPCRCreate\n");
    return status;
}

/**
 * Function definition: ixHssAccComCCRCreate
 */
PRIVATE IX_STATUS 
ixHssAccComCCRCreate (unsigned hdlcPortCount, 
		      BOOL loopback, 
		      IxHssAccHssPort hssPortId,
		      unsigned *ccr)
{
    IX_STATUS status = IX_SUCCESS;
    unsigned hfifoValue;
    BOOL regValue;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccComCCRCreate\n");

    hfifoValue = ixHssAccComHfifoValues[hdlcPortCount];
    /* ensure we only write TRUE or FALSE values to register */
    if (loopback) /* could be > 1 */
    {
	regValue = TRUE;
    }
    else
    {
	regValue = FALSE;
    }
    
    *ccr = 
	hfifoValue << IX_HSSACC_COM_HSSCCR_HFIFO_OFFSET |
	regValue   << IX_HSSACC_COM_HSSCCR_LBACK_OFFSET |
	hssPortId  << IX_HSSACC_COM_HSSCCR_COND_OFFSET;
    IX_HSSACC_TRACE1 (IX_HSSACC_DEBUG, "ccr = 0x%X\n", *ccr);

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccComCCRCreate\n");
    return status;
}

/**
 * Function definition: ixHssAccComClkCRCreate
 */
PRIVATE IX_STATUS 
ixHssAccComClkCRCreate (IxHssAccClkSpeed clkRate, unsigned *clkCR)
{
    IX_STATUS status = IX_SUCCESS;
    IxHssAccComSysClk clk;
    IxFeatureCtrlDeviceId deviceId;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccComClkCRCreate\n");

    /* Error check the parameters */
    if (IX_HSSACC_ENUM_INVALID (clkRate, IX_HSSACC_CLK_SPEED_MAX))
    {
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccComClkCRCreate - invalid parameter\n");
	/* return error */
	status = IX_HSSACC_PARAM_ERR;
    }
    else
    {
	/* Check whether to use the clock divider values for 425 or 465 */
	/* IXP43X uses same clock Divider Value of 465 */
        deviceId =  ixFeatureCtrlDeviceRead();
        if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X == deviceId)
	    clk = ixHssAcc42XComSysClk133M[clkRate];
        else if((IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X == deviceId) ||
	        (IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X == deviceId))
	    clk = ixHssAcc46XComSysClk133M[clkRate];
        else
	{
            /* report the error */
            IX_HSSACC_REPORT_ERROR ("ixHssAccComClkCRCreate - invalid device type\n");
            /* return error */
            return IX_HSSACC_PARAM_ERR;
        }
	*clkCR = 
	    clk.main  << IX_HSSACC_COM_HSSCLKCR_MAIN_OFFSET |
	    clk.num   << IX_HSSACC_COM_HSSCLKCR_NUM_OFFSET  |
	    clk.denom << IX_HSSACC_COM_HSSCLKCR_DENOM_OFFSET;
	IX_HSSACC_TRACE1 (IX_HSSACC_DEBUG, "clkCR = 0x%X\n", *clkCR);
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccComClkCRCreate\n");
    return status;
}

/**
 * Function definition: ixHssAccComFCRCreate
 */
PRIVATE IX_STATUS 
ixHssAccComFCRCreate (unsigned offset, unsigned size, unsigned *fcr)
{
    IX_STATUS status = IX_SUCCESS;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccComFCRCreate\n");

    /* Error check the parameters */
    if (offset > IX_HSSACC_COM_HSSFCR_OFFSET_MAX ||
	(size - 1) > IX_HSSACC_COM_HSSFCR_SIZE_MAX) /* frmsize is 1-1024 for client, 0-1023 for hss */
    {
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccComFCRCreate - invalid parameter\n");
	/* return error */
	status = IX_HSSACC_PARAM_ERR;
    }
    else
    {
	/* create the HSS co-p 32bit register format */
	*fcr = 
	    offset << IX_HSSACC_COM_HSSFCR_OFFSET_OFFSET |
	    (size - 1) << IX_HSSACC_COM_HSSFCR_SIZE_OFFSET;

	IX_HSSACC_TRACE1 (IX_HSSACC_DEBUG, "fcr = 0x%X\n", *fcr);
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccComFCRCreate\n");
    return status;
}

/**
 * Function definition: ixHssAccComClkSpeedToNumTrunks
 */
PRIVATE void
ixHssAccComClkSpeedToNumTrunks (IxHssAccClkSpeed clkSpeed, unsigned *numTrunks)
{
    switch (clkSpeed)
    {
	/* the following cases are intentional fall-through */
        case IX_HSSACC_CLK_SPEED_512KHZ:
        case IX_HSSACC_CLK_SPEED_1536KHZ:
        case IX_HSSACC_CLK_SPEED_1544KHZ:
        case IX_HSSACC_CLK_SPEED_2048KHZ:
	    *numTrunks = 1;
	    break;

        case IX_HSSACC_CLK_SPEED_4096KHZ:
	    *numTrunks = 2;
	    break;

        case IX_HSSACC_CLK_SPEED_8192KHZ:
	    *numTrunks = 4;
	    break;

        case IX_HSSACC_CLK_SPEED_MAX:
	    break;
    }
}

/**
 * Function definition: ixHssAccComHssLUTCreate
 */
PRIVATE IX_STATUS 
ixHssAccComHssLUTCreate (IxHssAccTdmSlotUsage *tdmMap,
			 unsigned numHdlcClients, 
			 unsigned numChans,
			 IxHssAccClkSpeed clkSpeed,
			 unsigned *pHssLUT,
			 unsigned *maxPktTrunkInTdmMap)
{
    IX_STATUS status = IX_SUCCESS;
    unsigned trunksWithHldc = 0;
    unsigned numChannelised = 0;
    unsigned tdmMapIndex = 0;
    unsigned numTrunks = 0;
    unsigned trunkIndex;
    IxHssAccTdmSlotUsage mapEntry;
    BOOL hdlcFoundInTrunk[IX_HSSACC_HDLC_PORT_MAX];
    IxHssAccHdlcPort hdlcPortIndex;
    int i;
    int j;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccComHssLUTCreate\n");

    /*
     * The following algorithm creates the LUT format required for loading to the
     * HSS CoProcessor. The tdmMap contains 128 words - each word represents its
     * associated timeslot type. The HSS CoP expects each timeslot to be represented
     * by just 2 bits (4 timeslot types).
     *
     *     words   0      1               15                          127
     *           +------+------+--------+------+---------------------+------+
     *  tdmMap   |      |      |  ...   |      |      ..........     |      |
     *           +------+------+--------+------+---------------------+------+
     *           |                             /
     *           |                           /
     *           |                        /
     *           |                    /
     *           |               /
     *           |           /
     *                   /
     *           +------+------+--------+------+
     *  pHssLUT  |||||||||||||||  ...   ||||||||
     *           +------+------+--------+------+
     *     words   0      1               7
     *
     *
     * Algo:
     *       for each word expected by the hss co-p (pHssLUT)
     *           for every tdmMap entry to be associated with that word (tdmMap)
     *               create the hss co-p word format from each individual entry
     */
    for (hdlcPortIndex = IX_HSSACC_HDLC_PORT_0;
	 hdlcPortIndex < IX_HSSACC_HDLC_PORT_MAX;
	 hdlcPortIndex++)
    {
	/*
	 * a single hdlc client may only occupy timeslots in one
	 * trunk. Need to check how many trunks the client has requested
	 * hdlc for and verify that this is not greater than the number of
	 * hdlc clients - the trunk timeslots are interleaved, as the NPE
	 * only supported byte interleaved mode on the HSS line.
	 */
	hdlcFoundInTrunk[hdlcPortIndex] = FALSE;
    }
    /* 
     * determine the number of trunks - 1=>frame interleave style, >1=> byte
     * interleave style
     */
    ixHssAccComClkSpeedToNumTrunks (clkSpeed, &numTrunks);

    /* initialise the memory to be written to */
    ixOsalMemSet (pHssLUT, 0, IX_HSSACC_LUT_WORDS_PER_LUT * IX_HSSACC_BYTES_PER_WORD);
    *maxPktTrunkInTdmMap = 0;
    for (i = 0; i < IX_HSSACC_LUT_WORDS_PER_LUT; i++)
    {
	for (j = 0; j < IX_HSSACC_LUT_TS_PER_WORD; j++)
	{
	    tdmMapIndex = (i * IX_HSSACC_LUT_TS_PER_WORD) + j;
	    mapEntry = tdmMap[tdmMapIndex];
	    *(pHssLUT + i) |= mapEntry << (j * IX_HSSACC_LUT_BITS_PER_TS);

	    /* check hdlc clients */
	    if (mapEntry == IX_HSSACC_TDMMAP_HDLC)
	    {
		trunkIndex = tdmMapIndex % numTrunks;
		/* take note of hdlc trunk occupancy for error check later */
		if (!hdlcFoundInTrunk[trunkIndex])
		{
		    trunksWithHldc++;
		    /* record the highest trunk used */
		    if (trunkIndex > *maxPktTrunkInTdmMap)
		    {
			*maxPktTrunkInTdmMap = trunkIndex;
		    }
		    hdlcFoundInTrunk[trunkIndex] = TRUE;
		}
	    }

	    /* check voice clients */
	    if ((mapEntry == IX_HSSACC_TDMMAP_VOICE56K) ||
		(mapEntry == IX_HSSACC_TDMMAP_VOICE64K))
	    {
		numChannelised++;
	    }

	} /* for (j) */
	IX_HSSACC_TRACE2 (IX_HSSACC_DEBUG, "LUT[%d] = %X\n", i, *(pHssLUT + i));
    } /* for (i) */

    /* check packetised service */
    if (trunksWithHldc != numHdlcClients)
    {
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccComHssLUTCreate - invalid pkt configuration\n");
	/* return error */
	status = IX_HSSACC_PARAM_ERR;
    }
	
    /* check channelised service */
    if ((numChannelised > IX_HSSACC_MAX_CHAN_TIMESLOTS) ||
	numChannelised != numChans)
    {
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccComHssLUTCreate - invalid chan configuration\n");
	/* return error */
	status = IX_HSSACC_PARAM_ERR;
    }
	
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccComHssLUTCreate\n");

    return status;
}

/**
 * Function definition: ixHssAccComLastHssErrGet
 */
IX_STATUS 
ixHssAccComLastHssErrGet (IxHssAccHssPort hssPortId)
{
    IX_STATUS status;
    IxNpeMhMessage message;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccComLastHssErrGet\n");

    /* create the NpeMh message - NPE_A message format */
    ixHssAccComNpeCmdMsgCreate (IX_NPE_A_MSSG_HSS_PORT_ERROR_READ, 
				   0, hssPortId, 0, 0, &message);

    /* send the message */
    status = ixHssAccComNpeCmdMsgSend (message, FALSE, /* response asychronous */
				       IX_NPE_A_MSSG_HSS_PORT_ERROR_READ);

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccComLastHssErrGet\n");
    return status;
}

/**
 * Function definition: ixHssAccComNpeErrorTranslate
 */
PRIVATE void
ixHssAccComNpeErrorTranslate (unsigned lastNpeError, unsigned *lastError, 
			      unsigned *servicePort)
{
    unsigned hssTxError = 0;
    unsigned swTxError = 0;
    unsigned hssRxError = 0;
    unsigned swRxError = 0;

    /* initialise output params */
    *lastError = IX_HSSACC_NO_ERROR;
    *servicePort = 0;

    /* report first error detected in the lastNpeError register */

    /* check for HSS TX h/w error */
    hssTxError = (lastNpeError & IX_HSSACC_NPE_ERR_HSSTXERR_MASK) >> 
	IX_HSSACC_NPE_ERR_HSSTXERR_OFFSET;
    switch (hssTxError)
    {
        case IX_HSSACC_NPE_HSSTX_NO_ERR1:
        case IX_HSSACC_NPE_HSSTX_NO_ERR2:
	    break;

        case IX_HSSACC_NPE_HSSTX_FRMSYNC_ERR:
	    *lastError = IX_HSSACC_TX_FRM_SYNC_ERR;
	    return;

        case IX_HSSACC_NPE_HSSTX_OVERRUN_ERR:
	    *lastError = IX_HSSACC_TX_OVER_RUN_ERR;
	    *servicePort = (lastNpeError & IX_HSSACC_NPE_ERR_TXBUF_MASK) >>
		IX_HSSACC_NPE_ERR_TXBUF_OFFSET;
	    return;
    }

    /* if no error yet, check for NPE-A TX s/w error */
    swTxError = (lastNpeError & IX_HSSACC_NPE_ERR_SWTXERR_MASK) >> 
	IX_HSSACC_NPE_ERR_SWTXERR_OFFSET;
    switch (swTxError)
    {
        case IX_HSSACC_NPE_SWTX_NO_ERR:
	    break;

        case IX_HSSACC_NPE_SWTX_CHAN_ERR:
	    *lastError = IX_HSSACC_CHANNELISED_SW_TX_ERR;
	    return;

        case IX_HSSACC_NPE_SWTX_PKT_ERR:
	    *lastError = IX_HSSACC_PACKETISED_SW_TX_ERR;
	    *servicePort = (lastNpeError & IX_HSSACC_NPE_ERR_TXBUF_MASK) >>
		IX_HSSACC_NPE_ERR_TXBUF_OFFSET;
	    return;
    }

    /* if no error yet, check for HSS RX h/w error */
    hssRxError = (lastNpeError & IX_HSSACC_NPE_ERR_HSSRXERR_MASK) >> 
	IX_HSSACC_NPE_ERR_HSSRXERR_OFFSET;
    switch (hssRxError)
    {
        case IX_HSSACC_NPE_HSSRX_NO_ERR1:
        case IX_HSSACC_NPE_HSSRX_NO_ERR2:
	    break;

        case IX_HSSACC_NPE_HSSRX_FRMSYNC_ERR:
	    *lastError = IX_HSSACC_RX_FRM_SYNC_ERR;
	    return;

        case IX_HSSACC_NPE_HSSRX_OVERRUN_ERR:
	    *lastError = IX_HSSACC_RX_OVER_RUN_ERR;
	    *servicePort = (lastNpeError & IX_HSSACC_NPE_ERR_RXBUF_MASK) >>
		IX_HSSACC_NPE_ERR_RXBUF_OFFSET;
	    return;
    }

    /* if no error yet, check for NPE-A RX s/w error */
    swRxError = (lastNpeError & IX_HSSACC_NPE_ERR_SWRXERR_MASK) >> 
	IX_HSSACC_NPE_ERR_SWRXERR_OFFSET;
    switch (swRxError)
    {
        case IX_HSSACC_NPE_SWRX_NO_ERR:
	    break;

        case IX_HSSACC_NPE_SWRX_CHAN_ERR:
	    *lastError = IX_HSSACC_CHANNELISED_SW_RX_ERR;
	    return;

        case IX_HSSACC_NPE_SWRX_PKT_ERR:
	    *lastError = IX_HSSACC_PACKETISED_SW_RX_ERR;
	    *servicePort = (lastNpeError & IX_HSSACC_NPE_ERR_RXBUF_MASK) >>
		IX_HSSACC_NPE_ERR_RXBUF_OFFSET;
	    return;
    }
}

/**
 * Function definition: ixHssAccComNpeCmdRespCallback
 */
void 
ixHssAccComNpeCmdRespCallback (IxNpeMhNpeId npeId, IxNpeMhMessage msg)
{
    unsigned cmdType;

    /* this callback services all responses from the IxNpeMh for HssAccess */

    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
					   "Entering ixHssAccComNpeCmdRespCallback\n"));
    cmdType = (msg.data[0] & IX_HSSACC_NPE_CMD_ID_MASK) >>
	IX_HSSACC_NPE_CMD_ID_OFFSET;

    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE2 (
	IX_HSSACC_DEBUG, 
	"ixHssAccComNpeCmdRespCallback: [0] = 0x%X, [1] = 0x%X\n",
	msg.data[0], msg.data[1]));
    ixHssAccComStats.npeCmdResps++;
    switch (cmdType)
    {
        case IX_NPE_A_MSSG_HSS_PORT_CONFIG_LOAD_RESP:
        case IX_NPE_A_MSSG_HSS_CHAN_TSLOTSWITCH_ENABLE_RESP:
        case IX_NPE_A_MSSG_HSS_CHAN_TSLOTSWITCH_DISABLE_RESP:
        case IX_NPE_A_MSSG_HSS_CHAN_TSLOTSWITCH_GCT_DOWNLOAD_RESP:
	    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_DEBUG, "SEM POST\n"));
	    /* post the semphore to release the pending code */ 
            IX_HSSACC_COM_MUT_UNLOCK ();
            
            /* 
             * If ETH - HSS services co-exist then we need to release 
             * the common mutex here 
             */   
             if ( ixEthHssAccCoexistEnable )
             {
                IX_ETH_HSS_COM_MUT_UNLOCK();
             }
	    break;
       
          default:
		ixHssAccComStats.npeCmdInvResps++;
	    break;

    }

IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
					   "Exiting ixHssAccComNpeCmdRespCallback\n"));
}

/**
 * Function definition: ixHssAccComNpeReadRespCallback
 */
void 
ixHssAccComNpeReadRespCallback (IxNpeMhNpeId npeId, IxNpeMhMessage msg)
{
    unsigned cmdType;
    unsigned lastNpeError;
    unsigned errorCount;
    unsigned lastError;
    unsigned servicePort;
    IxHssAccHssPort hssPortId;

    /* this callback services all read responses from the IxNpeMh */

    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
					   "Entering ixHssAccComNpeReadRespCallback\n"));
    cmdType = (msg.data[0] & IX_HSSACC_NPE_CMD_ID_MASK) >>
	IX_HSSACC_NPE_CMD_ID_OFFSET;

    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE2 (
	IX_HSSACC_DEBUG, 
	"ixHssAccComNpeReadRespCallback: [0] = 0x%X, [1] = 0x%X\n",
	msg.data[0], msg.data[1]));
    ixHssAccComStats.npeReadResps++;
    switch (cmdType)
    {
        case IX_NPE_A_MSSG_HSS_PORT_ERROR_READ_RESP:
	    /* process the data read - last hss error */
	    lastNpeError = (msg.data[1] & IX_HSSACC_NPE_LASTERR_MASK) >>
		IX_HSSACC_NPE_LASTERR_OFFSET;
	    errorCount = (msg.data[1] & IX_HSSACC_NPE_ERRCOUNT_MASK) >>
		IX_HSSACC_NPE_ERRCOUNT_OFFSET;
	    /* translate error to def'd value */
	    ixHssAccComNpeErrorTranslate (lastNpeError, &lastError, 
					  &servicePort);
	    hssPortId = (msg.data[0] & IX_HSSACC_NPE_CMD_HSSPORTID_MASK) >> 
		IX_HSSACC_NPE_CMD_HSSPORTID_OFFSET;
	    if (!IX_HSSACC_ENUM_INVALID (hssPortId, hssPortMax))
	    {
		ixHssAccComConfiguration[hssPortId].lastErrorCallback (lastError, servicePort);
	    }
	    else
	    {
		ixHssAccComStats.npeReadInvResps++;
	    }
	    break;

        default:
		ixHssAccComStats.npeReadInvResps++;
	    break;

    }
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
					   "Exiting ixHssAccComNpeReadRespCallback\n"));
}

/**
 * Function definition: ixHssAccComNpeCmdMsgCreate
 */
void 
ixHssAccComNpeCmdMsgCreate (unsigned byte0, unsigned byte1, 
			    unsigned byte2, unsigned byte3,
			    unsigned data,
			    IxNpeMhMessage *pMessage)
{
    /* create the NpeMh message - NPE_A message format */
    pMessage->data[0] = 
	byte3 << IX_HSSACC_COM_BYTE3_OFFSET |
	byte2 << IX_HSSACC_COM_BYTE2_OFFSET |
	byte1 << IX_HSSACC_COM_BYTE1_OFFSET |
	byte0 << IX_HSSACC_COM_BYTE0_OFFSET;
    pMessage->data[1] = data; 
    IX_HSSACC_TRACE2 (IX_HSSACC_DEBUG, 
		      "ixHssAccComNpeCmdMsgCreate: 0x%08X, 0x%08X\n",
		      pMessage->data[0], pMessage->data[1]);
}

/**
 * Function definition: ixHssAccComNpeCmdMsgSend
 */
IX_STATUS 
ixHssAccComNpeCmdMsgSend (IxNpeMhMessage message, 
			  BOOL reqResp, 
			  unsigned npeMsgId)
{
    IX_STATUS status = IX_SUCCESS;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccComNpeCmdMsgSend\n");
    
    ixHssAccComStats.npeCmdSends++;

    /* If ETH-HSS co-exist exists we need to acquire this common mutex
       before sending a message to NPE 
     */
    if ( ixEthHssAccCoexistEnable )
    {
        IX_ETH_HSS_COM_MUT_LOCK(status);
        if ( IX_SUCCESS != status )
        {
	   /* report the error */
	   IX_HSSACC_REPORT_ERROR ("IX_ETH_HSS_COM_MUT_LOCK failed to acquire \n");
	   /* return error */
	   return IX_FAIL;	
        }	    
    }

    /* check if a response is required */
    if (reqResp)
    {
	IX_HSSACC_TRACE0 (IX_HSSACC_DEBUG, 
			  "Calling ixNpeMhMessageWithResponseSend\n");
    
	/* send the message to the NpeMh */
	status = ixNpeMhMessageWithResponseSend (IX_NPEMH_NPEID_NPEA,
						 message, npeMsgId,
						 ixHssAccComNpeCmdRespCallback,
						 IX_HSSACC_NPEMH_SEND_RETRIES);
    }
    else
    {
	IX_HSSACC_TRACE0 (IX_HSSACC_DEBUG, "Calling ixNpeMhMessageSend\n");
	/* send the message to the NpeMh */
	status = ixNpeMhMessageSend (IX_NPEMH_NPEID_NPEA, message,
				     IX_HSSACC_NPEMH_SEND_RETRIES);       
    }

    /* check the return from the NpeMh and block for response if requested */
    if (status != IX_SUCCESS)
    {
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccComNpeCmdMsgSend - NpeMh failed to send\n");
	/* return error */
	status = IX_FAIL;		    
    }
    else
    {
	/* wait for a NPE_A response if one is expected */
	if (reqResp)
	{
	    IX_HSSACC_TRACE0 (IX_HSSACC_DEBUG, "SEM PEND\n");
       	    /* pend on a semaphore - the callback will set this */
	    IX_HSSACC_COM_MUT_LOCK ();
        }
        else
        {
            if ( ixEthHssAccCoexistEnable )
            {
                IX_ETH_HSS_COM_MUT_UNLOCK(); 
            }
        }
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccComNpeCmdMsgSend\n");
    return status;
}

/**
 * Function definition: ixHssAccComTdmToNpeVoiceChanTranslate
 */
UINT32 
ixHssAccComTdmToNpeVoiceChanTranslate (IxHssAccHssPort hssPortId,
				       UINT32 tdmSlotId)
{
    /*
     * Note (Please read before using this API):
     * This API performs no error checking on the tdmSlotId passed in by 
     * client. Hence, before this API is called, client should ensure 
     * that the tdmSlotId passed in has been configured as a channelised 
     * timeslot on the specified HSS port.
    */
    
    IxHssAccTdmSlotUsage tslotUsage;
    UINT32 index1;
    UINT32 npeVoiceChanId = 0;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccComTdmToNpeVoiceChanTranslate\n");

    /* Search for each TDM timeslot configured as channelised timeslot starting
     * from TS0 till tdmSlotId (not inclusive). If channelised timeslot found, 
     * increment npeVoiceChanId
     */
    for (index1 = 0; index1 < tdmSlotId; index1++)
    {
    	/* Extract the timeslot usage for TDM timeslot pointed by index1 from
    	 * HSS LUT word. Please refer to the description of this macro (located
    	 * in this source file) for more details.
    	 */
     	IX_HSSACC_TSLOTUSAGE_GET(hssPortId, index1, tslotUsage)

        /* if index1 is a channelised timeslot, then increment npeVoiceChanId */
	if ((tslotUsage == IX_HSSACC_TDMMAP_VOICE56K) || 
	    (tslotUsage == IX_HSSACC_TDMMAP_VOICE64K))
    	{
	    npeVoiceChanId++;		    
    	}
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccComTdmToNpeVoiceChanTranslate\n");
    
    /* return NPE voice channel Id */
    return npeVoiceChanId;
}

/**
 * Function definition: ixHssAccComPortConfigLoad
 */
PRIVATE IX_STATUS 
ixHssAccComPortConfigLoad (IxHssAccHssPort hssPortId)
{
    IX_STATUS status = IX_SUCCESS;
    IxNpeMhMessage message;
    IxFeatureCtrlReg ixResetCop;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccComPortConfigLoad\n");

    /* create the NpeMh message - NPE_A message format */
    ixHssAccComNpeCmdMsgCreate (IX_NPE_A_MSSG_HSS_PORT_CONFIG_LOAD, 0, 
				   hssPortId, 0, 0, &message);
    if (0 == PortConfigFlag)    
    {    			
        /* Reset HSS Coprocessor Only */	
        ixResetCop = ixFeatureCtrlRead ();	
        ixResetCop |= 0x80;	
        ixFeatureCtrlWrite (ixResetCop);	
        ixResetCop &= (~(0x80));	
        ixFeatureCtrlWrite (ixResetCop);    	    	
        PortConfigFlag = 1;   
    }
    /* send the message */
    status = ixHssAccComNpeCmdMsgSend (message, 
				       TRUE, /* block for response */
				       IX_NPE_A_MSSG_HSS_PORT_CONFIG_LOAD);

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccComPortConfigLoad\n");
    return status;
}

/**
 * Function definition: ixHssAccComEmptyLastErrorCallback
 */
PRIVATE void
ixHssAccComEmptyLastErrorCallback (unsigned lastHssError, 
				   unsigned servicePort)
{
    ixHssAccComStats.emptyLastErrCbs++;
}

/**
 * Function definition: ixHssAccComIsChanTimeslot
 */
BOOL 
ixHssAccComIsChanTimeslot (IxHssAccHssPort hssPortId,
			   UINT32 tdmSlotId)
{
    IxHssAccTdmSlotUsage tslotUsage;
    BOOL isChanTimeslot = TRUE;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccComIsChanTimeslot\n");

    /* Extract the timeslot usage for TDM timeslot pointed by tdmSlotId from
     * HSS LUT words. Please refer to the description of this macro (located
     * in this source file) for more details. 
     */
    IX_HSSACC_TSLOTUSAGE_GET(hssPortId, tdmSlotId, tslotUsage)

    /* check whether tdmSlotId is a channelised timeslot */
    if ((tslotUsage != IX_HSSACC_TDMMAP_VOICE56K) && 
        (tslotUsage != IX_HSSACC_TDMMAP_VOICE64K))
    {
	isChanTimeslot = FALSE;		    
    }
        
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccComIsChanTimeslot\n");
    
    return isChanTimeslot;
}

/**
 * Function definition: ixHssAccComNumChanTimeslotGet
 */
UINT32 
ixHssAccComNumChanTimeslotGet (IxHssAccHssPort hssPortId)
{
    return ixHssAccComConfiguration[hssPortId].numChannelised;
}

/**
 * Function definition: ixHssAccComShow
 */
void 
ixHssAccComShow (void)
{
    IxHssAccHssPort hssPortIndex;

    printf ("\nixHssAccComShow:\n");
    printf ("\t   validConfigs: %d \t  invalidConfigs: %d\n", 
	    ixHssAccComStats.validConfigs, 
	    ixHssAccComStats.invalidConfigs);
    printf ("\t    npeCmdSends: %d \t emptyLastErrCbs: %d\n", 
	    ixHssAccComStats.npeCmdSends, 
	    ixHssAccComStats.emptyLastErrCbs);
    printf ("\t    npeCmdResps: %d \t    npeReadResps: %d\n", 
	    ixHssAccComStats.npeCmdResps, 
	    ixHssAccComStats.npeReadResps);
    printf ("\t npeCmdInvResps: %d \t npeReadInvResps: %d\n", 
	    ixHssAccComStats.npeCmdInvResps, 
	    ixHssAccComStats.npeReadInvResps);
    
    for (hssPortIndex = IX_HSSACC_HSS_PORT_0;
	 hssPortIndex < hssPortMax;
	 hssPortIndex++)
    {
    	printf ("\t CurrentConfig Port %d -  txPCR: 0x%X\n", 
		hssPortIndex, 
		ixHssAccComConfiguration[hssPortIndex].txPCR);
    	printf ("\t                       -  rxPCR: 0x%X\n", 
		ixHssAccComConfiguration[hssPortIndex].rxPCR);
    	printf ("\t                       -    cCR: 0x%X\n", 
		ixHssAccComConfiguration[hssPortIndex].cCR);
        printf ("\t                       -  clkCR: 0x%X\n", 
		ixHssAccComConfiguration[hssPortIndex].clkCR);
    	printf ("\t                       -  txFCR: 0x%X\n", 
	    ixHssAccComConfiguration[hssPortIndex].txFCR);
    	printf ("\t                       -  rxFCR: 0x%X\n", 
	    ixHssAccComConfiguration[hssPortIndex].rxFCR);
    	printf ("\t                       - LUT[0]: 0x%X\n", 
		ixHssAccComConfiguration[hssPortIndex].hssTxLUT[0]);
    	printf ("\t                       - LUT[1]: 0x%X\n", 
		ixHssAccComConfiguration[hssPortIndex].hssTxLUT[1]);
    	printf ("\t                       - LUT[2]: 0x%X\n", 
		ixHssAccComConfiguration[hssPortIndex].hssTxLUT[2]);
    	printf ("\t                       - LUT[3]: 0x%X\n", 
		ixHssAccComConfiguration[hssPortIndex].hssTxLUT[3]);
    	printf ("\t                       - LUT[4]: 0x%X\n", 
		ixHssAccComConfiguration[hssPortIndex].hssTxLUT[4]);
    	printf ("\t                       - LUT[5]: 0x%X\n", 
		ixHssAccComConfiguration[hssPortIndex].hssTxLUT[5]);
    	printf ("\t                       - LUT[6]: 0x%X\n", 
		ixHssAccComConfiguration[hssPortIndex].hssTxLUT[6]);
    	printf ("\t                       - LUT[7]: 0x%X\n", 
		ixHssAccComConfiguration[hssPortIndex].hssTxLUT[7]);
    	printf ("\t                       -  #chan: 0x%X\n", 
		ixHssAccComConfiguration[hssPortIndex].numChannelised);
    	printf ("\t                       -  #pktd: 0x%X\n", 
		ixHssAccComConfiguration[hssPortIndex].numPacketised);
    #if ((CPU!=SIMSPARCSOLARIS) && (CPU!=SIMLINUX))
    	printf ("\t                       - lastErr: %p\n", 
		ixHssAccComConfiguration[hssPortIndex].lastErrorCallback);
    #endif
    }
}

/**
 * Function definition: ixHssAccComStatsInit
 */
void 
ixHssAccComStatsInit (void)
{
    ixHssAccComStats.validConfigs = 0;
    ixHssAccComStats.invalidConfigs = 0;
    ixHssAccComStats.emptyLastErrCbs = 0;
    ixHssAccComStats.npeCmdSends = 0;
    ixHssAccComStats.npeCmdResps = 0;
    ixHssAccComStats.npeReadResps = 0;
    ixHssAccComStats.npeCmdInvResps = 0;
}

/**
 * Function definition: ixHssAccComInit
 */
IX_STATUS 
ixHssAccComInit (void)
{
    IxHssAccHssPort hssPortIndex;
    IX_STATUS status = IX_SUCCESS;
    static BOOL mutexInitialised = FALSE;


    mutexInitialised = ixHssAccMutexInitialised;


    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccComInit\n");

    /* initialise stats */
    ixHssAccComStatsInit ();

    /* initialise the hssAccess mutex once */
    if (!mutexInitialised)
    {
	status = ixOsalMutexInit (&ixHssAccComMutex);
	if (status == IX_SUCCESS)
	{
	    mutexInitialised = TRUE;

        ixHssAccMutexInitialised = mutexInitialised;

	    IX_HSSACC_COM_MUT_LOCK(); /* lock the mutex initially */
	}
	else
	{
	    /* report the error */
	    IX_HSSACC_REPORT_ERROR ("ixHssAccComInit - mutex initialisation failed\n");
	    status = IX_FAIL;
	}
    }

    /* setup the default lastErrCallback function */
    for (hssPortIndex = IX_HSSACC_HSS_PORT_0;
	 hssPortIndex < IX_HSSACC_HSS_PORT_MAX;
	 hssPortIndex++)
    {
	ixHssAccComConfiguration[hssPortIndex].lastErrorCallback =
	    ixHssAccComEmptyLastErrorCallback;
    }

    if (status == IX_SUCCESS)
    {
	/* register read-response handler to IxNpeMh */
	status = ixNpeMhUnsolicitedCallbackRegister (IX_NPEMH_NPEID_NPEA,
						     IX_NPE_A_MSSG_HSS_PORT_ERROR_READ,
						     ixHssAccComNpeReadRespCallback);
	if (status != IX_SUCCESS)
	{
	    /* report the error */
	    IX_HSSACC_REPORT_ERROR ("ixHssAccComInit - failed to register ReadRespCallback\n");
	    status = IX_FAIL;
	}
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccComInit\n");
    return status;
}

/*
 * Function definition: ixHssAccComUninit
 */
IX_STATUS
ixHssAccComUninit (void)
{
    IX_STATUS status;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT,
                      "Entering ixHssAccComUninit\n");

    /*  unregister read-response handler to IxNpeMh */
    status = ixNpeMhUnsolicitedCallbackRegister (IX_NPEMH_NPEID_NPEA,
                                                 IX_NPE_A_MSSG_HSS_PORT_ERROR_READ,
                                                 NULL);
    if (IX_SUCCESS != status)
    {
        /* report the error */
        IX_HSSACC_REPORT_ERROR ("ixHssAccComUninit - failed to unregister ReadRespCallback\n");
        status = IX_FAIL;
    }

     /* uninitialise the hssAccess mutex */
    if (ixHssAccMutexInitialised)
    {
        status = ixOsalMutexDestroy (&ixHssAccComMutex);
        if (IX_SUCCESS == status)
        {
            ixHssAccMutexInitialised = FALSE;
        }
        else
        {
            /* report the error */
            IX_HSSACC_REPORT_ERROR ("ixHssAccComUninit - mutex uninitialisation failed\n");
            status = IX_FAIL;
        }
    }
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT,"Exiting ixHssAccComUninit\n");
    return status;
}

IX_STATUS 
ixHssAccComMbufToNpeFormatConvert (IX_OSAL_MBUF *bufPtr, UINT32 *chainCount, UINT32 *npeAddr ) 
{
    IX_OSAL_MBUF *bufChainPtr;
    IX_OSAL_MBUF *bufTemp;
    UINT32 npeAddrTemp;
    int iterationCount;

    bufChainPtr = bufPtr;

    /* Iterate through the IX_OSAL_BUF buffer chain */
    iterationCount = 0;
    while (bufChainPtr)
    {
        iterationCount++;

        /* Save the next pointer */
        bufTemp = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (bufChainPtr);

        /* Copy the relevant fields from OS dependant region to the NPE shared region of the IX_OSAL_BUF buffer */
        IX_HSSACC_MBUF_TO_IX_NE_SWAP (bufChainPtr);

        /* Set the status field in the NPE shared region of the IX_OSAL_BUF buffer */
        IX_HSSACC_IX_NE_SHARED_STATUS (bufChainPtr) = (UINT8)IX_HSSACC_PKT_OK;

        /* Set the error_count field in the NPE shared region of the IX_OSAL_BUF buffer */
        IX_HSSACC_IX_NE_SHARED_ERR_CNT (bufChainPtr) = (UINT8)0;

        /* Perform Endian conversion for the NPE shared region of the IX_OSAL_BUF buffer */
        IX_HSSACC_IX_NE_ENDIAN_SWAP (bufChainPtr);

        /* Force the NPE shared region of the IX_OSAL_BUF buffer to be stored in physical memory */
        IX_HSSACC_IX_NE_SHARED_CACHE_FLUSH (bufChainPtr);

        /* Go to the next IX_OSAL_BUF buffer in the chain */
        bufChainPtr = bufTemp;
    } /* end of while(bufChainPtr) */
    
    *chainCount = iterationCount;

    /* Convert the pointer to the physical address of the NPE shared region of the first IX_OSAL_BUF buffer */
    npeAddrTemp = (UINT32) IX_HSSACC_PKT_MMU_VIRT_TO_PHY (IX_HSSACC_IX_NE_SHARED(bufPtr));

    /* Ensure the bits which are reserved to exchange information with
     * the NPE are cleared.
     * If the IX_OSAL_BUF buffer address is not correctly aligned, or from an
     * incompatible memory range, then Log a message ...
     */
    IX_OSAL_ENSURE (((npeAddrTemp & ~IX_HSSACC_QM_Q_ADDR_MASK) == 0),
                    "Invalid address range");
    *npeAddr = npeAddrTemp;
    return IX_SUCCESS;

}

IX_OSAL_MBUF *
ixHssAccComMbufFromNpeFormatConvert (UINT32 qEntry,
                                  BOOL invalidateCache, 
                                  UINT32 *chainCount)
{
    IX_OSAL_MBUF *bufChainPtr;
    IX_OSAL_MBUF *bufPtr;
    UINT32 iterationCount;

   /* Extract the address from the Queue entry */
    qEntry = qEntry & IX_HSSACC_QM_Q_ADDR_MASK ;

    /* Assign the bufPtr pointer to the first IX_OSAL_BUF buffer */
    bufPtr = IX_HSSACC_IX_OSAL_MBUF_FROM_IX_NE (IX_HSSACC_PKT_MMU_PHY_TO_VIRT (qEntry));

    bufChainPtr = bufPtr;
    iterationCount = 0;
    
    /* Iterate through the IX_OSAL_BUF buffer chain */
    while (bufChainPtr)
    {
        iterationCount++;

        if (invalidateCache)
        {
            /* Force data to be read from physical memory */
            IX_HSSACC_IX_NE_SHARED_CACHE_INVALIDATE(bufChainPtr);
        }

        /* Perform Endian conversion for the NPE shared region of the IX_OSAL_BUF buffer */
        IX_HSSACC_IX_NE_ENDIAN_SWAP (bufChainPtr);

        /* Copy the required fields from NPE shared region of the IX_OSAL_BUF buffer 
         *  to the OS dependant region of the IX_OSAL_BUF buffer 
         */
        IX_HSSACC_MBUF_FROM_IX_NE_SWAP (bufChainPtr);

        /* Go to the next IX_OSAL_BUF buffer in the chain */
        bufChainPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (bufChainPtr);
    
    } /* end of while(bufChainPtr) */

    /* Set the chainCount to the number of IX_OSAL_BUF buffers in the chain */
    *chainCount = iterationCount;
    
    /* Return the address of the first IX_OSAL_BUF buffer in the chain */
    return (bufPtr);

}
