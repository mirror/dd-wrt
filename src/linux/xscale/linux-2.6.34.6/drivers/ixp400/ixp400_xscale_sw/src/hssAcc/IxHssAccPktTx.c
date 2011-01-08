/**
 * @file IxHssAccPktTx.c
 *
 * @author Intel Corporation
 * @date 10 Jan 2002
 *
 * @brief This file contains the implementation of the private API for the 
 * Transmit Module.
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

#ifndef IXHSSACCPKTTX_P_H
#   define IXHSSACCPKTTX_C
#else
#   error "Error: IxHssAccPktTx_p.h should not be included before this definition."
#endif

/**
 * Put the system defined include files required.
 */


/**
 * Put the user defined include files required.
 */
#include "IxHssAcc.h"
#include "IxOsal.h"
#include "IxNpeA.h"
#include "IxHssAccPktTx_p.h"
#include "IxHssAccError_p.h"
#include "IxHssAccPCM_p.h"
#include "IxHssAccCommon_p.h"

/*
 * Defines
 */
/* The following define allows the Nearly Empty watermark level */
/* of the TxDone Q to be configurable.*/
#define	IX_HSSACC_PKT_TXDONE_Q_WATERMARK IX_QMGR_Q_WM_LEVEL0

/* 
 * The following macro is used to calculate the remaining free space in an
 * mbuf that can be used to store data.  buffer->pClBlk->clSize can no
 * longer be used as pClBlk is a vxWorks dependent implementation, and is
 * not present in the POSIX mbuf.  The macro M_TRAILINGSPACE is present in
 * both vxWorks and POSIX.
 */
#define IX_HSSACC_MBUF_TRAILINGSPACE(buffer) M_TRAILINGSPACE (buffer);

/*
 * Typedefs
 */


/**
 * Variable declarations global to this file only. 
 *  Externs are followed by static variables.
 */
IxHssAccPktTxStats ixHssAccPktTxStats;

IxQMgrQId ixHssAccPktTxDoneQId[IX_HSSACC_HSS_PORT_MAX] = 
{    IX_NPE_A_QMQ_HSS0_PKT_TX_DONE,       
     IX_NPE_A_QMQ_HSS1_PKT_TX_DONE
};

/**
 * Function definition: ixHssAccPktTxDoneCallback
 */
void 
ixHssAccPktTxDoneCallback (IxQMgrQId ixHssAccPktTxDoneQueueId, 
			   IxQMgrCallbackId cbId)
{
    /* This function is called from within an ISR */
    IX_STATUS status;
    UINT32 qEntry = 0;
    UINT32 chainCount = 0;
    IX_OSAL_MBUF *pRootBuf = NULL;
    IxHssAccHdlcPort hdlcPortId = 0;
    IxHssAccHssPort hssPortId =  cbId;
    unsigned numOfTxDoneQReads = 0;
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering "
                       "ixHssAccPktTxDoneCallback\n"));

    do 
    {
    
        /* Read the queue entry from the TxDone queue */
        status = ixQMgrQRead (ixHssAccPktTxDoneQueueId, &qEntry); 
        numOfTxDoneQReads++;
        if (status == IX_SUCCESS)
        {

#ifndef NDEBUG
            if ((UINT32 *)qEntry != NULL)
            {
#endif
                /* Get the HDLC port number from the queue entry. */
                hdlcPortId = (IxHssAccHdlcPort) (qEntry & IX_HSSACC_QM_Q_CHAN_NUM_MASK);
                
                /* Copy relevant fields from the NPE shared region to the OS dependant region */
                pRootBuf = ixHssAccComMbufFromNpeFormatConvert ( qEntry, FALSE , &chainCount);


#ifndef NDEBUG

                /* 
                 * Ensure that the HSS Port Id is lower than the max hssPortId.
                 * Also ensure that the HDLC Port Id is lower than the max hdlcPortId. 
                 */
                if (!(IX_HSSACC_ENUM_INVALID (hssPortId, hssPortMax)) && 
                    !(IX_HSSACC_ENUM_INVALID (hdlcPortId, IX_HSSACC_HDLC_PORT_MAX)))
                {
#endif
                    ixHssAccPCMTxDoneCallbackRun (hssPortId,
                                                  hdlcPortId,
                                                  pRootBuf, 
                                                  IX_HSSACC_IX_NE_SHARED_ERR_CNT(pRootBuf),
                                                  IX_HSSACC_IX_NE_SHARED_STATUS(pRootBuf));
                    ixHssAccPktTxStats.txDones++;
#ifndef NDEBUG      
                }
                else
                {
                    IX_HSSACC_DEBUG_OFF (IX_HSSACC_REPORT_ERROR ("ixHssAccPktTxDoneCallback: Invalid "
                                         "buffer was received.\n"));
                    ixHssAccPktTxStats.txInvalidBuffers++;
                }
#endif
                /* Decrement the number of buffers in use count for the specified client */
                if(ixHssAccPCMnoBuffersInUseCountDec(hssPortId, hdlcPortId, chainCount) != IX_SUCCESS)
                {
                    IX_HSSACC_DEBUG_OFF (IX_HSSACC_REPORT_ERROR ("ixHssAccPktTxDoneCallback: "
                                        "ixHssAccPCMinUseCountDec "
                                        "failed while decrementing usage count.\n"));
                }
#ifndef NDEBUG
            }
            else
            {
                    IX_HSSACC_DEBUG_OFF (IX_HSSACC_REPORT_ERROR ("ixHssAccPktTxDoneCallback: Invalid "
                                        "buffer was received.\n"));
                    ixHssAccPktTxStats.txInvalidBuffers++;
            } /* end of if-else ((UINT32 *)qEntry != NULL)*/
#endif
        }

    else if (status == IX_FAIL)
    {
        IX_HSSACC_DEBUG_OFF 
        (IX_HSSACC_REPORT_ERROR ("ixHssAccPktTxDoneCallback:"
                     "TxDone QRead failed\n"));
        ixHssAccPktTxStats.txDoneQReadFails++;
    }
    } while (status == IX_SUCCESS);
    
    if (numOfTxDoneQReads > ixHssAccPktTxStats.maxEntriesInTxDoneQ)
    {
    ixHssAccPktTxStats.maxEntriesInTxDoneQ = numOfTxDoneQReads;
    }
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting "
                       "ixHssAccPktTxDoneCallback\n"));
}


/**
 * Function definition: ixHssAccPktTxShow
 */
void 
ixHssAccPktTxShow (void)
{
    printf ("\nixHssAccPktTxShow:\n");
    printf ("           txs: %d \t     txDones: %d\n",
        ixHssAccPktTxStats.txs,
        ixHssAccPktTxStats.txDones);
    printf ("    writeFails: %d \t   readFails: %d \t  invalidBuffers: %d\n",
        ixHssAccPktTxStats.qWriteFails,
        ixHssAccPktTxStats.txDoneQReadFails,
        ixHssAccPktTxStats.txInvalidBuffers);
    printf ("writeOverflows: %d \tmaxInTxDoneQ: %d\n",
        ixHssAccPktTxStats.qWriteOverflows,
        ixHssAccPktTxStats.maxEntriesInTxDoneQ);
}


/**
 * Function definition: ixHssAccPktTxStatsInit
 */
void 
ixHssAccPktTxStatsInit (void)
{
    ixHssAccPktTxStats.txs         = 0;
    ixHssAccPktTxStats.txDones     = 0;
    ixHssAccPktTxStats.qWriteFails = 0;
    ixHssAccPktTxStats.qWriteOverflows  = 0;
    ixHssAccPktTxStats.txDoneQReadFails = 0;
    ixHssAccPktTxStats.maxEntriesInTxDoneQ = 0;
    ixHssAccPktTxStats.txInvalidBuffers = 0;
    
}

/**
 * Function definition: ixHssAccPktRxInit
 */
IX_STATUS 
ixHssAccPktTxInit (void)
{
    IX_STATUS status = IX_SUCCESS;
    IxHssAccHssPort hssPortIndex;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccPktTxInit\n");

    /* initialise stats */
    ixHssAccPktTxStatsInit ();

    for (hssPortIndex = IX_HSSACC_HSS_PORT_0; 
	 hssPortIndex < hssPortMax; 
	 hssPortIndex++)
    {
        /* Set the callback for the HSS Port 0/1 TxDone Q */
        status = ixQMgrNotificationCallbackSet (
	    ixHssAccPktTxDoneQId[hssPortIndex],
	    ixHssAccPktTxDoneCallback,
	    hssPortIndex);
    
        if (status != IX_SUCCESS)
        {
	    /* report the error */
	    IX_HSSACC_REPORT_ERROR_WITH_ARG ("ixHssAccPktTxInit:"
	        "Notification CallbackSet failed for HSS%d TxDone Q\n",
		hssPortIndex,
		0, 0, 0, 0, 0);
            /* return error */
	    return IX_FAIL;
        }
    
        /* Set the Watermark for the TxDone Q for HSS Port 0/1 */
        status = ixQMgrWatermarkSet (
	    ixHssAccPktTxDoneQId[hssPortIndex],
	    IX_HSSACC_PKT_TXDONE_Q_WATERMARK/*NE flag*/,
	    0/*NF flag*/);
    
        if (status != IX_SUCCESS)
        {
	    /* report the error */
	    IX_HSSACC_REPORT_ERROR_WITH_ARG ("ixHssAccPktTxInit:"
	        "ixQMgrWatermarkSet failed for the HSS%d TxDone Q\n",
		hssPortIndex,
		0, 0, 0, 0, 0);
            /* return error */
	    return IX_FAIL;
        }    

        /* Enable notification for the HSS Port 0/1 TxDone Q */
        status = ixQMgrNotificationEnable (ixHssAccPktTxDoneQId[hssPortIndex],
				           IX_QMGR_Q_SOURCE_ID_NOT_NE);
        if (status != IX_SUCCESS)
        {
	    /* report the error */
	    IX_HSSACC_REPORT_ERROR_WITH_ARG ("ixHssAccPktTxInit:"
	        "Notification enable failed for HSS%d TxDone Q\n",
		hssPortIndex,
		0, 0, 0, 0, 0);
            /* return error */
            return IX_FAIL;
        }
    }
    
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccPktTxInit\n");
    return status;
}



/**
 * Function definition: ixHssAccPktTxUninit
 */

IX_STATUS
ixHssAccPktTxUninit (void)
{
    IX_STATUS       status = IX_SUCCESS;
    IxHssAccHssPort hssPortIndex;
    INT8            errorString[80];

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT,
                      "Entering ixHssAccPktTxUninit\n");

   for (hssPortIndex = IX_HSSACC_HSS_PORT_0;
         hssPortIndex < hssPortMax;
         hssPortIndex++)
    {
        /* Disable notification for the HSS Port 0/1 TxDone Q */
        status = ixQMgrNotificationDisable (ixHssAccPktTxDoneQId[hssPortIndex]);
        if (IX_SUCCESS != status)
        {
            sprintf ((char *)errorString, "ixHssAccPktTxUninit:"
                     "Notification disable failed for HSS%d TxDone Q\n",
                     hssPortIndex);
            /* report the error */
            IX_HSSACC_REPORT_ERROR ((char *)errorString);
            /* return error */
            return IX_FAIL;
        }
        /* Set the callback to dummy callback for the HSS Port 0/1 TxDone Q */
        status = ixQMgrNotificationCallbackSet (
            ixHssAccPktTxDoneQId[hssPortIndex],
            NULL,
            hssPortIndex);
        if (IX_SUCCESS != status)
        {
            sprintf ((char *)errorString, "ixHssAccPktTxUninit:"
                     "Notification CallbackSet to dummy failed for HSS%d TxDone Q\n",
                     hssPortIndex);
            /* report the error */
            IX_HSSACC_REPORT_ERROR ((char *)errorString);
            /* return error */
            return IX_FAIL;
        }

    }
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT,
                      "Exiting ixHssAccPktTxUninit\n");
    return status;
}



