/**
* @file IxAtmdRxCfgIf.c
*
 * @author Intel Corporation
* @date 17 March 2002
*
* @brief ATM RX configuration interface
*
* This interface is used by IxAtmdAcc client to setup a RX channel
*
* Design Notes:
*    All function of this interface are protected by a lock
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
* Sytem defined include files
*/

/*
* Put the user defined include files required.
*/

#include "IxOsal.h"

#include "IxAtmdAccCtrl.h"
#include "IxAtmdDefines_p.h"
#include "IxAtmdAssert_p.h"
#include "IxAtmdUtil_p.h"
#include "IxAtmdNpe_p.h"
#include "IxAtmdPortMgmt_p.h"
#include "IxAtmdRxCfgInfo_p.h"
#include "IxAtmdRxCfgIf_p.h"

/**
*
* @struct IxAtmdAccRxStats
*
* @brief Rx ststs
*
* This structure is used to collect stats informations abour rx activity
*
*/
typedef struct
{
    /* rx control counters */
    unsigned int connectCount;
    unsigned int disconnectCount;
    unsigned int connectDenied;
    unsigned int disconnectDenied;

    /* rx error counters */
    unsigned int rxNotificationSetErrorCount;
    unsigned int rxFreeThresholdSetErrorCount;
} IxAtmdAccRxStats;

static IxAtmdAccRxStats ixAtmdAccRxStats;

/* ---------------------------------------------------------
*  interface lock and initialisation protection
*/
static IxOsalMutex rxControlLock;
static BOOL ixAtmdAccRxCfgInitDone = FALSE;

#define IX_ATMDACC_RX_LOCK() (void)ixOsalMutexLock (&rxControlLock, IX_OSAL_WAIT_FOREVER)
#define IX_ATMDACC_RX_UNLOCK() (void)ixOsalMutexUnlock (&rxControlLock)

/* function prototype */
PRIVATE IX_STATUS
ixAtmdAccRxVcConnectParamsValidate (IxAtmLogicalPort port,
                                    unsigned int vpi,
                                    unsigned int vci,
                                    IxAtmdAccAalType aalServiceType,
                                    IxAtmRxQueueId rxQueueId,
                                    IxAtmdAccRxVcRxCallback rxCallback,
                                    unsigned int minimumQueueSize,
                                    IxAtmConnId * connIdPtr,
                                    IxAtmNpeRxVcId * npeVcIdPtr );

PRIVATE IX_STATUS
ixAtmdAccRxVcConnectPerform (IxAtmLogicalPort port,
                             unsigned int vpi,
                             unsigned int vci,
                             IxAtmdAccAalType aalServiceType,
                             IxAtmRxQueueId atmdRxQueueId,
                             IxAtmdAccUserId userId,
                             IxAtmdAccRxVcRxCallback rxCallback,
                             unsigned int minimumQueueSize,
                             IxAtmConnId * connIdPtr,
                             IxAtmNpeRxVcId * npeVcIdPtr);

/* ---------------------------------------------------------
* Initialisations
*/
IX_STATUS
ixAtmdAccRxCfgIfInit (void)
{
    IX_STATUS returnStatus;

    if(ixAtmdAccRxCfgInitDone)
    {
        /* init already done, this is an error */
        returnStatus = IX_FAIL;
    }
    else
    {
        ixAtmdAccRxCfgIfStatsReset ();

        /* initialise dependant component */
        returnStatus = ixAtmdAccRxCfgInfoInit ();

        if (returnStatus == IX_SUCCESS)
        {
            /* initialise the internal lock */
            returnStatus = ixOsalMutexInit (&rxControlLock);

            /* map the return status */
            if (returnStatus != IX_SUCCESS)
            {
                returnStatus = IX_FAIL;
            }
        } /* end of if(returnStatus) */
    }

    if (returnStatus == IX_SUCCESS)
    {
        /* mark initialisation as done */
        ixAtmdAccRxCfgInitDone = TRUE;
    }

    return returnStatus;
}


/* Uninitialisation of Rx sub component */
IX_STATUS
ixAtmdAccRxCfgIfUninit (void)
{
    IX_STATUS returnStatus;

    if (!ixAtmdAccRxCfgInitDone)
    {
        /* init not done, this is an error */
        returnStatus = IX_FAIL;
    }
    else
    {
    /* destroy the internal lock */
      returnStatus = ixOsalMutexDestroy (&rxControlLock);

        if (IX_SUCCESS == returnStatus)
        {
          /* uninitialise dependant component */
             returnStatus = ixAtmdAccRxCfgInfoUninit ();
        } /* end of if(returnStatus) */
    }

    if (IX_SUCCESS == returnStatus)
    {
        /* mark uninitialisation as done */
        ixAtmdAccRxCfgInitDone = FALSE;
    }
    return returnStatus;
}




/* ---------------------------------------------------------
*  initialisation
*/
void
ixAtmdAccRxCfgIfStatsReset (void)
{
    /* initialise the stats counters */
    ixOsalMemSet(&ixAtmdAccRxStats, 0, sizeof(ixAtmdAccRxStats));

    /* initialise the stats of the dependant component */
    ixAtmdAccRxCfgInfoStatsReset();
}

/* -------------------------------------------------
*  validate input params for ixAtmdAccRxVcConnect
*/
PRIVATE IX_STATUS
ixAtmdAccRxVcConnectParamsValidate (IxAtmLogicalPort port,
                                    unsigned int vpi,
                                    unsigned int vci,
                                    IxAtmdAccAalType aalServiceType,
                                    IxAtmRxQueueId rxQueueId,
                                    IxAtmdAccRxVcRxCallback rxCallback,
                                    unsigned int minimumQueueSize,
                                    IxAtmConnId * connIdPtr,
                                    IxAtmNpeRxVcId * npeVcIdPtr )
{
    /* check the following :
    * - value ranges
    * - null callback pointers
    * - vpi 0 / vci 0 cannot be used (idle cells)
    * - if the minimum queue size requested is
    *   not a default value, it should be
    *   an acceptable value, i.e less than the biggest
    *   size that the qmgr can handle
    */
    if ((vpi > IX_ATM_MAX_VPI)
         || (vci > IX_ATM_MAX_VCI)
         || (rxCallback == NULL)
         || ((rxQueueId != IX_ATM_RX_A) &&
             (rxQueueId != IX_ATM_RX_B))
         || (connIdPtr == NULL)
         || (npeVcIdPtr == NULL)
         || ((minimumQueueSize != IX_ATMDACC_DEFAULT_REPLENISH_COUNT) &&
         (minimumQueueSize > IX_ATMDACC_QMGR_MAX_QUEUE_SIZE)))
    {
        return IX_FAIL;
    } /* end of if(port) */

    
    /* perform sevice specific checks */
    switch (aalServiceType)
    {
    case IX_ATMDACC_AAL5:
    case IX_ATMDACC_AAL0_48:
    case IX_ATMDACC_AAL0_52:
        if ((port >= IX_UTOPIA_MAX_PORTS)
            || (port < IX_UTOPIA_PORT_0)
            ||((vpi == IX_ATMDACC_OAM_RX_VPI)
            && (vci == IX_ATMDACC_OAM_RX_VCI)))
        {
            return IX_FAIL;
        }
        break;
    case IX_ATMDACC_OAM:
        if ((port != IX_ATMDACC_OAM_RX_PORT)
            || (vpi != IX_ATMDACC_OAM_RX_VPI)
            || (vci != IX_ATMDACC_OAM_RX_VCI))
        {
            return IX_FAIL;
        }
        break;
    default:
        return IX_FAIL;
        break;
    }
    return IX_SUCCESS;
}

/* ---------------------------------------------------------
*    do real function of rx connect
*/
PRIVATE IX_STATUS
ixAtmdAccRxVcConnectPerform (IxAtmLogicalPort port,
                             unsigned int vpi,
                             unsigned int vci,
                             IxAtmdAccAalType aalServiceType,
                             IxAtmRxQueueId atmdRxQueueId,
                             IxAtmdAccUserId userId,
                             IxAtmdAccRxVcRxCallback rxCallback,
                             unsigned int minimumQueueSize,
                             IxAtmConnId * connIdPtr,
                             IxAtmNpeRxVcId * npeVcIdPtr)
{
    IX_STATUS returnStatus = IX_SUCCESS;

    /* check that the port has been configured and is UP */
    if ((aalServiceType != IX_ATMDACC_OAM) && !(ixAtmdAccPortIsEnabled (port)))
    {
        returnStatus = IX_FAIL;
    }

    if (returnStatus == IX_SUCCESS)
    {
        if (ixAtmdAccRxCfgRxVcExists (port, vpi, vci))
        {
            returnStatus = IX_FAIL;
        }
    }

    if (returnStatus == IX_SUCCESS)
    {
        /* get a free channel from the pool of available channels */
        returnStatus = ixAtmdAccRxCfgChannelGet ( port, vpi, vci, aalServiceType, npeVcIdPtr, connIdPtr);
    }

    if (returnStatus == IX_SUCCESS)
    {
        /* get a rx free queue from the ppol of existing rx free queues */
        returnStatus = ixAtmdAccRxCfgFreeQueueGet (aalServiceType, minimumQueueSize,
            *npeVcIdPtr);
    }

    if (returnStatus == IX_SUCCESS)
    {
        /* all parameters are valid, resources are available
        *  so we configure the channel
        */
        returnStatus = ixAtmdAccRxCfgChannelSet (*connIdPtr,
            *npeVcIdPtr,
            port,
            vpi,
            vci,
            aalServiceType,
            atmdRxQueueId,
            userId,
            rxCallback);

    } /* end of if(returnStatus) */

    return returnStatus;
}

/* ---------------------------------------------------------
*    rx connect
*/
PUBLIC IX_STATUS
ixAtmdAccRxVcConnect (IxAtmLogicalPort port,
                      unsigned int vpi,
                      unsigned int vci,
                      IxAtmdAccAalType aalServiceType,
                      IxAtmRxQueueId rxQueueId,
                      IxAtmdAccUserId userId,
                      IxAtmdAccRxVcRxCallback rxCallback,
                      unsigned int minimumQueueSize,
                      IxAtmConnId * connIdPtr,
                      IxAtmNpeRxVcId * npeVcIdPtr )
{
    IX_STATUS returnStatus;

    /* we have not been initialised no point
    *   in continuing
    */
    if(!ixAtmdAccRxCfgInitDone)
    {
        return IX_FAIL;
    }

    /* check the input params */
    returnStatus = ixAtmdAccRxVcConnectParamsValidate (port,
        vpi,
        vci,
        aalServiceType,
        rxQueueId,
        rxCallback,
        minimumQueueSize,
        connIdPtr,
        npeVcIdPtr);

    IX_ATMDACC_RX_LOCK ();

    if(returnStatus == IX_SUCCESS)
    {
        returnStatus = ixAtmdAccRxVcConnectPerform (port,
            vpi,
            vci,
            aalServiceType,
            rxQueueId,
            userId,
            rxCallback,
            minimumQueueSize,
            connIdPtr,
            npeVcIdPtr);
    }

    if (returnStatus == IX_SUCCESS)
    {
        /* increment statistics */
        ixAtmdAccRxStats.connectCount++;
    }
    else
    {
        /* increment statistics */
        ixAtmdAccRxStats.connectDenied++;
    } /* end of if-else(returnStatus) */

    IX_ATMDACC_RX_UNLOCK ();

    return returnStatus;
}

/* ---------------------------------------------------------
* rx start
*/
PUBLIC IX_STATUS
ixAtmdAccRxVcEnable (IxAtmConnId connId)
{
    IX_STATUS returnStatus = IX_FAIL;
    unsigned int npeVcId = 0;

    /* if we have not been initialised no point
    *   in continuing
    */
    if(ixAtmdAccRxCfgInitDone)
    {
        IX_ATMDACC_RX_LOCK ();

        /* check the connId and retrieve the channel id */
        if(ixAtmdAccRxCfgNpeVcIdGet (connId, &npeVcId) == IX_SUCCESS)
        {
            /* check if there is a disconnect in progress */
            if(!ixAtmdAccRxCfgVcIsDisconnecting (npeVcId))
            {
                /* check if the VC is already disabled */
                if(ixAtmdAccRxCfgVcIsDisabled (npeVcId))
                {
                    /* Rx is Disabled so we enable it */
                    ixAtmdAccRxCfgVcEnable (npeVcId);
                    returnStatus = ixAtmdAccRxCfgNpeVcLookupTableUpdate(npeVcId);
                    if (returnStatus != IX_SUCCESS)
                    {
                        /* rollback on failure */
                        ixAtmdAccRxCfgVcEnableRollback (npeVcId);
                    }
                }
                else
                {
                    /* check if the VC is already enabled */
                    if(ixAtmdAccRxCfgVcIsEnabled (npeVcId))
                    {
                        /* Rx is already enabled */
                        returnStatus = IX_ATMDACC_WARNING;
                    }
                    else
                    {
                        /* Rx is not enabled (disable is pending) */
                        returnStatus = IX_FAIL;
                    } /* end of if-else(ixAtmdAccRxCfgVcIsEnabled) */
                } /* end of if-else(ixAtmdAccRxCfgVcIsDisabled) */
            }
            else
            {
                /* Rx is disconnecting */
                returnStatus = IX_FAIL;
            }
        }

        IX_ATMDACC_RX_UNLOCK ();
    }
    return returnStatus;
}

/* ---------------------------------------------------------
* rx stop
*/
PUBLIC IX_STATUS
ixAtmdAccRxVcDisable (IxAtmConnId connId)
{
    IX_STATUS returnStatus = IX_FAIL;
    unsigned int npeVcId = 0;

    /* if we have not been initialised no point
    *   in continuing
    */
    if(ixAtmdAccRxCfgInitDone)
    {
        IX_ATMDACC_RX_LOCK ();

        /* check the connId and retrieve the channel id */
        if(ixAtmdAccRxCfgNpeVcIdGet (connId, &npeVcId) == IX_SUCCESS)
        {
            if(ixAtmdAccRxCfgVcIsEnabled(npeVcId))
            {
                /* Rx is enabled so initiate disabling */
                ixAtmdAccRxCfgVcDisable (npeVcId);
                returnStatus = ixAtmdAccUtilNpeMsgSend(IX_NPE_A_MSSG_ATM_RX_DISABLE,
                    npeVcId,
                    NPE_IGNORE);
                if (returnStatus != IX_SUCCESS)
                {
                    /* rollback on failure */
                    ixAtmdAccRxCfgVcEnable (npeVcId);
                }
            }
            else
            {               
                /* Rx is already disabled or disabled pending */
                returnStatus = IX_ATMDACC_WARNING;
            } /* end of if-else(ixAtmdAccRxCfgVcIsEnabled) */
        }

        IX_ATMDACC_RX_UNLOCK ();
    }
    return returnStatus;
}

/* ---------------------------------------------------------
*    rx disconnect
*/
PUBLIC IX_STATUS
ixAtmdAccRxVcTryDisconnect (IxAtmConnId connId)
{
    IX_STATUS returnStatus = IX_FAIL;
    unsigned int npeVcId = 0;

    /* if we have not been initialised no point
    *   in continuing
    */
    if(ixAtmdAccRxCfgInitDone)
    {
        IX_ATMDACC_RX_LOCK ();

        /* check the connId and retrieve the channel id */
        if(ixAtmdAccRxCfgNpeVcIdGet (connId, &npeVcId) == IX_SUCCESS)
        {
            /* we must be disabled before a disconnect is allowed */
            if(ixAtmdAccRxCfgVcIsDisabled (npeVcId))
            {
                /* invalidate the internal connId  */
                returnStatus = ixAtmdAccRxCfgConnIdInvalidate (npeVcId);

                if (returnStatus == IX_SUCCESS)
                {
                    /* disable the interrupts for this queue */
                    returnStatus = ixAtmdAccRxCfgRxFreeCallbackDisable (npeVcId);
                }

                if (returnStatus == IX_SUCCESS)
                {
                    /* check if resources are still allocated */
                    returnStatus = ixAtmdAccRxCfgResourcesRelease (npeVcId);
                }

                if (returnStatus == IX_SUCCESS)
                {
                    /* reset internal data */
                    returnStatus = ixAtmdAccRxCfgChannelReset (npeVcId);
                }
            }
            else
            {
                /* Rx has not been disabled, or has not completed */
                returnStatus = IX_ATMDACC_RESOURCES_STILL_ALLOCATED;
            } /* end of if-else(returnStatus) */
        }

        if (returnStatus == IX_SUCCESS)
        {
            /* increment statistics */
            ixAtmdAccRxStats.disconnectCount++;
        }
        else
        {
            ixAtmdAccRxStats.disconnectDenied++;
        } /* end of if-else(returnStatus) */

        IX_ATMDACC_RX_UNLOCK ();
    }

    return returnStatus;
}

/* ---------------------------------------------------------
* rx free threshold set
*/
PUBLIC IX_STATUS
ixAtmdAccRxVcFreeLowCallbackRegister (IxAtmConnId connId,
                                      unsigned int mbufNumber,
                                      IxAtmdAccRxVcFreeLowCallback callback)
{
    IX_STATUS returnStatus = IX_FAIL;
    unsigned int npeVcId = 0;

    /* we have not been initialised no point
    *   in continuing
    */
    if(ixAtmdAccRxCfgInitDone)
    {
        IX_ATMDACC_RX_LOCK ();

        if(callback != NULL)
        {
            /* check the connId and retrieve the channel id */
            if(ixAtmdAccRxCfgNpeVcIdGet (connId, &npeVcId) == IX_SUCCESS)
            {
                /* we only allow callbacks to be set when idle */
                if(ixAtmdAccRxCfgVcIsDisabled(npeVcId))
                {
                    returnStatus = ixAtmdAccRxCfgRxFreeCallbackSet (npeVcId, mbufNumber, callback);
                }
            }
        }

        if(returnStatus != IX_SUCCESS)
        {
            /* increment error counter */
            ixAtmdAccRxStats.rxFreeThresholdSetErrorCount++;
        }

        IX_ATMDACC_RX_UNLOCK ();
    }
    return returnStatus;
}

/* ---------------------------------------------------------
* rx notification set
*/
PUBLIC IX_STATUS
ixAtmdAccRxDispatcherRegister (IxAtmRxQueueId atmdQueueId,
                               IxAtmdAccRxDispatcher callback)
{
    IX_STATUS returnStatus = IX_FAIL;
    IxQMgrQId qMgrQueueId;

    /* we have not been initialised no point
    *   in continuing
    */
    if(ixAtmdAccRxCfgInitDone)
    {
        IX_ATMDACC_RX_LOCK ();

        /* the call back must be valid,
        * no Vcs must exist yet, and the atmdQueueId
        * must convert to valid QmgrQueueId
        */
        if ((callback != NULL)             &&
            (!ixAtmdAccRxCfgRxVcsExist ()) &&
            (ixAtmdAccUtilQmgrQIdGet (atmdQueueId, &qMgrQueueId) == IX_SUCCESS))
        {
            /* set the callback */
            returnStatus = ixAtmdAccRxCfgRxCallbackSet (atmdQueueId, 
                qMgrQueueId, 
                callback);
        }

        if (returnStatus != IX_SUCCESS)
        {
            /* increment error counter */
            ixAtmdAccRxStats.rxNotificationSetErrorCount++;
        }
        
        IX_ATMDACC_RX_UNLOCK ();
    }

    return returnStatus;
}



/* ---------------------------------------------------------
* rx notification reset
*/
PUBLIC IX_STATUS
ixAtmdAccRxDispatcherUnregister (IxAtmRxQueueId atmdQueueId)
{
    IX_STATUS returnStatus = IX_FAIL;
    IxQMgrQId qMgrQueueId;

    /* we have not been initialised no point in continuing */
    if (ixAtmdAccRxCfgInitDone)
    {
        IX_ATMDACC_RX_LOCK ();

        /* the call back must be valid,
        * no Vcs must exist yet, and the atmdQueueId
        * must convert to valid QmgrQueueId
        */
        if (ixAtmdAccUtilQmgrQIdGet (atmdQueueId, &qMgrQueueId) == IX_SUCCESS)
        {
            /* set the callback */
            returnStatus = ixAtmdAccRxCfgRxCallbackReset (atmdQueueId,qMgrQueueId);
        }
        IX_ATMDACC_RX_UNLOCK ();
    }
    return returnStatus;
}



/* ----------------------------------------------------
*       get the Rx Queue size
*/
PUBLIC IX_STATUS
ixAtmdAccRxQueueSizeQuery (IxAtmRxQueueId atmdQueueId,
                           unsigned int *numberOfPdusPtr)
{
    IX_STATUS returnStatus;
    IxQMgrQId qMgrQueueId;

    /* check parameters */
    if (numberOfPdusPtr == NULL)
    {
        returnStatus = IX_FAIL;
    }
    else
    {
        /* convert amd check input parameter */
        returnStatus = ixAtmdAccUtilQmgrQIdGet (atmdQueueId, &qMgrQueueId);
    } /* end of if-else(numberOfPdusPtr) */

    if (returnStatus == IX_SUCCESS)
    {
        /* read the queue size */
        returnStatus =
            ixAtmdAccUtilQueueSizeQuery (qMgrQueueId, numberOfPdusPtr);

    }

    return returnStatus;
}

/* -----------------------------------------------------
*   display rx configuration
*/
void
ixAtmdAccRxCfgIfChannelShow (IxAtmLogicalPort port)
{
    if(ixAtmdAccRxCfgInitDone)
    {
        IX_ATMDACC_RX_LOCK ();
        
        ixAtmdAccRxCfgInfoChannelShow (port);
 
        IX_ATMDACC_RX_UNLOCK ();
    }
 }

/* ---------------------------------------------------------
*  internal display
*/
void
ixAtmdAccRxCfgIfStatsShow (void)
{
    IxAtmdAccRxStats statsSnapshot;

    /* get a snapshort */
    IX_ATMDACC_RX_LOCK ();
    statsSnapshot = ixAtmdAccRxStats;
    IX_ATMDACC_RX_UNLOCK ();

    printf ("AtmdRx\n");
    printf ("Rx Connect ok ............................. : %10u\n",
        statsSnapshot.connectCount);
    printf ("Rx Disconnect ok .......................... : %10u\n",
        statsSnapshot.disconnectCount);
    printf ("Rx Connect denied ......................... : %10u (should be 0)\n",
        statsSnapshot.connectDenied);
    printf ("Rx Disconnect denied ...................... : %10u\n",
        statsSnapshot.disconnectDenied);
    printf ("Rx NotificationSet errors ................. : %10u (should be 0)\n",
        statsSnapshot.rxNotificationSetErrorCount);
    printf ("Rx Free ThresholdSet errors ............... : %10u (should be 0)\n",
        statsSnapshot.rxFreeThresholdSetErrorCount);

    ixAtmdAccRxCfgInfoStatsShow ();
}


