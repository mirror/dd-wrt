/**
 * @file IxHssAccPktTx_p.h
 *
 * @author Intel Corporation
 * @date  14 Dec 2001
 *
 * @brief This file contains the private API of the HSS Packetised Tx  module
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
 * @defgroup IxHssAccPktTx_p IxHssAccPktTx_p
 *
 * @brief IXP400 HSS Access Packet Tx Module
 * 
 * @{
 */

#include "IxQMgr.h"
#include "IxHssAcc.h"
#include "IxOsal.h"
#include "IxHssAccPCM_p.h"
#include "IxHssAccError_p.h"

#include "IxHssAccCommon_p.h"

#ifndef IXHSSACCPKTTX_P_H
#define IXHSSACCPKTTX_P_H 

/*
 * inline definition
 */
#ifdef IXHSSACCPKTTX_C
/* Non-inline functions will be defined in this translation unit. 
 * Reason is that in GNU Compiler, if the optimization flag is turned off, 
 * all extern inline functions will not be compiled.
 */
#   ifndef __wince
#       ifndef IXHSSACCPKTTX_INLINE
#           define IXHSSACCPKTTX_INLINE
#	endif
#   else
#	ifndef IXHSSACCPKTTX_INLINE
#	    define IXHSSACCPKTTX_INLINE IX_OSAL_INLINE_EXTERN
#	endif
#   endif /* __wince*/
#else
#   ifndef IXHSSACCPKTTX_INLINE
#       ifdef _DIAB_TOOL
            /* DIAB does not allow both the funtion prototype and
             * defintion to use extern
             */
#           define IXHSSACCPKTTX_INLINE IX_OSAL_INLINE
#       else
#           define IXHSSACCPKTTX_INLINE IX_OSAL_INLINE_EXTERN
#       endif
#   endif
#endif /* IXHSSACCPKTTX_C */

/*
 * Typedefs
 */
typedef struct
{
    unsigned txs;     
    unsigned txDones;
    unsigned qWriteFails;
    unsigned qWriteOverflows;
    unsigned txDoneQReadFails;
    unsigned maxEntriesInTxDoneQ;
    unsigned txInvalidBuffers;
} IxHssAccPktTxStats;

/**
 * Variable declarations.  Externs are followed by static variables.
 */
extern IxHssAccPktTxStats ixHssAccPktTxStats;

/**
 * Prototypes for interface functions.
 */

/**
 * @fn IX_STATUS ixHssAccPktTxInternal (IxHssAccHssPort hssPortId,
                                        IxHssAccHdlcPort hdlcPortId,
					IX_OSAL_MBUF *buffer)
 *
 * @brief This inline function is the transmit function which is called by the 
 * client.
 *
 * @param IxHssAccHssPort hssPortId (in) -  The HSS port number on which the
 *  calling client is operating on i.e.(0-1)
 * @param IxHssAccHdlcPort hdlcPortId (in) - The HDLC port number on which 
 * the calling client is operating on i.e. 1, 2, 3 or 4.
 * @param IX_OSAL_MBUF *buffer (in) - This is the client supplied buffer which holds 
 * the data the client wants to transmit.
 *
 * The IX_OSAL_BUF buffer is passed to this function. The function first checks if the 
 * client is connected. Then the relevant fields are copied from the OS dependant 
 * region to the NPE shared region of IX_OSAL_BUF buffer. Also do endian conversion 
 * of the NPE shared region. All this is done for all the buffers in the chain.
 * As the addresses are 32 byte aligned the least significant five bits of the 
 * IX_OSAL_BUF buffer address will be free. The least significant two bits are set to 
 * the HDLC port number and the resultant value is posted in the appropriate QMgr Tx
 * queue. The number of buffers in use count for the client is incremented by the 
 * chain count.
 * 
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_RESOURCE_ERR The function did not execute successfully due
 *                          to a resource error
 * 
 */
IXHSSACCPKTTX_INLINE
IX_STATUS 
ixHssAccPktTxInternal (IxHssAccHssPort hssPortId, 
		       IxHssAccHdlcPort hdlcPortId,
		       IX_OSAL_MBUF *buffer);

/**
 * @fn void ixHssAccPktTxDoneCallback (IxQMgrQId qId, 
                                       IxQMgrCallbackId cbId);
 *
 * @brief This function is called by the TxDone QMQ interrupt 
 * (Callback Mechanism)
 *
 * @param IxQMgrQId qId (in) -  The IxQMQ Queue Id, this is the ID number 
 * of the queue
 * @param IxQMgrCallbackId cbId  (in) - The IxQMQ callback ID
 * not to call this callback again in the same context.
 * @return void
 */
void 
ixHssAccPktTxDoneCallback (IxQMgrQId qId, 
			   IxQMgrCallbackId cbId);


/**
 * @fn  void ixHssAccPktTxShow (void)
 *
 * @brief This function is called by to display the stats in the Tx Module
 *
 * @return void
 */
void 
ixHssAccPktTxShow (void);


/**
 * @fn void ixHssAccPktTxStatsInit (void)
 *
 * @brief This function is called by to initialise the stats in the Tx Module
 *
 * @return void
 */
void 
ixHssAccPktTxStatsInit (void);

/**
 * @fn IX_STATUS ixHssAccPktTxInit (void)
 *
 * @brief This function will initialise the PktTx module
 *
 * @return 
 *           - IX_SUCCESS Function executed successfully
 *           - IX_FAIL Function failed to execute
 */
IX_STATUS 
ixHssAccPktTxInit (void);

/**
 * @fn IX_STATUS ixHssAccPktTxUninit (void)
 *
 * @brief  This function Uninitialises all resources for Packetised Tx module.
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_RESOURCE_ERR The function did not execute successfully 
 *                                   due to a resource error
 */

IX_STATUS ixHssAccPktTxUninit (void);


/*
 * Inline functions
 */
 
/*
 * This inline function is the transmit function which is called by the client.
 */
IXHSSACCPKTTX_INLINE
IX_STATUS 
ixHssAccPktTxInternal (
    IxHssAccHssPort hssPortId, 
    IxHssAccHdlcPort hdlcPortId,
    IX_OSAL_MBUF *buffer)
{
    UINT32 qEntry = 0;
    UINT32 chainCount = 0;
    IX_STATUS status = IX_SUCCESS;
#ifndef NDEBUG
    unsigned packetLength;
    IX_OSAL_MBUF *tmpBuffer;
#endif
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering "
		      "ixHssAccPktTxInternal\n");
    
    if (ixHssAccPCMCheckTxOk (hssPortId, hdlcPortId))
    {
#ifndef NDEBUG
	packetLength = IX_OSAL_MBUF_MLEN(buffer);
	tmpBuffer = buffer;
	while (IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(tmpBuffer) != NULL)
	{
	    tmpBuffer = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(tmpBuffer);
	    packetLength += IX_OSAL_MBUF_MLEN(tmpBuffer);
	}
	
	if ((UINT32)IX_OSAL_MBUF_PKT_LEN(buffer) != packetLength) 
	{
	    IX_HSSACC_REPORT_ERROR ("ixHssAccPktTxInternal:Invalid packet length,"
				    " packetLength not set correctly\n");
	    return IX_FAIL;
	}
#endif
        /* Copy relevant fields from OS dependant region to the NPE shared region */
        ixHssAccComMbufToNpeFormatConvert (buffer, &chainCount, &qEntry); 

        /* Set the least significant 2 bits of the queue entry to the HDLC port number. */
        qEntry |= (hdlcPortId & IX_HSSACC_QM_Q_CHAN_NUM_MASK);

        /* Write the queue entry to the Tx Q for this client */
        status = ixQMgrQWrite (ixHssAccPCMTxQIdGet (hssPortId, hdlcPortId),
                               &qEntry);
        if (status != IX_SUCCESS)
        {
            if (status == IX_FAIL)
            {
                IX_HSSACC_REPORT_ERROR ("ixHssAccPktTxInternal:Writing "
                                        "a queue entry "
                                        "to the Tx Queue failed\n");
                ixHssAccPktTxStats.qWriteFails++;
            }
            else if (status == IX_QMGR_Q_OVERFLOW)
            {
                ixHssAccPktTxStats.qWriteOverflows++;
                status = IX_HSSACC_Q_WRITE_OVERFLOW;
            }
            
            /* Return the status */
            return status;
        } /* end of if (status != IX_SUCCESS) */

        /* Increment the number of buffers in use count for the specified hss and hdlc port combination */
        ixHssAccPCMnoBuffersInUseCountInc (hssPortId, hdlcPortId, chainCount);
    }
    else 
    {
	IX_HSSACC_REPORT_ERROR ("ixHssAccPktTxInternal:This HDLC Port is either"
				" not connected or not started\n");
	return IX_FAIL;
    }

    /* Update the statistics and return the status */
    ixHssAccPktTxStats.txs++;
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting "
		      "ixHssAccPktTxInternal\n");
    return status;
}

#endif /* IXHSSACCPKTTX_P_H */

/**
 * @} defgroup  IxHssAccPktTx_p
 */
