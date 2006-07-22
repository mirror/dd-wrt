/*
 * @file        IxAtmSch_p.h
 * @author Intel Corporation
 * @date        13-12-2000  Ported:      1-Mar-2002
 *
 * @brief Private header file for AtmSch component.
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

#ifndef IXATMSCH_P_H
#define IXATMSCH_P_H

#include "IxOsalTypes.h"
#include "IxAtmTypes.h"

/*
 * Defines
 */
#define IX_ATMSCH_nS_PER_SECOND      1000000000
#define IX_ATMSCH_US_PER_SECOND      1000000
#define IX_ATMSCH_MAX_TABLE_ENTRIES  10
#define IX_ATMSCH_NULL_INDEX         -1
#define IX_ATMSCH_UINT_MASK          0x80000000
#define IX_ATMSCH_SCR_PERIOD_IN_SEC  60 /* seconds */

/*
* Execute statements if NDEBUG not defined.
*/
#ifndef NDEBUG
#define IX_ATMSCH_STATS(statements) statements
#else
#define IX_ATMSCH_STATS(x)
#endif


/*
 * Typedefs
 */
typedef struct
{
    UINT32 cet;               /* cell emission time = MAX(cet_scr, cet_pcr) */
    UINT32 cetScr;            /* CET calculated with SCR */
    UINT32 cetPcr;            /* CET calculated with PCR */
    UINT32 bt;                /* Burst Tolerance = ((MBS - 1) * (scr_us - pcr_us)) */
    UINT32 usScr;             /* SCR expressed in microsecond/cell, (represents 1/SCR) */
    UINT32 usPcr;             /* PCR expressed in microsecond/cell, (represents 1/PCR) */
    UINT32 mbs;               /* Maximum Burst Size in cells */
    UINT32 baseBt;            /* Burst Tolerance base value */
    UINT32 scr;
    UINT32 pcr;
} IxAtmSchVcSchedInfo;

typedef struct
{
    IxAtmServiceCategory atmService; /* The ATM service category */
    UINT32 vbrPcrCellsCnt;     /* The remaining of VBR burst cells */
    UINT32 vbrScrCellsCnt;     /* The remaining of VBR burst cells */
    BOOL inUse;			 /* indicates whether the table
				  * element is currently in use */
    IxAtmLogicalPort port;       /* The port on which the VC is enabled */
    UINT32 count;		 /* a count of the queued cells for the VC */
    INT32 nextVc;	         /* the next VC in the chain */
    IxAtmConnId connId;          /* connId which the scheduling client knows
                                    the VC. */
    IxAtmSchVcSchedInfo schInfo; /* Scheduling information for this VC */
} IxAtmSchVcInfo;

typedef struct
{
    UINT32 idleCellsScheduled;
    UINT32 cellsQueued;
    UINT32 updateCalls;
    UINT32 queueFull;
    UINT32 cellsScheduled;
    UINT32 scheduleTableCalls;
} IxAtmSchStats;

/*
 * Function Prototypes
 */
void
ixAtmSchedulingInit(void);

void
ixAtmSchCellTimeSet(IxAtmLogicalPort port, UINT32 cellTime );

UINT32
ixAtmSchCellTimeGet(IxAtmLogicalPort port);

void
ixAtmSchMinCellsSet(IxAtmLogicalPort port, UINT32 minCellsToSchedule);

UINT32
ixAtmSchMinCellsGet(IxAtmLogicalPort port);

void
ixAtmSchBubbleSortRtVcQueue(IxAtmLogicalPort port);

void
ixAtmSchTimerOverrun (IxAtmLogicalPort port);

IX_STATUS
ixAtmSchBaseTimeSet (IxAtmLogicalPort port, UINT32 baseTime);

#endif /* IXATMSCH_P_H */
