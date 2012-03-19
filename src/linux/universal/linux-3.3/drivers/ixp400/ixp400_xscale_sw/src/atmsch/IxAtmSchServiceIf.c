/* 
 * @file        IxAtmSchServiceIf.c
 *
 *              
 * 
 *
 * @brief Implementation file for Service Interface of the Atm scheduler.
 * 
 * Design Notes:
 *    All non data path invoked functionality is contained in this
 *    file.  Other functionality is separated as it is included 
 *    in internal IXP22x RAM.
 *
 *    26-FEB-02 Ported this code from IXP22x and modified to support
 *    multiple ports.
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
 * Put the system defined include files required
 */

#include "IxOsal.h"
#include "IxAtmSch.h"
#include "IxAtmSch_p.h"
#include "IxAtmSchUtils_p.h"

/*
 * Variable declarations global to this file only. Externs are followed by
 * static variables.  */
static UINT64                 schPortRate[IX_UTOPIA_MAX_PORTS];
static BOOL                   schInitDone = FALSE;
static IxAtmTrafficDescriptor schTd[IX_ATM_MAX_NUM_AAL_OAM_TX_VCS];

/* External variables defined in the AtmSch component.*/
extern BOOL               ixAtmSchedulingEnabled[IX_UTOPIA_MAX_PORTS];
extern IxAtmSchVcInfo     ixAtmSchVcTable[IX_ATM_MAX_NUM_AAL_OAM_TX_VCS];
extern IxAtmSchedulerVcId ixAtmSchNextUbrToBeScheduled[IX_UTOPIA_MAX_PORTS];
extern IxAtmSchedulerVcId ixAtmSchRtQueueHead[IX_UTOPIA_MAX_PORTS];
extern IxAtmSchStats      ixAtmSchStats[IX_UTOPIA_MAX_PORTS];
extern UINT64             ixAtmSchBaseTime[IX_UTOPIA_MAX_PORTS];
extern UINT64             ixAtmSchTime[IX_UTOPIA_MAX_PORTS];
extern UINT64             ixAtmSchCacPortAllocated[IX_UTOPIA_MAX_PORTS];

/* Function definition */
PRIVATE int
ixAtmSchCac(IxAtmLogicalPort port, IxAtmTrafficDescriptor *trafficDesc);

PRIVATE IxAtmSchedulerVcId 
ixAtmSchFreeVcIdGet( void );

PRIVATE void
ixAtmSchUbrChainVcInsert(IxAtmLogicalPort port, IxAtmSchedulerVcId vcId);

PRIVATE void
ixAtmSchUbrChainVcRemove(IxAtmLogicalPort port, IxAtmSchedulerVcId vcId);

PRIVATE BOOL
ixAtmSchParamIsValid(IxAtmLogicalPort port, IxAtmSchedulerVcId vcId);

PRIVATE IX_STATUS
ixAtmSchRtVcInsert (IxAtmLogicalPort port, IxAtmSchedulerVcId vcId);

PRIVATE void
ixAtmSchRtVcRemove(IxAtmLogicalPort port, IxAtmSchedulerVcId vcId);

/************************************************************************/
/* This function is to initialize the scheduler component. It initialize
 * the variables and statistics value.  This has to be called first.
*/
PUBLIC IX_STATUS 
ixAtmSchInit(void)
{
    if (!schInitDone ) 
    {
        ixAtmSchedulingInit();
        ixAtmSchStatsClear();
        schInitDone = TRUE;
        return IX_SUCCESS;
    }
    return IX_FAIL;
}

PUBLIC IX_STATUS
ixAtmSchUninit(void)
{
    if (schInitDone)
    {
        schInitDone = FALSE;
        return IX_SUCCESS;
    }
    return IX_FAIL;
}

/************************************************************************/
/* This function is to show the information about the scheduler. 
 */
PUBLIC void
ixAtmSchShow(void)
{
    IxAtmSchedulerVcId i;
    IxAtmLogicalPort port;
    UINT32 vcCnt;

    if (!schInitDone) 
    {
	printf("\n=========== IxAtmSch not initialized ==============\n");
    }
    else
    {
	printf("\n================== IxAtmSch State ==================");
	for (port=0; port<IX_UTOPIA_MAX_PORTS; port++)
	{
	    if (ixAtmSchedulingEnabled[port])
	    {
		printf("\n--- UTOPIA_PORT_%d Info ---",port);
		printf("\nRate = %llu, UBR VC VcId = %d, Real-Time VcId = %d", 
		       schPortRate[port],
		       ixAtmSchNextUbrToBeScheduled[port],
		       ixAtmSchRtQueueHead[port]);
		
		/* */
		printf("\n--- Shaping Statistics:"); 
		printf("\nDemand Update Calls \t= %llu", ixAtmSchStats[port].updateCalls);
		printf("\nCells Queued \t\t= %llu", ixAtmSchStats[port].cellsQueued);
		printf("\nTable Update Calls \t= %llu", ixAtmSchStats[port].scheduleTableCalls);
		printf("\nCells Scheduled \t= %llu", ixAtmSchStats[port].cellsScheduled);
		printf("\nIdle Cells Scheduled \t= %llu", ixAtmSchStats[port].idleCellsScheduled);
		printf("\nQueue Full Occurences\t= %llu", ixAtmSchStats[port].queueFull);
		printf ("\nAllocated Bandwidth \t= %llu\n",ixAtmSchCacPortAllocated[port]);

		/* */
		printf("\n--- VC information:");
		printf("\n%-4s %-4s %-8s %-8s %-8s %-4s %-4s %-8s %-7s %-8s",     
		       "VcId",
		       "serv",
		       "pcr",
		       "scr",
		       "cdvt",
		       "mbs",
		       "mfs",
		       "count",
		       "nextVc",
		       "connId");
		
		for (i=0, vcCnt = 0; i<IX_ATM_MAX_NUM_AAL_OAM_TX_VCS; i++)
		{
		    if ((ixAtmSchVcTable[i].inUse == TRUE) &&
			(ixAtmSchVcTable[i].port == port))
		    {
			vcCnt++;
			printf("\n%-4d %-4d %-8llu %-8llu %-8llu %-4llu %-4llu %-8llu %-7d %-8u", 
			       i,
			       schTd[i].atmService,
			       schTd[i].pcr,
			       schTd[i].scr,
			       schTd[i].cdvt,
			       schTd[i].mbs,
			       schTd[i].mfs,
			       ixAtmSchVcTable[i].count,
			       ixAtmSchVcTable[i].nextVc,
			       ixAtmSchVcTable[i].connId);
		    }
		}
		if ( vcCnt == 0 ) 
		{
		    printf("\nNo VCs"); 
		}
	    }
	}
	printf("\n====================================================\n");
    }
}

/************************************************************************/
/* This function is to clear the scheduler statistics */
PUBLIC void
ixAtmSchStatsClear(void)
{
    IxAtmLogicalPort port;

    /* clear all port statistics */
    for ( port = 0 ; port < IX_UTOPIA_MAX_PORTS ; port++ ) 
    {
        ixAtmSchStats[port].idleCellsScheduled = 0;
        ixAtmSchStats[port].updateCalls = 0;
        ixAtmSchStats[port].cellsQueued = 0;
        ixAtmSchStats[port].cellsScheduled = 0;
        ixAtmSchStats[port].queueFull = 0;
        ixAtmSchStats[port].scheduleTableCalls = 0;
    }

    return;
}

/************************************************************************/
/* This is to initialize the scheduler model by setting the portRate and
 * the minimum cells to scheduler for that particular port. This is done
 * once the IxAtmSchInit is completed. Also, the PortEnabling for that 
 * port is set to TRUE to indicate that the port is enabled */
PUBLIC IX_STATUS
ixAtmSchPortModelInitialize( IxAtmLogicalPort port, 
                             UINT64 portRate,
                             unsigned int minCellsToSchedule)
{
    if (!schInitDone||
        (port<IX_UTOPIA_PORT_0)||
        (port>=IX_UTOPIA_MAX_PORTS)||
        (ixAtmSchedulingEnabled[port])||
        (portRate == 0))
    {
       	return IX_FAIL;
    }
    else 
    {
	/* Store the port rate into a global variable for a particular port */
	schPortRate[port] = portRate;

	/* set the time between cell */
	if (portRate > IX_ATMSCH_nS_PER_SECOND)
	{
	    ixAtmSchCellTimeSet(port, 0);
	}
	else
	{
	    ixAtmSchCellTimeSet(port, IX_ATMSCH_nS_PER_SECOND /
		(UINT32)portRate);
	}

	/* Set the min cell that can be scheduled */
        ixAtmSchMinCellsSet(port, minCellsToSchedule);
        ixAtmSchedulingEnabled[port] = TRUE;
	return IX_SUCCESS;
    }
}

/************************************************************************/
PUBLIC IX_STATUS
ixAtmSchPortModelUninitialize( IxAtmLogicalPort port)
{

    if (!schInitDone||                          /* Check the port limits */
        (port<IX_UTOPIA_PORT_0)||
        (port>=IX_UTOPIA_MAX_PORTS)||
        (!ixAtmSchedulingEnabled[port]))
    {
        return IX_FAIL;
    }
    else
    {
        ixAtmSchedulingEnabled[port] = FALSE;       /* Set the port to disabled */
        return IX_SUCCESS;
    }
}

/************************************************************************/
/* This function allows the modification of port rate for that particular
 * port. The requested port rate can be set below the CacPortAllocated value,
 * i.e. allows oversubscription */
PUBLIC IX_STATUS
ixAtmSchPortRateModify( IxAtmLogicalPort port, 
                        UINT64 portRate)
{
    IxAtmSchedulerVcId thisRtVc;

    if (!schInitDone||
        (port<IX_UTOPIA_PORT_0)||
        (port>=IX_UTOPIA_MAX_PORTS)||
        !ixAtmSchedulingEnabled[port])
    {
        return IX_FAIL;
    }

    thisRtVc = ixAtmSchRtQueueHead[port];

    schPortRate[port] = portRate;

    if (portRate > IX_ATMSCH_nS_PER_SECOND)
    {
    	ixAtmSchCellTimeSet(port, 0);
    }
    else
    {
	ixAtmSchCellTimeSet(port, IX_ATMSCH_nS_PER_SECOND/(UINT32)portRate);
    }

    return IX_SUCCESS;
}

/************************************************************************/
/* This function is to check whether atm services complies with the CAC 
*/
PRIVATE int
ixAtmSchCac(IxAtmLogicalPort port, IxAtmTrafficDescriptor *trafficDesc)
{
    IX_STATUS retval = IX_SUCCESS;

    switch(trafficDesc->atmService)
    {
	case IX_ATM_ABR:
	    return IX_FAIL;
	case IX_ATM_CBR:    
	    if ((trafficDesc->pcr > (schPortRate[port] - ixAtmSchCacPortAllocated[port])) ||
		(trafficDesc->pcr == 0))
	    {
		retval = IX_ATMSCH_RET_NOT_ADMITTED;
	    }
	    else 
	    {
		ixAtmSchCacPortAllocated[port] += trafficDesc->pcr;
	    }	
	    break;
	case IX_ATM_UBR:
	    /* UBR PCR must be at least equal to the port rate */
	    if (trafficDesc->pcr < schPortRate[port]) 
	    {
		retval = IX_ATMSCH_RET_NOT_ADMITTED;
	    }
	    break;
	case IX_ATM_VBR:
	case IX_ATM_RTVBR:
	    if ((trafficDesc->scr > (schPortRate[port] - ixAtmSchCacPortAllocated[port])) ||
		(trafficDesc->scr > trafficDesc->pcr) ||
		(trafficDesc->pcr > schPortRate[port]) ||
		(trafficDesc->pcr == 0) ||
		(trafficDesc->scr == 0))
	    {
		retval = IX_ATMSCH_RET_NOT_ADMITTED;
	    }
	    else
	    {
		ixAtmSchCacPortAllocated[port] += trafficDesc->scr;
	    }
	    break;
	default:
	    /* Unknown service type */
	    IX_ATMSCH_ASSERT(FALSE);
	    retval = IX_FAIL;
    }
    return retval;
}

/************************************************************************/
/* This function is to search for the free VC ID
 */
PRIVATE IxAtmSchedulerVcId
ixAtmSchFreeVcIdGet( void ) 
{
    IxAtmSchedulerVcId i;
    
    for (i=0; i<IX_ATM_MAX_NUM_AAL_OAM_TX_VCS; i++) 
    {
	if (ixAtmSchVcTable[i].inUse == FALSE)
	{
	    return i;
	}
    }
    return IX_ATMSCH_NULL_INDEX;
}

/************************************************************************/
/* This function is used to insert the real-time VCs into a list. It also
 * calculates the PCR and SCR in microseconds, and BT.
 */
PRIVATE IX_STATUS
ixAtmSchRtVcInsert (IxAtmLogicalPort port, IxAtmSchedulerVcId vcId)
{
    IxAtmSchedulerVcId qPtr;
    IX_STATUS retVal = IX_SUCCESS;

    /* Port and vcId already been check by the caller function. No 
     * additional check */

    /* Calculate #microseconds between each VBR cell for PCR and SCR */
    if (schTd[vcId].pcr > IX_ATMSCH_nS_PER_SECOND)
    {
	ixAtmSchVcTable[vcId].schInfo.usPcr = 0;
    }
    else
    {
	ixAtmSchVcTable[vcId].schInfo.usPcr = IX_ATMSCH_nS_PER_SECOND/ 
	    (UINT32)schTd[vcId].pcr;
    }
    ixAtmSchVcTable[vcId].schInfo.pcr = schTd[vcId].pcr;

    /* insert the ATM service category for that VC */
    ixAtmSchVcTable[vcId].atmService = schTd[vcId].atmService; 

    if (schTd[vcId].atmService != IX_ATM_CBR)
    {
	/* This is for VBR */
	if (schTd[vcId].scr > IX_ATMSCH_nS_PER_SECOND)
	{
	    ixAtmSchVcTable[vcId].schInfo.usScr = 0;
	}
	else
	{
	    ixAtmSchVcTable[vcId].schInfo.usScr = IX_ATMSCH_nS_PER_SECOND/ 
	    	(UINT32)schTd[vcId].scr;
	}
	ixAtmSchVcTable[vcId].schInfo.scr = schTd[vcId].scr;

	if (schTd[vcId].mbs > 0)
	{
	    /* BT = ((MBS - 1) * (1/SCR - 1/PCR)) */
	    ixAtmSchVcTable[vcId].schInfo.bt = (schTd[vcId].mbs - 1) * 
		(ixAtmSchVcTable[vcId].schInfo.usScr - ixAtmSchVcTable[vcId].schInfo.usPcr);

	    /* Store mbs locally and VBR's PCR cell count for future ref */
	    ixAtmSchVcTable[vcId].vbrPcrCellsCnt =
		ixAtmSchVcTable[vcId].schInfo.mbs = schTd[vcId].mbs;

	    /* Store BT for future reference */
	    ixAtmSchVcTable[vcId].schInfo.baseBt =
		ixAtmSchVcTable[vcId].schInfo.bt;
	}
	else
	{
	    /* No MBS value. Hence set bt to 0 */
	    ixAtmSchVcTable[vcId].schInfo.bt = 0;
	}
	    
	/* The largest Burst Tolerance of the system determines the 
	 * lowest value of 'time' that will be used in the scheduling
	 * algorithm */

	if (ixAtmSchBaseTime[port] <= ixAtmSchVcTable[vcId].schInfo.bt)
	{
	    retVal = ixAtmSchBaseTimeSet(port,ixAtmSchVcTable[vcId].schInfo.bt + 1);
	    if (IX_SUCCESS != retVal)
	    {
		return retVal;
	    }
	}
    }
    else
    { /* CBR does not use these fields.  The fact that they are zero
       * identifies this VC as CBR more efficiently. */
	ixAtmSchVcTable[vcId].schInfo.usScr = 0;
	ixAtmSchVcTable[vcId].schInfo.bt = 0;
    }

    /* Once the calculation is completed. The next thing to do is to insert
     * the VC into the list */
    qPtr = ixAtmSchRtQueueHead[port];
    if (qPtr == IX_ATMSCH_NULL_INDEX) 
    {
	/* First entry in the queue */
	ixAtmSchVcTable[vcId].nextVc = IX_ATMSCH_NULL_INDEX;
 	ixAtmSchRtQueueHead[port] = vcId; 
    }
    else 
    { 
	/* Insert VC at the end of the queue */
	while (ixAtmSchVcTable[qPtr].nextVc != IX_ATMSCH_NULL_INDEX) 
	{
	    qPtr = ixAtmSchVcTable[qPtr].nextVc;
	}
	ixAtmSchVcTable[qPtr].nextVc = vcId;
	ixAtmSchVcTable[vcId].nextVc = IX_ATMSCH_NULL_INDEX;
    }
    
    /* This initial setting will cause the first cell to be scheduled 
     * immediately on this VC */
    ixAtmSchVcTable[vcId].schInfo.cet = ixAtmSchTime[port] | IX_ATMSCH_UINT_MASK;

    return retVal;
}

/* This function is used to remove the real-time VC from the list
 */
PRIVATE void
ixAtmSchRtVcRemove(IxAtmLogicalPort port, IxAtmSchedulerVcId vcId)
{

    IxAtmSchedulerVcId thisVc = ixAtmSchRtQueueHead[port];
    IxAtmSchedulerVcId prevVc = IX_ATMSCH_NULL_INDEX;

    while (thisVc != IX_ATMSCH_NULL_INDEX)
    {
	if (thisVc == vcId)
	{
	    /* VC is found */

	    if (prevVc == IX_ATMSCH_NULL_INDEX)
	    {
		/* The required VC to be removed is at the head of the list */
		ixAtmSchRtQueueHead[port] = ixAtmSchVcTable[vcId].nextVc;
	    }
	    else
	    {
		/* VC found somewhere in the list. Hence, the predecessor 
		   now contains the removed VC's nextVc value */
		ixAtmSchVcTable[prevVc].nextVc = ixAtmSchVcTable[vcId].nextVc;
	    }

	    /* Decrements the allocated port capacity */
	    if (schTd[vcId].atmService == IX_ATM_CBR)
	    {	/* This is CBR */
		ixAtmSchCacPortAllocated[port] -= schTd[vcId].pcr;
		/* break from the while loop */
		break;
	    }
	    else
	    {	/* this is VBR */
		ixAtmSchCacPortAllocated[port] -= schTd[vcId].scr;
		/* break from the while loop */
		break;
	    }
	}
	else
	{
	    prevVc = thisVc;
	    thisVc = ixAtmSchVcTable[thisVc].nextVc;
	}
    } /* while */

    if (thisVc == IX_ATMSCH_NULL_INDEX)
    {
	IX_ATMSCH_ERROR_REPORT("VC not in real-time chain when removed.");
    }
}

/************************************************************************/
/* This function is used to insert UBR in a circular chain 
 */
PRIVATE void
ixAtmSchUbrChainVcInsert(IxAtmLogicalPort port, IxAtmSchedulerVcId vcId)
{
    IX_ATMSCH_ASSERT(vcId != IX_ATMSCH_NULL_INDEX);

    ixAtmSchVcTable[vcId].atmService = schTd[vcId].atmService;

    if (ixAtmSchNextUbrToBeScheduled[port] == IX_ATMSCH_NULL_INDEX) 
    {
	/* This is true for the first VC in the queue */
	ixAtmSchVcTable[vcId].nextVc = vcId;
	ixAtmSchNextUbrToBeScheduled[port] = vcId;
    }
    else 
    {
	/* It is important to update the nextVc fields in this order
	 * as this table may be simultaneously accessed by the
	 * updateTable interrupt. The insert into the chain is atomic.
	 */
	IX_ATMSCH_ASSERT(ixAtmSchVcTable[ixAtmSchNextUbrToBeScheduled[port]].nextVc 
                         != IX_ATMSCH_NULL_INDEX);
	ixAtmSchVcTable[vcId].nextVc 
             = ixAtmSchVcTable[ixAtmSchNextUbrToBeScheduled[port]].nextVc;
	ixAtmSchVcTable[ixAtmSchNextUbrToBeScheduled[port]].nextVc = vcId;
    }
}

/* This function is used to remove the UBR VC from the circular chain*/
PRIVATE void
ixAtmSchUbrChainVcRemove(IxAtmLogicalPort port, IxAtmSchedulerVcId vcId) 
{
    IxAtmSchedulerVcId i = 0;
    
    for (i=0; i<IX_ATM_MAX_NUM_AAL_OAM_TX_VCS; i++) 
    {
	if ( (ixAtmSchVcTable[i].inUse == TRUE ) && 
	     (ixAtmSchVcTable[i].nextVc == vcId) &&
             (ixAtmSchVcTable[i].port == port))
	{
	    IX_ATMSCH_ASSERT(ixAtmSchVcTable[vcId].nextVc != IX_ATMSCH_NULL_INDEX);

	    ixAtmSchVcTable[i].nextVc = ixAtmSchVcTable[vcId].nextVc;
	    if (ixAtmSchNextUbrToBeScheduled[port] == vcId) 
	    {
		/* This code is interruptible by the scheduler
		 * function while modifying the value of
		 * ixAtmSchNextUbrToBeScheduled, but the worst consequence is
		 * that it would cause a UBR Vc to be skipped in the
		 * chain */
		if (ixAtmSchVcTable[vcId].nextVc == vcId)
		{
		    /* this vc is the only one in the chain */
		    ixAtmSchNextUbrToBeScheduled[port] = IX_ATMSCH_NULL_INDEX;
		}
		else
		{
		    ixAtmSchNextUbrToBeScheduled[port] = ixAtmSchVcTable[vcId].nextVc;
		}
	    }
	    return;
	}
    }
    if (i == IX_ATM_MAX_NUM_AAL_OAM_TX_VCS) 
    {
	IX_ATMSCH_ERROR_REPORT("UBR VC not in UBR chain when removed.");
    }
}

/************************************************************************/
/* This function sets the scheduler model */
PUBLIC IX_STATUS
ixAtmSchVcModelSetup(IxAtmLogicalPort port, 
		     IxAtmTrafficDescriptor *trafficDesc, 
		     IxAtmSchedulerVcId *vcId)
{
    IX_STATUS retval;
    IxAtmSchedulerVcId schVcId;

    if (!schInitDone||
        (port<IX_UTOPIA_PORT_0)||
        (port>=IX_UTOPIA_MAX_PORTS)||
        !ixAtmSchedulingEnabled[port])
    {
        return IX_FAIL;
    }
    
    retval = ixAtmSchCac(port ,trafficDesc);
    if (retval != IX_SUCCESS) 
    {	
	return retval;
    }

    schVcId = ixAtmSchFreeVcIdGet();
    if (schVcId == IX_ATMSCH_NULL_INDEX) 
    {
	return IX_ATMSCH_RET_NOT_ADMITTED;
    }

    /* Save the port */
    ixAtmSchVcTable[schVcId].port   = port;

    /* Initialize the VC table for the registered VC */
    ixAtmSchVcTable[schVcId].count  = 0;
    ixAtmSchVcTable[schVcId].vbrPcrCellsCnt = 0;
    ixAtmSchVcTable[schVcId].vbrScrCellsCnt = 0;
    ixAtmSchVcTable[schVcId].connId = IX_ATM_IDLE_CELLS_CONNID; 
    ixAtmSchVcTable[schVcId].schInfo.cet = 0;
    ixAtmSchVcTable[schVcId].schInfo.cetPcr = 0;
    ixAtmSchVcTable[schVcId].schInfo.cetScr = 0;
    ixAtmSchVcTable[schVcId].schInfo.bt = 0;
    ixAtmSchVcTable[schVcId].schInfo.usPcr = 0;
    ixAtmSchVcTable[schVcId].schInfo.usScr = 0;

    /* Save the trafficDescriptor into a global variable */
    schTd[schVcId] = *trafficDesc;

    if (schTd[schVcId].atmService == IX_ATM_UBR) 
    {
	/* insert UBR VC into list */
	
	ixAtmSchUbrChainVcInsert(port,schVcId);
    }
    else if ((schTd[schVcId].atmService == IX_ATM_RTVBR) ||
	     (schTd[schVcId].atmService == IX_ATM_VBR)   ||
	     (schTd[schVcId].atmService == IX_ATM_CBR) )
    {
	/* Insert real time VC into list */
	retval = ixAtmSchRtVcInsert(port, schVcId);
	if (IX_SUCCESS != retval)
	{
	    /* Failed to insert the VC due to failure to set
	     * the base timer (this is for VBR only). 
	     * Ensure that parameters are set correctly */
	    return IX_FAIL;
	}
    }
    else
    {
	/* Unknown service type! */
	IX_ATMSCH_ASSERT( FALSE );
    }

    /* Set the status of the VC inUse to TRUE indicating that
       this VC table is in use */
    ixAtmSchVcTable[schVcId].inUse = TRUE;

    /* The vcId is passed back to the caller */
    *vcId = schVcId;

    return IX_SUCCESS;
}

/************************************************************************/
PRIVATE BOOL
ixAtmSchParamIsValid(IxAtmLogicalPort port, IxAtmSchedulerVcId vcId)
{
    if ((vcId >= IX_ATM_MAX_NUM_AAL_OAM_TX_VCS) || 
	(vcId < 0)||
        (ixAtmSchVcTable[vcId].inUse == FALSE)||
        (ixAtmSchVcTable[vcId].port != port)) 
    {
	return FALSE;
    }
    return TRUE;
}

/************************************************************************/
PUBLIC IX_STATUS
ixAtmSchVcConnIdSet( IxAtmLogicalPort port,
                     IxAtmSchedulerVcId vcId,
                     IxAtmConnId connId)
{
    if (!schInitDone||
        ixAtmSchParamIsValid(port, vcId) == FALSE)
    {
	return IX_FAIL;
    }
    
    ixAtmSchVcTable[vcId].connId = connId;

    return IX_SUCCESS;
}

/************************************************************************/
PUBLIC IX_STATUS
ixAtmSchVcModelRemove(IxAtmLogicalPort port, IxAtmSchedulerVcId vcId)
{
    if (!schInitDone||
        ixAtmSchParamIsValid(port, vcId) == FALSE)
    {
	return IX_FAIL;
    }
    if (schTd[vcId].atmService == IX_ATM_UBR) 
    {
	ixAtmSchUbrChainVcRemove( port, vcId );
    }
    else if ((schTd[vcId].atmService == IX_ATM_RTVBR) ||
	     (schTd[vcId].atmService == IX_ATM_VBR) ||
	     (schTd[vcId].atmService == IX_ATM_CBR))
    {
	ixAtmSchRtVcRemove(port,vcId);
    }
    else /* Invalid atm service type */
    {
	IX_ATMSCH_ASSERT( FALSE );
    }

    if (ixAtmSchVcTable[vcId].count != 0)
    {
	IX_ATMSCH_WARNING_REPORT( "Removal of VC with pending cells!");
    }

    /* Set the remove VC's inUse status to FALSE indicating that
     * it will be overwritten. */
    ixAtmSchVcTable[vcId].inUse = FALSE;
    ixAtmSchVcTable[vcId].port  = IX_UTOPIA_MAX_PORTS;

    /* Reset the base timer and scheduler timer to zero for the case
     * where all the VCs (UBR, VBR and CBR) are removed. If there are
     * VCs in either one of them, the timer remains */
    if ((ixAtmSchRtQueueHead[port] == IX_ATMSCH_NULL_INDEX) &&
	(ixAtmSchNextUbrToBeScheduled[port] == IX_ATMSCH_NULL_INDEX))
    {
	ixAtmSchBaseTime[port] = 0;
	ixAtmSchTime[port] = 0;
    }

    return IX_SUCCESS;
}

/************************************************************************/
/* Setting the base time for a real-time VC. 
 */
IX_STATUS
ixAtmSchBaseTimeSet (IxAtmLogicalPort port, UINT64 baseTime)
{
    UINT64 increment;
    IxAtmSchedulerVcId rtQPtr;
    UINT64 newMask = 0x0;

    /* Verify whether the basetime is equal or greater than the MASK (right
     * shift by 1). This is to ensure that the overrun would occur 
     * earlier.*/
    if (baseTime & (IX_ATMSCH_UINT_MASK))
    {
	/* Fail as the base time has to be less than the MASK value */
	return IX_FAIL;
    }

    increment = baseTime - ixAtmSchBaseTime[port];
    ixAtmSchBaseTime[port] = baseTime;
    if (ixAtmSchTime[port] < ixAtmSchBaseTime[port])
    {
	ixAtmSchTime[port] = ixAtmSchBaseTime[port];
    }
    
    /* This is applicable when there are more than one VC in the list. */
    rtQPtr = ixAtmSchRtQueueHead[port];
    while (rtQPtr != IX_ATMSCH_NULL_INDEX) 
    {
	if ( (ixAtmSchVcTable[rtQPtr].schInfo.cet & IX_ATMSCH_UINT_MASK) !=
	     ((ixAtmSchVcTable[rtQPtr].schInfo.cet + increment) & IX_ATMSCH_UINT_MASK) )
	{ /* i.e. Adding  the increment will cause a timer overflow.
	   * This should happen only *VERY* rarely if a low-bandwidth VC 
	   * is added just as the timer is about to overflow.
	   */
	    newMask = ixAtmSchVcTable[rtQPtr].schInfo.cet & IX_ATMSCH_UINT_MASK;
	    ixAtmSchVcTable[rtQPtr].schInfo.cet = ixAtmSchTime[port] | newMask;
	    ixAtmSchVcTable[rtQPtr].schInfo.cetScr = ixAtmSchTime[port] | newMask;
	    ixAtmSchVcTable[rtQPtr].schInfo.cetPcr = ixAtmSchTime[port] | newMask;
	}
	else
	{
	    ixAtmSchVcTable[rtQPtr].schInfo.cetScr = 
		ixAtmSchVcTable[rtQPtr].schInfo.cetPcr = 
		(ixAtmSchVcTable[rtQPtr].schInfo.cet += increment);
	}
	rtQPtr = ixAtmSchVcTable[rtQPtr].nextVc;
    }

    return IX_SUCCESS;
}

/************************************************************************/
/* This is to sort the real-time VC list based on the cet values 
 */
void
ixAtmSchBubbleSortRtVcQueue(IxAtmLogicalPort port)
{
    IxAtmSchedulerVcId thisRtVc;
    IxAtmSchedulerVcId nextRtVc;
    IxAtmSchedulerVcId prevRtVc = IX_ATMSCH_NULL_INDEX;

    BOOL swap = TRUE;

    while (swap)
    {
	swap = FALSE;

	/* save the head of the queue. thisRtVc will be moving forward 
	 * in the list*/
	thisRtVc = ixAtmSchRtQueueHead[port];

	while ((thisRtVc != IX_ATMSCH_NULL_INDEX) &&
	       (ixAtmSchVcTable[thisRtVc].nextVc != IX_ATMSCH_NULL_INDEX))
	{
	    /* Use nextRtVc to point to the thisRtVc's neighbour */
	    nextRtVc = ixAtmSchVcTable[thisRtVc].nextVc;

	    if (ixAtmSchVcTable[thisRtVc].schInfo.cet >
		ixAtmSchVcTable[nextRtVc].schInfo.cet)
	    {
		/* Perform swap here */
		ixAtmSchVcTable[thisRtVc].nextVc = ixAtmSchVcTable[nextRtVc].nextVc;
		ixAtmSchVcTable[nextRtVc].nextVc = thisRtVc;

		if (prevRtVc == IX_ATMSCH_NULL_INDEX)
		{
		    /* position the sappwed VC to be at the head of the list */
		    ixAtmSchRtQueueHead[port] = nextRtVc;
		}
		else 
		{
		    /* position the VC in the list */
		    ixAtmSchVcTable[prevRtVc].nextVc = nextRtVc;
		}
		
		/* perform the swap again */
		swap = TRUE;
	    }
	    
	    /* Search the next VC in the list. Repeat the process of sorting 
	     * base on the the cet value*/
	    prevRtVc = thisRtVc;
	    thisRtVc = ixAtmSchVcTable[thisRtVc].nextVc;
	} /* inner while*/		
    } /* outer while */
}

/************************************************************************/
/* This funstion is called infrequently from within the 
 * bcAtmSchTableUpdate function, which is in turn invoked from 
 * an FIQ interrupts, and is therefore uninterruptible.
 */
void
ixAtmSchTimerOverrun (IxAtmLogicalPort port)
{
    IxAtmSchedulerVcId thisRtVc;
    UINT64 newCet;

    /* Time counter overrun.  Need to reset schTime back to base value
     * for all VCs.  This should happen once every ~35 minutes
     * (i.e. (2^31 - schBaseTime) us */

    ixAtmSchTime[port] = ixAtmSchBaseTime[port];
    thisRtVc = ixAtmSchRtQueueHead[port];

    while (thisRtVc != IX_ATMSCH_NULL_INDEX)
    {
	if (ixAtmSchVcTable[thisRtVc].schInfo.cet & (IX_ATMSCH_UINT_MASK >> 1))
	{
	    if (ixAtmSchVcTable[thisRtVc].count != 0)
	    { /* CET on VC has overrun also.  Ryesetting the cet back to schBaseTime
	       * will cause conformance errors, but this happens so infrequently that
	       * it shouldn't cause a problem */
		newCet = ixAtmSchBaseTime[port];
	    }
	    else 
	    {
		newCet = ixAtmSchBaseTime[port] | IX_ATMSCH_UINT_MASK;
	    }
	}
	else
	{
	    newCet = ixAtmSchBaseTime[port];
	}

	if ((ixAtmSchVcTable[thisRtVc].atmService == IX_ATM_VBR) ||
	    (ixAtmSchVcTable[thisRtVc].atmService == IX_ATM_RTVBR))
	{
	    /* If there are PCR cells bursting */
	    if (ixAtmSchVcTable[thisRtVc].vbrPcrCellsCnt > 0)
	    {
		ixAtmSchVcTable[thisRtVc].schInfo.bt =
		    (ixAtmSchVcTable[thisRtVc].vbrPcrCellsCnt) *
		    (ixAtmSchVcTable[thisRtVc].schInfo.usScr -
		     ixAtmSchVcTable[thisRtVc].schInfo.usPcr);
	    }
	    else if (ixAtmSchVcTable[thisRtVc].vbrPcrCellsCnt == 
		     ixAtmSchVcTable[thisRtVc].schInfo.mbs)
	    {
		/* Restore the BT value when at MBS */
		ixAtmSchVcTable[thisRtVc].schInfo.bt =
		    ixAtmSchVcTable[thisRtVc].schInfo.baseBt;
	    }
	    else if (ixAtmSchVcTable[thisRtVc].vbrPcrCellsCnt == 0)
	    {
		ixAtmSchVcTable[thisRtVc].schInfo.bt = 0;
	    }

	}

	/* Need to reset cetScr & cetPcr values back down to new time or the 
	 * VC will block until almost the next timer overflow.  This will 
	 * clear the burst history for VBR Vcs, resulting in non-conforming 
	 * cells.  Again, this happens only very infrequently however.
	 */
	ixAtmSchVcTable[thisRtVc].schInfo.cetPcr = 
	ixAtmSchVcTable[thisRtVc].schInfo.cetScr = 
	    (ixAtmSchVcTable[thisRtVc].schInfo.cet = newCet);

	thisRtVc = ixAtmSchVcTable[thisRtVc].nextVc;
    }

    ixAtmSchBubbleSortRtVcQueue(port);

}
