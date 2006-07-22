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
#include "IxHssAccPDM_p.h"
#include "IxHssAccPCM_p.h"
#include "IxHssAccError_p.h"

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
    unsigned invalidDescs;
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
 * client. The buffer is attached to a descriptor and placed on the Tx QMQ
 *
 * @param IxHssAccHssPort hssPortId (in) -  The HSS port number on which the
 *  calling client is operating on i.e.(0-1)
 * @param IxHssAccHdlcPort hdlcPortId (in) - The HDLC port number on which 
 * the calling client is operating on i.e. 1, 2, 3 or 4.
 * @param IX_OSAL_MBUF *buffer (in) - This is the client supplied buffer which holds 
 * the data the client wants to transmit, it is attached to the Packetised 
 * Descriptor and placed on the Tx QMQ
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
 * @brief  This function Uninitialises all resources for all descriptor pools 
 * and descriptors contained in these pools.
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
ixHssAccPktTxInternal (IxHssAccHssPort hssPortId, IxHssAccHdlcPort hdlcPortId,
			IX_OSAL_MBUF *buffer)
{
    IxHssAccPDMDescriptor *desc;   /* A desc ptr that we will write the 
				      clients mbuf to*/
    IxHssAccPDMDescriptor *physDesc;
    UINT32 pPhysDesc = 0;
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
	
	status = ixHssAccPDMDescGet (hssPortId, hdlcPortId, IX_HSSACC_PDM_TX_POOL,
				     &desc);
	if (status != IX_SUCCESS)
	{
	    return status;
	}
	
	/* Set desc up as required by the NPE */
	desc->npeDesc.pRootMbuf    = ixHssAccPDMMbufToNpeFormatConvert(buffer);
	desc->npeDesc.status       = IX_HSSACC_PKT_OK;
	desc->npeDesc.chainCount   = 0;
	desc->npeDesc.pMbufData    = (UINT8 *) IX_OSAL_MBUF_MDATA(buffer);
	desc->npeDesc.mbufLength   = IX_OSAL_MBUF_MLEN(buffer);
	desc->npeDesc.pNextMbuf    = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(buffer); 
	desc->npeDesc.errorCount   = 0;
	/* Before assigning IX_OSAL_MBUF_PKT_LEN to npeDesc.packetLength, 
	 *  IX_OSAL_MBUF_PKT_LEN needs to be swapped back because it's swapped in 
	 *  ixHssAccPDMMbufToNpeFormatConvert before. The main reason for
	 *  this back & forth swapping is because npeDesc.packetLength is
	 *  UINT16 in size while IX_OSAL_MBUF_PKT_LEN is INT32. Assigning an
	 *  INT32 data to UINT16 variable will cause the 2 most significant
	 *  bytes to be chopped off. Hence, if no reverse conversion is
	 *  performed before assigning the packet length value, the value
	 *  will be chopped off wrongly. For e.g.:
	 *    IX_OSAL_MBUF_PKT_LEN (before conversion) 
	 *        = 0x0000AABB
	 *    IX_OSAL_MBUF_PKT_LEN (after conversion in 
	 *                     ixHssAccPDMMbufToNpeFormatConvert) 
	 *        = 0xBBAA0000
	 *    npeDesc.packetLength (if no proper conversion before assigning 
	 *                          IX_OSAL_MBUF_PKT_LEN to npeDesc.packetLength)
	 *        = 0x0000 (NOT 0xAABB)
	 *  Note: The above discussion only applies to Little Endian mode
	 */
	desc->npeDesc.packetLength = (UINT16)IX_OSAL_SWAP_BE_SHARED_LONG( 
	    (UINT32) IX_OSAL_MBUF_PKT_LEN(buffer));

        /* endian conversion for the NpePacket Descriptor */
	desc->npeDesc.pRootMbuf = (IX_OSAL_MBUF *)IX_OSAL_SWAP_BE_SHARED_LONG(
	    (UINT32) desc->npeDesc.pRootMbuf);	
        desc->npeDesc.packetLength = (UINT16)IX_OSAL_READ_BE_SHARED_SHORT(
	    (UINT16 *) &(desc->npeDesc.packetLength));
	
	IX_HSSACC_PKT_DATA_CACHE_FLUSH (desc, sizeof(IxHssAccPDMDescriptor));
	physDesc = (IxHssAccPDMDescriptor *) IX_HSSACC_PKT_MMU_VIRT_TO_PHY (desc);
	/* Write the desc to the Tx Q for this client */
        pPhysDesc = (UINT32) physDesc;
	status = ixQMgrQWrite (ixHssAccPCMTxQIdGet (hssPortId, hdlcPortId), 
			       &pPhysDesc);
	if (status != IX_SUCCESS)
	{
	    if (status == IX_FAIL)
	    {
		IX_HSSACC_REPORT_ERROR ("ixHssAccPktTxInternal:Writing "
					"a Tx descriptor "
					"to the Tx Queue failed\n");
		ixHssAccPktTxStats.qWriteFails++;
	    }
	    else if (status == IX_QMGR_Q_OVERFLOW)
	    {
		ixHssAccPktTxStats.qWriteOverflows++;
		status = IX_HSSACC_Q_WRITE_OVERFLOW;
	    }

            /* endian conversion for the NpePacket Descriptor */
	    desc->npeDesc.pRootMbuf = (IX_OSAL_MBUF *)(IX_OSAL_SWAP_BE_SHARED_LONG (
	        (UINT32)desc->npeDesc.pRootMbuf));
	    desc->npeDesc.packetLength = (UINT16)(IX_OSAL_READ_BE_SHARED_SHORT (
          	(UINT16 *) &(desc->npeDesc.packetLength)));
	    desc->npeDesc.rsvdShort0 = (UINT16)(IX_OSAL_READ_BE_SHARED_SHORT (
	       	(UINT16 *) &(desc->npeDesc.rsvdShort0)));	
	    desc->npeDesc.pNextMbuf = (IX_OSAL_MBUF *)(IX_OSAL_SWAP_BE_SHARED_LONG (
		(UINT32) desc->npeDesc.pNextMbuf)); 
	    desc->npeDesc.pMbufData = (UINT8 *)(IX_OSAL_SWAP_BE_SHARED_LONG (
		(UINT32) desc->npeDesc.pMbufData)); 
	    desc->npeDesc.mbufLength = IX_OSAL_SWAP_BE_SHARED_LONG (
	        desc->npeDesc.mbufLength); 

	    /* we need to restore the buffer to the original (virtual) address
	     * before we return
	     */
	    ixHssAccPDMMbufFromNpeFormatConvert(desc->npeDesc.pRootMbuf, FALSE);
	    
	    /* Now we can free the descriptor back to the pool. */
	    ixHssAccPDMDescFree (desc, IX_HSSACC_PDM_TX_POOL);

	    return status;
	}
    }
    else 
    {
	IX_HSSACC_REPORT_ERROR ("ixHssAccPktTxInternal:This HDLC Port is either"
				" not connected or not started\n");
	return IX_FAIL;
    }

    ixHssAccPktTxStats.txs++;
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting "
		      "ixHssAccPktTxInternal\n");
    return status;
}

#endif /* IXHSSACCPKTTX_P_H */

/**
 * @} defgroup  IxHssAccPktTx_p
 */
