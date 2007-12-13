/*
 * @file        IxAtmSch.c
 * @author Intel Corporation
 * @date        29-JUL-2000
 *
 * @brief Implementiation file for the example Atm scheduler.
 * 
 * Design Notes:
 *    14-SEP-00  A different implementation of the UBR chaining
 *               which keeps only those UBR VCs which have pending
 *               cells on the chain is available in CVS version 1.16.
 *    18-DEC-00  Added a 'sch' prefix to all global variables.  The 
 *               coding standards specify that this prefix should be
 *               IxAtmSch, but due to the complexity of these 
 *               functions, names are being maintained as short as
 *               possible to retain readability.
 *    02-JAN-01  This file is compiled automatically into the BSP
 *               by the ixp22x BSP Makefile.  It is 
 *               not, therefore, built by a standard arm_sw build.
 *
 *    26-FEB-02  Ported this code from IXP22X. Expanded to support
 *               multiple ports. Adjusted API to accommodate new 
 *               IxAtmdAcc component.
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

#include "IxOsal.h"
#include "IxAtmSch.h"
#include "IxAtmSch_p.h"
#include "IxAtmSchUtils_p.h"

/*
 * #defines and macros used in this file.
 */
#define IX_ATMSCH_QUEUE_LIMIT 100000
#define MAX(a, b) ((a > b)?a:b)
#define MIN(a, b) ((a < b)?a:b)

typedef struct 
{
    UINT64 timePerCell;         /* Time per cell in us */
    UINT64 minCellsToSchedule;  /* minimum number of cells to schedule */
    UINT64 currentSchTime;
    IxAtmScheduleTable schTable;/* A pointer to the schedule table for this port */
    IxAtmScheduleTableEntry schTableEntries[IX_ATMSCH_MAX_TABLE_ENTRIES]; /* The Sched table */

}IxAtmSchPortSchedulingInfo;

/*
 * Variable declarations global to this file only. Externs are followed by
 * static variables.  */

/* Holds the per port scheduling state information */
static IxAtmSchPortSchedulingInfo portSchedulingInfo[IX_UTOPIA_MAX_PORTS];

/* 
 * Variable declarations used by other source files in this component 
 */
BOOL               ixAtmSchedulingEnabled[IX_UTOPIA_MAX_PORTS];
IxAtmSchedulerVcId ixAtmSchNextUbrToBeScheduled[IX_UTOPIA_MAX_PORTS];
IxAtmSchedulerVcId ixAtmSchRtQueueHead[IX_UTOPIA_MAX_PORTS];
IxAtmSchVcInfo     ixAtmSchVcTable[IX_ATM_MAX_NUM_AAL_OAM_TX_VCS]; 
IxAtmSchStats      ixAtmSchStats[IX_UTOPIA_MAX_PORTS]; 
UINT64             ixAtmSchBaseTime[IX_UTOPIA_MAX_PORTS];
UINT64             ixAtmSchTime[IX_UTOPIA_MAX_PORTS];
UINT64             ixAtmSchCacPortAllocated[IX_UTOPIA_MAX_PORTS];


PRIVATE IX_STATUS
ixAtmSchValidRtVcSearch(IxAtmLogicalPort port, 
			IxAtmServiceCategory atmService, 
			IxAtmSchedulerVcId* schRtQueueHead);

/************************************************************************/
void
ixAtmSchedulingInit(void)
{
    IxAtmLogicalPort i;

    /* Initialise the scheduling info for all ports */
    ixOsalMemSet(portSchedulingInfo, 0, sizeof(portSchedulingInfo));

    for (i=0; i<IX_UTOPIA_MAX_PORTS; i++) 
    {
        ixAtmSchNextUbrToBeScheduled[i] = IX_ATMSCH_NULL_INDEX;
	ixAtmSchRtQueueHead[i]          = IX_ATMSCH_NULL_INDEX;
        ixAtmSchedulingEnabled[i]       = FALSE;
	ixAtmSchBaseTime[i]             = 0;
	ixAtmSchTime[i]                 = 0;
	ixAtmSchCacPortAllocated[i]     = 0;
    }

    for (i=0; i<IX_ATM_MAX_NUM_AAL_OAM_TX_VCS; i++)
    {
        ixAtmSchVcTable[i].inUse = FALSE;
        ixAtmSchVcTable[i].port  = IX_UTOPIA_MAX_PORTS;
    }
}

/************************************************************************/

void
ixAtmSchCellTimeSet(IxAtmLogicalPort port, UINT64 cellTime)
{
    portSchedulingInfo[port].timePerCell = cellTime;
}

/************************************************************************/

UINT64
ixAtmSchCellTimeGet(IxAtmLogicalPort port)
{
    return portSchedulingInfo[port].timePerCell;
}

/************************************************************************/

void
ixAtmSchMinCellsSet(IxAtmLogicalPort port, UINT64 minCellsToSchedule)
{
    portSchedulingInfo[port].minCellsToSchedule = minCellsToSchedule;
}

/************************************************************************/

UINT64
ixAtmSchMinCellsGet(IxAtmLogicalPort port)
{
    return portSchedulingInfo[port].minCellsToSchedule;
}

/************************************************************************/

PUBLIC IX_STATUS 
ixAtmSchVcQueueUpdate( IxAtmLogicalPort port, 
                       IxAtmSchedulerVcId vcId, 
                       UINT64 numberOfCells)
{
    IxAtmSchedulerVcId thisRtVc = IX_ATMSCH_NULL_INDEX;



    /* 
     *
     * Putting the most common scenario in as the successful
     * branch of the if statement saves another 10%.  
     */

    if ((vcId < IX_ATM_MAX_NUM_AAL_OAM_TX_VCS) &&
	(vcId >= 0) &&
	(ixAtmSchVcTable[vcId].inUse == TRUE)&&
	(ixAtmSchVcTable[vcId].port  == port))
    {
	/* This is hard to read but is a slightly faster than doing a
	   seperate increment and test */
	if ((ixAtmSchVcTable[vcId].count += numberOfCells) < IX_ATMSCH_QUEUE_LIMIT)
	{
	    IX_ATMSCH_STATS(ixAtmSchStats[port].updateCalls++;);
	    IX_ATMSCH_STATS(ixAtmSchStats[port].cellsQueued += numberOfCells;);
	    
	    /* Performing queue update for real-time VC */
	    if (ixAtmSchVcTable[vcId].schInfo.cet & IX_ATMSCH_UINT_MASK)
	    {
		/* Real time list exist. Perform queue update here. No need to check
		   whether the list is NULL*/
		
		if (ixAtmSchRtQueueHead[port] != vcId)
		{
		    /* The interested VC is not at the head of the queue */
		    thisRtVc = ixAtmSchRtQueueHead[port];
		    
		    while (thisRtVc != IX_ATMSCH_NULL_INDEX)
		    {
			/* The loop searches for the correct VC */			      
			if (ixAtmSchVcTable[thisRtVc].nextVc == vcId)
			{
			    /* Found the VC that the current one is pointing to 
			     * The interested VC's that it is pointing to is stored into
			     * the current's nextVc */
			    ixAtmSchVcTable[thisRtVc].nextVc = ixAtmSchVcTable[vcId].nextVc;
				
			    /* break from the while loop */
			    break;
			}
			/* If not found,gGet the next VC in the list */
			thisRtVc = ixAtmSchVcTable[thisRtVc].nextVc;
		    } /* while */
			
		    /* The interested VC is brought forward to the head of the queue 
		     * and its nextVc is now pointing to the old queue head */
		    ixAtmSchVcTable[vcId].nextVc = ixAtmSchRtQueueHead[port];
		}
		
		/* Do this here so the queue remains sorted at all times */
		ixAtmSchVcTable[vcId].schInfo.cetScr     = 
		    ixAtmSchVcTable[vcId].schInfo.cetPcr     = 
		    ixAtmSchVcTable[vcId].schInfo.cet        = 
		    MAX(ixAtmSchTime[port], (ixAtmSchVcTable[vcId].schInfo.cet ^ IX_ATMSCH_UINT_MASK));
		
		/* Now the interested VC is the head of the queue */
		ixAtmSchRtQueueHead[port] = vcId;
	    } /* End if statement for real time*/
	    
	    return IX_SUCCESS;
	}
	else
	{
	    /* doing this is faster than a test and increment on every
	       call, we will rarely go inside this  */
	    ixAtmSchVcTable[vcId].count -= numberOfCells;
	    IX_ATMSCH_STATS(ixAtmSchStats[port].queueFull ++;);
	    return IX_ATMSCH_RET_QUEUE_FULL;
	}
    }
    return IX_FAIL;
}

/************************************************************************/
PUBLIC IX_STATUS
ixAtmSchVcQueueClear( IxAtmLogicalPort port, 
                      IxAtmSchedulerVcId vcId)
{
    if ((vcId < IX_ATM_MAX_NUM_AAL_OAM_TX_VCS) &&
	(vcId >= 0) &&
        (ixAtmSchVcTable[vcId].inUse == TRUE)&&
	(ixAtmSchVcTable[vcId].port  == port))
    {
	ixAtmSchVcTable[vcId].count = 0;

	if (ixAtmSchVcTable[vcId].schInfo.cetPcr != 0)
	{
	    /* i.e. this is a RT VC */
	    ixAtmSchVcTable[vcId].schInfo.cet |= IX_ATMSCH_UINT_MASK;

	    /* This is to sort the real-time list */
	    ixAtmSchBubbleSortRtVcQueue(port);
	}

	return IX_SUCCESS;
    }
    return IX_FAIL;
}


/************************************************************************/
/* This function is big because we do not have the performance budget
 * to make sub functions out of it. 
 *
 * This is called when the client decides it needs a new sequence of
 * cells to send (probably because the transmit queue is near to
 * empty for this ATM port)
 *
 * This function cannot be interrupted by a call to ubrVcChainRemove 
 * since such a call could cause errors through a non-atomic modification 
 * of the ixAtmSchNextUbrToBeScheduled variable.
 *
 * SchTime in this function is maintained in microseconds and is updated
 * by the SEND_CELLS macro every time cells are sent.
 *
 * The algorithm will schedule at least the lesser of minCellsToSchedule
 * cells and maxCells. The value of minCellsToSchedule should be a trade-off
 * between maximum VBR cell delay (low value) and efficient execution of the sch 
 * table (high value). Tuning of this value is expected to be performed on a
 * per-application basis.
 */

/* This "inline" function is only used in the following ixAtmSchTableUpdate() */
#define SEND_CELLS(COUNT, CONNID, TBL_INDEX)                           \
{					                               \
    portSchInfo->schTableEntries[(TBL_INDEX)].connId        = (CONNID);\
    portSchInfo->schTableEntries[(TBL_INDEX)].numberOfCells = (COUNT); \
    ixAtmSchTime[port] += (timePerCell * (COUNT));                     \
    schTable->tableSize = (TBL_INDEX)+1;                               \
    cellCount += (COUNT);		                               \
}

/* Parameters are simply index vars used to reference the table */
#define RESORT_RT_QUEUE_HEAD()                         		\
{								\
    prevRtVc = schRtQueueHead;			                \
    thisRtVc = ixAtmSchVcTable[schRtQueueHead].nextVc;		\
    while( (thisRtVc != IX_ATMSCH_NULL_INDEX) &&	        \
	   ( ixAtmSchVcTable[thisRtVc].schInfo.cet <=           \
	     ixAtmSchVcTable[schRtQueueHead].schInfo.cet) )     \
    {		                                                \
	prevRtVc = thisRtVc;					\
	thisRtVc = ixAtmSchVcTable[thisRtVc].nextVc;		\
    }								\
    								\
    /* Move the head of the queue into the new position */      \
    if (prevRtVc != schRtQueueHead) /* i.e. >1 RT VC */		\
    {								\
	ixAtmSchVcTable[prevRtVc].nextVc = schRtQueueHead;	\
	tmpRtVc = ixAtmSchVcTable[schRtQueueHead].nextVc;       \
	ixAtmSchVcTable[schRtQueueHead].nextVc = thisRtVc;      \
	schRtQueueHead = tmpRtVc;				\
        ixAtmSchRtQueueHead[port] = schRtQueueHead;             \
    }	                                                        \
}

#define RESORT_RT_QUEUE_PRIORITY(PORT, schRtQueueHead)                          	\
{                                                               		                \
    if (ixAtmSchValidRtVcSearch(PORT, IX_ATM_CBR, &schRtQueueHead) != IX_SUCCESS) {		\
	if (ixAtmSchValidRtVcSearch(PORT,IX_ATM_RTVBR, &schRtQueueHead) != IX_SUCCESS) { 	\
	    if(ixAtmSchValidRtVcSearch(PORT, IX_ATM_VBR, &schRtQueueHead) != IX_SUCCESS) {	\
	    }											\
	}											\
    }                                                                                           \
}

PRIVATE IX_STATUS
ixAtmSchValidRtVcSearch(IxAtmLogicalPort port, 
			IxAtmServiceCategory atmService, 
			IxAtmSchedulerVcId* schRtQueueHead)
{
    IxAtmSchedulerVcId thisRtVc = IX_ATMSCH_NULL_INDEX;
    IxAtmSchedulerVcId tmpRtVc = IX_ATMSCH_NULL_INDEX;

    thisRtVc = *schRtQueueHead;

    /* If there is no rt queue && count is zero */
    if ((thisRtVc == IX_ATMSCH_NULL_INDEX) ||
	(ixAtmSchVcTable[thisRtVc].count == 0))
    {
	return IX_SUCCESS;
    }

    if (ixAtmSchVcTable[thisRtVc].atmService != atmService)
    {
 	while (((tmpRtVc = ixAtmSchVcTable[thisRtVc].nextVc) != IX_ATMSCH_NULL_INDEX) &&
	       !(ixAtmSchVcTable[tmpRtVc].schInfo.cet & IX_ATMSCH_UINT_MASK))
    	{
	    if ((ixAtmSchVcTable[tmpRtVc].atmService == atmService) &&
	        (ixAtmSchVcTable[tmpRtVc].schInfo.cet <= ixAtmSchTime[port]) &&
	        (ixAtmSchVcTable[tmpRtVc].count != 0))
	    {
	        /* found that VC. Swap it to the head of the list */
	   	ixAtmSchVcTable[thisRtVc].nextVc = ixAtmSchVcTable[tmpRtVc].nextVc;
		ixAtmSchVcTable[tmpRtVc].nextVc = *schRtQueueHead;
		*schRtQueueHead = tmpRtVc;
		ixAtmSchRtQueueHead[port] = tmpRtVc;
	    	return IX_SUCCESS;
	    }

	    /* continue to search for the next VC of same ATM service category */
	    thisRtVc = ixAtmSchVcTable[thisRtVc].nextVc;
	}

	/* End of the queue is reached, i.e. no VC found in the list 
	 * or CET is not complied or count is zero. So return FALSE */
	return IX_FAIL;

    } /* if the rt queue list is actived, search for correct atmService */

    return IX_SUCCESS;
}

IX_STATUS
ixAtmSchTableUpdate(IxAtmLogicalPort port, 
                    UINT64 maxCells,
                    IxAtmScheduleTable **retTable)
{
    int entryCnt; /* a counter to create the sch table */
    UINT64 numUbrToSend; /* number of cells (UBR or idle) that can be sched */
    UINT64 cellCount = 0; /* counter for the number of cell sched */
    IxAtmSchPortSchedulingInfo *portSchInfo; /* contains info for that port */
    UINT64 timePerCell; /* the time for each cell */
    IxAtmScheduleTable *schTable; /* contains scheduling table info */
    
    /* These variables are used for UBR list */
    IxAtmSchedulerVcId prevUBR = IX_ATMSCH_NULL_INDEX;
    IxAtmSchedulerVcId schNextUbrToBeScheduled;

    /* thisSchInfo is used to hold the information of the real-time VC
    * scheduling information */
    IxAtmSchVcSchedInfo *thisSchInfo;

    /* These variables are used for real-time list */
    IxAtmSchedulerVcId schRtQueueHead;
    IxAtmSchedulerVcId prevRtVc;
    IxAtmSchedulerVcId thisRtVc;
    IxAtmSchedulerVcId tmpRtVc;

    /* Update the count for calls to this function */
    IX_ATMSCH_STATS(ixAtmSchStats[port].scheduleTableCalls ++;);

    if ((port>=IX_UTOPIA_PORT_0) && 
        (port< IX_UTOPIA_MAX_PORTS)&&
        (ixAtmSchedulingEnabled[port]))
    {
        portSchInfo = &portSchedulingInfo[port];

        /* make a local copy. The time per cell and current 
	 * scheduler time for that particular port */ 
        timePerCell    = portSchInfo->timePerCell;

        schNextUbrToBeScheduled = ixAtmSchNextUbrToBeScheduled[port];
       	schRtQueueHead = ixAtmSchRtQueueHead[port];

	/* Point to the scheduler table for this port */
        schTable            = &portSchInfo->schTable;
	schTable->tableSize = 0;                   

	for (entryCnt=0;(entryCnt < IX_ATMSCH_MAX_TABLE_ENTRIES )&&(cellCount < maxCells); entryCnt++) 
	{
	    RESORT_RT_QUEUE_PRIORITY(port, schRtQueueHead);

	    if ((ixAtmSchTime[port] & (IX_ATMSCH_UINT_MASK >> 1)) ||
		((schRtQueueHead != IX_ATMSCH_NULL_INDEX) && 
		 (ixAtmSchVcTable[schRtQueueHead].schInfo.cet & (IX_ATMSCH_UINT_MASK >> 1))))
	    {
		/* This should happen rarely so it is
		 * acceptable to perform a function call as it is so
		 * infrequent */
		ixAtmSchTimerOverrun(port);
	    }

	    /* Process for real time VC */
	    while ((schRtQueueHead != IX_ATMSCH_NULL_INDEX) &&
		   (ixAtmSchTime[port] >= ixAtmSchVcTable[schRtQueueHead].schInfo.cet) &&
		   (entryCnt < IX_ATMSCH_MAX_TABLE_ENTRIES))
	    {
		/* first schedule one real-time cell if there any */

		/* make a local copy of the scheduling information of the VC */
		thisSchInfo = &ixAtmSchVcTable[schRtQueueHead].schInfo;

		if (ixAtmSchVcTable[schRtQueueHead].count > 0)
		{
		    /* Send one real-time cell. Decrement the cell count &
		     * increment the table entry count*/
		    SEND_CELLS(1,
			       ixAtmSchVcTable[schRtQueueHead].connId, 
			       entryCnt);
		    ixAtmSchVcTable[schRtQueueHead].count--;
		    entryCnt++;

		    /* calculate the next tx time for this VC */
		    if ((ixAtmSchVcTable[schRtQueueHead].atmService == IX_ATM_RTVBR) ||
			(ixAtmSchVcTable[schRtQueueHead].atmService == IX_ATM_VBR))
		    {
			/* This is for VBR - based on PCR, SCR and BT value */
			thisSchInfo->cetPcr += thisSchInfo->usPcr;
			thisSchInfo->cetScr += thisSchInfo->usScr;
			thisSchInfo->cet = MAX(thisSchInfo->cetPcr,
					       thisSchInfo->cetScr - thisSchInfo->bt);

			if (ixAtmSchVcTable[schRtQueueHead].vbrPcrCellsCnt > 0)
			{
			    ixAtmSchVcTable[schRtQueueHead].vbrPcrCellsCnt--;
			}
			else
			{
			    if (ixAtmSchVcTable[schRtQueueHead].vbrScrCellsCnt == 0)
			    {
				ixAtmSchTimerOverrun(port);
			    }

			    if (ixAtmSchVcTable[schRtQueueHead].vbrScrCellsCnt ==
				(ixAtmSchVcTable[schRtQueueHead].schInfo.scr * 
				 IX_ATMSCH_SCR_PERIOD_IN_SEC))
			    {
				/* Restore MBS value. Burst Tolerance will be calc by
				 * TimerOverrun */
				ixAtmSchVcTable[schRtQueueHead].vbrPcrCellsCnt = 
				    ixAtmSchVcTable[schRtQueueHead].schInfo.mbs;
				
				/* Reset SCR cell count */
				ixAtmSchVcTable[schRtQueueHead].vbrScrCellsCnt = 0;
				
				ixAtmSchTimerOverrun(port);
			    }
			    else
			    {
				ixAtmSchVcTable[schRtQueueHead].vbrScrCellsCnt++;
			    }
			}
		    }
		    else 
		    {
			/* This is for CBR - based on PCR value only */
			thisSchInfo->cetPcr += thisSchInfo->usPcr;
			thisSchInfo->cet = thisSchInfo->cetPcr;
		    }

		    if (ixAtmSchVcTable[schRtQueueHead].count == 0)
		    {
			/* There are no more cells to be scheduled. Hence set the 
			 * cet's MSB with the MASK value. It will be placed at VC (now idle)
			 * at the back of the queue */
			ixAtmSchVcTable[schRtQueueHead].schInfo.cet |= IX_ATMSCH_UINT_MASK;
		    }
		}
		else 
		{
		    /* There are no cells to be scheduled. The VC becomes idle by
		     * setting the most significant bit */
		    ixAtmSchVcTable[schRtQueueHead].schInfo.cet|= IX_ATMSCH_UINT_MASK;
		}

		RESORT_RT_QUEUE_HEAD();
		RESORT_RT_QUEUE_PRIORITY(port,schRtQueueHead);

	    } /* while*/

	    /* Leading UBR cells in this table, recalculate the
	     * number of UBR cells to send based on the current
	     * schTime. */
	    if (schRtQueueHead != IX_ATMSCH_NULL_INDEX)
	    {
		if ((cellCount >= portSchInfo->minCellsToSchedule) &&
		    (ixAtmSchVcTable[schRtQueueHead].count == 0))
		{
		    /* the cell counter exceeds the min table size and there are no
		     * cells to be schedule */
		    break;
		}
		else if (ixAtmSchVcTable[schRtQueueHead].schInfo.cet & (IX_ATMSCH_UINT_MASK))
		{
		    if (cellCount < portSchInfo->minCellsToSchedule)
		    {
			numUbrToSend = MAX(((INT32)portSchInfo->minCellsToSchedule - (INT32)cellCount), 1);
		    }
		    else
		    {
			break; /* for (entryCnt ...) */
		    }
		}
		else 
		{		   
		    numUbrToSend = ixAtmSchVcTable[schRtQueueHead].schInfo.cet - 
				     ixAtmSchTime[port];
		    numUbrToSend = IX_OSAL_UDIV64_32(numUbrToSend, timePerCell) + 1;
		}
	    }
	    else 
	    {
		/* No real-time VC registered */
		if (cellCount < portSchInfo->minCellsToSchedule)
		{
		    numUbrToSend = MAX(((INT32)portSchInfo->minCellsToSchedule - (INT32)cellCount), 1);
		}
		else
		{
		    break; /* for (entryCnt...) */
		}
	    }

	    /* Modify the number of UBR so the maxCells is not exceeded.
	     * The maxCells and cellCount are cast with INT32 so that the 
	     * negative value can be compared. Now the schTable is expecting
	     * an entry (after completing the above's algorithm). Hence the 
	     * numUbrToSend must have at least 1. Zero is not valid as it may 
	     * cause the system failure (i.e. no such thing as sending zero cell) */
            if ( (numUbrToSend+cellCount) > maxCells )
            {
                numUbrToSend = MAX( ((INT32) maxCells - (INT32) cellCount),1);
            }

	    /* Now fill the gap between real-time cells with cells from the
	     * UBR VCs. */
	    while ((numUbrToSend > 0) && (entryCnt < IX_ATMSCH_MAX_TABLE_ENTRIES))
	    {
		if ( (schNextUbrToBeScheduled != IX_ATMSCH_NULL_INDEX) &&
		     (ixAtmSchVcTable[schNextUbrToBeScheduled].count >= numUbrToSend) )
		{
                  /* CASE 1 - enough UBR in this VC to fulfill numUbrToSend */
                    IX_OSAL_ENSURE( (ixAtmSchVcTable[schNextUbrToBeScheduled].port == port),
                                "IxAtmSch Major internal error") ;
		    
                    SEND_CELLS (numUbrToSend, 
                                ixAtmSchVcTable[schNextUbrToBeScheduled].connId, 
                                entryCnt);

		    ixAtmSchVcTable[schNextUbrToBeScheduled].count -= numUbrToSend;
		    numUbrToSend = 0;
		}
		else if ( (schNextUbrToBeScheduled != IX_ATMSCH_NULL_INDEX) &&
			  (ixAtmSchVcTable[schNextUbrToBeScheduled].count > 0) )
		{
		    /* CASE 2 - some, but not enough UBR in this VC to fulfill numUbrToSend */
                    IX_OSAL_ENSURE((ixAtmSchVcTable[schNextUbrToBeScheduled].port == port),
                               "IxAtmSch Major internal error") ;
		    
                    SEND_CELLS(ixAtmSchVcTable[schNextUbrToBeScheduled].count, 
                               ixAtmSchVcTable[schNextUbrToBeScheduled].connId, 
			       entryCnt);
		    numUbrToSend -= ixAtmSchVcTable[schNextUbrToBeScheduled].count;
		    ixAtmSchVcTable[schNextUbrToBeScheduled].count = 0;                    
		    entryCnt++;
		}
		else if ( (schNextUbrToBeScheduled == IX_ATMSCH_NULL_INDEX) ||
			  (prevUBR == schNextUbrToBeScheduled) )
		{
		    /* CASE 3 - There is no UBR enabled or we 
                     * have gone all the way around the UBR
		     * chain and there is no data in it. If we
		     * have real-time cells or we have sent UBR this
		     * time we insert required idle
		     * cells. Otherwise the table is empty.
		     */
		    if (((schRtQueueHead != IX_ATMSCH_NULL_INDEX) && 
			 (ixAtmSchVcTable[schRtQueueHead].count > 0)) ||
			(cellCount > 0) )
		    {
			SEND_CELLS(numUbrToSend, 
                                   IX_ATM_IDLE_CELLS_CONNID, 
                                   entryCnt);
			IX_ATMSCH_STATS(ixAtmSchStats[port].idleCellsScheduled += numUbrToSend;);
			numUbrToSend = 0;
		    }
		    else
		    {
			*retTable = NULL;
                        ixAtmSchNextUbrToBeScheduled[port] = schNextUbrToBeScheduled;
			
                        return IX_ATMSCH_RET_QUEUE_EMPTY;
		    }
		}

		/* Find the next UBR VC with cells, stop if we
		 * have gone all the way around the UBR chain.
		 */
		if (prevUBR != schNextUbrToBeScheduled)
		{
		    prevUBR = schNextUbrToBeScheduled;
		    do
		    {
			schNextUbrToBeScheduled 
			    = ixAtmSchVcTable[schNextUbrToBeScheduled].nextVc;
		    }
		    while ( (prevUBR != schNextUbrToBeScheduled) &&
			    (ixAtmSchVcTable[schNextUbrToBeScheduled].count == 0) );
		}

	    } /* while ((numUbrToSend > 0) ... */
	} /* for */

        ixAtmSchNextUbrToBeScheduled[port] = schNextUbrToBeScheduled;

	schTable->totalCellSlots    = cellCount;
	IX_ATMSCH_STATS(ixAtmSchStats[port].cellsScheduled    += cellCount;);
	schTable->table             = portSchInfo->schTableEntries;

	*retTable = schTable;

	return IX_SUCCESS;
    }
    return IX_FAIL;
}
/************************************************************************/

