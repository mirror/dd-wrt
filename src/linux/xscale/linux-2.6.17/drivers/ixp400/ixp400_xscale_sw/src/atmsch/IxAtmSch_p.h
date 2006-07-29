/*
 * @file        IxAtmSch_p.h
 * @author Intel Corporation
 * @date        13-12-2000  Ported:      1-Mar-2002
 *
 * @brief Private header file for AtmSch component.
 * 
 * @par
 * IXP400 SW Release Crypto version 2.1
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2005, Intel Corporation.
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
