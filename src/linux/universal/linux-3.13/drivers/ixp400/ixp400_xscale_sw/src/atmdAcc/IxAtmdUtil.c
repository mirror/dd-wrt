/**
* @file IxAtmdUtil.c
*
 * @author Intel Corporation
* @date 17 March 2002
*
* @brief IxAtmdAcc misc functions
*
* ISO 3309 and RFC 1331 implementation
*
* Design Notes:
*        This should be checked again when NPE is using a
*        hardware hash algorithm
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

/*
* Put the system defined include files required.
*/

/*
* Put the user defined include files required.
*/

#include "IxOsal.h"
#include "IxNpeMh.h"

#include "IxAtmdDefines_p.h"
#include "IxAtmdNpe_p.h"
#include "IxAtmdAssert_p.h"
#include "IxAtmdUtil_p.h"

/*
*  Atmd Connection Id algorithm implementation
*
* Connections Id returned to the user are made from 3 parts
* - a unique number which prevent a misuse of random connIds
* - a flag indicating if the channel is dieconnecting
* - a channel id (an index into s pool of channels)
*
* The following defines are related to the unique ID
* calculations
*
*/
#define IX_ATMDACC_UNIQUEID_SEED 1
#define IX_ATMDACC_UNIQUEID_INCR 2
#define IX_ATMDACC_UNIQUEID_WORD_MAX 0xffffffff
#define IX_ATMDACC_UNIQUEID_WRAP_LIMIT \
    ((IX_ATMDACC_UNIQUEID_WORD_MAX/IX_ATM_MAX_NUM_VC) - \
    IX_ATMDACC_UNIQUEID_INCR)


/*
*  Atmd internal retry for NpeMh fifo full conditions
*
*/
#define IX_ATMDACC_SEND_RETRIES_DEFAULT (8 * IX_NPEMH_SEND_RETRIES_DEFAULT)

/*
* Variable declarations global to this file only.  Externs are followed by
* static variables.
*/

static IxOsalMutex utilLock;
static unsigned int uniqueId = IX_ATMDACC_UNIQUEID_SEED;
static BOOL initDone = FALSE;
static BOOL queueInitDone = FALSE;

/*
* Function definition.
*/

/* ---------------------------------------------------------
*        lock utilities
*/
#define IX_ATMDACC_UTIL_LOCK() (void)ixOsalMutexLock (&utilLock, IX_OSAL_WAIT_FOREVER)
#define IX_ATMDACC_UTIL_UNLOCK() (void)ixOsalMutexUnlock (&utilLock)

/* ---------------------------------------------------------
*   Module Initialisation
*/
IX_STATUS
ixAtmdAccUtilInit (void)
{
    IX_STATUS returnStatus = IX_FAIL;

    if (initDone == FALSE)
    {
        returnStatus = ixOsalMutexInit (&utilLock);

        if (returnStatus != IX_SUCCESS)
        {
            returnStatus = IX_FAIL;
        }
        else
        {
            initDone = TRUE;
        } /* end of if-else(returnStatus) */
    } /* end of if(initDone) */

    return returnStatus;
}



/* ---------------------------------------------------------
*   Module Uninitialisation
*/
IX_STATUS
ixAtmdAccUtilUninit (void)
{
    IX_STATUS returnStatus = IX_FAIL;

    if (TRUE == initDone)
    {
        returnStatus = ixOsalMutexDestroy (&utilLock);

        if (IX_SUCCESS != returnStatus)
        {
            returnStatus = IX_FAIL;
        }
        else
        {
            initDone = FALSE;
        } /* end of if-else(returnStatus) */
    } /* end of if(initDone) */
    return returnStatus;
}



/* ---------------------------------------------------------
*   Queues Initialisation
*/
IX_STATUS
ixAtmdAccUtilQueuesInit (void)
{
    IX_STATUS returnStatus = IX_SUCCESS;
    IxQMgrQId txQueueIdx;
    IxQMgrQId rxFreeQueueIdx;
    unsigned int rxFreeQueueCount;
    unsigned int txQueueIdSize[IX_QMGR_MAX_NUM_QUEUES];

    if (queueInitDone == TRUE)
    {
       return IX_FAIL;
    }
    
    /* initialise the array for the tx queue size */
    txQueueIdx = IX_NPE_A_QMQ_ATM_TXID_MIN;
    txQueueIdSize[txQueueIdx++] = IX_ATMDACC_TXQUEUE0_SIZE;
#ifdef IX_NPE_MPHYMULTIPORT
    txQueueIdSize[txQueueIdx++] = IX_ATMDACC_TXQUEUE1_SIZE;
    txQueueIdSize[txQueueIdx++] = IX_ATMDACC_TXQUEUE2_SIZE;
    txQueueIdSize[txQueueIdx++] = IX_ATMDACC_TXQUEUE3_SIZE;
    txQueueIdSize[txQueueIdx++] = IX_ATMDACC_TXQUEUE4_SIZE;
    txQueueIdSize[txQueueIdx++] = IX_ATMDACC_TXQUEUE5_SIZE;
    txQueueIdSize[txQueueIdx++] = IX_ATMDACC_TXQUEUE6_SIZE;
    txQueueIdSize[txQueueIdx++] = IX_ATMDACC_TXQUEUE7_SIZE;
    txQueueIdSize[txQueueIdx++] = IX_ATMDACC_TXQUEUE8_SIZE;
    txQueueIdSize[txQueueIdx++] = IX_ATMDACC_TXQUEUE9_SIZE;
    txQueueIdSize[txQueueIdx++] = IX_ATMDACC_TXQUEUE10_SIZE;
    txQueueIdSize[txQueueIdx++] = IX_ATMDACC_TXQUEUE11_SIZE;
#endif

    /* initialise the RX queues */
    returnStatus = ixQMgrQConfig ("RX high", 
        IX_NPE_A_QMQ_ATM_RX_HI, 
        IX_ATMDACC_RX_QUEUE_SIZE,
        IX_QMGR_Q_ENTRY_SIZE1);

    if (returnStatus == IX_SUCCESS)
    {
        returnStatus = ixQMgrQConfig ("RX low", 
            IX_NPE_A_QMQ_ATM_RX_LO, 
            IX_ATMDACC_RX_QUEUE_SIZE,
            IX_QMGR_Q_ENTRY_SIZE1);
    }

    /* initialise the TX queues */
    for (txQueueIdx = IX_NPE_A_QMQ_ATM_TXID_MIN;
        (txQueueIdx <= IX_NPE_A_QMQ_ATM_TXID_MAX)
        && (returnStatus == IX_SUCCESS);
        txQueueIdx++)
    {
        returnStatus = ixQMgrQConfig ("Tx Vc", 
            txQueueIdx, 
            txQueueIdSize[txQueueIdx], 
            IX_QMGR_Q_ENTRY_SIZE1);
    }

    if (returnStatus == IX_SUCCESS)
    {
        /* initialise the tx done queue */
        returnStatus = ixQMgrQConfig ("TX Done", 
            IX_NPE_A_QMQ_ATM_TX_DONE, 
            IX_ATMDACC_TXDONE_QUEUE_SIZE, 
            IX_QMGR_Q_ENTRY_SIZE1);
    }

    /* initialise the first rx free queues with a big size 32 */
    for (rxFreeQueueCount = 0, 
        rxFreeQueueIdx = IX_NPE_A_QMQ_ATM_RXFREE_MIN;
        (rxFreeQueueCount < IX_ATMDACC_DOUBLE_SIZE_RXFREE_COUNT)
        && (rxFreeQueueIdx <= IX_NPE_A_QMQ_ATM_RXFREE_MAX)
        && (returnStatus == IX_SUCCESS);
        rxFreeQueueIdx++, 
        rxFreeQueueCount++)
    {
        /* allocate RX free queues with a 32 entries */
        returnStatus = ixQMgrQConfig ("Rx free", 
            rxFreeQueueIdx, 
            IX_QMGR_Q_SIZE32, 
            IX_QMGR_Q_ENTRY_SIZE1);
    }

    /* initialise the next rx free queues with smaller size 16 */
    for (;
        (rxFreeQueueIdx <= IX_NPE_A_QMQ_ATM_RXFREE_MAX)
        && (returnStatus == IX_SUCCESS);
        rxFreeQueueIdx++)
    {
        /* allocate RX free queues with 16 entries */
        returnStatus = ixQMgrQConfig ("Rx free", 
            rxFreeQueueIdx, 
            IX_ATMDACC_QMGR_OAM_FREE_QUEUE_SIZE, 
            IX_QMGR_Q_ENTRY_SIZE1);
    }


    /* initialise the OAM rx free queue */
    if(returnStatus == IX_SUCCESS)
    {
        returnStatus = ixQMgrQConfig ("Rx OAM free", 
            IX_NPE_A_QMQ_OAM_FREE_VC, 
            IX_QMGR_Q_SIZE16, 
            IX_QMGR_Q_ENTRY_SIZE1);
    }

    if (returnStatus != IX_SUCCESS)
    {
        returnStatus = IX_FAIL;
    }
    else
    {
        queueInitDone = TRUE;
    }

    return returnStatus;
}



/* ---------------------------------------------------------
*   Queues Unload
*/
IX_STATUS
ixAtmdAccUtilQueuesUninit (void)
{
    IX_STATUS returnStatus = IX_SUCCESS;

    if (TRUE != queueInitDone)
    {
       return IX_FAIL;
    }
    else
    {
        queueInitDone = FALSE;
        returnStatus = IX_SUCCESS;
    }
    return returnStatus;
}



/* ---------------------------------------------------------
*    Module stats display
*/
void
ixAtmdAccUtilStatsShow (void)
{
    IX_ATMDACC_FULL_STATS(
    printf("Next uniqueId ..... : %10u\n", uniqueId); 
    );
}

/* ---------------------------------------------------------
*    Module stats reset
*/
void
ixAtmdAccUtilStatsReset (void)
{
}

/* ---------------------------------------------------------
*    Set a QMgr callback
*/
IX_STATUS
ixAtmdAccUtilQmgrCallbackSet (IxQMgrQId qId,
                              unsigned int loThresholdLevel,
                              unsigned int hiThresholdLevel,
                              IxQMgrSourceId sourceId,
                              IxQMgrCallback callback,
                              IxQMgrCallbackId callbackId,
                              IxQMgrPriority priority)
{
    IX_STATUS returnStatus = IX_SUCCESS;

    /* disable queue interrupt and enable threshold */
    if((ixQMgrNotificationDisable (qId) != IX_SUCCESS)                            ||
        (ixQMgrWatermarkSet (qId, loThresholdLevel, hiThresholdLevel) != IX_SUCCESS)   ||
        (ixQMgrNotificationCallbackSet (qId, callback, callbackId) != IX_SUCCESS)  ||
        (ixQMgrNotificationEnable (qId, sourceId) != IX_SUCCESS) ||
        (ixQMgrDispatcherPrioritySet (qId, priority) != IX_SUCCESS))
    {
    /* on failure, attempt to disable the interrupts before
    * returning
        */
        ixQMgrNotificationDisable (qId);
        returnStatus = IX_FAIL;
    }
    return returnStatus;
}

/* -------------------------------------------------------
* CRC table as per RFC 1331 and ISO 3309
*   Point-to-Point Protocol
*.  Fast Frame Check Sequence (FCS) Implementation
* CRC table as per ISO 3309
*   ISO3309, "Information Technology - Telecommunications and
*   inform", ation exchange between systems - High-level data
*   link control procedures - Frame structure ISO/IEC 3309:
*   1993, 1993.
* See also Checksum (CCITT16) for packet corruption detection
*/

static unsigned int crctab[256] = {
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
        0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
        0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
        0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
        0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
        0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
        0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
        0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
        0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
        0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
        0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
        0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
        0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
        0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
        0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
        0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
        0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
        0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
        0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
        0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
        0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
        0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
        0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
        0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
        0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
        0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
        0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
        0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
        0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
        0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
        0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
        0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

/* -------------------------------------------------------
* CRC algorithm as per RFC 1331 and ISO 3309
*/
static unsigned int
crc16 (unsigned int fcs,
       unsigned char *cp,
       int len)
{
    while (len--)
    {
        fcs = (fcs >> 8) ^ crctab[(fcs ^ *cp++) & 0xff];
    }

    return (fcs);
}

/*------------------------------------------------------------------------
*  Compute a Hash value from a port/vpi/vci
*/
unsigned int
ixAtmdAccUtilHashVc (IxAtmLogicalPort port,
                     unsigned int vpi,
                     unsigned int vci)
{
    unsigned char hashData[5];

    /* endianness storage in an array of bytes */
    hashData[0] = 0;                           /* gfc / vpi */
    hashData[1] = (unsigned char) vpi;         /* vpi LSB */
    hashData[2] = (unsigned char) (vci >> 8);  /* vci MSB */
    hashData[3] = (unsigned char) vci;         /* vci LSB */
    hashData[4] = (unsigned char) port;        /* logical port */

   /* compute the crc of this array : the UTOPIA2 coprocessor is implementing
    * the same standard
    */
    return crc16 (0xffff, hashData, sizeof (hashData));

}

/*------------------------------------------------------------------------
* Allocate a unique ID used for connections security and sanity checks
*/
unsigned int
ixAtmdAccUtilUniqueIdGet (void)
{
    unsigned int result = 0;

    IX_ATMDACC_UTIL_LOCK ();

    /* grab a lock and increment the number by IX_ATMDACC_UNIQUEID_INCR
    to provide a part of a connection ID.
    */
    uniqueId += IX_ATMDACC_UNIQUEID_INCR;

    /* test for wrap around */
    if (uniqueId >= IX_ATMDACC_UNIQUEID_WRAP_LIMIT)
    {
        uniqueId = IX_ATMDACC_UNIQUEID_SEED;
    }

    result = uniqueId;

    IX_ATMDACC_UTIL_UNLOCK ();

    return result;
}

/*------------------------------------------------------------------------
* convert chained mbufs to BIG endian (npe format)
*/
IX_OSAL_MBUF *
ixAtmdAccUtilMbufToNpeFormatConvert (IX_OSAL_MBUF *mbufPtr)
{
    IX_ATMDACC_CONVERT_TO_PHYSICAL_ADDRESS (IX_OSAL_MBUF_MDATA(mbufPtr));
    IX_ATMDACC_CONVERT_TO_BIG_ENDIAN (UINT8 *,IX_OSAL_MBUF_MDATA(mbufPtr));
    IX_ATMDACC_CONVERT_TO_BIG_ENDIAN (int,IX_OSAL_MBUF_MLEN(mbufPtr));

    /* quick check for unchained mbufs */
    if (IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr) == NULL)
    {
        /* force data to be stored in physical memory */
        IX_ATMDACC_DATA_CACHE_FLUSH(mbufPtr, sizeof(IX_OSAL_MBUF));

        /* convert the pointer to the first mbuf */
        IX_ATMDACC_CONVERT_TO_PHYSICAL_ADDRESS (mbufPtr);
        IX_ATMDACC_CONVERT_TO_BIG_ENDIAN (IX_OSAL_MBUF *,mbufPtr);
    }
    else
    {
        IX_OSAL_MBUF *mbufChainPtr;
        IX_OSAL_MBUF *mbufTempPtr;
        int iterationCount = 1;
        
        mbufChainPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr);

        IX_ATMDACC_CONVERT_TO_PHYSICAL_ADDRESS (IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr));
        IX_ATMDACC_CONVERT_TO_BIG_ENDIAN (IX_OSAL_MBUF *,IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr));
        
        /* force data to be stored in physical memory */
        IX_ATMDACC_DATA_CACHE_FLUSH(mbufPtr, sizeof(IX_OSAL_MBUF));

        /* iterate through the mbuf chain */
        while (mbufChainPtr)
        {
            iterationCount++;
            
            /* check about possible  mbuf infinite chain or loop */
            IX_ATMDACC_ENSURE (iterationCount <= IX_NPE_A_CHAIN_DESC_COUNT_MAX, "Infinite mbuf chain");
            IX_ATMDACC_ENSURE (IX_OSAL_MBUF_MLEN(mbufChainPtr) != 0, "Null length mbuf");
            
            /* save the next pointer */
            mbufTempPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufChainPtr);
            
            /* convert the current mbuf header */
            IX_ATMDACC_CONVERT_TO_PHYSICAL_ADDRESS (IX_OSAL_MBUF_MDATA(mbufChainPtr));
            IX_ATMDACC_CONVERT_TO_BIG_ENDIAN (UINT8 *,IX_OSAL_MBUF_MDATA(mbufChainPtr));
            IX_ATMDACC_CONVERT_TO_BIG_ENDIAN (int,IX_OSAL_MBUF_MLEN(mbufChainPtr));

            if (mbufTempPtr)
            {
                IX_ATMDACC_CONVERT_TO_PHYSICAL_ADDRESS (IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufChainPtr));
                IX_ATMDACC_CONVERT_TO_BIG_ENDIAN (IX_OSAL_MBUF *,IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufChainPtr));
            }

            /* force data to be stored in physical memory */
            IX_ATMDACC_DATA_CACHE_FLUSH(mbufChainPtr, sizeof(IX_OSAL_MBUF));
            
            /* next element */
            mbufChainPtr = mbufTempPtr;
        } /* end of while(mbufChainPtr) */

        /* convert the pointer to the first mbuf */
        IX_ATMDACC_CONVERT_TO_PHYSICAL_ADDRESS (mbufPtr);
        IX_ATMDACC_CONVERT_TO_BIG_ENDIAN (IX_OSAL_MBUF *,mbufPtr);
    }
    return (mbufPtr);
}

/*------------------------------------------------------------------------
* convert chained mbufs from BIG endian (npe format)
*/
IX_OSAL_MBUF *
ixAtmdAccUtilMbufFromNpeFormatConvert (IX_OSAL_MBUF *mbufPtr,
                                       BOOL invalidateCache)
{
    if (invalidateCache)
    {
        /* force data to be read from physical memory */
        IX_ATMDACC_DATA_CACHE_INVALIDATE(mbufPtr, sizeof(IX_OSAL_MBUF));
    }
    
    /* convert the current mbuf header */
    IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN (int,IX_OSAL_MBUF_MLEN(mbufPtr));
    IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN (UINT8 *,IX_OSAL_MBUF_MDATA(mbufPtr));
    IX_ATMDACC_CONVERT_TO_VIRTUAL_ADDRESS (IX_OSAL_MBUF_MDATA(mbufPtr));
    
    /* quick check for unchained mbufs */
    if (IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr) != NULL)
    {
        IX_OSAL_MBUF *mbufChainPtr;
        int iterationCount = 1;

        IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN (IX_OSAL_MBUF *,IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr));
        IX_ATMDACC_CONVERT_TO_VIRTUAL_ADDRESS (IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr));

        mbufChainPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr);

        /* iterate through the mbuf chain */
        while (mbufChainPtr)
        {
            /* update the mbuf count */
            iterationCount++;
            
            /* check about possible  mbuf infinite chain or loop */
            IX_ATMDACC_ABORT (iterationCount <= IX_NPE_A_CHAIN_DESC_COUNT_MAX, 
                "Infinite mbuf chain");
            
            if (invalidateCache)
            {
                /* force data to be read from physical memory */
                IX_ATMDACC_DATA_CACHE_INVALIDATE(mbufChainPtr, sizeof(IX_OSAL_MBUF));
            }
            
            /* convert the next mbuf pointer */
            if (IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufChainPtr))
            {
                IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN (IX_OSAL_MBUF *,IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufChainPtr));
                IX_ATMDACC_CONVERT_TO_VIRTUAL_ADDRESS (IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufChainPtr));
            }
            
            /* convert the current mbuf header */
            IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN (int,IX_OSAL_MBUF_MLEN(mbufChainPtr));
            IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN (UINT8 *, IX_OSAL_MBUF_MDATA(mbufChainPtr));
            IX_ATMDACC_CONVERT_TO_VIRTUAL_ADDRESS (IX_OSAL_MBUF_MDATA(mbufChainPtr));
            
            mbufChainPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufChainPtr);
        } /* end of while(mbufChainPtr) */
    }

    return (mbufPtr);
}

/*------------------------------------------------------------------------
* convert RX chained mbufs from BIG endian (npe format)
*/
IX_OSAL_MBUF *
ixAtmdAccUtilRxMbufFromNpeFormatConvert (IX_OSAL_MBUF *mbufPtr, 
                                         unsigned int pduLength,
                                         unsigned int *mbufCount)
{
    /* no PFU can be 0 bytes long, because there is always a 
    * AAL5 trailer or a cell
    */
    IX_OSAL_ENSURE(pduLength >= IX_ATM_CELL_PAYLOAD_SIZE, 
        "Wrong pdu length in RX direction");

    /* force data to be read from physical memory */
    IX_ATMDACC_DATA_CACHE_INVALIDATE(mbufPtr, sizeof(IX_OSAL_MBUF));
    
    /* convert the current mbuf header */
    IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN (UINT8 *,IX_OSAL_MBUF_MDATA(mbufPtr));
    IX_ATMDACC_CONVERT_TO_VIRTUAL_ADDRESS (IX_OSAL_MBUF_MDATA(mbufPtr));
    
    /* quick check for unchained mbufs */
    if (IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr) == NULL)
    {
        /* the last mbuf length is not uipdated by the NPE, and
        * the pdu length is not updated by the NPE in the first mbuf 
        */
        IX_OSAL_MBUF_MLEN(mbufPtr) = (int)pduLength;
        IX_OSAL_MBUF_PKT_LEN(mbufPtr) = pduLength;
        *mbufCount = 1;
    }
    else
    {
        int mbufChainLength;
        IX_OSAL_MBUF *mbufChainPtr;
        IX_OSAL_MBUF *mbufChainLastPtr = mbufPtr;
        int iterationCount = 1;

        IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN (int,IX_OSAL_MBUF_MLEN(mbufPtr));
        IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN (IX_OSAL_MBUF *,IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr));
        IX_ATMDACC_CONVERT_TO_VIRTUAL_ADDRESS (IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr));

        IX_OSAL_ENSURE(IX_OSAL_MBUF_MLEN(mbufPtr) != 0, "0 byte mbuf in RX direction");

        mbufChainLength = IX_OSAL_MBUF_MLEN(mbufPtr);
        mbufChainPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr);

        /* iterate through the mbuf chain */
        while (mbufChainPtr)
        {
            /* update the mbuf count */
            iterationCount++;
            
            /* check about possible  mbuf infinite chain or loop */
            IX_ATMDACC_ABORT (iterationCount <= IX_NPE_A_CHAIN_DESC_COUNT_MAX, 
                "Infinite mbuf chain");
            
            /* force data to be read from physical memory */
            IX_ATMDACC_DATA_CACHE_INVALIDATE(mbufChainPtr, sizeof(IX_OSAL_MBUF));
            
            /* convert the next mbuf pointer */
            if (IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufChainPtr))
            {
                IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN (IX_OSAL_MBUF *,IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufChainPtr));
                IX_ATMDACC_CONVERT_TO_VIRTUAL_ADDRESS (IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufChainPtr));
            }
            else
            {
                /* no mbuf should be 0 bytes long, except the last one where
                * the NPE didn't update the header
                */
                IX_OSAL_ENSURE(IX_OSAL_MBUF_MLEN(mbufPtr) != 0, "0 byte mbuf in RX direction");
            }
            
            /* convert the current mbuf header */
            IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN (UINT8 *,IX_OSAL_MBUF_MDATA(mbufChainPtr));
            IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN (int,IX_OSAL_MBUF_MLEN(mbufChainPtr));
            IX_ATMDACC_CONVERT_TO_VIRTUAL_ADDRESS (IX_OSAL_MBUF_MDATA(mbufChainPtr));
            

            mbufChainLength += IX_OSAL_MBUF_MLEN(mbufChainPtr);
            /* keep track of the lst element */
            mbufChainLastPtr = mbufChainPtr;
            /* next element */
            mbufChainPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufChainPtr);
        } /* end of while(mbufChainPtr) */
        
        /* update the number of mbufs found */
        *mbufCount = iterationCount;
        
        /* the last mbuf length is not uipdated by the NPE, and
        * the pdu length is not updated by the NPE in the first mbuf 
        */
        IX_OSAL_MBUF_MLEN(mbufChainLastPtr) += (int)pduLength - mbufChainLength;
        IX_OSAL_MBUF_PKT_LEN(mbufPtr) = pduLength;
    }

    return (mbufPtr);
}

/*------------------------------------------------------------------------
* Atmd stream Id check and conversion to QMgr id enum
*/
IX_STATUS
ixAtmdAccUtilQmgrQIdGet (IxAtmRxQueueId atmdQueueId,
                         IxQMgrQId *qMgrQueueIdPtr)
{
    IX_STATUS returnStatus = IX_SUCCESS;

    switch (atmdQueueId)
    {
    case IX_ATM_RX_A:
        *qMgrQueueIdPtr = IX_NPE_A_QMQ_ATM_RX_HI;
        break;
    case IX_ATM_RX_B:
        *qMgrQueueIdPtr = IX_NPE_A_QMQ_ATM_RX_LO;
        break;
    default:
        returnStatus = IX_FAIL;
        break;
    } /* end of switch(atmdQueueId) */
    return returnStatus;
}

/*------------------------------------------------------------------------
* QMgr Id check and conversion to IxAtmdAcc enum
*/
IX_STATUS
ixAtmdAccUtilAtmdQIdGet (IxQMgrQId qMgrQueueId,
                         IxAtmRxQueueId *atmdQueueIdPtr)
{
    IX_STATUS returnStatus = IX_SUCCESS;

    switch (qMgrQueueId)
    {
    case IX_NPE_A_QMQ_ATM_RX_HI :
        *atmdQueueIdPtr = IX_ATM_RX_A;
        break;
    case IX_NPE_A_QMQ_ATM_RX_LO:
        *atmdQueueIdPtr = IX_ATM_RX_B;
        break;
    default:
        returnStatus = IX_FAIL;
        break;
    } /* end of switch(atmdQueueId) */
    return returnStatus;
}

/* ----------------------------------------------------
* send a message to the NPE
*/
IX_STATUS
ixAtmdAccUtilNpeMsgSend(unsigned int msgType,
                        unsigned int param0,
                        unsigned int param1)
{
    IxNpeMhMessage npeMhMessage;
    IX_STATUS returnStatus;

    npeMhMessage.data[0] = (msgType << NPE_MSG_COMMAND_BIT_OFFSET) | param0;
    npeMhMessage.data[1] = param1;


    /* send msg to NPE */
    returnStatus = ixNpeMhMessageSend (IX_NPEMH_NPEID_NPEA, 
        npeMhMessage, 
        IX_ATMDACC_SEND_RETRIES_DEFAULT);

    /* map status error */
    if (returnStatus != IX_SUCCESS)
    {
        returnStatus = IX_FAIL;
    }
    return returnStatus;
}

/* ----------------------------------------------------
* send a message to the NPE, wait for a response
*/
IX_STATUS
ixAtmdAccUtilNpeMsgWithResponseSend(unsigned int msgType,
                                    unsigned int param0,
                                    unsigned int param1,
                                    IxNpeMhCallback callback)
{
    IxNpeMhMessage npeMhMessage;
    IX_STATUS returnStatus;

    npeMhMessage.data[0] = (msgType << NPE_MSG_COMMAND_BIT_OFFSET) | param0;
    npeMhMessage.data[1] = param1;


    /* send msg to NPE */
    returnStatus = ixNpeMhMessageWithResponseSend (IX_NPEMH_NPEID_NPEA,
        npeMhMessage,
        msgType,
        callback,
        IX_ATMDACC_SEND_RETRIES_DEFAULT);

    /* map status error */
    if (returnStatus != IX_SUCCESS)
    {
        returnStatus = IX_FAIL;
    }
    return returnStatus;
}

/* --------------------------------------------------
* Query a queue size
*/
IX_STATUS
ixAtmdAccUtilQueueSizeQuery (IxQMgrQId qmgrQueueId,
                             unsigned int *queueSizePtr)
{
    IX_STATUS returnStatus;

    /* get the queue size */
    returnStatus = ixQMgrQSizeInEntriesGet(qmgrQueueId, queueSizePtr);

    /* map the error */
    if (returnStatus != IX_SUCCESS)
    {
        returnStatus = IX_FAIL;
    }

    return returnStatus;
}

