/* 
 * @file    IxOamCodelet.c
 *
 * @date    21-July-2002
 *
 * @brief   The module implements the loopback functionality
 *          functions of the Oam Codelet.
 *
 *
 * IxOamCodelet API functions:
 *       ixOamCodeletInit
 *       ixOamCodeletOamF5EtePing
 *       ixOamCodeletOamF5SegPing
 *       ixOamCodeletOamF4EtePing
 *       ixOamCodeletOamF4SegPing
 *       ixOamCodeletShow
 *
 * @par
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
 * Put the user defined include files required
 */

#if defined(__wince) && defined(IX_USE_SERCONSOLE)
	#include "IxSerConsole.h"
	#define printf ixSerPrintf
	#define gets ixSerGets
#endif


#include "IxAtmTypes.h"
#include "IxOsal.h"
#include "IxAtmdAcc.h"
#include "IxAtmm.h"
#include "IxAtmCodelet.h"
#include "IxAtmCodelet_p.h"
/*
 * #defines and macros used in this file.
 */

/* IRQ safe logs, for use in callbacks which could be in the context of an IRQ */
#define IX_OAM_CODELET_IRQ_SAFE_LOG(msg, param1, param2, param3, param4) \
 (void) ixOsalLog(IX_OSAL_LOG_LVL_USER,IX_OSAL_LOG_DEV_STDOUT, msg, param1, param2, param3, param4, 0, 0);
#define IX_OAM_CODELET_IRQ_SAFE_LOG_ERROR(msg) \
 (void) ixOsalLog (IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, msg, 0, 0, 0, 0, 0, 0);


/* Normal Log */
#define IX_OAM_CODELET_LOG printf
#define IX_OAM_CODELET_LOG_ERROR printf

#define IX_OAMCODELET_NUM_MBUFS_IN_SW_MBUF_Q 64 /* must be power of 2 */

#define IX_OAM_REPLENISH_WATERMARK  2

#define IX_OAM_TX_RETRY_DELAY 20
#define IX_OAM_TX_RETRY_COUNT_MAX 50

#define IX_OAM_RX_QUEUE_POLL_INTERVAL 100

/* endianness handling */
#define IX_OAM_HEADER_GET(cell, hdr)                                      \
(hdr) = (((*((UCHAR *)(&(cell)->header))) & 0xFF) << 24);                 \
(hdr) = (hdr) | (((*(((UCHAR *)(&(cell)->header)) + 1)) & 0xFF) << 16);   \
(hdr) = (hdr) | (((*(((UCHAR *)(&(cell)->header)) + 2)) & 0xFF) << 8);    \
(hdr) = (hdr) | (((*(((UCHAR *)(&(cell)->header)) + 3)) & 0xFF) << 0); 

#define IX_OAM_VCI_GET(oamHdr)  (((oamHdr) >> 4) & 0xFFFF)
#define IX_OAM_VPI_GET(oamHdr)  (((oamHdr) >> 20) & 0xFF)
#define IX_OAM_PTI_GET(oamHdr)  (((oamHdr) >> 1) & 0x7)

#define IX_OAM_HEADER_SET(cell, hdr)      do {               \
     register UCHAR *tmpVal;                                 \
     tmpVal = ((UCHAR *)(&(cell)->header));                  \
     *tmpVal++ = (((hdr) >> 24) & 0xFF);                     \
     *tmpVal++ = (((hdr) >> 16) & 0xFF);                     \
     *tmpVal++ = (((hdr) >> 8) & 0xFF);                      \
     *tmpVal++ = ((hdr) & 0xFF);                             \
 } while(0);

#define IX_OAM_VCI_SET(oamHdr, vci)  ( (oamHdr) = ((oamHdr) | (( (vci) & 0xFFFF ) << 4)) )
#define IX_OAM_VPI_SET(oamHdr, vpi)  ( (oamHdr) = ((oamHdr) | (( (vpi) & 0xFF ) << 20)) )
#define IX_OAM_PTI_SET(oamHdr, pti)  ( (oamHdr) = ((oamHdr) | (( (pti) & 0x7 ) << 1)) )

#define IX_OAM_TYPE_AND_FUNC_GET(cell) ((cell)->payload.genericPayload.oamTypeAndFunction)
#define IX_OAM_LOOPBACK_INDICATION_GET(cell) ((cell)->payload.lbPayload.loopbackIndication)						   

#define IX_OAM_TYPE_AND_FUNC_SET(payload, type) (((payload)->oamTypeAndFunction) = (type))
#define IX_OAM_LOOPBACK_INDICATION_SET(payload, loop) (((payload)->loopbackIndication) = (loop))

/*
 * Typedefs whose scope is limited to this file.
 */

/*
 * This structure defines a software queue that stores mbufs.
 * NOTE: The head and tail pointers are incremented each time an mbuf is added
 * to the head or removed from the tail. The range of values that the head and
 * tail pointers can assume are in the range 0....2^32. The size of this queue
 * _must_ be a power of 2. The mask is set to (size - 1). Whenever the head or
 * tail pointers are used they are masked with this mask. The following is an
 * example of how this works(size = 128, 26 elements in the queue):
 * mask = 127   (0x007f)
 * head = 35535 (0x8acf)
 * tail = 35509 (0x8ab5)
 * numElementsInQueue = (head   & mask  ) - (tail   & mask  )
 *                    = (0x8acf & 0x007f) - (0x8ab5 & 0x007f)
 *                    = (0x4f)            - (0x35)
 *                    = 79 - 53
 * numElementsInQueue = 26
 */
typedef struct
{
    volatile UINT32 head; /* Points to the head of the queue */
    volatile UINT32 tail; /* Points to the tail of the queue */
    UINT32 size;          /* The size of the queue           */
    UINT32 mask;          /* Head and tail mask              */
    IX_OSAL_MBUF *array[IX_OAMCODELET_NUM_MBUFS_IN_SW_MBUF_Q];
} IxOamCodeletSwMbufQ;


/*
 * Global Variables
 */
static UINT32 lbCorrelationTag = 0; /* Correlation Tag of Loopback in progress, see I-610 */

static UINT32 lbVci = 0;    /* VCI of Loopback in progress */
static UINT32 lbVpi = 0;    /* VPI of Loopback in progress */
static UINT32 lbPti = 0;    /* PTI of Loopback in progress */
static UINT32 lbPort = 0;   /* Port of Loopback in progress */

static int ixOamCodeletNumPorts;

/* receive software queues */
static IxOamCodeletSwMbufQ ixOamSwQueue[IX_UTOPIA_MAX_PORTS];

/* Cell statistics */
static UINT32 parentLbCellTxCount[IX_UTOPIA_MAX_PORTS]; /* Count of parent cells sent */
static UINT32 childLbCellTxCount[IX_UTOPIA_MAX_PORTS]; /* Count of LB child cells sent */

static UINT32 parentLbCellRxCount[IX_UTOPIA_MAX_PORTS]; /* Count of Rx Parent Loopback cells, 
							  i.e. Loopbacks initiated at far end */

static UINT32 childLbCellRxCount[IX_UTOPIA_MAX_PORTS]; /* Count of expected Rx Child Loopback cells */

static UINT32 childLbCellRxErrCount[IX_UTOPIA_MAX_PORTS]; /* Count of unexpected Rx Child Loopback cells, 
							    e.g. loopback not in progress, wrong VPI, VCI etc. */

static UINT32 parentLbCellRxErrCount[IX_UTOPIA_MAX_PORTS]; /* Count of unexpected Rx Parent Loopback cells,
							     e.g. LLID didn't match this CPID */

static UINT32 unsupportedOamRxCount[IX_UTOPIA_MAX_PORTS]; /* Count of OAM Rx cells that are not supported,
							    i.e. any cells != OAM LB */

static UINT32 txDoneCallbackCount[IX_UTOPIA_MAX_PORTS]; /* Count of tx done callbacks for a port */
static UINT32 replenishCallbackCount; /* Count of Rx Q replenish callbacks, all ports */

/* This CPID is set to all 1's, i.e. no specific coding structure I-610 */
static UINT8 oamCpid[IX_OAM_ITU610_LOCATION_ID_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  0xFF, 0xFF, 
				  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/* A location Id of all 1's corresponds to end point */
static UINT8 allOnesLocId[IX_OAM_ITU610_LOCATION_ID_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  0xFF, 0xFF, 
				       0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/* A location Id of all 0's corresponds to segment end point */
static UINT8 allZerosLocId[IX_OAM_ITU610_LOCATION_ID_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static UINT32 ixOamCodeletNumPortsConfigured = 0;

static IxAtmConnId oamRxConnId;
static IxAtmConnId oamTxConnId[IX_UTOPIA_MAX_PORTS];
static IxAtmdAccUserId oamRxUserId = 99; /* For debug purposes */

static BOOL ixOamCodeletInitialized = FALSE;


/*
 * Static function prototypes
 */
PRIVATE void
ixOamChildLbCellRx (IxAtmLogicalPort port, IX_OSAL_MBUF *rxMbuf);

PRIVATE IX_STATUS
ixOamParentLbCellTx (IxAtmLogicalPort port, UINT32 vpi, UINT32 vci, UINT32 pti);

PRIVATE IX_STATUS
ixOamChildLbCellTx (IxAtmLogicalPort port, IX_OSAL_MBUF *rxMbuf);

PRIVATE void
ixOamParentLbCellRx (IxAtmLogicalPort port, IX_OSAL_MBUF *rxMbuf);

PRIVATE IX_STATUS
ixOamRxLlidCheck (IxOamITU610Cell *oamCell);

PRIVATE void 
ixOamCellRxTask( void );

PRIVATE void
ixOamCellRxCallback (IxAtmLogicalPort port,
		     IxAtmdAccUserId userId,
		     IxAtmdAccPduStatus status,
		     IxAtmdAccClpStatus clp,
		     IX_OSAL_MBUF *rxMbuf);

PRIVATE IX_STATUS
ixOamCodeletOamPing(IxAtmLogicalPort port, 
		    UINT32 vpi,
		    UINT32 vci, 
		    UINT32 pti,
		    UINT32 numCells);

PRIVATE void
ixOamTxDoneCallback (IxAtmdAccUserId userId,
		     IX_OSAL_MBUF *mbufPtr);

PRIVATE void
ixOamRxFreeLowReplenishCallback (IxAtmdAccUserId userId);

PRIVATE IX_STATUS 
ixOamTxAndRetry(IxAtmConnId connId,
                IX_OSAL_MBUF * mbufPtr,
                IxAtmdAccClpStatus clp,
                UINT32 numberOfCells);


PRIVATE void
ixOamTxFlush(IX_OSAL_MBUF * mbufPtr);

PRIVATE void
ixOamRxInvalidate(IX_OSAL_MBUF * mbufPtr);

PRIVATE void ixOamQueueInit( int port );

PRIVATE BOOL 
ixOamQueueEmptyQuery (IxOamCodeletSwMbufQ *s);

PRIVATE IX_OSAL_MBUF * 
ixOamMBufQueueGet (IxOamCodeletSwMbufQ *s);

PRIVATE void 
ixOamQueuePut (IxOamCodeletSwMbufQ *s, IX_OSAL_MBUF *buf);

PRIVATE BOOL
ixOamMemCmp(UCHAR *destBuf, UCHAR *srcBuf, UINT32 len);

PRIVATE void
ixOamMemCpy(UCHAR *destBuf, UCHAR* srcBuf, UINT32 len);

PRIVATE void
ixOamMemSet(UCHAR *destBuf, UCHAR data, UINT32 len);

/* */
PUBLIC IX_STATUS
ixOamCodeletInit (UINT32 numPorts)
{
    IX_STATUS retval;
    UINT32 txRate[IX_UTOPIA_MAX_PORTS];
    UINT32 rxRate;    
    IxAtmmVc txVc;
    IxAtmmVc rxVc;
    IxAtmLogicalPort port;
    IxAtmSchedulerVcId rxVcId;
    IxAtmSchedulerVcId txVcId;
    IxAtmNpeRxVcId npeRxVcId;
    IxOsalThread tid;
    IxOsalThreadAttr threadAttr;
    char *pThreadName = "OAM Receive";

    if (ixOamCodeletInitialized)
    {
	IX_OAM_CODELET_LOG_ERROR("Already initialized");
	return IX_FAIL;
    }

    /* Check parameters */
    if (numPorts < 1 || numPorts > IX_UTOPIA_MAX_PORTS)
    {
	IX_OAM_CODELET_LOG_ERROR("ixOamCodeletInit(): numPorts (%u) invalid\n", numPorts);
	return IX_FAIL;
    }

    ixOamCodeletNumPorts = numPorts;

    /* Check how many ports have been configured, rxRate not used */
    for (port =0; port< (IxAtmLogicalPort)numPorts; port++)
    {
	if (ixAtmmPortQuery (port, &txRate[port], &rxRate) != IX_SUCCESS)
	{
	    IX_OAM_CODELET_LOG_ERROR("ixOamCodeletInit(): numPorts (%u) not configured, %u ports configured\n", 
				     numPorts, port);	    
	}
    }

    /* Initialize stats */
    for (port = 0; port < IX_UTOPIA_MAX_PORTS; port++)
    {
	parentLbCellTxCount[port] = 0;
	childLbCellTxCount[port] = 0;
	parentLbCellRxCount[port] = 0;
	childLbCellRxCount[port] = 0;
	childLbCellRxErrCount[port] = 0;
	parentLbCellRxErrCount[port] = 0;
	unsupportedOamRxCount[port] = 0;
	txDoneCallbackCount[port] = 0;	
    }
    replenishCallbackCount = 0; /* 1 for all ports */

    /* Setup the OAM Tx Port Channels */
    for (port=0; port< (IxAtmLogicalPort)numPorts; port++)
    {
	ixOsalMemSet(&txVc, 0, sizeof(txVc));

	/* Setup Tx Vc descriptor */
	txVc.vpi = IX_ATMDACC_OAM_TX_VPI;
	txVc.vci = IX_ATMDACC_OAM_TX_VCI;
	txVc.direction = IX_ATMM_VC_DIRECTION_TX;
	txVc.trafficDesc.atmService = IX_ATM_UBR;
	txVc.trafficDesc.pcr = txRate[port];
	
	/* Setup tx VC, N.B. TxVcId not used in this codelet, 
	 * would typically be used for Vc Deregister
	 */
	retval = ixAtmmVcRegister (port,
				   &txVc,
				   &txVcId);

	if (retval != IX_SUCCESS)
	{
	    IX_OAM_CODELET_LOG_ERROR("Failed to register Tx Port VC\n");
	    return IX_FAIL;
	}


	retval = ixAtmdAccTxVcConnect (port,
				       IX_ATMDACC_OAM_TX_VPI,
				       IX_ATMDACC_OAM_TX_VCI,
				       IX_ATMDACC_OAM,
				       port, /* set userId to port */
				       ixOamTxDoneCallback,
				       &oamTxConnId[port]);

	if (retval != IX_SUCCESS)
	{
	    IX_OAM_CODELET_LOG_ERROR ("Failed to connect Tx Port Channel\n");
	    return IX_FAIL;
	}

        /* setup the Rx sw queues */
        ixOamQueueInit( port );
    }

    /* 
     * Setup OAM Rx Channel
     * N.B. OAM traffic for all ports is received on this VC
     */

    /*Set thread attributes*/
    threadAttr.name = pThreadName;
    threadAttr.stackSize = IX_ATMCODELET_QMGR_DISPATCHER_THREAD_STACK_SIZE;
    threadAttr.priority = IX_ATMCODELET_QMGR_DISPATCHER_PRIORITY;

    /* start a receive task */
    if (ixOsalThreadCreate(&tid,
			   &threadAttr,
    		           (IxOsalVoidFnVoidPtr)ixOamCellRxTask, 
    		           NULL) != IX_SUCCESS) 
    { 
        IX_OAM_CODELET_LOG_ERROR ("Error spawning SduSend task\n"); 
        return IX_FAIL; 
    }  	

    if(IX_SUCCESS != ixOsalThreadStart(&tid))
    {
       printf("Error starting dispatch task\n");
       return IX_FAIL;
    } 

    ixOsalMemSet(&rxVc, 0, sizeof(rxVc));

    /* Setup Rx Vc descriptor */
    rxVc.vpi = IX_ATMDACC_OAM_RX_VPI;
    rxVc.vci = IX_ATMDACC_OAM_RX_VCI;
    rxVc.direction = IX_ATMM_VC_DIRECTION_RX;
    rxVc.trafficDesc.atmService = IX_ATM_UBR;
    rxVc.trafficDesc.pcr = 0; /* Ignored for Rx */

    /* Setup rx VC, N.B. RxVcId not used in this codelet, 
     * would typically be used for Vc Deregister
     */  

    retval = ixAtmmVcRegister (IX_ATMDACC_OAM_RX_PORT,
			       &rxVc,
			       &rxVcId);
    
    if (retval != IX_SUCCESS)
    {
	IX_OAM_CODELET_LOG_ERROR("Failed to register Rx Channel\n");
	return IX_FAIL;
    }

    /* Connect Rx VC, N.B. npeRxVcId not used again
     */
    retval = ixAtmdAccRxVcConnect (IX_ATMDACC_OAM_RX_PORT,
				   IX_ATMDACC_OAM_RX_VPI,
				   IX_ATMDACC_OAM_RX_VCI,
				   IX_ATMDACC_OAM,
				   IX_ATM_RX_B, /* low priority Q */
				   oamRxUserId,
				   ixOamCellRxCallback,
				   IX_ATMDACC_DEFAULT_REPLENISH_COUNT,
				   &oamRxConnId,
				   &npeRxVcId);

    if (retval != IX_SUCCESS)
    {
	IX_OAM_CODELET_LOG_ERROR("Failed to connect Rx Channel\n");
	return IX_FAIL;
    }
   
    /* Replenish Rx then register replenish callback */
    ixOamRxFreeLowReplenishCallback (oamRxUserId);
    
    retval = ixAtmdAccRxVcFreeLowCallbackRegister (oamRxConnId,
						   IX_OAM_REPLENISH_WATERMARK,
						   ixOamRxFreeLowReplenishCallback);
   
    if (retval != IX_SUCCESS)
    {
	IX_OAM_CODELET_LOG_ERROR("Failed to register Rx replenish callback\n");
	return IX_FAIL;
    }

    /* enable the oam vc */
   
    retval = ixAtmdAccRxVcEnable (oamRxConnId);
   
    if (retval != IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, "Failed to enable Rx VC \n", 0, 0, 0, 0, 0, 0);
	return retval;
    }

    /* Set the number of ports configured */
    ixOamCodeletNumPortsConfigured = numPorts;

    /* set initialised flag */
    ixOamCodeletInitialized = TRUE;

    IX_OAM_CODELET_LOG ("Initialization Phase Complete\n");

    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOamCodeletOamF5EtePing(IxAtmLogicalPort port, 
			 UINT32 vpi,
			 UINT32 vci, 
			 UINT32 numCells)
{
    return ixOamCodeletOamPing(port,
			       vpi,
			       vci,
			       IX_OAM_ITU610_F5_ETE_PTI,
			       numCells);
}

PUBLIC IX_STATUS
ixOamCodeletOamF5SegPing(IxAtmLogicalPort port, 
			 UINT32 vpi,
			 UINT32 vci, 
			 UINT32 numCells)
{
    return ixOamCodeletOamPing(port,
			       vpi,
			       vci,
			       IX_OAM_ITU610_F5_SEG_PTI,
			       numCells);
}


PUBLIC IX_STATUS
ixOamCodeletOamF4EtePing(IxAtmLogicalPort port, 
			 UINT32 vpi,
			 UINT32 numCells)
{
    return ixOamCodeletOamPing(port,
			       vpi,
			       IX_OAM_ITU610_F4_ETE_VCI,
			       0, /* PTI is don't care for F4, use 0 */
			       numCells);
}

PUBLIC IX_STATUS
ixOamCodeletOamF4SegPing(IxAtmLogicalPort port, 
			 UINT32 vpi,
			 UINT32 numCells)
{
    return ixOamCodeletOamPing(port,
			       vpi,
			       IX_OAM_ITU610_F4_SEG_VCI,
			       0, /* PTI is don't care for F4, use 0 */
			       numCells);
}



PRIVATE IX_STATUS
ixOamCodeletOamPing(IxAtmLogicalPort port, 
		    UINT32 vpi,
		    UINT32 vci, 
		    UINT32 pti,
		    UINT32 numCells)
{
    UINT32 sendCount = 0;

    if (!ixOamCodeletInitialized)
    {
	IX_OAM_CODELET_LOG_ERROR("Codelet not initialized\n");
	return IX_FAIL;
    }

    if (port >= (IxAtmLogicalPort)ixOamCodeletNumPortsConfigured)
    {
	IX_OAM_CODELET_LOG_ERROR("Port not configured\n");
	return IX_FAIL;
    }

    if (numCells == 0)
    {
	IX_OAM_CODELET_LOG_ERROR("numCells should be > 0\n");
	return IX_FAIL;
    }

    /* Setup LB info */
    lbVpi = vpi;
    lbVci = vci;
    lbPti = pti;
    lbPort = port;

    while (numCells--)
    {

	/* Notify user that a loopback cell is being sent */
	IX_OAM_CODELET_LOG ("Sending loopback cell %u\n", sendCount++);

	/* Send the first cell */
	if( ixOamParentLbCellTx (port, vpi, vci, pti) != IX_SUCCESS )
        {
	    IX_OAM_CODELET_LOG_ERROR("Failed to send parent loopback cell #%u",sendCount);
	    return IX_FAIL;
        }

	/*
	 * A message is displayed at the command shell if a correct loopback child cell has been received in
	 * response to the sending of the parent cell. No message is displayed if no correct child cell
	 * has been received
	 */

	/* Wait for 5 seconds for response, see ITU-610 */
	ixOsalSleep (IX_OAM_ITU610_LB_TIMEOUT_PERIOD_MSECS);
    }

    return IX_SUCCESS;
}

void
ixOamCodeletShow (void)
{
    UINT32 i;

    IX_OAM_CODELET_LOG("replenishCallbackCount = %u\n",
		       replenishCallbackCount);

    for (i = 0; i < ixOamCodeletNumPortsConfigured; i++)
    {
	IX_OAM_CODELET_LOG("OAM codelet stats for Port %u\n", i);

	IX_OAM_CODELET_LOG("parentLbCellTxCount \t= %u\n",
			   parentLbCellTxCount[i]);
	IX_OAM_CODELET_LOG("childLbCellTxCount \t= %u\n",
			   childLbCellTxCount[i]);
	IX_OAM_CODELET_LOG("parentLbCellRxCount \t= %u\n",
			   parentLbCellRxCount[i]);
	IX_OAM_CODELET_LOG("childLbCellRxCount \t= %u\n",
			   childLbCellRxCount[i]);
	IX_OAM_CODELET_LOG("childLbCellRxErrCount \t= %u\n",
			   childLbCellRxErrCount[i]);
	IX_OAM_CODELET_LOG("parentLbCellRxErrCount \t= %u\n",
			   parentLbCellRxErrCount[i]);
	IX_OAM_CODELET_LOG("unsupportedOamRxCount \t= %u\n",
			   unsupportedOamRxCount[i]);
	IX_OAM_CODELET_LOG("txDoneCallbackCount \t= %u\n",
			   txDoneCallbackCount[i]);

	IX_OAM_CODELET_LOG("\n");
    }
}

PRIVATE void 
ixOamCellRxTask( void )
{
    IxOamITU610Cell *rxCell;
    UINT32 pti;
    UINT32 vci;
    UINT32 vpi;
    UINT32 oamRxHdr=0x0;

    IX_OSAL_MBUF *rxMbuf;

    int port;

    while(1)
    {
        for( port=0; port<ixOamCodeletNumPorts; port++ )
        {
            if(!ixOamQueueEmptyQuery(&ixOamSwQueue[port]))
            {
                rxMbuf = ixOamMBufQueueGet(&ixOamSwQueue[port]);
                IX_OSAL_ASSERT( rxMbuf != NULL );
    
                /* invalidate cache */
                ixOamRxInvalidate( rxMbuf );
                
                rxCell = (IxOamITU610Cell *)(IX_OSAL_MBUF_MDATA(rxMbuf));
                
                /* Read the VCI & Payload Type */
		IX_OAM_HEADER_GET(rxCell, oamRxHdr);
		vci = IX_OAM_VCI_GET(oamRxHdr);
		vpi = IX_OAM_VPI_GET(oamRxHdr);
		pti = IX_OAM_PTI_GET(oamRxHdr);
                
                /* Ensure access layer delivered a correct OAM cell */
                IX_OSAL_ASSERT ((vci == IX_OAM_ITU610_F4_SEG_VCI) || (vci == IX_OAM_ITU610_F4_ETE_VCI) || 
                           (pti == IX_OAM_ITU610_F5_ETE_PTI) || (pti == IX_OAM_ITU610_F5_SEG_PTI));                
                
                /* Is it an Loopback cell ? */
                if ( (IX_OAM_TYPE_AND_FUNC_GET(rxCell)) == IX_OAM_ITU610_TYPE_FAULT_MAN_LB )
                {
                    /* Is Parent Loopback Indication field in cell set? */
                    if ( (IX_OAM_LOOPBACK_INDICATION_GET(rxCell)) == IX_OAM_ITU610_LB_INDICATION_PARENT )
                    {
                        ixOamParentLbCellRx (port, rxMbuf);
                    }
                    /* Otherwise child cell, i.e. response to loopback cell sent earlier */
                    else 
                    {
                        ixOamChildLbCellRx(port, rxMbuf);
                    }
                }
                else
                {
                    unsupportedOamRxCount[port]++;
                    /* free the buffer */
                    ixAtmUtilsMbufFree (rxMbuf);
                }
            }
        }
    
	/* share the CPU */
        ixOsalSleep(IX_OAM_RX_QUEUE_POLL_INTERVAL);
    }
}


PRIVATE void
ixOamCellRxCallback (IxAtmLogicalPort port,
		     IxAtmdAccUserId userId,
		     IxAtmdAccPduStatus status,
		     IxAtmdAccClpStatus clp,
		     IX_OSAL_MBUF *rxMbuf)
{
    IX_OSAL_ASSERT(userId == oamRxUserId);
    
    /* Status should always be OAM Valid */
    IX_OSAL_ASSERT(status == IX_ATMDACC_OAM_VALID);

    /* queue the mbuf to an rx task */
    ixOamQueuePut( &ixOamSwQueue[port], rxMbuf );
    
}    


PRIVATE void
ixOamChildLbCellRx (IxAtmLogicalPort port, IX_OSAL_MBUF *rxMbuf)
{
    UINT32 pti;
    UINT32 vpi;
    UINT32 vci;
    UINT32 oamHdr = 0x0;
    IxOamITU610Cell *rxCell;
    IxOamITU610LbPayload *lbPayload;

    rxCell = (IxOamITU610Cell *)(IX_OSAL_MBUF_MDATA(rxMbuf));

    /* Read the VPI, VCI & Payload Type */
    IX_OAM_HEADER_GET(rxCell, oamHdr);
    vci = IX_OAM_VCI_GET(oamHdr);
    vpi = IX_OAM_VPI_GET(oamHdr);
    pti = IX_OAM_PTI_GET(oamHdr);

    /* Check if the VPI, VCI and Port match */
    if ((vpi == lbVpi) && (vci == lbVci) && (port == (IxAtmLogicalPort)lbPort))
    {
	/* Check if F4 flow (F4 VCI) or PTI matches */
	if ((vci == IX_OAM_ITU610_F4_SEG_VCI) || (vci == IX_OAM_ITU610_F4_ETE_VCI) || (pti == lbPti))
	{
	    /* Setup the data pointers */
	    lbPayload = &(((IxOamITU610Cell *)(IX_OSAL_MBUF_MDATA(rxMbuf)))->payload.lbPayload);
	    
	    /* Source Id correct, all Ones supported only in this codelet */
	    if ( ixOamMemCmp(lbPayload->sourceId, (UCHAR *)allOnesLocId, IX_OAM_ITU610_LOCATION_ID_LEN) == 0 )
	    {
		/* Correlation tag correct? */
		if ( ixOamMemCmp(lbPayload->correlationTag, (UCHAR *)(&lbCorrelationTag), IX_OAM_ITU610_LB_CORRELATION_TAG_LEN) == 0)
		{
		    /* Notify user that a ping has been successful,
		     * N.B. the user is not notified if ping has failed
		     */
		    IX_OAM_CODELET_LOG ("OAM ping reply for VPI:%u VCI:%u PTI:%u on Port:%u\n",
						 lbVpi, lbVci, lbPti, lbPort);

		    /* free the buffer */
		    ixAtmUtilsMbufFree (rxMbuf);
		    childLbCellRxCount[port]++;
		    return;
		}
	    }
	}
    }

    /* free the buffer */
    ixAtmUtilsMbufFree (rxMbuf);
    childLbCellRxErrCount[port]++;
}

/*
 *
 * Transmit loopback parent cell
 */
PRIVATE IX_STATUS
ixOamParentLbCellTx (IxAtmLogicalPort port, UINT32 vpi, UINT32 vci, UINT32 pti)
{    
    IxOamITU610Cell *txCell;
    IxOamITU610LbPayload *lbPayload;
    IX_OSAL_MBUF *txMbuf;
    IX_STATUS retval = IX_FAIL;
    UINT32 lockKey;
    UINT32 oamHdr = 0x0;

    /* Get a Tx mbuf */
    ixAtmUtilsMbufGet(IX_ATM_OAM_CELL_SIZE_NO_HEC, &txMbuf);
    if (txMbuf != NULL)
    {		
	/* set the packet header len */
	IX_OSAL_MBUF_PKT_LEN(txMbuf) = IX_OSAL_MBUF_MLEN(txMbuf);

	txCell = (IxOamITU610Cell *)(IX_OSAL_MBUF_MDATA(txMbuf));

	/* Setup shortcut pointers */
	lbPayload = &(txCell->payload.lbPayload);
	
	/* Set the OAM function type to LB */
	IX_OAM_TYPE_AND_FUNC_SET(lbPayload, IX_OAM_ITU610_TYPE_FAULT_MAN_LB);
	
	/* Setup the loopback indication */
	IX_OAM_LOOPBACK_INDICATION_SET(lbPayload, IX_OAM_ITU610_LB_INDICATION_PARENT);
	
	/* Increment the correlation tag and write this into the cell */
 	lbCorrelationTag++;
	ixOamMemCpy(lbPayload->correlationTag, (UCHAR *)(&lbCorrelationTag), IX_OAM_ITU610_LB_CORRELATION_TAG_LEN);
	
	/* Set Loopback Location Id */
	ixOamMemCpy(lbPayload->llid, allOnesLocId, IX_OAM_ITU610_LOCATION_ID_LEN);
	
	/* Store the source Id as the CPID */
	ixOamMemCpy(lbPayload->sourceId, oamCpid, IX_OAM_ITU610_LOCATION_ID_LEN);
	
	/* Set the reserved fields to 0x6a */
	ixOamMemSet(lbPayload->reserved, 
		    IX_OAM_ITU610_RESERVED_BYTE_VALUE, 
		    IX_OAM_ITU610_LB_RESERVED_BYTES_LEN);
	
	/* Set the transmit VPI */
	IX_OAM_VPI_SET(oamHdr, vpi);

	/* Set the transmit VCI TX Vci */
	IX_OAM_VCI_SET(oamHdr, vci);
	
	/* Set the PTI */
	IX_OAM_PTI_SET(oamHdr, pti);

	/* Set the header in buffer */
	IX_OAM_HEADER_SET(txCell, oamHdr);

        /* flush the mbuf frm cache */
        ixOamTxFlush( txMbuf );

	/* Transmit the cell, not reentrant on a VC basis so protect */
	lockKey = ixOsalIrqLock();
	retval = ixOamTxAndRetry (oamTxConnId[port],
			          txMbuf,
			          0,  /* CLP 0 */
			          1); /* Cells per packet */
	ixOsalIrqUnlock (lockKey);

	if (retval == IX_SUCCESS)
	{
	    /* Increment the loopback out counter */
	    parentLbCellTxCount[port]++;
	}
	else
	{
	    /* Release the MBUF */
	    ixAtmUtilsMbufFree (txMbuf);
	}
    }
    else
    {
	IX_OAM_CODELET_IRQ_SAFE_LOG_ERROR("ixOamParentLbCellTx: Failed to get a buffer for transmit");
    }

    return retval;
}

PRIVATE void
ixOamParentLbCellRx (IxAtmLogicalPort port, IX_OSAL_MBUF *rxMbuf)
{     
    /* Loopback location ID correct? */
    if (ixOamRxLlidCheck (((IxOamITU610Cell *)(IX_OSAL_MBUF_MDATA(rxMbuf)))) == IX_SUCCESS)
    {
	parentLbCellRxCount[port]++;

	/* Send loopback child cell back to originator */
	ixOamChildLbCellTx (port, rxMbuf);
    }
    else
    {
	parentLbCellRxErrCount[port]++;

	/* free the buffer */
	ixAtmUtilsMbufFree (rxMbuf);
    }
}

/*
 * Reuse the received parent LB cell
 * and modify some of the fields
 */
PRIVATE IX_STATUS
ixOamChildLbCellTx (IxAtmLogicalPort port, IX_OSAL_MBUF *rxMbuf)
{    
    IxOamITU610Cell *txCell;
    IxOamITU610LbPayload *lbPayload;
    IX_STATUS retval = IX_FAIL;
    UINT32 lockKey;

    /* Setup a pointer to the cell data */
    txCell = (IxOamITU610Cell *) IX_OSAL_MBUF_MDATA(rxMbuf);
        
    /* Setup pointers to Lb fields */
    lbPayload = &(txCell->payload.lbPayload);
    
    /* Change the loopback indication flag */
    IX_OAM_LOOPBACK_INDICATION_SET(lbPayload, IX_OAM_ITU610_LB_INDICATION_CHILD);

    /* Set the LLID to the CPID */
    ixOamMemCpy(lbPayload->llid, oamCpid, IX_OAM_ITU610_LOCATION_ID_LEN);

    /* flush the mbuf from cache */
    ixOamTxFlush( rxMbuf );

    /* Transmit the cell, not reentrant on a VC basis so protect */
    lockKey = ixOsalIrqLock();
    retval =  ixOamTxAndRetry (oamTxConnId[port],
			       rxMbuf,
			       0, /* CLP 0 */
			       1); /* Cells per packet */
    ixOsalIrqUnlock (lockKey);

    if (retval == IX_SUCCESS)
    {
	/* Increment the loopback out counter */
	childLbCellTxCount[port]++;
    }
    else
    {
	/* Release the MBUF */
	ixAtmUtilsMbufFree (rxMbuf);
    }

    return retval;
}

PRIVATE IX_STATUS
ixOamRxLlidCheck (IxOamITU610Cell *oamCell)
{
    UINT8 *llid;
    UINT32 pti;
    UINT32 vci;
    UINT32 oamHdr = 0x0;

    /* Get the LLID from the payload */
    llid = oamCell->payload.lbPayload.llid;

    /* Get the Pt and Vci */
    IX_OAM_HEADER_GET(oamCell, oamHdr);
    vci = IX_OAM_VCI_GET(oamHdr);
    pti = IX_OAM_PTI_GET(oamHdr);

    /* Compare LLID to the CPID */
    if (ixOamMemCmp (llid, oamCpid, IX_OAM_ITU610_LOCATION_ID_LEN) == 0)
    {
	return IX_SUCCESS;
    }
    /* Compare to all 1's, i.e. end point  */
    else if (ixOamMemCmp (llid, allOnesLocId, IX_OAM_ITU610_LOCATION_ID_LEN) == 0)
    {
	return IX_SUCCESS;
    }
    /* is this a segment loopback cell */
    else if ((vci == IX_OAM_ITU610_F4_SEG_VCI) || (pti == IX_OAM_ITU610_F5_SEG_PTI))
    {
	/* Compare to all 0's */
	if (ixOamMemCmp (llid, allZerosLocId, IX_OAM_ITU610_LOCATION_ID_LEN) == 0)
	{
	    return IX_SUCCESS;
	}
    }

    return IX_FAIL;
}

PRIVATE void
ixOamTxDoneCallback (IxAtmdAccUserId userId,
		     IX_OSAL_MBUF *mbufPtr)
{
    ixAtmUtilsMbufFree(mbufPtr);

    txDoneCallbackCount[userId]++; /* userId == port */
}

PRIVATE void
ixOamRxFreeLowReplenishCallback (IxAtmdAccUserId userId)
{
    IX_OSAL_MBUF *mBuf;
    UINT32 numFreeEntries;
    UINT32 cnt;
    IX_STATUS retval;
   


    IX_OSAL_ASSERT(userId == oamRxUserId);
    
    retval = ixAtmdAccRxVcFreeEntriesQuery (oamRxConnId, &numFreeEntries);
    if (retval != IX_SUCCESS)
    {
	IX_OAM_CODELET_IRQ_SAFE_LOG_ERROR("Failed to query depth of Oam Rx Free Q");
	return;
    }

    /* Replenish Rx buffers  */
    for (cnt=0; cnt<numFreeEntries; cnt++)
    {
     
	ixAtmUtilsMbufGet(IX_ATM_OAM_CELL_SIZE_NO_HEC, &mBuf);

	if (mBuf == NULL)
	{
	    IX_OAM_CODELET_IRQ_SAFE_LOG_ERROR("Failed to get rx free buffer");
	    return;
	}
	
	/* 
	 * Set the number of bytes of data in this MBUF to 1 OAM cell
	 */
      
	IX_OSAL_MBUF_MLEN(mBuf) = IX_ATM_OAM_CELL_SIZE_NO_HEC;
	
        /* invalidate cache */
        ixOamRxInvalidate( mBuf );
        
	/* Send free buffers to NPE */
	retval = ixAtmdAccRxVcFreeReplenish (oamRxConnId, mBuf);
      
	if (retval != IX_SUCCESS)
	{
	    /* Free the allocated buffer */
	    ixAtmUtilsMbufFree(mBuf);
	    IX_OAM_CODELET_IRQ_SAFE_LOG_ERROR("Failed to pass Oam Rx free buffers to Atmd");
	    return;
	}	
    }
    replenishCallbackCount++;
}

/* ---------------------------------------------------
*/
PRIVATE IX_STATUS 
ixOamTxAndRetry(IxAtmConnId connId,
                IX_OSAL_MBUF * mbufPtr,
                IxAtmdAccClpStatus clp,
                UINT32 numberOfCells)
{
     IX_STATUS status = IX_FAIL;
     int retryCount = IX_OAM_TX_RETRY_COUNT_MAX;
        
     /* retry until the PDU is successfully submitted,
        or the submit fails, or the maximum number of
        retries is exceede.
        
     */ 
     while(retryCount-- )
     {
        status = ixAtmdAccTxVcPduSubmit (connId,
                                         mbufPtr,
                                         clp,
                                         numberOfCells);

        if( status != IX_ATMDACC_BUSY )
        {
            return status;
        }
        ixOsalSleep(IX_OAM_TX_RETRY_DELAY);

     }
     return IX_FAIL;
}
                        

/* ---------------------------------------------------
*/
PRIVATE void
ixOamTxFlush(IX_OSAL_MBUF * mbufPtr)
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
ixOamRxInvalidate(IX_OSAL_MBUF * mbufPtr)
{
   while (mbufPtr != NULL)
      {
      IX_OSAL_CACHE_INVALIDATE(IX_OSAL_MBUF_MDATA(mbufPtr), IX_OSAL_MBUF_MLEN(mbufPtr));
      mbufPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr);
       }
    return;
}


/* --------------------------------------------------------------
   Software queue manipulation functions.
   -------------------------------------------------------------- */
PRIVATE void ixOamQueueInit( int port )
{
    ixOsalMemSet(&ixOamSwQueue[port], 0, sizeof(ixOamSwQueue[port]));
    ixOamSwQueue[port].size = IX_OAMCODELET_NUM_MBUFS_IN_SW_MBUF_Q;
    ixOamSwQueue[port].mask = IX_OAMCODELET_NUM_MBUFS_IN_SW_MBUF_Q - 1;
}

PRIVATE BOOL 
ixOamQueueEmptyQuery (IxOamCodeletSwMbufQ *s)
{
    return (s->head == s->tail);
}

PRIVATE IX_OSAL_MBUF * 
ixOamMBufQueueGet (IxOamCodeletSwMbufQ *s)
{
    IX_OSAL_ASSERT (s->head != s->tail);
    return (s->array[s->tail++ & s->mask]);
}

PRIVATE void 
ixOamQueuePut (IxOamCodeletSwMbufQ *s, IX_OSAL_MBUF *buf)
{
    IX_OSAL_ASSERT (s->head - s->tail != s->size);
    s->array[s->head++ & s->mask] = buf;
}


/* ---------------------------------------------------
 */
PRIVATE BOOL
ixOamMemCmp(UCHAR *destBuf, UCHAR* srcBuf, UINT32 len)
{
    /* params check */
    if ( (srcBuf == NULL) || (destBuf == NULL) || (len <= 0) )
	return 1;

    /* data check */
    while(len--)
    {
	if ( *srcBuf++ != *destBuf++ )
	    return 1;
    }
    
    /* success */
    return 0;
}

/* ---------------------------------------------------
 */
PRIVATE void
ixOamMemCpy(UCHAR *destBuf, UCHAR* srcBuf, UINT32 len)
{
    /* params check */
    if ( (srcBuf == NULL) || (destBuf == NULL) || (len <= 0) )
	return;

    /* data copy */
    while(len--)
    {
	*destBuf++ = *srcBuf++;
    }

    return;
}

/* ---------------------------------------------------
 */
PRIVATE void
ixOamMemSet(UCHAR *destBuf, UCHAR data, UINT32 len)
{
    /* params check */
    if ( (destBuf == NULL) )
	return;

    /* set the data */
    while (len--)
    {
	*destBuf++ = data;
    }

    return;
}
                      
