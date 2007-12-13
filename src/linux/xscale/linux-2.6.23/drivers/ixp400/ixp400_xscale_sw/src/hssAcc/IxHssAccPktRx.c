/**
 * @file IxHssAccPktRx.c
 *
 * @author Intel Corporation
 * @date 10 Jan 2002
 *
 * @brief This file contains the implementation of the private API for the
 * Rx Module.
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
#include "IxHssAccPktRx_p.h"
#include "IxOsal.h"
#include "IxHssAccError_p.h"
#include "IxHssAccPCM_p.h"
#include "IxHssAccCommon_p.h"

/*
 * Defines
 */

/*
 * Typedefs
 */
typedef struct
{    
    unsigned rxCallbacks;
    unsigned qReadFailures;
    unsigned rxFreeQWriteFails;
    unsigned rxQWriteOverflows;
    unsigned rxFreeReplenishs;
    unsigned maxEntriesInRxQ;
    unsigned rxInvalidBuffers;
} IxHssAccPktRxStats;


/**
 * Variable declarations global to this file only.  
 * Externs are followed by static variables.
 */
static IxHssAccPktRxStats ixHssAccPktRxStats;

IxQMgrQId ixHssAccPktRxQId[IX_HSSACC_HSS_PORT_MAX] = 
{    IX_NPE_A_QMQ_HSS0_PKT_RX,       
     IX_NPE_A_QMQ_HSS1_PKT_RX
};

/*
 *Function :ixHssAccPktRxFreeLowCallback 
 */
void 
ixHssAccPktRxFreeLowCallback (IxQMgrQId qId, 
			      IxQMgrCallbackId cbId)
{
    /* This function is called from within an ISR */
    IxHssAccHdlcPort hdlcPortId = IX_HSSACC_PKT_CBID_HDLC_MASK & (unsigned)cbId;
    IxHssAccHssPort hssPortId  = cbId >> IX_HSSACC_PKT_CBID_HSS_OFFSET;
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering "
					   "ixHssAccPktRxFreeLowCallback\n"));


    /* if the notification was from a Q that is a packetied RX Free Q*/
#ifndef NDEBUG
	if ((hssPortId < hssPortMax) && 
	    (hdlcPortId < IX_HSSACC_HDLC_PORT_MAX))
	{
	    ixHssAccPCMRxFreeLowCallbackRun (hssPortId, hdlcPortId);
	    
	}
	else
	{
	    IX_HSSACC_DEBUG_OFF 
		(IX_HSSACC_REPORT_ERROR ("ixHssAccPktRxFreeReqCallback: The"
					 " hdlcPortId or hssPortId has been "
					 "corrupted by the NPE\n"));
	}
#else
	ixHssAccPCMRxFreeLowCallbackRun (hssPortId, hdlcPortId);
#endif
	IX_HSSACC_DEBUG_OFF 
	    (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting "
			       "ixHssAccPktRxFreeLowCallback \n"));
}


/*
 *Function :ixHssAccPktRxCallback
 */
void 
ixHssAccPktRxCallback (IxQMgrQId qId, 
		       IxQMgrCallbackId cbId)
{
    /* This function is called from within an ISR */
    IX_STATUS status;
    UINT32 chainCount = 0;
    unsigned numOfEntriesInRxQ = 0;
    UINT32 qEntry = 0;
    IX_OSAL_MBUF *pRootBuf = NULL;
    IxHssAccHssPort hssPortId = cbId;
    IxHssAccHdlcPort hdlcPortId ;

    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering "
                        "ixHssAccPktRxCallback \n"));
   
    do 
    {
        /* Read the queue entry from the queue */
        status = ixQMgrQRead (qId, &qEntry); 
        if (status == IX_SUCCESS)
        {     
#ifndef NDEBUG
            if (qEntry != 0)
            {
#endif
                /* Extract the HDLC Channel Number from the queue entry */
                hdlcPortId = qEntry & IX_HSSACC_QM_Q_CHAN_NUM_MASK;

                /* Copy relevant fields from the NPE shared region to the OS dependant region */
                pRootBuf = ixHssAccComMbufFromNpeFormatConvert (qEntry, TRUE, &chainCount);


#ifndef NDEBUG
                /* Check if the HSS Port Id is lower than the max hssPortId */
                /* AND if the HDLC Port Id is lower than the max hdlcportId */
                if (!(IX_HSSACC_ENUM_INVALID (hssPortId, hssPortMax)) && 
                    !(IX_HSSACC_ENUM_INVALID (hdlcPortId, IX_HSSACC_HDLC_PORT_MAX)))
                {
#endif
                    ixHssAccPCMRxCallbackRun (hssPortId,
                                              hdlcPortId,
                                              pRootBuf,
                                              IX_HSSACC_IX_NE_SHARED_ERR_CNT(pRootBuf),
                                              IX_HSSACC_IX_NE_SHARED_STATUS (pRootBuf),
                                              IX_HSSACC_IX_NE_SHARED_PKT_LEN(pRootBuf));

                    ixHssAccPktRxStats.rxCallbacks++;
                    numOfEntriesInRxQ++;
                    
#ifndef NDEBUG      
                } /* 
                   * end of if (!(IX_HSSACC_ENUM_INVALID (hssPortId, hssPortMax)) && 
                   * !(IX_HSSACC_ENUM_INVALID (hdlcPortId, IX_HSSACC_HDLC_PORT_MAX)))
                   */
                else
                {
                    IX_HSSACC_DEBUG_OFF (IX_HSSACC_REPORT_ERROR ("ixHssAccPktRxCallback: An invalid buffer "
                                                                 "was received.\n"));
                    
                    /* Increment the Rx Invalid buffers count by one */
                    ixHssAccPktRxStats.rxInvalidBuffers++;
                } /*end of if - else */
#endif
                if(ixHssAccPCMnoBuffersInUseCountDec (hssPortId , hdlcPortId, chainCount) != IX_SUCCESS)
                {
                    IX_HSSACC_DEBUG_OFF (IX_HSSACC_REPORT_ERROR ("ixHssAccPktRxCallback: "
                                                                 "ixHssAccPCMinUseCountDec "
                                                                 "failed while decrementing usage count.\n"));
                }
#ifndef NDEBUG
            } /* end of if (qEntry != NULL) */
            else
            {
                IX_HSSACC_DEBUG_OFF (IX_HSSACC_REPORT_ERROR ("ixHssAccPktRxCallback: An invalid buffer "
                                                             "was received.\n"));

                /* Increment the Rx Invalid buffers count by one */
                ixHssAccPktRxStats.rxInvalidBuffers++;
            } /* end of if - else */
#endif
        } /* end of if (status == IX_SUCCESS) */
        else if (status == IX_FAIL)
        {
            IX_HSSACC_DEBUG_OFF
            (IX_HSSACC_REPORT_ERROR ("ixHssAccPktRxCallback: ixQMgrQRead"
                         " failed while reading from PktRx Q.\n"));
            ixHssAccPktRxStats.qReadFailures++;
        }
    } while (status == IX_SUCCESS);

    if (numOfEntriesInRxQ > ixHssAccPktRxStats.maxEntriesInRxQ)
    {
    ixHssAccPktRxStats.maxEntriesInRxQ = numOfEntriesInRxQ;
    }
    
    IX_HSSACC_DEBUG_OFF
    (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
               "Exiting ixHssAccPktRxCallback\n"));
}


/**
 * Function definition: ixHssAccPktRxShow
 */
void ixHssAccPktRxShow (void)
{
   
    printf ("\nixHssAccPktRxShow:\n");
    printf ("      rxs: %d \t    rxFreeRepls: %d\n",
        ixHssAccPktRxStats.rxCallbacks,
        ixHssAccPktRxStats.rxFreeReplenishs);
    printf ("readFails: %d \t writeOverFlows: %d \t  invalidBuffers: %d\n",
        ixHssAccPktRxStats.qReadFailures,
        ixHssAccPktRxStats.rxQWriteOverflows,
        ixHssAccPktRxStats.rxInvalidBuffers);
    printf (" maxInRxQ: %d \t     writeFails: %d\n",
        ixHssAccPktRxStats.maxEntriesInRxQ,
        ixHssAccPktRxStats.rxFreeQWriteFails);
}


/**
 * Function definition: ixHssAccPktRxStatsInit
 */
void ixHssAccPktRxStatsInit (void)
{
    ixHssAccPktRxStats.rxCallbacks   = 0;
    ixHssAccPktRxStats.qReadFailures = 0;
    ixHssAccPktRxStats.rxFreeQWriteFails = 0;
    ixHssAccPktRxStats.rxQWriteOverflows = 0;
    ixHssAccPktRxStats.rxFreeReplenishs = 0;
    ixHssAccPktRxStats.maxEntriesInRxQ = 0;
    ixHssAccPktRxStats.rxInvalidBuffers = 0;
}


/*
 *Function :ixHssAccPktRxFreeReplenish
 */
IX_STATUS 
ixHssAccPktRxFreeReplenish (IxHssAccHssPort hssPortId, 
                IxHssAccHdlcPort hdlcPortId, 
                IX_OSAL_MBUF *buffer)
{
    /* This function may be called from within an ISR */
    IX_STATUS status = IX_SUCCESS;
 
    UINT32 qEntry = 0; 

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering "
                      "ixHssAccPktRxFreeReplenish \n");

    /* Check that the client is connected */
    if (ixHssAccPCMCheckReplenishOk (hssPortId, hdlcPortId))
    {

        /* Set up the NPE shared region of the IX_OSAL_BUF buffer */
        IX_HSSACC_IX_NE_SHARED_STATUS(buffer) = (UINT8) IX_HSSACC_PKT_OK;
        IX_HSSACC_IX_NE_SHARED_PKT_LEN(buffer) = (UINT16) 0;
        IX_HSSACC_IX_NE_SHARED_DATA(buffer) = (UINT8 *)IX_HSSACC_PKT_MMU_VIRT_TO_PHY(IX_OSAL_MBUF_MDATA(buffer));
        IX_HSSACC_IX_NE_SHARED_LEN(buffer) = (UINT16) IX_OSAL_MBUF_MLEN(buffer);
        IX_HSSACC_IX_NE_SHARED_NEXT(buffer) = (UINT32 *) NULL;
        IX_HSSACC_IX_NE_SHARED_ERR_CNT(buffer) = (UINT8) 0;

        /* Endian conversion is performed on the NPE shared region of the buffer */
        IX_HSSACC_IX_NE_ENDIAN_SWAP(buffer);
        
        /* The cache is flushed for the NPE shared region of the buffer */
        IX_HSSACC_IX_NE_SHARED_CACHE_FLUSH(buffer);

        /* Form the Q entry and Write it to the appropriate RxFree Q*/
        qEntry = (UINT32) IX_HSSACC_PKT_MMU_VIRT_TO_PHY (IX_HSSACC_IX_NE_SHARED(buffer));
        qEntry |= (hdlcPortId & IX_HSSACC_QM_Q_CHAN_NUM_MASK);
        status = ixQMgrQWrite (ixHssAccPCMRxFreeQIdGet (hssPortId, hdlcPortId),
                               &qEntry);

        if (status != IX_SUCCESS)
        {
            if (status == IX_FAIL)
            {
                IX_HSSACC_REPORT_ERROR ("ixHssAccPktRxFreeReplenish:"
                            "Writing a queue entry to the RXFREE"
                            " Queue failed\n");
                ixHssAccPktRxStats.rxFreeQWriteFails++;
            }
            else if (status == IX_QMGR_Q_OVERFLOW)
            {
                ixHssAccPktRxStats.rxQWriteOverflows++;
      		status = IX_HSSACC_Q_WRITE_OVERFLOW;
            }
            return status;
        } /* end of if (status != IX_SUCCESS)*/

        /* Increment the number of buffers in use count for the client by one*/
        ixHssAccPCMnoBuffersInUseCountInc (hssPortId, hdlcPortId, 1); 

    ixHssAccPktRxStats.rxFreeReplenishs++;
    }
    else
    {
    IX_HSSACC_REPORT_ERROR ("ixHssAccPktRxFreeReplenish:"
                "Called ixHssAccPktRxInternalFreeBufReplenish "
                "on a port that is not connected\n");
    return IX_FAIL;
    }
   
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting "
              "ixHssAccPktRxFreeReplenish \n");
    return status;
}

/**
 * Function definition: ixHssAccPktRxInit
 */
IX_STATUS 
ixHssAccPktRxInit (void)
{
    IX_STATUS status = IX_SUCCESS;
    IxHssAccHssPort hssPortIndex;
    IxHssAccHdlcPort hdlcPortIndex;
    unsigned hssHdlcPortIdAsCbId;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccPktRxInit\n");

    /* initialise stats */
    ixHssAccPktRxStatsInit ();

    /* Set the Callback for the RxFree Q for this client */
    for (hssPortIndex = IX_HSSACC_HSS_PORT_0; 
	 hssPortIndex < hssPortMax; 
	 hssPortIndex++)
    {
	for (hdlcPortIndex = IX_HSSACC_HDLC_PORT_0; 
	     hdlcPortIndex < IX_HSSACC_HDLC_PORT_MAX; 
	     hdlcPortIndex++)
	{
	    hssHdlcPortIdAsCbId = (hssPortIndex << IX_HSSACC_PKT_CBID_HSS_OFFSET) | 
		hdlcPortIndex;
	    
	    status = ixQMgrNotificationCallbackSet (
		ixHssAccPCMRxFreeQIdGet (hssPortIndex, hdlcPortIndex),
		ixHssAccPktRxFreeLowCallback,
		hssHdlcPortIdAsCbId);

	    if (status != IX_SUCCESS)
	    {
		IX_HSSACC_REPORT_ERROR ("ixHssAccPktRxInit:"
					"ixQMgrNotificationCallbackSet failed\n");
		return IX_FAIL;
	    }
	}
    }

    for (hssPortIndex = IX_HSSACC_HSS_PORT_0; 
	 hssPortIndex < hssPortMax; 
	 hssPortIndex++)
    {
        /* Set the callback for the HSS Port 0/1 Rx Q */
        status = ixQMgrNotificationCallbackSet (
	    ixHssAccPktRxQId[hssPortIndex],
	    ixHssAccPktRxCallback,
	    hssPortIndex);
        if (status != IX_SUCCESS)
        {
	    /* report the error */
	    IX_HSSACC_REPORT_ERROR_WITH_ARG ("ixHssAccPktRxInit:"
	        "Setting callback for the HSS%d PktRx Q failed\n",
		hssPortIndex,
		0, 0, 0, 0, 0);
            /* return error */
            return IX_FAIL;
        }
    
        /* Enable notification for the HSS Port 0/1 Rx Q */
        status =  ixQMgrNotificationEnable (ixHssAccPktRxQId[hssPortIndex],
	                                    IX_QMGR_Q_SOURCE_ID_NOT_E);
        if (status != IX_SUCCESS)
        {
	    /* report the error */
	    IX_HSSACC_REPORT_ERROR_WITH_ARG ("ixHssAccPktRxInit:"
	        "Notification enable for the HSS%d PktRx Q failed\n",
		hssPortIndex,
		0, 0, 0, 0, 0);
            /* return error */
	    return IX_FAIL;
        }
    }
	
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccPktRxInit\n");
    return status;
}


/*
 * Function definition: ixHssAccPktRxUninit
 */
IX_STATUS
ixHssAccPktRxUninit (void)
{
    IX_STATUS        status = IX_SUCCESS;
    IxHssAccHssPort  hssPortIndex;
    IxHssAccHdlcPort hdlcPortIndex;
    UINT32           hssHdlcPortIdAsCbId;
    INT8             errorString[80];

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT,
                      "Entering ixHssAccPktRxUninit\n");

    for (hssPortIndex = IX_HSSACC_HSS_PORT_0;
         hssPortIndex < hssPortMax;
         hssPortIndex++)
    {  /* Disable notification for the HSS Port 0/1 Rx Q */
        status =  ixQMgrNotificationDisable (ixHssAccPktRxQId[hssPortIndex]);
        if (IX_SUCCESS != status)
        {
            sprintf ((char *)errorString, "ixHssAccPktRxUninit:"
                     "Notification disable for the HSS %d PktRx Q failed\n",
                     hssPortIndex);
            /* report the error */
            IX_HSSACC_REPORT_ERROR ((char *)errorString);
            /* return error */
            return IX_FAIL;
        }
        /* Set the callback for the HSS Port 0/1 Rx Q */
        status = ixQMgrNotificationCallbackSet (
            ixHssAccPktRxQId[hssPortIndex],
            NULL,
            hssPortIndex);
        if (IX_SUCCESS != status)
        {
            sprintf ((char *)errorString, "ixHssAccPktRxUninit:"
                     "Setting dummy callback for the HSS%d PktRx Q failed\n",
                     hssPortIndex);
            /* report the error */
            IX_HSSACC_REPORT_ERROR ((char *)errorString);
            /* return error */
            return IX_FAIL;
        }
    }
    /* Set the Callback for the RxFree Q  */
    for (hssPortIndex = IX_HSSACC_HSS_PORT_0;
         hssPortIndex < hssPortMax;
         hssPortIndex++)
    {
        for (hdlcPortIndex = IX_HSSACC_HDLC_PORT_0;
             hdlcPortIndex < IX_HSSACC_HDLC_PORT_MAX;
             hdlcPortIndex++)
        {
            hssHdlcPortIdAsCbId = (hssPortIndex << IX_HSSACC_PKT_CBID_HSS_OFFSET) |
                hdlcPortIndex;

            status = ixQMgrNotificationCallbackSet (
                ixHssAccPCMRxFreeQIdGet (hssPortIndex, hdlcPortIndex),
                NULL,
                hssHdlcPortIdAsCbId);

            if (IX_SUCCESS != status)
            {
                IX_HSSACC_REPORT_ERROR ("ixHssAccPktRxUninit:"
                                        "ixQMgrNotificationCallbackSet (dummy callback) failed\n");
                return IX_FAIL;
             }
        }
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT,
                     "Exiting ixHssAccPktRxUninit\n");
    return status;
}



