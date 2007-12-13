/*
 * @file        IxAtmSch_p.h
 * @author Intel Corporation
 * @date        13-12-2000  Ported:      1-Mar-2002
 *
 * @brief Private header file for AtmSch component.
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

#ifndef IXATMSCH_P_H
#define IXATMSCH_P_H

#include "IxOsal.h"
#include "IxAtmTypes.h"

/*
 * Defines
 */
/* The Value of IX_ATMSCH_nS_PER_SECOND is currently defined to be 
 * a 32 bit value.Because the current Port rate Value cannot be defined 
 * more than 32 bits.In Future when faster Phy's are supported this
 * value can be defined do be more than 32 bits value. 
 */
#define IX_ATMSCH_nS_PER_SECOND      1000000000 /* Value should be < 32 bits */
#define IX_ATMSCH_MAX_TABLE_ENTRIES  10
#define IX_ATMSCH_NULL_INDEX         -1
#define IX_ATMSCH_UINT_MASK          0x8000000000000000ULL
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
    UINT64 cet;               /* cell emission time = MAX(cet_scr, cet_pcr) */
    UINT64 cetScr;            /* CET calculated with SCR */
    UINT64 cetPcr;            /* CET calculated with PCR */
    UINT64 bt;                /* Burst Tolerance = ((MBS - 1) * (scr_us - pcr_us)) */
    UINT64 usScr;             /* SCR expressed in microsecond/cell, (represents 1/SCR) */
    UINT64 usPcr;             /* PCR expressed in microsecond/cell, (represents 1/PCR) */
    UINT64 mbs;               /* Maximum Burst Size in cells */
    UINT64 baseBt;            /* Burst Tolerance base value */
    UINT64 scr;
    UINT64 pcr;
} IxAtmSchVcSchedInfo;

typedef struct
{
    IxAtmServiceCategory atmService; /* The ATM service category */
    UINT64 vbrPcrCellsCnt;     /* The remaining of VBR burst cells */
    UINT64 vbrScrCellsCnt;     /* The remaining of VBR burst cells */
    BOOL inUse;			 /* indicates whether the table
				  * element is currently in use */
    IxAtmLogicalPort port;       /* The port on which the VC is enabled */
    UINT64 count;		 /* a count of the queued cells for the VC */
    INT32 nextVc;	         /* the next VC in the chain */
    IxAtmConnId connId;          /* connId which the scheduling client knows
                                    the VC. */
    IxAtmSchVcSchedInfo schInfo; /* Scheduling information for this VC */
} IxAtmSchVcInfo;

typedef struct
{
    UINT64 idleCellsScheduled;
    UINT64 cellsQueued;
    UINT64 updateCalls;
    UINT64 queueFull;
    UINT64 cellsScheduled;
    UINT64 scheduleTableCalls;
} IxAtmSchStats;

/*
 * Function Prototypes
 */
void
ixAtmSchedulingInit(void);

void
ixAtmSchCellTimeSet(IxAtmLogicalPort port, UINT64 cellTime );

UINT64
ixAtmSchCellTimeGet(IxAtmLogicalPort port);

void
ixAtmSchMinCellsSet(IxAtmLogicalPort port, UINT64 minCellsToSchedule);

UINT64
ixAtmSchMinCellsGet(IxAtmLogicalPort port);

void
ixAtmSchBubbleSortRtVcQueue(IxAtmLogicalPort port);

void
ixAtmSchTimerOverrun (IxAtmLogicalPort port);

IX_STATUS
ixAtmSchBaseTimeSet (IxAtmLogicalPort port, UINT64 baseTime);

#endif /* IXATMSCH_P_H */
