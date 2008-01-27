/**
* @file ixAtmdTxCfgIf.c
*
 * @author Intel Corporation
* @date 17 March 2002
*
* @brief ATM TX configuration and management
*
* Client interface for Tx configuration
*
* Design Notes:
*    All function share the same local lock
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
#include "IxAtmdAssert_p.h"
#include "IxAtmdTxCfgInfo_p.h"
#include "IxAtmdTxCfgIf_p.h"
#include "IxAtmdPortMgmt_p.h"
#include "IxAtmdUtil_p.h"
#include "IxAtmdNpe_p.h"

/**
*
* @struct IxAtmdAccTxStats
*
* @brief Tx ststs
*
* This structure is used to collect stats informations abour tx activity
*
*/
typedef struct
{
    /* tx control counters */
    unsigned int connectCount;
    unsigned int disconnectCount;
    unsigned int connectDenied;
    unsigned int disconnectDenied;
} IxAtmdAccTxStats;

static IxAtmdAccTxStats ixAtmdAccTxStats;

static BOOL ixAtmdAccTxCfgInitDone = FALSE;

/* ------------------------------------------------
tx lock get and release
*/
static IxOsalMutex txControlLock;

#define IX_ATMDACC_TX_LOCK_INIT() \
ixOsalMutexInit (&txControlLock)
#define IX_ATMDACC_TX_LOCK_GET() \
(void)ixOsalMutexLock (&txControlLock, IX_OSAL_WAIT_FOREVER)
#define IX_ATMDACC_TX_LOCK_RELEASE() \
(void)ixOsalMutexUnlock (&txControlLock)

/* function prototype */
PRIVATE BOOL
ixAtmdAccTxPortIsEnabledCheck(IxAtmLogicalPort port);

PRIVATE BOOL
ixAtmdAccTxPortIsDisabledCheck(IxAtmLogicalPort port);

PRIVATE IX_STATUS
ixAtmdAccTxPortStateChange(IxAtmLogicalPort port, IxAtmdAccPortState requestedState);

PRIVATE void
ixAtmdAccTxPortSetupNotify(unsigned int numPort);

PRIVATE IX_STATUS
ixAtmdAccTxVcConnectParamsValidate (IxAtmLogicalPort port,
                                    unsigned int vpi,
                                    unsigned int vci,
                                    IxAtmdAccAalType aalServiceType,
                                    IxAtmdAccTxVcBufferReturnCallback txDoneCallback,
                                    IxAtmConnId * connIdPtr);

PRIVATE IX_STATUS
ixAtmdAccTxVcConnectPerform (IxAtmLogicalPort port,
                             unsigned int vpi,
                             unsigned int vci,
                             IxAtmdAccAalType aalServiceType,
                             IxAtmdAccUserId userId,
                             IxAtmdAccTxVcBufferReturnCallback txDoneCallback,
                             IxAtmConnId * connIdPtr);

PUBLIC IX_STATUS
ixAtmdAccPortTxScheduledModeEnablePararmsValidate (IxAtmLogicalPort port,
                                                   IxAtmdAccTxVcDemandUpdateCallback demandUpdate,
                                                   IxAtmdAccTxVcDemandClearCallback demandClear,
                                                   IxAtmdAccTxSchVcIdGetCallback vcIdGet);

/* ------------------------------------------------------
* definition of private functions
*/

/* ------------------------------------------------------
* Query the if the port is enabled safely with locks
*/
PRIVATE BOOL
ixAtmdAccTxPortIsEnabledCheck(IxAtmLogicalPort port)
{
    BOOL result;
    IX_ATMDACC_TX_LOCK_GET ();
    result = ixAtmdAccTxPortEnabledQuery(port);
    IX_ATMDACC_TX_LOCK_RELEASE ();
    return result;
}

/* ------------------------------------------------------
* Query the if the port is disabled safely with locks
*/
PRIVATE BOOL
ixAtmdAccTxPortIsDisabledCheck(IxAtmLogicalPort port)
{
    BOOL result;
    IX_ATMDACC_TX_LOCK_GET ();
    result = ixAtmdAccTxPortDisabledQuery(port);
    IX_ATMDACC_TX_LOCK_RELEASE ();
    return result;
}

/* ------------------------------------------------------
* Change the port state, safely within locks
*/
PRIVATE IX_STATUS
ixAtmdAccTxPortStateChange(IxAtmLogicalPort port, IxAtmdAccPortState requestedState)
{
    IX_STATUS returnStatus;
    IX_ATMDACC_TX_LOCK_GET ();
    returnStatus = ixAtmdAccTxPortStateChangeHandler(port, requestedState);
    IX_ATMDACC_TX_LOCK_RELEASE ();
    return returnStatus;
}

/* ------------------------------------------------------
* Set the number of ports configured in the system
*/
PRIVATE void
ixAtmdAccTxPortSetupNotify(unsigned int numPort)
{
    IX_ATMDACC_TX_LOCK_GET ();
    ixAtmdAccTxPortSetupNotifyHandler(numPort);
    IX_ATMDACC_TX_LOCK_RELEASE ();
    return;
}

/* -------------------------------------------
*   Initialise the tx subcomponent
*/
IX_STATUS
ixAtmdAccTxCfgIfInit (void)
{
    IX_STATUS returnStatus = IX_SUCCESS;

    /* reset statistics counters */
    ixAtmdAccTxCfgIfStatsReset ();

    /* initialise tx data structures */
    if((ixAtmdAccTxCfgInitDone == FALSE)
        && (ixAtmdAccTxCfgInfoInit () == IX_SUCCESS) &&
        (ixOsalMutexInit (&txControlLock) == IX_SUCCESS))
    {
        /* register port state interface to port management */
        ixAtmdAccPortStateHandlersRegister(
            ixAtmdAccTxPortSetupNotify,
            ixAtmdAccTxPortStateChange,
            ixAtmdAccTxPortIsEnabledCheck,
            ixAtmdAccTxPortIsDisabledCheck);

        /* initialisae a security flag */
        ixAtmdAccTxCfgInitDone = TRUE;
    }
    else
    {
        returnStatus = IX_FAIL;
    } /* end of if-else(powerOf2) */
    return returnStatus;
}


/* -------------------------------------------
*   Uninitialise the tx subcomponent
*/
IX_STATUS
ixAtmdAccTxCfgIfUninit (void)
{
    IX_STATUS returnStatus = IX_SUCCESS;

      /* uninitialise tx data structures */
    if ((TRUE == ixAtmdAccTxCfgInitDone))
    {
        /* unregister port state interface to port management */
        ixAtmdAccPortStateHandlersUnregister ();

        if (IX_SUCCESS == ixOsalMutexDestroy (&txControlLock))
        {
            /* uninitialisae the security flag */
            ixAtmdAccTxCfgInitDone = FALSE;
        }
        else
        {
            returnStatus = IX_FAIL;
        }
    }
    else
    {
        returnStatus = IX_FAIL;
    } /* end of if-else */
    return returnStatus;
}



/* -----------------------------------------------------
*   display tx statistics
*/
void
ixAtmdAccTxCfgIfStatsShow (void)
{
    IxAtmdAccTxStats statsSnapshot;
    
    if(ixAtmdAccTxCfgInitDone)
    {
        /* get a stats snapshot */
        IX_ATMDACC_TX_LOCK_GET ();
        
        statsSnapshot = ixAtmdAccTxStats;
        
        IX_ATMDACC_TX_LOCK_RELEASE ();
        
        printf ("AtmdTx\n");
        printf ("Tx Connect ok ................. : %10u\n",
            statsSnapshot.connectCount);
        printf ("Tx Disconnect ok .............. : %10u\n",
            statsSnapshot.disconnectCount);
        printf ("Tx Connect denied ............. : %10u (should be 0)\n",
            statsSnapshot.connectDenied);
        printf ("Tx Disconnect denied .......... : %10u\n",
            statsSnapshot.disconnectDenied);
        
        ixAtmdAccTxCfgInfoStatsShow ();
    }
}

/* -----------------------------------------------------
*   display tx configuration
*/
void
ixAtmdAccTxCfgIfPortShow (IxAtmLogicalPort port)
{
    if(ixAtmdAccTxCfgInitDone)
    {
        IX_ATMDACC_TX_LOCK_GET ();
        
        ixAtmdAccTxCfgInfoPortShow (port);
        
        IX_ATMDACC_TX_LOCK_RELEASE ();
    }
}

/* -----------------------------------------------------
*   display tx configuration
*/
void
ixAtmdAccTxCfgIfChannelShow (IxAtmLogicalPort port)
{
    if(ixAtmdAccTxCfgInitDone)
    {
        IX_ATMDACC_TX_LOCK_GET ();
        
        ixAtmdAccTxCfgInfoChannelShow (port);
        
        IX_ATMDACC_TX_LOCK_RELEASE ();
    }
}

/* -----------------------------------------------------
*   reset tx statistics
*/
void
ixAtmdAccTxCfgIfStatsReset (void)
{
    /* initialise the error counters */
    ixOsalMemSet(&ixAtmdAccTxStats, 0, sizeof(ixAtmdAccTxStats));

    /* initialise the stats of the dependant component */
    ixAtmdAccTxCfgInfoStatsReset();
}

/* -----------------------------------------------
*   validate inputs for ixAtmdAccTxVcConnect
*/
PRIVATE IX_STATUS
ixAtmdAccTxVcConnectParamsValidate (IxAtmLogicalPort port,
                                    unsigned int vpi,
                                    unsigned int vci,
                                    IxAtmdAccAalType aalServiceType,
                                    IxAtmdAccTxVcBufferReturnCallback txDoneCallback,
                                    IxAtmConnId * connIdPtr)
{
    /* Sanity check input params */
    if ((port >= IX_UTOPIA_MAX_PORTS)    ||
        (port < IX_UTOPIA_PORT_0)        ||
        (vpi > IX_ATM_MAX_VPI)           ||
        (vci > IX_ATM_MAX_VCI)           ||
        (txDoneCallback == NULL)         ||
        (connIdPtr == NULL))
    {
        return IX_FAIL;
    }

    
    /* Service specific checks */
    switch(aalServiceType)
    {    
    case IX_ATMDACC_AAL5:
    case IX_ATMDACC_AAL0_48:
    case IX_ATMDACC_AAL0_52:
        if ((vpi  == IX_ATMDACC_OAM_TX_VPI)   &&
            (vci  == IX_ATMDACC_OAM_TX_VCI))
        {
            return IX_FAIL;
        }
        break;
    case IX_ATMDACC_OAM:
        if ((vpi  != IX_ATMDACC_OAM_TX_VPI)   ||
            (vci  != IX_ATMDACC_OAM_TX_VCI))
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

/* ---------------------------------------------------
*   do the real function of ixAtmdAccTxVcConnect
*/
PRIVATE IX_STATUS
ixAtmdAccTxVcConnectPerform (IxAtmLogicalPort port,
                             unsigned int vpi,
                             unsigned int vci,
                             IxAtmdAccAalType aalServiceType,
                             IxAtmdAccUserId userId,
                             IxAtmdAccTxVcBufferReturnCallback txDoneCallback,
                             IxAtmConnId * connIdPtr)
{
    IxAtmSchedulerVcId schedulerId;
    IX_STATUS returnStatus =IX_FAIL;

    /* check that the port has been configured and is UP */
    if (ixAtmdAccTxPortEnabledQuery (port))
    {
        /* check if this VC is already in use */
        if (!ixAtmdAccTxCfgVcConfigured (port, vpi, vci))
        {
            /* allocate a channel and get a connection Id for this connection */
            if(ixAtmdAccTxCfgFreeChannelGet (aalServiceType, connIdPtr) != IX_SUCCESS)
            {
                returnStatus = IX_ATMDACC_BUSY;
            }
            else
            {
                /* get scheduler id for this VC, using the user scheduler callback */
                if(ixAtmdAccTxCfgSchVcIdGet (port,
                    vpi,
                    vci,
                    *connIdPtr,
                    &schedulerId) == IX_SUCCESS)
                {
                    /* configure the channel */
                    returnStatus = ixAtmdAccTxCfgChannelSet (*connIdPtr,
                        schedulerId,
                        port,
                        vpi,
                        vci,
                        aalServiceType,
                        userId,
                        txDoneCallback);
                    if(returnStatus != IX_SUCCESS)
                    {
                        /* roll back by clearing the requested scheduler demand */
                        ixAtmdAccTxCfgVcDemandCancel (*connIdPtr, port, schedulerId);
                    }
                }
            }
        } /* end of if(ixAtmdAccTxCfgVcConfigured) */
    } /* end of if(ixAtmdAccTxPortEnabledQuery) */

    return returnStatus;
}

/* ---------------------------------------------------
*   establish a connection on this port, vpi and vci
*/
PUBLIC IX_STATUS
ixAtmdAccTxVcConnect (IxAtmLogicalPort port,
                      unsigned int vpi,
                      unsigned int vci,
                      IxAtmdAccAalType aalServiceType,
                      IxAtmdAccUserId userId,
                      IxAtmdAccTxVcBufferReturnCallback txDoneCallback,
                      IxAtmConnId * connIdPtr)
{

    IX_STATUS returnStatus = IX_FAIL;

    /* we have not been initialised, or we
    have be passed a null poiter, so no point
    in continuing
    */
    if(ixAtmdAccTxCfgInitDone)
    {
        /* protect against re-entrancy */
        IX_ATMDACC_TX_LOCK_GET ();

        if(ixAtmdAccTxVcConnectParamsValidate (port,
            vpi,
            vci,
            aalServiceType,
            txDoneCallback,
            connIdPtr) == IX_SUCCESS)
        {
            returnStatus = ixAtmdAccTxVcConnectPerform (port,
                vpi,
                vci,
                aalServiceType,
                userId,
                txDoneCallback,
                connIdPtr);
        }

        if(returnStatus == IX_SUCCESS)
        {
            /* increment statistics */
            ixAtmdAccTxStats.connectCount++;
        }
        else
        {
            /* increment statistics */
            ixAtmdAccTxStats.connectDenied++;
        } /* end of if-else(ixAtmdAccTxCfgVcConfigured) */

        IX_ATMDACC_TX_LOCK_RELEASE ();
    }
    return returnStatus;
}

/* ---------------------------------------------------
*   remove a connection
*/
PUBLIC IX_STATUS
ixAtmdAccTxVcTryDisconnect (IxAtmConnId connId)
{
    unsigned int txId;
    IX_STATUS returnStatus = IX_FAIL;

    /* we have not been initialised no point
    in continuing
    */
    if(ixAtmdAccTxCfgInitDone)
    {
        IX_ATMDACC_TX_LOCK_GET ();

        /* check the connId and retrieve the channel id */
        if(ixAtmdAccTxCfgIndexGet (connId, &txId) == IX_SUCCESS)
        {
            /* invalidate the connection Id */
            ixAtmdAccTxCfgConnIdInvalidate (txId);

            /* remove entries from the tx queue if the port is down */
            if(ixAtmdAccTxCfgPortResourcesRelease (connId) == IX_SUCCESS)
            {
                /* check if resources are still allocated */
                if(!ixAtmdAccTxCfgFreeResourcesCheck (txId))
                {
                    returnStatus = IX_ATMDACC_RESOURCES_STILL_ALLOCATED;
                }
                else
                {
                    /* clear any scheduler demand
                    * this must be done before reset because reset
                    * destroys the schedulerId
                    */
                    ixAtmdAccTxCfgVcDemandClear (connId);

                    /* reset internal data */
                    returnStatus = ixAtmdAccTxCfgChannelReset (txId);
                } /* end of if-else(qEntry) */
            }
        }

        if(returnStatus == IX_SUCCESS)
        {
            ixAtmdAccTxStats.disconnectCount++;
        }
        else
        {
            ixAtmdAccTxStats.disconnectDenied++;
        } /* end of if-else(ixAtmdAccTxCfgVcConfigured) */

        IX_ATMDACC_TX_LOCK_RELEASE ();
    }
    return returnStatus;
}

/* ------------------------------------------------
*   set the tx queue nearly-empty threshold
*/
PUBLIC IX_STATUS
ixAtmdAccPortTxCallbackRegister (IxAtmLogicalPort port,
                                 unsigned int numberOfCells,
                                 IxAtmdAccPortTxLowCallback callback)
{
    IX_STATUS returnStatus = IX_FAIL;

    if (ixAtmdAccTxCfgInitDone &&
        (port >= IX_UTOPIA_PORT_0) &&
        (port < IX_UTOPIA_MAX_PORTS) &&
        (callback != NULL))
    {
        IX_ATMDACC_TX_LOCK_GET ();

        if(ixAtmdAccPortConfigured (port) &&
            !ixAtmdAccTxCfgPortVcsExist (port))
        {
            returnStatus = ixAtmdAccTxCfgTxCallbackRegister (port, numberOfCells, callback);
        }

        IX_ATMDACC_TX_LOCK_RELEASE ();
    } /* end of if(ixAtmdAccTxCfgInitDone) */

    return returnStatus;
}


/* ------------------------------------------------
*   set the tx queue nearly-empty threshold
*/
PUBLIC void
ixAtmdAccPortTxCallbackUnregister (IxAtmLogicalPort port)
{
    ixAtmdAccTxCfgTxCallbackUnregister(port);
}



/* ------------------------------------------------
*   set the transmit dome nearly-full threshold
*/
PUBLIC IX_STATUS
ixAtmdAccTxDoneDispatcherRegister (unsigned int numberOfMbufs,
                                   IxAtmdAccTxDoneDispatcher callback)
{
    IX_STATUS returnStatus = IX_FAIL;

    /* Check that initialisation has been done,
    that we have not been passed a null pointer,
    and that the port is idle, the threshold should
    not be set whilst there is traffic present
    */
    if(ixAtmdAccTxCfgInitDone &&
        (callback != NULL))
    {
        IX_ATMDACC_TX_LOCK_GET ();

        if(!ixAtmdAccTxCfgVcsExist ())
        {
            returnStatus = ixAtmdAccTxCfgTxDoneCallbackRegister (numberOfMbufs, callback);
        }

        IX_ATMDACC_TX_LOCK_RELEASE ();
    }

    return returnStatus;
}


/* --------------------------------------------
*   Unregister the transmit done queue dispatcher
*/
PUBLIC void
ixAtmdAccTxDoneDispatcherUnregister (void)
{
    ixAtmdAccTxCfgTxDoneCallbackUnregister ();
}


/* ----------------------------------------------------
*   check the input params for ixAtmdAccPortTxScheduledModeEnable
*/
PUBLIC IX_STATUS
ixAtmdAccPortTxScheduledModeEnablePararmsValidate (IxAtmLogicalPort port,
                                                   IxAtmdAccTxVcDemandUpdateCallback demandUpdate,
                                                   IxAtmdAccTxVcDemandClearCallback demandClear,
                                                   IxAtmdAccTxSchVcIdGetCallback vcIdGet)
{
    IX_STATUS returnStatus = IX_SUCCESS;

    /* check parameters */
    if ((port >= IX_UTOPIA_MAX_PORTS) ||
        (port < IX_UTOPIA_PORT_0)  ||
        (demandUpdate == NULL)     ||
        (demandClear == NULL)      ||
        (vcIdGet == NULL)          ||
        !ixAtmdAccPortConfigured (port))
    {
        returnStatus = IX_FAIL;
    } /* end of if(port) */

    return returnStatus;
}

/* ----------------------------------------------------
*        regsiter the scheduler
*/
PUBLIC IX_STATUS
ixAtmdAccPortTxScheduledModeEnable (IxAtmLogicalPort port,
                                    IxAtmdAccTxVcDemandUpdateCallback demandUpdate,
                                    IxAtmdAccTxVcDemandClearCallback demandClear,
                                    IxAtmdAccTxSchVcIdGetCallback vcIdGet)
{
    IX_STATUS returnStatus = IX_FAIL;

    /* we have not been initialised no point
    in continuing
    */
    if(ixAtmdAccTxCfgInitDone)
    {
        IX_ATMDACC_TX_LOCK_GET ();

        /* check input params */
        if(ixAtmdAccPortTxScheduledModeEnablePararmsValidate (port,
            demandUpdate,
            demandClear,
            vcIdGet) == IX_SUCCESS)
        {
            /* check no channel are already set on this port */
            if (!ixAtmdAccTxCfgPortVcsExist (port))
            {
                ixAtmdAccTxCfgSchCallbackRegister (port,
                    demandUpdate,
                    demandClear,
                    vcIdGet);
                returnStatus = IX_SUCCESS;
            } /* end of if(ixAtmdAccTxCfgPortVcsExist) */
        }
        IX_ATMDACC_TX_LOCK_RELEASE ();
    }

    return returnStatus;
}



/* ----------------------------------------------------
*       deregister the scheduler
*/
PUBLIC IX_STATUS
ixAtmdAccPortTxScheduledModeDisable (IxAtmLogicalPort port)
{
    IX_STATUS returnStatus = IX_SUCCESS;

     /* check parameters */
    if ((IX_UTOPIA_MAX_PORTS <= port) ||
        (IX_UTOPIA_PORT_0 > port) ||
        (!ixAtmdAccPortConfigured (port)))
    {
        returnStatus = IX_FAIL;
    }  /* end of if(port) */

    if (ixAtmdAccTxCfgInitDone)
    {
        IX_ATMDACC_TX_LOCK_GET ();

        /* check no channel is already set on this port */
            if (!ixAtmdAccTxCfgPortVcsExist (port))
            {
                ixAtmdAccTxCfgSchCallbackUnregister (port);
                returnStatus = IX_SUCCESS;
            } /* end of if(ixAtmdAccTxCfgPortVcsExist) */

        IX_ATMDACC_TX_LOCK_RELEASE ();
    }

    return returnStatus;
}



/* ----------------------------------------------------
*       get the tx done queue size
*/
PUBLIC IX_STATUS
ixAtmdAccTxDoneQueueSizeQuery (unsigned int *numberOfPdusPtr)
{
    IX_STATUS returnStatus = IX_FAIL;

    /* check parameters */
    if (ixAtmdAccTxCfgInitDone &&
        numberOfPdusPtr != NULL)
    {
        /* query the queue size */
        returnStatus =
            ixAtmdAccUtilQueueSizeQuery(IX_NPE_A_QMQ_ATM_TX_DONE,
            numberOfPdusPtr);
    } /* end of if(ixAtmdAccTxCfgInitDone) */
    return returnStatus;
}


