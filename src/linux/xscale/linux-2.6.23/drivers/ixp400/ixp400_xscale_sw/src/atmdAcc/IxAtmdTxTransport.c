/**
* @file IxAtmdTxTransport.c
*
 * @author Intel Corporation
* @date 17 March 2002
*
* @brief ATM Tx real time interface
*
*        This module implements the transmit data path part
*        of Atmd. It provides public interfaces that enable
*        the user to submit and schedule PDUs for transmission,
*        and to control the re-cycling of the resources belonging to
*        transmitted PDUs.
*
* Design Notes:
*    This code is real-time critical
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
* Put the user defined include files required.
*/

#include "IxOsal.h"

#include "IxAtmdAccCtrl.h"
#include "IxAtmdDefines_p.h"
#include "IxAtmdAssert_p.h"
#include "IxAtmdUtil_p.h"
#include "IxAtmdTxCfgInfo_p.h"

#define IX_ATMDACC_TX_TRANSPORT_EXTERN
#include "IxAtmdTxTransport_p.h"

/* function prototype */
PRIVATE INLINE IX_STATUS
ixAtmdAccTxQueueEntriesGet(IxQMgrQId qMgrQueueId,
                           unsigned int *numberOfEntriesPtr,
                           unsigned int defaultNumberOfEntries);
PRIVATE INLINE IX_STATUS
ixAtmdAccTxQFreeEntriesQuery (IxAtmLogicalPort port,
                              unsigned int *numberOfCellsPtr);

PRIVATE INLINE UINT32
ixAtmdAccTxQueueEntryBuild (unsigned int physicalAddress,
                            unsigned int cellCount,
                            unsigned int cellType);

/* Remove INLINE keyword to resolve cannot inline warning (Linux). SCR1421 */
PRIVATE void
ixAtmdAccCellTx (UINT32 **qMgrEntryPtr,
                 unsigned int physicalAddress,
                 unsigned int cellCount,
                 unsigned int cellType);

/* Remove INLINE keyword to resolve cannot inline warning (Linux). SCR1421 */
PRIVATE void
ixAtmdAccChannelTx (UINT32 **qMgrEntryPtr,
                    IxAtmdAccTxVcDescriptor * vcDescriptor,
                    unsigned int cellCount);

PRIVATE void
ixAtmdAccTxScheduleProcess (UINT32 **qMgrEntryPtr,
                            IxAtmScheduleTableEntry *scheduleTableEntryPtr,
                            unsigned int lastScheduleTableEntry);

PRIVATE void
ixAtmdAccTxDoneProcess (unsigned int physicalAddress);

PRIVATE void
ixAtmdAccTxShutdownAck (IxAtmLogicalPort port);

/*
* Function definition.
*/

/* ---------------------------------------------------------
* Get the number of entries from a queue and handle the case
* where queues are moving during the operation
*/
PRIVATE INLINE IX_STATUS
ixAtmdAccTxQueueEntriesGet(IxQMgrQId qMgrQueueId,
                           unsigned int *numberOfEntriesPtr,
                           unsigned int defaultNumberOfEntries)
{
    /* read the number from the queue manager */
    IX_STATUS returnStatus = ixQMgrQNumEntriesGet (qMgrQueueId,
        numberOfEntriesPtr);
    
    /* check read success */
    if (returnStatus == IX_QMGR_WARNING)
    {
        /* read again the number from the queue manager */
        returnStatus = ixQMgrQNumEntriesGet (qMgrQueueId,
            numberOfEntriesPtr);
        
        /* cannot get the number of entries : this is because the queue is full */
        if (returnStatus == IX_QMGR_WARNING)
        {
            /* get the number of entries */
            returnStatus = IX_SUCCESS;
            *numberOfEntriesPtr = defaultNumberOfEntries;
        }
        else if (returnStatus != IX_SUCCESS)
        {
            /* map the return status */
            returnStatus = IX_FAIL;
        }
    } /* end of if(returnStatus) */
    else if (returnStatus != IX_SUCCESS)
    {
        /* map the return status */
        returnStatus = IX_FAIL;
    }
    return returnStatus;
}

/* ---------------------------------------------------------
*        Return the number of free entries in the transmit
*        queue for this port
*/
PRIVATE INLINE IX_STATUS
ixAtmdAccTxQFreeEntriesQuery (IxAtmLogicalPort port,
                              unsigned int *numberOfCellsPtr)
{
    unsigned int numEntries;
    IxAtmdAccPortDescriptor  *portDescriptor;
    IX_STATUS returnStatus = IX_SUCCESS;

    portDescriptor = &ixAtmdAccPortDescriptor[port];

    /* get the current number of entries */
    returnStatus = ixAtmdAccTxQueueEntriesGet (portDescriptor->txQueueId,
        &numEntries,
        0);

    /* map the return status */
    if (returnStatus != IX_SUCCESS)
    {
        returnStatus = IX_FAIL;
    }
    
    *numberOfCellsPtr = portDescriptor->txQueueSize - numEntries;

    return returnStatus;
}

/* ---------------------------------------------------------
* Write a queue entry to the qmgr tx queue
*/
PRIVATE INLINE UINT32
ixAtmdAccTxQueueEntryBuild (unsigned int physicalAddress,
                            unsigned int cellCount,
                            unsigned int cellType)
{
    /* build a tx queue entry 
    * For Data Cells
    *  - MSB 31-6 : descriptor physical address
    *  - bits 5-2 : number of cells
    *  - bits 1 : set to 0 (reserved)
    *  - bit 0 : set to 0 (flag data cell )
    * For Idle Cells
    *  - MSB 31-6 : 0
    *  - bits 5-2 : number of cells
    *  - bits 1 : set to 0 (reserved)
    *  - bit 0 : set to 0 (flag idle cell )
    */

    

    UINT32 qEntry = (cellCount << NPE_TX_CELLCOUNT_OFFSET) | physicalAddress | cellType;


    return qEntry;
}

/* ---------------------------------------------------------
*  Convert the requested number of cells to queue entries for NPE-A
*
*  qMgrEntryPtr is a pointer to an internal array of entries
*               This array, when ready, will be flushed by the qMgr
*  physicalAddress : physical address of a NPE descriptor which is
*               bound to the PDU to be sent, or 0 for Idle cells
*  cellCount : number of cells to send (data cell or idle cell)
*  cellType : Type of the cells to send (data cells or idle cells)
*/
PRIVATE void
ixAtmdAccCellTx (UINT32 **qMgrEntryPtr,
                 unsigned int physicalAddress,
                 unsigned int cellCount,
                 unsigned int cellType)
{
    IX_ATMDACC_ENSURE (cellCount != 0, "Bad cell count!");

    /* The following algorthim is designed for performance and
    * is a variation on Duff's Device, which is widely documented.
    * - ``Duff's device'' had come up in comp.lang.c as soon as 1983
    * - The device is to express general loop unrolling directly in C.  
    * - The device is legal dpANS C.  X3J11 considered it carefully and decided that it 
    *   was legal. The device is also legal C++, since Bjarne uses it in his book. 
    *
    * The algorithm is designed to replace the following slow code
    *
    * while (cellCount > 15)
    * {  qEntry = buildEntry(15 cells, physicaladdress, type);
    *    qMgrWriteReturnStatus = qMgrWrite(...qEntry)
    *    cellCount -= 15;
    * }
    * if (cellCount > 0)
    * {  qEntry = buildEntry(cellCount, physicaladdress, type);
    *    qMgrWriteReturnStatus = qMgrWrite(...qEntry)
    * }
    * return qMgrWriteReturnStatus
    *
    * The algorithm exploits
    * the fall through of the switch statement to process
    * successive queue entries of NPE_TX_MAXCELLS_PER_QENTRY cells,
    * Any residual cells are placed in the final queue entryin a separate step
    * after the loop ends.
    * This reduces the number of a control loop iterations by a factor of 8.
    */
    if(cellCount > NPE_TX_MAXCELLS_PER_QENTRY)
    {
        /* get the number of qEntries with a full count of cells
        * using an integer division with round-down to the previous integer 
        */
        unsigned int qEntryCount = cellCount / NPE_TX_MAXCELLS_PER_QENTRY;
        /* get the number of iterations using an integer division 
        * with round-up to the next integer 
        */
        unsigned int n = (qEntryCount + 7)/8;
        /* build one entry with the maximum of cells per entry . This queue 
        * entry is always
        */
        UINT32 qEntry = ixAtmdAccTxQueueEntryBuild (physicalAddress,
            NPE_TX_MAXCELLS_PER_QENTRY,
            cellType);
        
       /* store X queue entries with a number of cells
        * equal to NPE_TX_MAXCELLS_PER_QENTRY
        */
        switch(qEntryCount & 7)
        {
        case 0: do {  *(*qMgrEntryPtr)++ = qEntry;
        case 7:       *(*qMgrEntryPtr)++ = qEntry;
        case 6:       *(*qMgrEntryPtr)++ = qEntry;
        case 5:       *(*qMgrEntryPtr)++ = qEntry;
        case 4:       *(*qMgrEntryPtr)++ = qEntry;
        case 3:       *(*qMgrEntryPtr)++ = qEntry;
        case 2:       *(*qMgrEntryPtr)++ = qEntry;
        case 1:       *(*qMgrEntryPtr)++ = qEntry;
                } while (--n > 0);
        }

        /* check for any more cells to process (sub and mul are faster than mod)
        * cellCount  = qEntryCount % NPE_TX_MAXCELLS_PER_QENTRY
        */
        cellCount -= qEntryCount * NPE_TX_MAXCELLS_PER_QENTRY;
        if(cellCount == 0)
        {
            /* all done (the number of cells was an
            * exact multiple of NPE_TX_MAXCELLS_PER_QENTRY
            */
            return;
        }
    } 

    /* store 1 queue entry with a number of cells
    * between 1 and NPE_TX_MAXCELLS_PER_QENTRY (included)
    */
    *(*qMgrEntryPtr)++ = ixAtmdAccTxQueueEntryBuild (physicalAddress,
        cellCount, 
        cellType);
    return;
}

/*--------------------------------------------------------
* Transmit cells over this channel
*/
PRIVATE void
ixAtmdAccChannelTx (UINT32 **qMgrEntryPtr,
                    IxAtmdAccTxVcDescriptor * vcDescriptor,
                    unsigned int cellCount)
{
    IX_ATMDACC_TX_QUEUE *txSwQueue = &vcDescriptor->queue;

    /* process the number of cells for this channel */
    while (cellCount != 0)
    {
        /* check if all cells are from the current PDU */
        if (cellCount <= vcDescriptor->remainingPduCellCount)
        {
            /* update the current cell count for this pdu */
            vcDescriptor->remainingPduCellCount -= cellCount;

            /* reuse the same descriptor and transmit the data */
            ixAtmdAccCellTx (qMgrEntryPtr,
                vcDescriptor->currentNpeDesc->atmd.physicalAddress,
                cellCount,
                NPE_TX_DATACELL);

            /* update stats */
            IX_ATMDACC_FULL_STATS(
                ixAtmdAccTxTransportStats[vcDescriptor->port].dataCellScheduledCount += cellCount; );

            /* all data is transmitted, update the current cell count */
            cellCount = 0;
        } /* end of if(cellCount) */
        else
        {
        /* we may have to cross a PDU boundary this occur when
        * the scheduler schedule cells over PDU boundaries, or
        * when the last pdu is completely sent on a previous call
        */
            if (vcDescriptor->remainingPduCellCount > 0)
            {
                /* now send the remaining of current PDU */
                ixAtmdAccCellTx (qMgrEntryPtr,
                    vcDescriptor->currentNpeDesc->atmd.physicalAddress,
                    vcDescriptor->remainingPduCellCount,
                    NPE_TX_DATACELL);

                /* update stats */
                IX_ATMDACC_FULL_STATS(
                    ixAtmdAccTxTransportStats[vcDescriptor->port].dataCellScheduledCount +=
                    vcDescriptor->remainingPduCellCount; );

                /* update the current cell count */
                cellCount -= vcDescriptor->remainingPduCellCount;
            } /* end of if(vcDescriptor) */

            /* are there any more PDUs to transmit ? */
            if (!IX_ATMDACC_TXQ_SCHEDULE_PENDING (txSwQueue))
            {
                /* no data available for TX : this should never occur */
                /* transmit idle cells as a possible workaround, */
                ixAtmdAccCellTx (qMgrEntryPtr, 
                    NPE_TX_IDLECELL_ADDRESS, 
                    cellCount, 
                    NPE_TX_IDLECELL);

                /* invalidate the current PDU cell count so that on subsequent
                * invocation we behave correctly
                */

                /* update stats */
                IX_ATMDACC_FULL_STATS(
                    ixAtmdAccTxTransportStats[vcDescriptor->port].overScheduledCellCount +=
                    cellCount; );

                cellCount = 0;
                vcDescriptor->remainingPduCellCount = 0;
            } /* end of if(IX_ATMDACC_TXQ_SCHEDULE_PENDING) */                                       
            else
            {
                /* there are more PDUs to transmit
                * get the next descriptor with the PDU attached 
                */

                /* check unreachable conditions */
                IX_ATMDACC_ENSURE (IX_ATMDACC_TXQ_CONSISTENT (txSwQueue), "corrupted sw tx queue");

                /* get the current npeDescriptor from the sw queue */
                vcDescriptor->currentNpeDesc = IX_ATMDACC_TXQ_TAIL_ENTRY_GET (txSwQueue);
                IX_ATMDACC_TXQ_TAIL_INCR (txSwQueue);

                /* check unreachable conditions */
                IX_ATMDACC_ENSURE (IX_ATMDACC_TXQ_CONSISTENT (txSwQueue), "corrupted sw tx queue");

                /* update the remaining PDU cell count from the PDU size in cells */
                vcDescriptor->remainingPduCellCount = vcDescriptor->currentNpeDesc->atmd.totalCell;

                /* update stats */
                IX_ATMDACC_FULL_STATS( ixAtmdAccTxTransportStats[vcDescriptor->port].pduScheduledCount++; );

            } /* end of if-else(IX_ATMDACC_TXQ_SCHEDULE_PENDING) */
        } /* end of if-else(cellCount) */
    } /* end of while(cellCount) */
}

/* -----------------------------------------------------
* process the tx schedule array
* - qMgrEntryPtr : pointer to a storage area containing requests to the NPE
* - scheduleTableEntryPtr : array of entries
* - lastScheduleTableEntry : number of entries
*/
PRIVATE void
ixAtmdAccTxScheduleProcess (UINT32 **qMgrEntryPtr,
                            IxAtmScheduleTableEntry *scheduleTableEntryPtr,
                            unsigned int lastScheduleTableEntry)
{
    /* iterate through each entry of scheduler table */
    IxAtmConnId connId;
    unsigned int descriptorIndex;
    unsigned int cellCount;
    IxAtmdAccTxVcDescriptor *vcDescriptor;
    IxAtmScheduleTableEntry *lastScheduleTableEntryPtr;
    
    IX_ATMDACC_FULL_STATS( IxAtmdAccTxTransportStats *portStats; );

    /* pointer to the last table entry + 1 : this is used as an address comparison
    * to check the end of table (more efficient than an index and a loop)
    */
    lastScheduleTableEntryPtr = &scheduleTableEntryPtr[lastScheduleTableEntry];

    /* Iterate through the schedule table and transmit idle
    * or data cells on each VC as requested
    */
    while (scheduleTableEntryPtr != lastScheduleTableEntryPtr)
    {
        /* get the connection Id from this table entry */
        connId = scheduleTableEntryPtr->connId;
        
        /* get the cell count from this table entry */
        cellCount = scheduleTableEntryPtr->numberOfCells;

        /* extract the VC descriptor pool index from the connId */
        descriptorIndex = IX_ATMDACC_TX_VC_INDEX_GET (connId);
        vcDescriptor = &ixAtmdAccTxVcDescriptor[descriptorIndex];

        IX_ATMDACC_FULL_STATS( 
        portStats = &ixAtmdAccTxTransportStats[vcDescriptor->port]; );

        /* adavnce to the next table entry */
        scheduleTableEntryPtr++;

        if(cellCount != 0)
        {
            if (connId == IX_ATM_IDLE_CELLS_CONNID)
            {
                /* transmit idle cells */
                ixAtmdAccCellTx (qMgrEntryPtr,
                    NPE_TX_IDLECELL_ADDRESS,
                    cellCount,
                    NPE_TX_IDLECELL);
                
                /* update stats */
                IX_ATMDACC_FULL_STATS(
                    portStats->idleCellScheduledCount +=
                    cellCount; );
            } /* end of if(connId) */
            else
            {
                if (vcDescriptor->connId == connId)
                {
                    /* transmit data cells as requested */
                    ixAtmdAccChannelTx (qMgrEntryPtr,
                        vcDescriptor,
                        cellCount);
                }
                else
                {
                    /* check for disconnected channels */
                    if (IX_ATMDACC_TX_DISCONNECTCHECK(connId,vcDescriptor->connId))
                    {
                        /* the connId is obsolete, this may occur if the scheduling entity is
                        * processing is schedule table exactly during a disconnect. The
                        * action is to transmit data cells as requested. This way, recycling
                        * can be complete through txDone mechanism
                        */
                        ixAtmdAccChannelTx (qMgrEntryPtr,
                            vcDescriptor,
                            cellCount);

                        /* update stats */
                        IX_ATMDACC_FULL_STATS( portStats->disconnectScheduledCount++; );
                    } /* end of if(vcDescriptor) */
                    else
                    {
                        /* The connId provided is obsolete. To fullfill the port
                        * contract, idle cells are transmitted instead of data cells
                        */
                        ixAtmdAccCellTx (qMgrEntryPtr,
                            NPE_TX_IDLECELL_ADDRESS,
                            cellCount,
                            NPE_TX_IDLECELL);
                        
                        /* update stats */
                        IX_ATMDACC_FULL_STATS( 
                            portStats->wrongConnIdScheduledCount++;
                            portStats->idleCellScheduledCount += cellCount; );
                    } /* end of if-else(vcDescriptor) */
                } /* end of if-else(vcDescriptor) */
            } /* end of if-else(connId) */
        }
        else
        {
            /* update stats */
            IX_ATMDACC_FULL_STATS( portStats->zeroCellCount++; );
        } /* end of if-else(cellCount) */
    } /* end of for(scheduleEntry) */
}

/* -----------------------------------------------------
* process the tx schedule table for this port
* the schedule table contains the following fields :
* - a table size
* - an array pointer
* Each elementy of the array contains a connId and a 
* cell count (the number of cells to be transmitted 
* for the connection referenced by the connId)
*/
PUBLIC IX_STATUS
ixAtmdAccPortTxProcess (IxAtmLogicalPort port,
                        IxAtmScheduleTable * scheduleTablePtr)
{
    IxAtmdAccPortDescriptor *portDescriptor;
    unsigned int scheduleTableSizeInEntries;
    UINT32 *qMgrEntryPtr;
    unsigned int numberOfExtraEntriesRequired;
    unsigned int scheduleNewTableSizeInEntries;
    IxAtmConnId connId;
    unsigned int numberOfCells;              
    IxAtmScheduleTableEntry *scheduleNewTableEntryPtr;
    IX_STATUS returnStatus;

    IX_ATMDACC_FULL_STATS( IxAtmdAccTxTransportStats *portStats; );

    returnStatus = IX_SUCCESS;
    
    IX_ATMDACC_PARAMS_CHECK(
    /* check the port range and schedule table validity */
    if ((port >= IX_UTOPIA_MAX_PORTS)        ||
        (port < IX_UTOPIA_PORT_0)            ||
        (scheduleTablePtr == NULL)           ||
        (scheduleTablePtr->table == NULL)    ||
        (scheduleTablePtr->totalCellSlots >= IX_ATMDACC_TX_SCHEDULE_TABLE_SIZE)    ||
        (scheduleTablePtr->tableSize >= IX_ATMDACC_TX_SCHEDULE_TABLE_SIZE)    ||
        (scheduleTablePtr->tableSize == 0))
    {
        IX_ATMDACC_FULL_STATS(
        /* update the stats per port if the port range is valid */
        if ((port < IX_UTOPIA_MAX_PORTS) &&
            (port >= IX_UTOPIA_PORT_0))
        {
            portStats = &ixAtmdAccTxTransportStats[port];
            IX_ATMDACC_FULL_STATS( portStats->txProcessFailedCount++; );
        });
        return IX_FAIL;
    });

    scheduleTableSizeInEntries = scheduleTablePtr->tableSize;
    portDescriptor = &ixAtmdAccPortDescriptor[port];
    qMgrEntryPtr = portDescriptor->qMgrEntries; 

    IX_ATMDACC_FULL_STATS( 
        portStats = &ixAtmdAccTxTransportStats[port];
        portStats->txProcessInvokeCount++; );

    /* indicate that schedule processing is in progress
    * i.e. it is not safe to disconnect
    */
    portDescriptor->schedulingInProgress = TRUE;

    if (portDescriptor->status == IX_ATMDACC_PORT_UP)
    {
        /* check if there is a threshold set. If so, fill the
        * tx queue with single cells to meet the threshold requirements
        */
        if (portDescriptor->txQueueThreshold > 0)
        {
            /* move scheduled entries from the schedule table to 
            * a temporary table,and fill this table with single entries.
            * This will guarantee the accuracy of the threshold event.
            */
            numberOfExtraEntriesRequired = portDescriptor->txQueueThreshold + 1;
            scheduleNewTableSizeInEntries = 0;
            scheduleNewTableEntryPtr = &portDescriptor->scheduleTable[IX_ATMDACC_TX_SCHEDULE_TABLE_SIZE - 1];

            /* reads backwards the schedule table and fill an other schedule
            * table containing the exact number of entries required, with
            * one cell per entry.
            */
            while (scheduleTableSizeInEntries > 0 && numberOfExtraEntriesRequired > 0)
            {
                /* remove the last schedule table entry */
                scheduleTableSizeInEntries--;
                connId = scheduleTablePtr->table[scheduleTableSizeInEntries].connId;
                numberOfCells = scheduleTablePtr->table[scheduleTableSizeInEntries].numberOfCells;
                
                /* build many new entries with this last schedule table entry */
                while (numberOfCells > 0 && numberOfExtraEntriesRequired > 0)
                {
                    /* add 1 table entry per 1 cell */
                    scheduleNewTableEntryPtr--;
                    scheduleNewTableSizeInEntries++;
                    scheduleNewTableEntryPtr->connId = connId;
                    scheduleNewTableEntryPtr->numberOfCells = 1;
                    numberOfExtraEntriesRequired--;
                    numberOfCells--;
                }
                if (numberOfCells > 0)
                {
                    /* update the current new entry with remaining cells 
                    */
                    scheduleNewTableEntryPtr->numberOfCells += numberOfCells;
                }
            }
            
            IX_ATMDACC_FULL_STATS(
            if ((scheduleTableSizeInEntries + scheduleNewTableSizeInEntries) <= portDescriptor->txQueueThreshold)
            {
                portStats->txProcessUnderscheduledCount++;
            });

            /* process the initial schedule table (if any remaining) */
            if (scheduleTableSizeInEntries > 0)
            {
                /* process the schedule table */
                ixAtmdAccTxScheduleProcess (&qMgrEntryPtr,
                    scheduleTablePtr->table,
                    scheduleTableSizeInEntries);
            }
            /* process the additional schedule table (if any generated) */
            if (scheduleNewTableSizeInEntries > 0)
            {
                /* process all the remaining of the schedule table */
                ixAtmdAccTxScheduleProcess (&qMgrEntryPtr,
                    scheduleNewTableEntryPtr,
                    scheduleNewTableSizeInEntries);
            }
        }
        else
        {
            /* the threshold is set to 0. There is no need to ensure
            * that there are enough entries in the txVc queues to
            * guarantee a constant interrupt rate. the schedule
            * table is directly processed */
            ixAtmdAccTxScheduleProcess (&qMgrEntryPtr,
                scheduleTablePtr->table,
                scheduleTableSizeInEntries);
        }

        /* check unreachable condition : the number of entries should
        * not be greater than the size of the buffer. By construction
        * and after the tests of input parameters, this should never occur
        * at this point.
        */
        IX_ATMDACC_ENSURE(qMgrEntryPtr - portDescriptor->qMgrEntries < IX_ATMDACC_TX_SCHEDULE_TABLE_SIZE, 
            "Number of entries exceeded limit");

        /* write all entries */
        returnStatus = ixQMgrQBurstWrite (portDescriptor->txQueueId, 
            (UINT32)(qMgrEntryPtr - portDescriptor->qMgrEntries), 
            portDescriptor->qMgrEntries);

        if (returnStatus != IX_SUCCESS)
        {
            /* an unexpected error from qMgr occured.(tx queue full ?)
            * The scheduler is now out of sync with npe : Tr traffic get dropped, 
            * but the npe and scheduler and atmd will get in sync 
            * again when processing further traffic. 
            *
            * But at this point, if there is an attempt to disconnect,
            * disconnect will permanently fail (except if the port is set down)
            */
            returnStatus = IX_ATMDACC_WARNING;
        }
    } /* end of if(portDescriptor) */
    else
    {
        /* we will not process a schedule table when
        * the port is DOWN. just update stats 
        */
        IX_ATMDACC_FULL_STATS( portStats->txProcessFailedCount++; );
        returnStatus = IX_FAIL;
    } /* end of if-else(portDescriptor) */

    portDescriptor->schedulingInProgress = FALSE;

    return returnStatus;
}

/* -------------------------------------------------------------
* Get the maximum number of individual cells that can be
* scheduled. This is deduced from the queu size and the
* current queue level.
*/
PUBLIC IX_STATUS
ixAtmdAccPortTxFreeEntriesQuery (IxAtmLogicalPort port,
                                 unsigned int *numberOfCellsPtr)
{
    /* check inputs */
    IX_ATMDACC_PARAMS_CHECK(
    if ((port >= ixAtmdAccTxNumPortConfigured) ||
        (port < IX_UTOPIA_PORT_0)     ||
        (numberOfCellsPtr == NULL))
    {
        return IX_FAIL;
    });

    return ixAtmdAccTxQFreeEntriesQuery (port, numberOfCellsPtr);
}

/* -----------------------------------------------------------
* Get the number of free entries in the Tx queue and
* pass this back to the user via the user supplied callback
*/
void
ixAtmdAccTxLowCallBack (IxQMgrQId qId,
                        IxQMgrCallbackId cbId)
{
    IX_STATUS status;
    unsigned int cellNumber;

    /* When registering a cbId with the Q manager we actually gave
    * the port number, so we can get the port number back this way
    */
    IxAtmLogicalPort port = (IxAtmLogicalPort)cbId;

    /* check callback parameter */
    IX_ATMDACC_ENSURE (qId >= IX_NPE_A_QMQ_ATM_TXID_MIN, "Qmgr invalid callback qId");
    IX_ATMDACC_ENSURE (qId <= IX_NPE_A_QMQ_ATM_TXID_MAX, "Qmgr invalid callback qId");
    IX_ATMDACC_ENSURE (port < IX_UTOPIA_MAX_PORTS, "Qmgr invalid callback Id");
    IX_ATMDACC_ENSURE (port >= IX_UTOPIA_PORT_0, "Qmgr invalid callback Id");

    /* get the number of cells which can be scheduled for this port */
    status = ixAtmdAccTxQFreeEntriesQuery (port, &cellNumber);

    IX_ATMDACC_ENSURE (status == IX_SUCCESS, "can not get tx queue level");
    IX_ATMDACC_ENSURE (ixAtmdAccPortDescriptor[port].txLowCallback != NULL, "invalid callback pointer");

    /* invoke the user notification callback */
    (*ixAtmdAccPortDescriptor[port].txLowCallback) (port, cellNumber);
}

/* ----------------------------------------------------------
* Recycle NPE decsriptors referenced by physical address
* pass mbufs back to the user via user supplied callback
*/
PRIVATE void
ixAtmdAccTxDoneProcess (unsigned int physicalAddress)
{
    IxAtmdAccNpeDescriptor *npeDescriptor;
    IX_OSAL_MBUF *mbufPtr;
    IxAtmConnId connId;
    IxAtmdAccTxVcDescriptor *vcDescriptor;
    IX_ATMDACC_TX_QUEUE *txSwQueue;
    unsigned int descriptorIndex;

    /* convert the physical address to a logical address */
    npeDescriptor = (IxAtmdAccNpeDescriptor *) physicalAddress;
    IX_ATMDACC_CONVERT_TO_VIRTUAL_ADDRESS (npeDescriptor);

    /* invalidate the xscale MMU */
    IX_ATMDACC_DATA_CACHE_INVALIDATE(npeDescriptor, sizeof(npeDescriptor->npe.tx));

    IX_ATMD_DEBUG_DO( 
    /* check if the signature of the descriptor is still there */
    IX_ATMDACC_ABORT(npeDescriptor->atmd.signature == IX_ATMDACC_DESCRIPTOR_SIGNATURE, "Imvalid pointer returned by NPE"); );

    /* get values from the desriptor */
    connId = npeDescriptor->atmd.connId;

    /* extract the VC descriptor pool index from the connId
    * and get the descriptor
    */
    descriptorIndex = IX_ATMDACC_TX_VC_INDEX_GET (connId);
    vcDescriptor = &ixAtmdAccTxVcDescriptor[descriptorIndex];
    txSwQueue = &vcDescriptor->queue;

    /* check unreachable sw queue conditions */
    IX_ATMDACC_ENSURE (IX_ATMDACC_TXQ_CONSISTENT (txSwQueue), " corrupted s/w tx queue");

    /* check that NPE descriptor is the one we expected */
    if (IX_ATMDACC_TXQ_HEAD_ENTRY_GET (txSwQueue)->atmd.physicalAddress
        != physicalAddress)
    {
        unsigned int count = 0;
        
        IX_ATMDACC_FULL_STATS( ixAtmdAccTxDoneDispatchStats.descriptorOrderErrorCount++; );

        /* check if we missed a mbuf or a pdu (this occurs as part
        * of tx shutdown
        */
        do
        {
            /* get the first mbuf of the pdu */
            mbufPtr = ixAtmdAccUtilMbufFromNpeFormatConvert (
                    IX_ATMDACC_TXQ_HEAD_ENTRY_GET (txSwQueue)->atmd.pRootMbuf,
                    FALSE);

            /* pass the pdu to the user */
            (*vcDescriptor->txDoneCallback) (vcDescriptor->callbackId,
                mbufPtr);

            /* move forward in the queue */
            IX_ATMDACC_TXQ_HEAD_INCR(txSwQueue);
            
            /* check unreachable sw queue conditions */
            IX_ATMDACC_ENSURE (IX_ATMDACC_TXQ_CONSISTENT (txSwQueue), " corrupted s/w tx queue");
        
            count++;
        }
        while (count < IX_ATMDACC_TXQ_SIZE(txSwQueue) && 
            IX_ATMDACC_TXQ_HEAD_ENTRY_GET (txSwQueue)->atmd.physicalAddress
            != physicalAddress);
    
        /* check that NPE descriptor is the one we expected */
        IX_ATMDACC_ENSURE (IX_ATMDACC_TXQ_HEAD_ENTRY_GET (txSwQueue)->atmd.physicalAddress
            == physicalAddress,
            "NPE passed unexpected descriptor");
    }

    /* restore the mbuf address/endianness from the NPE to the user domain
    */
    mbufPtr = ixAtmdAccUtilMbufFromNpeFormatConvert (npeDescriptor->atmd.pRootMbuf, FALSE);

    /* recycle the descriptor using the descriptorIndex */
    IX_ATMDACC_TXQ_HEAD_INCR (txSwQueue);

    /* check unreachable sw queue conditions */
    IX_ATMDACC_ENSURE (IX_ATMDACC_TXQ_CONSISTENT (txSwQueue), " corrupted s/w tx queue");

    /* call the TX done event for this channel */
    vcDescriptor->txDoneCallback (vcDescriptor->callbackId, mbufPtr);
}

/* ------------------------------------------------
* Process a TX shutdown ack
*/
PRIVATE void
ixAtmdAccTxShutdownAck (IxAtmLogicalPort port)
{
    IxAtmdAccPortDescriptor *portDescriptor;

    if (port < IX_UTOPIA_MAX_PORTS)
    {
        portDescriptor = &ixAtmdAccPortDescriptor[port];
        if (portDescriptor->status == IX_ATMDACC_PORT_DOWN_PENDING)
        {
            /* set the status to down */
            portDescriptor->status = IX_ATMDACC_PORT_DOWN;
        }
    } /* end of if(port) */
    else
    {
        /* we ignore the ack */
    } /* end of if-else(port) */
    return;
}

/* ------------------------------------------------
* Try and process the specified number of entries
* from the tx done queue, return the number
* actually done
*/
PUBLIC IX_STATUS
ixAtmdAccTxDoneDispatch (unsigned int numberOfPdusToProcess,
                         unsigned int *numberOfPdusProcessedPtr)
{
    IX_STATUS returnStatus = IX_SUCCESS;
    IX_STATUS qmgrStatus;
    UINT32 qEntry;
    UINT32 *qEntryPtr;
    UINT32 numberOfEntriesToRead;

    IX_ATMDACC_FULL_STATS( unsigned int txDoneCount = 0; );
    /* update stats */
    IX_ATMDACC_FULL_STATS( ixAtmdAccTxDoneDispatchStats.invokeCount++; );


    /* check inputs */
    /* number of pdus to process must be > 0 */
    /* pointer for return param must not be null */
    if (numberOfPdusToProcess == 0 ||
        numberOfPdusProcessedPtr == NULL)
    {
        /* update stats */
        IX_ATMDACC_FULL_STATS( ixAtmdAccTxDoneDispatchStats.failedCount++; );
        return IX_FAIL;
    }

    /* initialise the number of pdu processed */
    *numberOfPdusProcessedPtr = 0;
    do
    {
        /* set the pointer to the beginning of the buffer */
        qEntryPtr = ixAtmdAccTxDoneDispatchBuffer;
        
        /* get the number of entries to read */
        if (numberOfPdusToProcess >= IX_ATMDACC_TXDONE_QUEUE_SIZE)
        {
            numberOfEntriesToRead = IX_ATMDACC_TXDONE_QUEUE_SIZE;
        }
        else
        {
            numberOfEntriesToRead = numberOfPdusToProcess;
            qEntryPtr[numberOfEntriesToRead] = 0;
        }

        /* read the TX Done queue. This algorithm suppose that
        * QMgr will store a null entry at the end of the buffer. The buffer size
        * is adjusted to contain an extra entry which is used to
        * mark the end of the buffer.
        */
        qmgrStatus = ixQMgrQBurstRead (IX_NPE_A_QMQ_ATM_TX_DONE, 
            numberOfEntriesToRead,
            qEntryPtr);

        /* get the first queue entry */
        qEntry = *qEntryPtr;

        /* iterate until there is a null entry */
        while (qEntry != 0)
        {
            /* update stats */
            IX_ATMDACC_FULL_STATS( txDoneCount++; );
            
            
            /* check the qentry type
            * Note for performance reasons the negative logical enables us to
            * deal with the normal case in the "then", whereas the untypical
            * is relegated to the "else". (hopefully avoiding a branch)
            */

            if ((qEntry & NPE_TX_SHUTDOWN_ACK_MASK) != NPE_TX_SHUTDOWN_ACK)
            {
                /* process NPE descriptor referenced by physical address */
                ixAtmdAccTxDoneProcess (qEntry & NPE_DESCRIPTOR_MASK);
                
                /* update stats */
                IX_ATMDACC_FULL_STATS( ixAtmdAccTxDoneDispatchStats.pduCount++; );
                
                /* increment the number of pdu processed */
                (*numberOfPdusProcessedPtr)++;
                numberOfPdusToProcess--;
            } /* end of if(qEntry) */
            else
            {
                /* process shutdown ack */
                ixAtmdAccTxShutdownAck ((unsigned int) ((qEntry & NPE_TX_SHUTDOWN_ACK_PORT_MASK) >> NPE_SHUTDOWN_ACK_SHIFT));
                
                /* update stats */
                IX_ATMDACC_FULL_STATS( ixAtmdAccTxDoneDispatchStats.ackCount++; );
            } /* end of if-else(qEntry) */

            /* get the next entry */
            qEntry =  *(++qEntryPtr);
        } /* end of while(qEntry) */
    }
    while ((qmgrStatus == IX_SUCCESS)  && (numberOfPdusToProcess > 0));

    IX_ATMDACC_FULL_STATS(
    if (returnStatus != IX_SUCCESS)
    {
        /* update stats */
        ixAtmdAccTxDoneDispatchStats.failedCount++;
    }
    else
    {
        /* update stats */
        if (txDoneCount > ixAtmdAccTxDoneDispatchStats.maxPduPerDispatch)
        {
            ixAtmdAccTxDoneDispatchStats.maxPduPerDispatch = txDoneCount;
        }
    });

    return returnStatus;
}

/* ----------------------------------------------------------
* Return the level of the Tx done queue
*/
PUBLIC IX_STATUS
ixAtmdAccTxDoneLevelQuery (unsigned int *numberOfPdusPtr)
{
    IX_STATUS returnStatus = IX_SUCCESS;

    /* check for null pointer */
    if (numberOfPdusPtr == NULL)
    {
        returnStatus = IX_FAIL;
    }
    else
    {
        /* get the number of entries from the Q manager */
        returnStatus = ixAtmdAccTxQueueEntriesGet (IX_NPE_A_QMQ_ATM_TX_DONE,
            numberOfPdusPtr,
            IX_ATMDACC_TXDONE_QUEUE_SIZE);
    } /* end of if-else(numberOfPdusPtr) */
    return returnStatus;
}

/* ----------------------------------------------------
* notify the user that the tx done queue level is high
*/
void
ixAtmdAccTxDoneCallBack (IxQMgrQId qId,
			 IxQMgrCallbackId cbId)
                         
{
#ifndef __wince
    IX_STATUS status;
#endif
    unsigned int numberOfPdus = IX_ATMDACC_ALLPDUS;
    unsigned int numberOfPdusProcessed = 0;

    /* WINCE_PORT NOTE: function pointer comparison across DLLs does not work. */
#ifndef __wince
    /* if the callback is not the internal dispatcher function
    * get the number of entries from the queue
    */
    if(ixAtmdAccTxDoneDispatcher != ixAtmdAccTxDoneDispatch)
    {
        status = ixAtmdAccTxQueueEntriesGet (IX_NPE_A_QMQ_ATM_TX_DONE, 
            &numberOfPdus,
            IX_ATMDACC_TXDONE_QUEUE_SIZE);

        /* no way to continue if the status is not success */
        IX_ATMDACC_ABORT(status == IX_SUCCESS, "TxDoneCallBack ixAtmdAccTxQueueEntriesGet failed");
    }
#endif

    /* process all entries from the queue */
    (*ixAtmdAccTxDoneDispatcher) (numberOfPdus, &numberOfPdusProcessed);
}

/* -------------------------------------------------------
* Submit a PDU for transmission
*/
PUBLIC IX_STATUS
ixAtmdAccTxVcPduSubmit (IxAtmConnId connId,
                        IX_OSAL_MBUF* mbufPtr,
                        IxAtmdAccClpStatus clp,
                        unsigned int numberOfCells)
{
    IX_STATUS status;
    UINT32 atmCellHeader;
    IxAtmdAccNpeDescriptor  *npeDescriptor;
    IxAtmdAccPortDescriptor *portDescriptor;
    IxAtmdAccTxVcDescriptor *vcDescriptor;
    IX_STATUS returnStatus = IX_SUCCESS;
    IX_ATMDACC_TX_QUEUE *txSwQueue;
    unsigned int descriptorIndex = IX_ATMDACC_TX_VC_INDEX_GET (connId);

    vcDescriptor = &ixAtmdAccTxVcDescriptor[descriptorIndex];

    /* indicate that a PDU transmit is in progress
    * i.e. not safe to disconnect
    */    
    vcDescriptor->pduTransmitInProgress = TRUE;

    /* sanity check the parameters, includes a check to ensure
    * that a disconnecting or obsolete connId are rejected
    */

    if ( (vcDescriptor->connId != connId)                                    ||
         (mbufPtr == NULL)                                                   ||
         (numberOfCells == 0)                                                ||
         ((clp != IX_ATMDACC_CLP_SET) && (clp != IX_ATMDACC_CLP_NOT_SET))
       )
    {
        /* param check failed */
        IX_ATMDACC_FULL_STATS(
        if(vcDescriptor->connId == connId)
        {   /* increment the stats of the correct channel */
            vcDescriptor->txVcPduSubmitFailureCount++;
        });
        vcDescriptor->pduTransmitInProgress = FALSE;
        return IX_FAIL;
    }

    IX_ATMDACC_PARAMS_CHECK(
    if ( ((unsigned int)IX_OSAL_MBUF_MLEN(mbufPtr) == 0)                            ||
         ((unsigned int)IX_OSAL_MBUF_PKT_LEN(mbufPtr) > IX_ATMDACC_MAX_PDU_LEN)     ||
         ((unsigned int)IX_OSAL_MBUF_PKT_LEN(mbufPtr) != numberOfCells * vcDescriptor->cellSize) 
       )
    {
        vcDescriptor->pduTransmitInProgress = FALSE;
        return IX_FAIL;
    });


    /* extract the VC descriptor pool index from the connId
    * and get the descriptor
    */
    txSwQueue = &vcDescriptor->queue;
    
    /* check the state of the tx queue */
    IX_ATMDACC_ENSURE (IX_ATMDACC_TXQ_CONSISTENT (txSwQueue), "corrupted sw tx queue");
    
    /* get a recycled descriptor */
    if (!IX_ATMDACC_TXQ_OVERLOADED (txSwQueue))
    {
    /* indicate that a PDU transmit is in progress
    * i.e. not safe to disconnect
        */
        
        /* get the current descriptor from the tx sw queue */
        npeDescriptor = IX_ATMDACC_TXQ_MID_ENTRY_GET (txSwQueue);
        
        /* get a port descriptor for this port */
        portDescriptor = &ixAtmdAccPortDescriptor[vcDescriptor->port];
        
        /* convert the mbuf/chain to Npe (physical) address, and
        * big endian format
        */
            npeDescriptor->atmd.pRootMbuf = mbufPtr;
            npeDescriptor->atmd.totalCell = numberOfCells;

            /* fill the NPE descriptor fields */
            atmCellHeader = npeDescriptor->npe.tx.atmCellHeader;
            IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN(UINT32, atmCellHeader);
            IX_NPE_A_ATMCELLHEADER_PTI_SET(atmCellHeader, 0);
            IX_NPE_A_ATMCELLHEADER_CLP_SET(atmCellHeader, clp);
            IX_ATMDACC_CONVERT_TO_BIG_ENDIAN(UINT32, atmCellHeader);
            npeDescriptor->npe.tx.atmCellHeader = atmCellHeader; 

            npeDescriptor->npe.tx.totalLen = IX_OSAL_MBUF_PKT_LEN(mbufPtr);
            npeDescriptor->npe.tx.currMbufLen = IX_OSAL_MBUF_MLEN(mbufPtr);
            IX_ATMDACC_CONVERT_TO_BIG_ENDIAN (UINT32, npeDescriptor->npe.tx.totalLen);
            IX_ATMDACC_CONVERT_TO_BIG_ENDIAN16 (npeDescriptor->npe.tx.currMbufLen);
            npeDescriptor->npe.tx.pCurrMbuf = ixAtmdAccUtilMbufToNpeFormatConvert (mbufPtr);
            npeDescriptor->npe.tx.pCurrMbufData = (unsigned char *)IX_OSAL_MBUF_MDATA(mbufPtr);
            npeDescriptor->npe.tx.pNextMbuf = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr);
            npeDescriptor->npe.tx.aal5CrcResidue = 0xffffffff;

            /* flush the xscale MMU */
            IX_ATMDACC_DATA_CACHE_FLUSH(npeDescriptor, sizeof(npeDescriptor->npe.tx));

            /* store the pdu in a sw queue */
            IX_ATMDACC_TXQ_MID_INCR (txSwQueue);

            /* check unreachable conditions */
            IX_ATMDACC_ENSURE (IX_ATMDACC_TXQ_CONSISTENT (txSwQueue), "corrupted sw tx queue");

            /* submit this request to the scheduler */
            status = (*(portDescriptor->schDemandUpdate)) (vcDescriptor->port,
                vcDescriptor->schedulerVcId,
                numberOfCells);

            switch (status)
            {
            case IX_SUCCESS:
                break;
            case IX_FAIL:
            case IX_ATMDACC_BUSY:
            /* If scheduler fails to accept this request we may need to roll back */
            /* perform a full rollback : remove the PDU from the queue */
            IX_ATMDACC_TXQ_MID_DECR (txSwQueue);

            /* if the sw queue is in a bad state, this is because there is over scheduling
            * during a channelUpdate failure (double failure from the scheduler). In this
            * situation, there is no possible recovery.
            */
            IX_ATMDACC_ABORT (IX_ATMDACC_TXQ_CONSISTENT (txSwQueue),
                "corrupted sw tx queue (overscheduling detected)");
            
            /* convert back to logical address for the user domain */
            ixAtmdAccUtilMbufFromNpeFormatConvert (mbufPtr, FALSE);
            
            /* update stats */
            IX_ATMDACC_FULL_STATS( vcDescriptor->txSubmitOverloadedCount++);

            /* no descriptor available */
            returnStatus = IX_ATMDACC_BUSY;
            break;
            default:
            /* any other kind of failure we dont roll back
             * because the NPE may have started to operate on
             * the PDU
             */
            IX_ATMDACC_ABORT(0, "ixAtmdAccTxPduSubmit non recoverable scheduling entity error");
            break;
        } /* end of switch(status) */
    } /* end of if(IX_ATMDACC_TXQ_OVERLOADED) */
    else
    {
        /* update stats */
        IX_ATMDACC_FULL_STATS( vcDescriptor->txSubmitOverloadedCount++; );
        
        /* no descriptor available */
        returnStatus = IX_ATMDACC_BUSY;
    } /* end of if-else(IX_ATMDACC_TXQ_OVERLOADED) */

   /* indicate that a PDU transmission is complete
    * i.e. safe to disconnect now
    */    
    vcDescriptor->pduTransmitInProgress = FALSE;
    return returnStatus;
}

/* -----------------------------------------------------------
*        transmit the specified number of cells emediately,
*        behaves like a dummy scheduler
*/
IX_STATUS
ixAtmdAccTxDummyDemandUpdate (IxAtmLogicalPort port,
                              int vcId,
                              unsigned int numberOfCells)
{
    IxAtmdAccTxVcDescriptor *vcDescriptor;
    IxAtmdAccNpeDescriptor *npeDescriptor;
    IxAtmdAccPortDescriptor *portDescriptor;
    IX_ATMDACC_TX_QUEUE *txSwQueue;
    unsigned int numFreeEntries;
    unsigned int numEntries;
    unsigned int numEntriesReqd;
    unsigned int descriptorIndex;
    IX_STATUS returnStatus;
    UINT32 *qMgrEntryPtr;

    portDescriptor = &ixAtmdAccPortDescriptor[port];

    /* ixAtmdAccTxUnscheduledModeVcIdGet ensures that VcId is a descriptorIndex */
    descriptorIndex = vcId;
    vcDescriptor = &ixAtmdAccTxVcDescriptor[descriptorIndex];
    txSwQueue = &vcDescriptor->queue;

    /* check the number of entries in the queue */
    returnStatus = ixAtmdAccTxQueueEntriesGet (portDescriptor->txQueueId,
        &numEntries, 
        0);

    if (returnStatus == IX_SUCCESS)
    {
        /* compute the number of free entries in the TX queue */
        numFreeEntries = portDescriptor->txQueueSize - (unsigned int) numEntries;

        /* compute the number of free entries required by the current demand */
        numEntriesReqd = (numberOfCells + (NPE_TX_MAXCELLS_PER_QENTRY - 1)) / NPE_TX_MAXCELLS_PER_QENTRY;

        /* compare the available queue level and the requested number of
        entries requested */
        if (numFreeEntries < numEntriesReqd)
        {
            /* cannot put all the PDU in the TX queue */
            returnStatus = IX_ATMDACC_BUSY;
        }
    } /* end of if(returnStatus) */

    if (returnStatus == IX_SUCCESS)
    {

        /* check unreachable conditions */
        IX_ATMDACC_ENSURE (IX_ATMDACC_TXQ_CONSISTENT (txSwQueue), "corrupted sw tx queue");
        /* check that there is something in the queue to schedule */
        IX_ATMDACC_ENSURE (IX_ATMDACC_TXQ_SCHEDULE_PENDING (txSwQueue), "corrupted sw tx queue");

        /* get the the descriptor from the tx queue */
        npeDescriptor = IX_ATMDACC_TXQ_TAIL_ENTRY_GET (txSwQueue);
        IX_ATMDACC_TXQ_TAIL_INCR (txSwQueue);

        /* check unreachable conditions */
        IX_ATMDACC_ENSURE (IX_ATMDACC_TXQ_CONSISTENT (txSwQueue), "corrupted sw tx queue");
        /* check that everything in the queue has been scheduled */
        IX_ATMDACC_ENSURE (!IX_ATMDACC_TXQ_SCHEDULE_PENDING (txSwQueue), "corrupted sw tx queue");

        qMgrEntryPtr = portDescriptor->qMgrEntries; 

        /* transmit all Cells from the current PDU */
        ixAtmdAccCellTx (&qMgrEntryPtr,
            npeDescriptor->atmd.physicalAddress,
            (unsigned int) numberOfCells,
            NPE_TX_DATACELL);

        /* get the number of entries by a pointer substraction and
        * burst write all entries if no error found before 
        */
        returnStatus = ixQMgrQBurstWrite (portDescriptor->txQueueId, 
            qMgrEntryPtr - portDescriptor->qMgrEntries, 
            portDescriptor->qMgrEntries);
        if (returnStatus != IX_SUCCESS)
        {
            returnStatus = IX_FAIL;
        }
        
        /* update stats */
        IX_ATMDACC_FULL_STATS(
            ixAtmdAccTxTransportStats[port].dataCellScheduledCount +=
            numberOfCells; );
    } /* end of if(returnStatus) */

    return returnStatus;
}


