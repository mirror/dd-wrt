/**
 * @file    IxQMgrDispatcher_p.h
 *
 * @author Intel Corporation
 * @date    07-Feb-2002
 *
 * @brief   This file contains the internal functions for dispatcher
 *
 * 
 * @par
 * IXP400 SW Release version  2.0
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright 2002-2005 Intel Corporation All Rights Reserved.
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

#ifndef IXQMGRDISPATCHER_P_H
#define IXQMGRDISPATCHER_P_H

/*
 * User defined include files
 */
#include "IxQMgr.h"

/*
 * This structure defines the statistic data for a queue
 */
typedef struct
{
    unsigned callbackCnt;       /* Call count of callback                    */
    unsigned priorityChangeCnt; /* Priority change count                     */
    unsigned intNoCallbackCnt;  /* Interrupt fired but no callback set count */
    unsigned intLostCallbackCnt;  /* Interrupt lost and detected ;  SCR541   */
    BOOL notificationEnabled;    /* Interrupt enabled for this queue         */
    IxQMgrSourceId srcSel;       /* interrupt source                         */
    unsigned enableCount;        /* num times notif enabled by LLP           */
    unsigned disableCount;       /* num of times notif disabled by LLP       */
} IxQMgrDispatcherQStats;

/*
 * This structure defines statistic data for the disatcher
 */
typedef struct
 {
    unsigned loopRunCnt;       /* ixQMgrDispatcherLoopRun count */

    IxQMgrDispatcherQStats queueStats[IX_QMGR_MAX_NUM_QUEUES];

} IxQMgrDispatcherStats;

/*
 * Initialise the dispatcher component
 */
void
ixQMgrDispatcherInit (void);

/*
 * Get the dispatcher statistics
 */
IxQMgrDispatcherStats*
ixQMgrDispatcherStatsGet (void);

/**
 * Retrieve the number of leading zero bits starting from the MSB 
 * This function is implemented as an (extremely fast) asm routine 
 * for XSCALE processor (see clz instruction) and as a (slower) C 
 * function for other systems.
 */
unsigned int
ixQMgrCountLeadingZeros(unsigned int value);

#endif/*IXQMGRDISPATCHER_P_H*/


