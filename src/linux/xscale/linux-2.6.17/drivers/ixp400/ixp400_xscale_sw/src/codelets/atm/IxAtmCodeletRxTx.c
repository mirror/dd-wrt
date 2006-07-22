/**
 * @file    IxAtmCodeletRxTx.c
 *
 * @date    20-May-2002
 *
 * @brief   Rx and Tx Transport for IXP400 Atm Codelet (IxAtmCodelet)
 *
 * Functions implemented in this file:
 *    ixAtmRxTxInit
 *    ixAtmRxTxStatsGet
 *    ixAtmRxTxChannelsProvision
 *    ixAtmRxTxSdusSend
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
 * User defined include files.
 */
#if defined(__wince) && defined(IX_USE_SERCONSOLE)
	#include "IxSerConsole.h"
	#define printf ixSerPrintf
	#define gets ixSerGets
#endif


#include "IxOsal.h"
#include "IxAtmdAcc.h"
#include "IxAtmCodelet_p.h"
#include "IxAtmCodelet.h"

/*
 * #defines and macros
 */
#define IX_ATMCODELET_TX_RETRY_CNT         (50)  /* Number of retries before failing to transmit a PDU */
#define IX_ATMCODELET_TX_RETRY_DELAY       (100) /* Number of Ms to delay on retry */

/*
  Aal5 PDU                      <------- trailer -------->
  +------------------------------------------------------+
  |    Payload           | PAD  | UU | CPI | Length | CRC|
  +------------------------------------------------------+
  |     1 - 65,535       | 0-47 | 1  |  1  |   2    |  4 |
 */
#define IX_ATMCODELET_TRAILER_SIZE         (8) /* Atm trailer size in bytes. UU+CPI+Length+CRC          */
#define IX_ATMCODELET_SDU_LEN_LO_BYTE      (3) /* Offset in trailer to the low byte of SDU length       */
#define IX_ATMCODELET_SDU_LEN_HI_BYTE      (2) /* Offset in trailer to the high bye of SDU length field */
#define IX_ATMCODELET_CPI_OFFSET           (1) /* Offset to Common Part Indicator field                 */
#define IX_ATMCODELET_UUI_OFFSET           (0) /* Offset to User-User field                             */
#define IX_ATMCODELET_DEFAULT_CPI          (0) /* Default Common Part Indicator field value             */
#define IX_ATMCODELET_DEFAULT_UUI          (9) /* Default User-User field value                         */
#define IX_ATMCODELET_SHIFT_EIGHT_BITS     (8) /* Operand to << and >> operators                        */
#define IX_ATMCODELET_MAX_SDU_LEN          (65535) /* Max length of Aal5 SDU                            */
#define IX_ATMCODELET_MAX_PACKET_LEN       (65535) /* Max length of Aal0 Packet                         */
/* Max offset in an ATM cell                         */
#define IX_ATMCODELET_FIRST_BYTE_MASK      (0x00ff)
#define IX_ATMCODELET_SECOND_BYTE_MASK     (0xff00)

#define IX_ATMCODELET_SAME_CELL_MAX_OFFSET (IX_ATM_CELL_PAYLOAD_SIZE-IX_ATMCODELET_TRAILER_SIZE)
#define IX_ATMCODELET_NEW_CELL_THRESHOLD (IX_ATMCODELET_SAME_CELL_MAX_OFFSET+1)
#define IX_ATMCODELET_NEW_CELL_MAX_OFFSET (IX_ATMCODELET_SAME_CELL_MAX_OFFSET+IX_ATM_CELL_PAYLOAD_SIZE)

/*
 * Typedefs
 */

typedef struct {
    IxAtmConnId connId;
    IxAtmdAccAalType aalType;
    UINT32 bytesPerCell;
    UINT32 atmHeader;
} VcInfo;

/*
 * Static variables
 */
static VcInfo ixAtmRxTxTxVcInfoTable[IX_ATM_MAX_NUM_VC];
static VcInfo ixAtmRxTxRxVcInfoTable[IX_ATM_MAX_NUM_VC];

static IxAtmLogicalPort ixAtmRxTx_port       = 0;
static UINT32     ixAtmRxTx_vci        = IX_ATMCODELET_START_VCI;
static UINT32     ixAtmRxTx_vpi        = IX_ATMCODELET_START_VPI;    
static UINT32     ixAtmRxTx_channelIdx = 0;

/* Number of VCs configured */
static UINT32 ixAtmRxTxNumChannelsConfigured = 0;

/* Statistics */
static IxAtmCodeletStats *ixAtmRxTxStats = NULL;

/*
 * Function definitions
 */
PRIVATE void
ixAtmRxTxStatsReset (void);


/* Recieve callback */
PRIVATE void
ixAtmRxTxRxCallback (IxAtmLogicalPort port,
		     IxAtmdAccUserId userId,
		     IxAtmdAccPduStatus pduStatus,
		     IxAtmdAccClpStatus clp,
		     IX_OSAL_MBUF *mbufPtr);

/* Transmit done callback */
PRIVATE void
ixAtmRxTxTxDoneCallback (IxAtmdAccUserId userId,
			 IX_OSAL_MBUF * mbufPtr);

/* Tansmit an Aal5 packet */
PRIVATE IX_STATUS
ixAtmRxTxAal5CpcsTx (IxAtmConnId connId,
		     UINT32 sduLen);

/* Tansmit an Aal0 packet */
PRIVATE IX_STATUS
ixAtmRxTxAal0Tx (int channelNum,
		 UINT32 sizeInCells);

/* Generic send with retries function */
PRIVATE IX_STATUS
ixAtmRxTxWithRetiesSend (IxAtmConnId connId, 
			 IX_OSAL_MBUF* mBuf, 
			 UINT32 sizeInCells);

/* flush an mbuf from cache (use before sending) */
PRIVATE void
ixAtmRxTxTxFlush(IX_OSAL_MBUF * mbufPtr);


/* invalidate cache (use before reading after rx) */
PRIVATE void
ixAtmRxTxRxInvalidate(IX_OSAL_MBUF * mbufPtr);

PRIVATE IX_STATUS
ixAtmRxTxAal0PacketBuild (int channelNum,
			  UINT32 sizeInCells,
			  IX_OSAL_MBUF **mBuf);

PRIVATE IX_STATUS
ixAtmRxTxAal5CpcsPduBuild (UINT32 sduLen,
			   IX_OSAL_MBUF **mBuf);

/* --------------------------------------------------------------
   Initialise the RxTx.
   -------------------------------------------------------------- */
IX_STATUS
ixAtmRxTxInit (IxAtmCodeletStats *stats)
{
    int i;

    ixAtmRxTxStats = stats;

    /* Reset statistics */
    ixAtmRxTxStatsReset();

    for( i=0; i < IX_ATM_MAX_NUM_VC; i++)
    {
	ixAtmRxTxRxVcInfoTable[i].aalType = IX_ATMDACC_MAX_SERVICE_TYPE;
	ixAtmRxTxTxVcInfoTable[i].aalType = IX_ATMDACC_MAX_SERVICE_TYPE;        
    }

    return IX_SUCCESS;
}

/* --------------------------------------------------------------
   Get the IxAtmCodeletRxTx counters.
   -------------------------------------------------------------- */
void
ixAtmRxTxStatsGet (IxAtmCodeletStats *stats)
{
    *stats = *ixAtmRxTxStats;
}


/* --------------------------------------------------------------
   Remove all provisioned channels.
   -------------------------------------------------------------- */
IX_STATUS
ixAtmRxTxChannelsRemove ( void )
{
    IX_STATUS retval;
    int i;

    ixAtmRxTx_port       = 0;
    ixAtmRxTx_vci        = IX_ATMCODELET_START_VCI;
    ixAtmRxTx_vpi        = IX_ATMCODELET_START_VPI;    
    ixAtmRxTx_channelIdx = 0;

    retval = ixAtmUtilsAtmAllVcsDisconnect ();
    
    if (IX_SUCCESS != retval)
    {
	IX_ATMCODELET_LOG ("Failed to remove channels\n");
    }

    for( i=0; i < IX_ATM_MAX_NUM_VC; i++)
    {
	ixAtmRxTxRxVcInfoTable[i].aalType = IX_ATMDACC_MAX_SERVICE_TYPE;
	ixAtmRxTxTxVcInfoTable[i].aalType = IX_ATMDACC_MAX_SERVICE_TYPE;        
    }

    return retval;
}


/* --------------------------------------------------------------
   Provision channels specifying the number of ports and channels.
   -------------------------------------------------------------- */
IX_STATUS
ixAtmRxTxChannelsProvision (UINT32 numPorts,
			    UINT32 numChannels,
			    IxAtmdAccAalType aalType)
{
    IX_STATUS retval;
    UINT32 cellHdr;
    UINT32 i;
    IxAtmServiceCategory atmServiceCat = IX_ATM_UBR; /* Set to default */
    
    /* Parameter validation */
    if ((numPorts == 0) || (numChannels == 0))
    {
	IX_ATMCODELET_LOG("Invalid parameters to channesls provision \n");
	return IX_FAIL;
    }

    /* N.B. channelIdx is static */
    for (i=0; i < numChannels; i++, ixAtmRxTx_channelIdx++)
    {

	/* Save the aalType and bytesPerCell in the Rx & Tx Vc Info tables */
	ixAtmRxTxRxVcInfoTable[ixAtmRxTx_channelIdx].aalType = aalType;
	ixAtmRxTxTxVcInfoTable[ixAtmRxTx_channelIdx].aalType = aalType;
	if (aalType == IX_ATMDACC_AAL0_52)
	{
	    ixAtmRxTxTxVcInfoTable[ixAtmRxTx_channelIdx].bytesPerCell
               = IX_ATM_AAL0_52_CELL_SIZE_NO_HEC;
	    ixAtmRxTxRxVcInfoTable[ixAtmRxTx_channelIdx].bytesPerCell 
	       = IX_ATM_AAL0_52_CELL_SIZE_NO_HEC;
	    
	    /* build the cell header for transmission */
            cellHdr = (((ixAtmRxTx_vci & 0xFFFF) << 4) | ((ixAtmRxTx_vpi & 0xFF) << 20));

	    ixAtmRxTxTxVcInfoTable[ixAtmRxTx_channelIdx].atmHeader = cellHdr;	    
            ixAtmRxTxRxVcInfoTable[ixAtmRxTx_channelIdx].atmHeader = cellHdr;
	}
	else
	{
	    ixAtmRxTxTxVcInfoTable[ixAtmRxTx_channelIdx].bytesPerCell
            = IX_ATM_AAL0_48_CELL_PAYLOAD_SIZE;
	    ixAtmRxTxRxVcInfoTable[ixAtmRxTx_channelIdx].bytesPerCell
            = IX_ATM_AAL0_48_CELL_PAYLOAD_SIZE;
	}
	
	/* Verify whether ATM codelet is going to use Real-time VCs for
	 ( UTOPIA/Remote loopback mode. Otherwise set to UBR VCs by default */
	if (ixAtmUtilsAtmRtVcsGet())
	{
	    if (IX_ATMCODELET_8VCS > i)
	    {
		/* 1st 8VCs setup with VBR */
		atmServiceCat = IX_ATM_VBR;
                IX_ATMCODELET_LOG("Setting up VBR: ");
	    }
	    else if ((IX_ATMCODELET_8VCS <= i) && 
		     (IX_ATMCODELET_16VCS > i))
	    {
		/* 2nd 8VCs setup with CBR */
		atmServiceCat = IX_ATM_CBR;
                IX_ATMCODELET_LOG("Setting up CBR: ");
	    }
	    else 
	    {
		/* Balance is 16 UBR VCs */
		atmServiceCat = IX_ATM_UBR;
                IX_ATMCODELET_LOG("Setting up UBR: ");
	    }
	}
	else
	{
	    IX_ATMCODELET_LOG("Setting up UBR: ");
	}

	/*
	 * Register this VC with IxAtmm and connect it to IxAtmdAcc
	 */
	retval = ixAtmUtilsAtmVcRegisterConnect(ixAtmRxTx_port,
						 ixAtmRxTx_vpi,
						 ixAtmRxTx_vci,
						 aalType,
						 atmServiceCat,
						 IX_ATM_RX_A,
						 ixAtmRxTxRxCallback,
						 IX_ATMCODELET_RX_FREE_Q_LOW_THRESHOLD,
						 ixAtmRxTxTxDoneCallback,
						 ixAtmRxTxRxFreeLowReplenish,
						 ixAtmRxTx_channelIdx,
						 &ixAtmRxTxRxVcInfoTable[ixAtmRxTx_channelIdx].connId,
						 &ixAtmRxTxTxVcInfoTable[ixAtmRxTx_channelIdx].connId);
	
	if (retval != IX_SUCCESS)
	{
	    IX_ATMCODELET_LOG("VC setup failed for port %2u vpi %3u vci %5u\n", 
			      ixAtmRxTx_port, ixAtmRxTx_vpi, ixAtmRxTx_vci);
	    return IX_FAIL;
	}
	else
	{
	    IX_ATMCODELET_LOG("VC configured for port %2u vpi %3u vci %5u\n", 
			      ixAtmRxTx_port, ixAtmRxTx_vpi, ixAtmRxTx_vci);
	}
	
	/* Use a different port per channel, depending on the maximum
	 * number of ports configured
	 */
	ixAtmRxTx_port++;

	/* Wrap the port back to 0 when numPorts used already */
	if (ixAtmRxTx_port >= (IxAtmLogicalPort)numPorts)
        {
	    ixAtmRxTx_port = 0;
        }

	/* Use the next vci */
	ixAtmRxTx_vci++;
       
	ixAtmRxTxNumChannelsConfigured++;
    }

    return IX_SUCCESS;
}



/* --------------------------------------------------------------
   Send a number of PDUs on the registered channels.
   -------------------------------------------------------------- */
IX_STATUS
ixAtmRxTxAal0PacketsSend (UINT32 cellsPerPacket,
			  UINT32 numPackets)
{
    UINT32 pktCnt;
    IX_STATUS retval;
    BOOL aal0Provisioned = FALSE;
    UINT32 channelIdx;

    /* check that there are any Aal0 channels provisioned */
    for(channelIdx=0; channelIdx<ixAtmRxTxNumChannelsConfigured; channelIdx++ )
    {
        if ((ixAtmRxTxTxVcInfoTable[channelIdx].aalType == IX_ATMDACC_AAL0_48) ||
	    (ixAtmRxTxTxVcInfoTable[channelIdx].aalType == IX_ATMDACC_AAL0_52))
            {
                aal0Provisioned = TRUE;
            }
    }
    
    if( aal0Provisioned == FALSE )
    {
        IX_ATMCODELET_LOG ("No AAL0 channels are provisioned\n");
        return IX_FAIL;
    } 

    for (pktCnt=0; pktCnt<numPackets;)
    {
	/* For each Aal0 VC */
	for (channelIdx=0; (pktCnt<numPackets) &&
		 (channelIdx<ixAtmRxTxNumChannelsConfigured); channelIdx++ )
	{
	    if ((ixAtmRxTxTxVcInfoTable[channelIdx].aalType == IX_ATMDACC_AAL0_48) ||
		(ixAtmRxTxTxVcInfoTable[channelIdx].aalType == IX_ATMDACC_AAL0_52))
	    {
		retval = ixAtmRxTxAal0Tx (channelIdx,
					      cellsPerPacket );
		
		if (retval == IX_SUCCESS)
                {
                        pktCnt++;
                }
                else
		{
		    IX_ATMCODELET_LOG ("Failed to send a pdu on channel...... %u\n", channelIdx);
		}
	    }
	}
    }

    return IX_SUCCESS;    
}

/* --------------------------------------------------------------
   Send a number of PDUs on the registered channels.
   -------------------------------------------------------------- */
IX_STATUS
ixAtmRxTxAal5CpcsSdusSend (UINT32 sduSize,
			   UINT32 numSdus)
{
    UINT32 sduCnt;
    IX_STATUS retval;
    UINT32 channelIdx;
    BOOL aal5Provisioned = FALSE;

    /* check that there are any Aal0 channels provisioned */
    for(channelIdx=0; channelIdx<ixAtmRxTxNumChannelsConfigured; channelIdx++ )
    {
        if  (ixAtmRxTxTxVcInfoTable[channelIdx].aalType == IX_ATMDACC_AAL5)
            {
                aal5Provisioned = TRUE;
            }
    }
    
    if( aal5Provisioned == FALSE )
    {
        IX_ATMCODELET_LOG ("No AAL5 channels are provisioned\n");
        return IX_FAIL;
    } 



    for (sduCnt=0; sduCnt<numSdus;)
    {
	/* For each Aal5 VC */
	for (channelIdx=0; (sduCnt<numSdus) &&
		 (channelIdx < ixAtmRxTxNumChannelsConfigured); channelIdx++, sduCnt++)
	{
	    if (ixAtmRxTxTxVcInfoTable[channelIdx].aalType == IX_ATMDACC_AAL5)
	    {
		retval = ixAtmRxTxAal5CpcsTx (ixAtmRxTxTxVcInfoTable[channelIdx].connId,
						  sduSize);
		
		if (retval != IX_SUCCESS)
		{
		    IX_ATMCODELET_LOG ("Failed to send a pdu on channel...... %u\n", channelIdx);
		}
	    }
	    else
	    {
		/* decrement the sduCnt a sdu was not sent on this vc */
		sduCnt--;
	    }
	}
    }    

    return IX_SUCCESS;    
}

/* --------------------------------------------------------------
   Replenish buffers to the RxFree queue. Buffers are taken from
   the mbuf pool
   -------------------------------------------------------------- */
void
ixAtmRxTxRxFreeLowReplenish (IxAtmdAccUserId userId)
{
    IX_OSAL_MBUF *mBuf;
    IX_STATUS retval;
    IxAtmConnId connId;
    UINT32 numFreeEntries;
    UINT32 cnt;
    
    connId = ixAtmRxTxRxVcInfoTable[userId].connId;
    
    /* Get the number of free entries in the RX Free queue */
    retval = ixAtmdAccRxVcFreeEntriesQuery (connId, &numFreeEntries);
    if (retval != IX_SUCCESS)
    {
	IX_ATMCODELET_LOG("Failed to query depth of Rx Free Q for connection %u\n",
			  connId);
	return;
    }
    
    /* Replenish Rx buffers  */
    for (cnt=0; cnt<numFreeEntries; cnt++)
    {
	/* Get an MBUF */
	ixAtmUtilsMbufGet(IX_ATMCODELET_MBUF_SIZE, &mBuf);
	
	if (mBuf == NULL)
	{
	    IX_ATMCODELET_LOG("Failed to get rx free mbuffers\n");
	    return;
	}

        if(IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mBuf) != 0)
	{
	    IX_ATMCODELET_LOG("Next Ptr != 0\n");
	    return;
	}

	IX_OSAL_ASSERT(IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mBuf) == 0);

	/* 
	 * Set the number of bytes of data in this MBUF to the number of
	 * payloads that will fit evenly into IX_ATMCODELET_MBUF_SIZE.
	 */
	IX_OSAL_MBUF_MLEN(mBuf) = 
	    (IX_ATMCODELET_MBUF_SIZE / ixAtmRxTxRxVcInfoTable[userId].bytesPerCell) *
	    ixAtmRxTxRxVcInfoTable[userId].bytesPerCell;
	
	/* Send free buffers to NPE */
	retval = ixAtmdAccRxVcFreeReplenish (connId, mBuf);
	
	if (retval != IX_SUCCESS)
	{
	    /* Free the allocated buffer */
	    IX_ATMCODELET_LOG("Failed to pass Rx free buffers to Atmd \n");
	    ixAtmUtilsMbufFree(mBuf);
	    return;
	}
	
	/* Update stats */
	ixAtmRxTxStats->rxFreeBuffers++;
    }
}

/* --------------------------------------------------------------
   Return an Aal0 packet. This assumes that the Packet will fit into a
   single mbuf. i.e. No chaining.
   -------------------------------------------------------------- */
PRIVATE IX_STATUS
ixAtmRxTxAal0PacketBuild (int channelNum,
			  UINT32 sizeInCells,
			  IX_OSAL_MBUF **mBuf)
{
    UINT8 *dataPtr;
    unsigned char headerChar1, headerChar2, headerChar3, headerChar4;
    UINT32 packetLen;
    UINT32 tmpLen;
    UINT32 cellDataSize = ixAtmRxTxTxVcInfoTable[channelNum].bytesPerCell;


    /* Initialise the returned pointer */
    *mBuf = NULL;
    
    /* calculate the packet length */
    packetLen = sizeInCells * cellDataSize;
    if (IX_ATMCODELET_MAX_PACKET_LEN < packetLen)
    {
	IX_ATMCODELET_LOG ("ixAtmRxTxAal0PacketBuild(): IX_ATMCODELET_MAX_PACKET_LEN < packetLen\n");
	return IX_FAIL;
    }

    /* Get the first buffer */
    ixAtmUtilsMbufGet (packetLen, mBuf);

    if (*mBuf == NULL)
    {
	IX_ATMCODELET_LOG ("ixAtmRxTxAal0PacketBuild() failed to get mBuf\n");
        return IX_FAIL;
    }

    /* if the aaltype is aal0_52 then insert the header into the buffer */
    if (ixAtmRxTxTxVcInfoTable[channelNum].aalType == IX_ATMDACC_AAL0_52)
    {
	dataPtr = (UINT8 *) IX_OSAL_MBUF_MDATA(*mBuf);
	tmpLen = packetLen;
	
	headerChar1 = (char)(ixAtmRxTxTxVcInfoTable[channelNum].atmHeader >> 24);
	headerChar2 = (char)((ixAtmRxTxTxVcInfoTable[channelNum].atmHeader >> 16) & 0xFF);
	headerChar3 = (char)((ixAtmRxTxTxVcInfoTable[channelNum].atmHeader >> 8) & 0xFF);
	headerChar4 = (char)(ixAtmRxTxTxVcInfoTable[channelNum].atmHeader & 0xFF);
	
	while(tmpLen > 0)
	{
	    /* get the dataPtr and insert the header */
	    *dataPtr++ = headerChar1;
	    *dataPtr++ = headerChar2;
	    *dataPtr++ = headerChar3;
	    *dataPtr++ = headerChar4;

	    /* increment to the next header position */
	    dataPtr = dataPtr + 48;

	    /* decrement the packetlen */
	    tmpLen = tmpLen - ixAtmRxTxTxVcInfoTable[channelNum].bytesPerCell;
	}
    }

    IX_OSAL_ASSERT(IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(*mBuf) == 0);

    /* Setup the MBUF,
     * N.B. for simplicity not filling in payload
     */
    IX_OSAL_MBUF_PKT_LEN(*mBuf) = packetLen;

    return IX_SUCCESS;
}

/* --------------------------------------------------------------
   Return an Aal5 PDU. This assumes that the PDU will fit into a
   single mbuf. i.e. No chaining.
   -------------------------------------------------------------- */
PRIVATE IX_STATUS
ixAtmRxTxAal5CpcsPduBuild (UINT32 sduLen,
			   IX_OSAL_MBUF **mBuf)
{
    int pduLen;
    unsigned char* trailer = NULL;

    if (IX_ATMCODELET_MAX_SDU_LEN < sduLen)
    {
	IX_ATMCODELET_LOG ("ixAtmRxTxAal5CpcsPduBuild(): IX_ATMCODELET_MAX_SDU_LEN < sduLen\n");
	return IX_FAIL;
    }


    /* CASE 1: Enough bytes left in CELL to store trailer */
    if ((sduLen % IX_ATM_CELL_PAYLOAD_SIZE) ==
	IX_ATMCODELET_SAME_CELL_MAX_OFFSET)
    {
	pduLen = sduLen + IX_ATMCODELET_TRAILER_SIZE;
    }
    /* CASE 2: More than enough bytes in CELL to store trailer */
    else if ((sduLen % IX_ATM_CELL_PAYLOAD_SIZE) <
	     IX_ATMCODELET_SAME_CELL_MAX_OFFSET)
    {
	pduLen = (sduLen - (sduLen % IX_ATM_CELL_PAYLOAD_SIZE)) +
	    IX_ATM_CELL_PAYLOAD_SIZE;
    }
    /* CASE 3: Not enough bytes in CELL to store trailer. Need another CELL */
    else
    {
	pduLen = (sduLen - (sduLen % IX_ATM_CELL_PAYLOAD_SIZE)) +
	    (IX_ATM_CELL_PAYLOAD_SIZE * 2/* Need another CELL*/);
    }

    /* Initialise the returned pointer */
    *mBuf = NULL;

    /* Get the first buffer */
    ixAtmUtilsMbufGet (pduLen, mBuf);

    if (*mBuf == NULL)
    {
	IX_ATMCODELET_LOG ("ixAtmRxTxAal5CpcsPduBuild() failed to get mBuf\n");
        return IX_FAIL;
    }

    IX_OSAL_ASSERT(IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(*mBuf) == 0);

    
    trailer = (unsigned char*)IX_OSAL_MBUF_MDATA(*mBuf) + pduLen - 8;
    
    /*Fill out the relevant inf/trailero in the trailer*/
    trailer[IX_ATMCODELET_UUI_OFFSET]      = IX_ATMCODELET_DEFAULT_UUI;
    trailer[IX_ATMCODELET_CPI_OFFSET]      = IX_ATMCODELET_DEFAULT_CPI;
    trailer[IX_ATMCODELET_SDU_LEN_LO_BYTE] = sduLen & IX_ATMCODELET_FIRST_BYTE_MASK;
    trailer[IX_ATMCODELET_SDU_LEN_HI_BYTE] = (sduLen & IX_ATMCODELET_SECOND_BYTE_MASK)
	>> IX_ATMCODELET_SHIFT_EIGHT_BITS;
        
    IX_OSAL_MBUF_PKT_LEN(*mBuf) = pduLen;
    IX_OSAL_MBUF_MLEN(*mBuf)    = pduLen;

    return IX_SUCCESS;
}

/* --------------------------------------------------------------
   Reset the counters.
   -------------------------------------------------------------- */
PRIVATE void
ixAtmRxTxStatsReset (void)
{
    if (NULL != ixAtmRxTxStats)
    {
	ixAtmRxTxStats->txPdus = 0;
	ixAtmRxTxStats->txBytes = 0;
	ixAtmRxTxStats->rxPdus = 0;
	ixAtmRxTxStats->rxBytes = 0;
	ixAtmRxTxStats->txDonePdus = 0;    
	ixAtmRxTxStats->rxFreeBuffers = 0;
	ixAtmRxTxStats->txPdusSubmitFail = 0;
	ixAtmRxTxStats->txPdusSubmitBusy = 0;
	ixAtmRxTxStats->rxPdusInvalid = 0;
    }
}

/* --------------------------------------------------------------
   The callback is called whenever the TxDone queue reaches a
   certain level. Mbufs are simply returned to the vxWorks pool.
   -------------------------------------------------------------- */
PRIVATE void
ixAtmRxTxTxDoneCallback (IxAtmdAccUserId userId,
			 IX_OSAL_MBUF *mbufPtr)
{
    ixAtmRxTxStats->txDonePdus++;

    /* Return the buffer to the vxWorks pool */
    ixAtmUtilsMbufFree(mbufPtr);
}


/* --------------------------------------------------------------
   This callback is called when a PDU is recieved. The Mbuf
   is freed to the vxWorks pool.
   -------------------------------------------------------------- */
PRIVATE void
ixAtmRxTxRxCallback (IxAtmLogicalPort port,
		     IxAtmdAccUserId userId,
		     IxAtmdAccPduStatus pduStatus,
		     IxAtmdAccClpStatus clp,
		     IX_OSAL_MBUF *mbufPtr)
{
    UINT32 sizeInCells;
    IxAtmdAccAalType aalType;


    if (mbufPtr == NULL)
    {
	IX_ATMCODELET_LOG("rxCallback called with NULL mbufPtr\n");
	return;
    }

    /* invalidate cache */
    ixAtmRxTxRxInvalidate( mbufPtr );

    /* Size of the data transmitted in CELLS */
    sizeInCells = IX_OSAL_MBUF_PKT_LEN(mbufPtr) / 
	ixAtmRxTxRxVcInfoTable[userId].bytesPerCell;

    if (pduStatus == IX_ATMDACC_MBUF_RETURN)
    {
	/* Return this buffer to the pool */
	ixAtmUtilsMbufFree (mbufPtr);
	return;
    }

    aalType = ixAtmRxTxRxVcInfoTable[userId].aalType;

    if ((aalType == IX_ATMDACC_AAL0_48) || (aalType == IX_ATMDACC_AAL0_52))
    {
	if (pduStatus != IX_ATMDACC_AAL0_VALID)
	{
	    ixAtmRxTxStats->rxPdusInvalid++;
	}

	/* Aal0 should always be valid */
	IX_OSAL_ASSERT(pduStatus == IX_ATMDACC_AAL0_VALID);
    }
    else 
    {
	/* Must be Aal5 */
	IX_OSAL_ASSERT(aalType == IX_ATMDACC_AAL5);

	/* Check PDU status, could be valid, partial or crc error */
	if (pduStatus != IX_ATMDACC_AAL5_VALID)
	{	
	    ixAtmRxTxStats->rxPdusInvalid++;
	    IX_ATMCODELET_LOG("rxCallback called with CRC error\n");
	}
    }

    /* Update stats */
    ixAtmRxTxStats->rxPdus++;
    ixAtmRxTxStats->rxBytes += IX_OSAL_MBUF_PKT_LEN(mbufPtr);
    
    /* Return this buffer to the pool */
    ixAtmUtilsMbufFree (mbufPtr);
    return;
}

/* --------------------------------------------------------------
   Transmit an Atm PDU of the specified length on the channel
   associated with connId.
   -------------------------------------------------------------- */
PRIVATE IX_STATUS
ixAtmRxTxAal5CpcsTx (IxAtmConnId connId,
		     UINT32 sduLen)
{
    int sizeInCells = 0;
    IX_OSAL_MBUF *mBuf;

    /* Build an Aal5 PDU for this SDU */
    if (ixAtmRxTxAal5CpcsPduBuild (sduLen, &mBuf) != IX_SUCCESS)
    {
	return IX_FAIL;
    }

    /* Size of the data transmitted in CELLS */
    sizeInCells = IX_OSAL_MBUF_PKT_LEN(mBuf) / IX_ATM_CELL_PAYLOAD_SIZE;

    return ixAtmRxTxWithRetiesSend (connId, mBuf, sizeInCells);
}

/* --------------------------------------------------------------
   Transmit an Aal0 Packet of the specified length on the channel
   associated with connId.
   -------------------------------------------------------------- */
PRIVATE IX_STATUS
ixAtmRxTxAal0Tx (int channelNum,
		 UINT32 sizeInCells)
{
    IX_OSAL_MBUF *mBuf;

    /* Build an Aal0 Packet */
    if (ixAtmRxTxAal0PacketBuild (channelNum, sizeInCells, &mBuf) != IX_SUCCESS)
    {
	return IX_FAIL;
    }

    return ixAtmRxTxWithRetiesSend (ixAtmRxTxTxVcInfoTable[channelNum].connId, mBuf, sizeInCells);
}

PRIVATE IX_STATUS
ixAtmRxTxWithRetiesSend (IxAtmConnId connId, IX_OSAL_MBUF* mBuf, UINT32 sizeInCells)
{
    IX_STATUS status;
    UINT32 retryCount;

    /* flush cache */
    ixAtmRxTxTxFlush( mBuf );
    

    /* Transmit with retries */
    for (status=IX_FAIL, retryCount=0; ((retryCount<IX_ATMCODELET_TX_RETRY_CNT) &&
					(status!=IX_SUCCESS)); retryCount++)
    {
	status = ixAtmdAccTxVcPduSubmit(connId,
					mBuf,
					IX_ATMCODELET_DEFAULT_CLP,
					sizeInCells);

        if (status != IX_SUCCESS)
        {
            if (status != IX_ATMDACC_BUSY)
            {
		ixAtmRxTxStats->txPdusSubmitFail++; /* stats for fail */
		IX_ATMCODELET_LOG("txAndRetry() failed when sending pkt\n");
                break;
            }

	    ixAtmRxTxStats->txPdusSubmitBusy++; /* stats for busy */
            ixOsalSleep(IX_ATMCODELET_TX_RETRY_DELAY);
        }
    }

    if (status == IX_SUCCESS)
    {	
	/* Update stats */
	ixAtmRxTxStats->txPdus++;
	ixAtmRxTxStats->txBytes += IX_OSAL_MBUF_PKT_LEN(mBuf);
    }
    else
    {
        /* release the mbuf */
        ixAtmUtilsMbufFree (mBuf);
	status = IX_FAIL;
    }
    
    return status;
}



/* ---------------------------------------------------
*/
PRIVATE void
ixAtmRxTxTxFlush(IX_OSAL_MBUF * mbufPtr)
{
    while (mbufPtr != NULL)
    {
      IX_OSAL_CACHE_FLUSH(IX_OSAL_MBUF_MDATA(mbufPtr), IX_OSAL_MBUF_MLEN(mbufPtr));
      mbufPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr);
    }
    return;
}

/* ---------------------------------------------------
*/
PRIVATE void
ixAtmRxTxRxInvalidate(IX_OSAL_MBUF * mbufPtr)
{
    while (mbufPtr != NULL)
    {
      IX_OSAL_CACHE_INVALIDATE(IX_OSAL_MBUF_MDATA(mbufPtr), IX_OSAL_MBUF_MLEN(mbufPtr));
      mbufPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr);
    }
    return;
}

           
