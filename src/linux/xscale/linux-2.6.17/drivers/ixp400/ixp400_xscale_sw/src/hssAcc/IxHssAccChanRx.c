/**
 * @file IxHssAccChanRx.c
 *
 * @author Intel Corporation
 * @date 30-Jan-02
 *
 * @brief HssAccess Channelised Rx Interface
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
 * Put the system defined include files required.
 */

/**
 * Put the user defined include files required.
 */

#include "IxHssAccError_p.h"
#include "IxQMgr.h"
#include "IxOsal.h"

#include "IxHssAccChanRx_p.h"
#include "IxHssAccCCM_p.h"
#include "IxHssAccCommon_p.h"
#include "IxHssAccNpeA_p.h"

/**
 * #defines and macros used in this file.
 */

/**
 * Typedefs whose scope is limited to this file.
 */

typedef struct
{
    unsigned validTrigCbs;
    unsigned invalidTrigCbs;
    unsigned validQReads;
    unsigned underflows;
    unsigned invalidQReads;
} IxHssAccChanRxStats;

/**
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */

static IxHssAccChanRxStats ixHssAccChanRxStats;

/**
 * Extern function prototypes.
 */

/**
 * Static function prototypes.
 */

/**
 * Function definition: ixHssAccChanRxTriggerCallback
 */
void 
ixHssAccChanRxTriggerCallback (IxQMgrQId qId, 
			       IxQMgrCallbackId cbId)
{
    IX_STATUS status;
    unsigned queueEntry = 0;
    unsigned rxOffset;
    unsigned txOffset;
    unsigned numHssErrs;
    unsigned internalReads = 0;
    IxHssAccHssPort hssPortId = (IxHssAccHssPort) cbId;    

    /* hssPortId is mapped to cbId on registration - one cb per port */
    if (!IX_HSSACC_ENUM_INVALID (hssPortId, hssPortMax))
    {
	/* stats */
	ixHssAccChanRxStats.validTrigCbs++;

	do 
	{
	    /* read the message off the queue */
	    status = ixQMgrQRead (qId, &queueEntry);
	    internalReads++;
	    if (status == IX_SUCCESS)
	    {
		ixHssAccChanRxStats.validQReads++;
		rxOffset = (queueEntry & IX_HSSACC_NPE_CHANRXQ_RXOFFSET_MASK) >> 
		    IX_HSSACC_NPE_CHANRXQ_RXOFFSET_OFFSET;
		txOffset = (queueEntry & IX_HSSACC_NPE_CHANRXQ_TXOFFSET_MASK) >> 
		    IX_HSSACC_NPE_CHANRXQ_TXOFFSET_OFFSET;
		numHssErrs = (queueEntry & IX_HSSACC_NPE_CHANRXQ_NUMERRS_MASK) >> 
		    IX_HSSACC_NPE_CHANRXQ_NUMERRS_OFFSET;
		ixHssAccCCMRxCallbackRun (hssPortId, rxOffset, txOffset, 
					  numHssErrs);
	    }
	    else if (status == IX_QMGR_Q_UNDERFLOW)
	    {
		/* stats */
		ixHssAccChanRxStats.underflows++;
	    }
	    else
	    {
		/* stats */
		ixHssAccChanRxStats.invalidQReads++;
	    }
	} while (status == IX_SUCCESS);

    }
    else
    {
	/* stats */
	ixHssAccChanRxStats.invalidTrigCbs++;
    }
}

/**
 * Function definition: ixHssAccChanRxQCheck
 */
IX_STATUS 
ixHssAccChanRxQCheck (IxHssAccHssPort hssPortId, 
		      BOOL *dataRecvd, 
		      unsigned *rxOffset, 
		      unsigned *txOffset, 
		      unsigned *numHssErrs)
{
    IX_STATUS status = IX_SUCCESS;
    IxQMgrQId qId;
    unsigned queueEntry = 0;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccChanRxQCheck\n");

    /* check to see if a service is enabled on the specified hssPortId */
    if (ixHssAccCCMIsPortEnabled (hssPortId) == FALSE)
    {
	/* report the error */
	IX_HSSACC_REPORT_ERROR ("ixHssAccChanRxQCheck - no service enabled on hssPortId\n");
	/* return error */
	return IX_FAIL;
    }

    *dataRecvd = FALSE;
    /* get the appropriate qId */
    qId = ixHssAccCCMQidGet (hssPortId);

    status = ixQMgrQRead (qId, &queueEntry);
    if (status == IX_SUCCESS)
    {
	ixHssAccChanRxStats.validQReads++;
	*dataRecvd = TRUE;
	*rxOffset = (queueEntry & IX_HSSACC_NPE_CHANRXQ_RXOFFSET_MASK) >> 
	    IX_HSSACC_NPE_CHANRXQ_RXOFFSET_OFFSET;
	*txOffset = (queueEntry & IX_HSSACC_NPE_CHANRXQ_TXOFFSET_MASK) >> 
	    IX_HSSACC_NPE_CHANRXQ_TXOFFSET_OFFSET;
	*numHssErrs = (queueEntry & IX_HSSACC_NPE_CHANRXQ_NUMERRS_MASK) >> 
	    IX_HSSACC_NPE_CHANRXQ_NUMERRS_OFFSET;
    }
    else if (status == IX_QMGR_Q_UNDERFLOW)
    {
	/* stats */
	ixHssAccChanRxStats.underflows++;
	status = IX_SUCCESS; /* call succeeded, but no data read */
    }
    else
    {
	/* stats */
	ixHssAccChanRxStats.invalidQReads++;
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccChanRxQCheck\n");
    return status;
}

/**
 * Function definition: ixHssAccChanRxShow
 */
void
ixHssAccChanRxShow (void)
{
    printf ("\nixHssAccChanRxShow:\n");
    printf ("\tvalidTrigCbs: %d \tinvalidTrigCbs: %d\n", 
	    ixHssAccChanRxStats.validTrigCbs, 
	    ixHssAccChanRxStats.invalidTrigCbs);
    printf ("\t validQReads: %d \t invalidQReads: %d\n", 
	    ixHssAccChanRxStats.validQReads, 
	    ixHssAccChanRxStats.invalidQReads);
    printf ("\t  underflows: %d\n", 
	    ixHssAccChanRxStats.underflows);
}

/**
 * Function definition: ixHssAccChanRxStatsInit
 */
void 
ixHssAccChanRxStatsInit (void)
{
    ixHssAccChanRxStats.validTrigCbs = 0;
    ixHssAccChanRxStats.invalidTrigCbs = 0;
    ixHssAccChanRxStats.validQReads = 0;
    ixHssAccChanRxStats.underflows = 0;
    ixHssAccChanRxStats.invalidQReads = 0;
}

/**
 * Function definition: ixHssAccChanRxInit
 */
IX_STATUS 
ixHssAccChanRxInit (void)
{
    IX_STATUS status = IX_SUCCESS;
    IxHssAccHssPort hssPortIndex;
    IxQMgrQId qId;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Entering ixHssAccChanRxInit\n");

    /* initialise stats */
    ixHssAccChanRxStatsInit ();
    
    /* set the callbacks for each specific Rx QMQ */
    for (hssPortIndex = IX_HSSACC_HSS_PORT_0; 
	 hssPortIndex < hssPortMax;
	 hssPortIndex++)
    {
	qId = ixHssAccCCMQidGet (hssPortIndex);
	status = ixQMgrNotificationCallbackSet (qId, 
						ixHssAccChanRxTriggerCallback, 
						hssPortIndex);
	if (status != IX_SUCCESS)
	{
	    /* report the error */
	    IX_HSSACC_REPORT_ERROR ("ixHssAccChanRxInit - failed to set QMQ callback\n");
	    /* return error */
	    return IX_FAIL;
	}
    }
    
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, 
		      "Exiting ixHssAccChanRxInit\n");

    return status;
}


/**
 * Function definition: ixHssAccChanRxUninit
 */
IX_STATUS
ixHssAccChanRxUninit (void)
{
    IX_STATUS       status = IX_SUCCESS;
    IxHssAccHssPort hssPortIndex;
    IxQMgrQId       qId;

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT,
                      "Entering ixHssAccChanRxUninit\n");

    /* set the callbacks to dummy callbacks for each specific Rx QMQ */
    for (hssPortIndex = IX_HSSACC_HSS_PORT_0;
         hssPortIndex < hssPortMax;
         hssPortIndex++)
    {
        qId = ixHssAccCCMQidGet (hssPortIndex);
        status = ixQMgrNotificationCallbackSet (qId,
                                                NULL,
                                                0);
        if (IX_SUCCESS != status)
        {
            /* report the error */
            IX_HSSACC_REPORT_ERROR ("ixHssAccChanRxUninit - failed to "
                                    "set QMQ dummy callback\n");
            /* return error */
            return IX_FAIL;
        }
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT,
                      "Exiting ixHssAccChanRxUninit\n");

    return status;
}



