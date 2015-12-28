/**
* @file IxAtmdPortMgmt.c
*
 * @author Intel Corporation
* @date 17 March 2002
*
* @brief ATM Port configuration and management
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
* User defined include files
*/
#include "IxOsal.h"
#include "IxQMgr.h"
#include "IxNpeMh.h"

#include "IxAtmdAccCtrl.h"
#include "IxAtmdDefines_p.h"
#include "IxAtmdNpe_p.h"
#include "IxAtmdAssert_p.h"
#include "IxAtmdUtil_p.h"
#include "IxAtmdPortMgmt_p.h"

/*
* #defines
*/
#define NPE_RESP_REQ NPE_RESP_REQ_OFF

#define IX_ATMDACC_PORTMGMT_LOCK_INIT() do{\
    IX_STATUS returnStatus = IX_SUCCESS;\
    IX_ATMDACC_ENSURE (ixAtmdAccPortMgmtInitDone == FALSE, "Initialisation Error");\
    returnStatus = ixOsalMutexInit (&portLock);\
    IX_ATMDACC_ENSURE (returnStatus == IX_SUCCESS, "Initialisation Error");\
    } while(0)

#define IX_ATMDACC_PORTMGMT_LOCK_GET() do{\
    ixOsalMutexLock (&portLock, IX_OSAL_WAIT_FOREVER);\
    } while(0)

#define IX_ATMDACC_PORTMGMT_LOCK_RELEASE() do{\
    ixOsalMutexUnlock (&portLock);\
    } while(0)


#define IX_ATMDACC_PORTMGMT_LOCK_DESTROY() do{\
    ixOsalMutexDestroy (&portLock);           \
    } while(0)


/* The npeResponse lock will serialise the excution of the functions
* ixAtmdAccUtopiaStatusRead, ixAtmdAccUtopiaConfigRead and
* ixAtmdAccUtopiaStatusRead with the IxNpeMh solicited callbacks.
* Each of these functions will again call IX_ATMDACC_PORTMGMT_LOCK_GET and
* block until the apropriate IxNpeMh solicited callback calls
* IX_ATMDACC_PORTMGMT_LOCK_RELEASE.
*
* NOTE: It would be more desirable if IxNpeMh provided an interface that would
* block until the response message from the NPE was available and return this
* message to the client instead of using the current callback mechanism.
*/
#define IX_ATMDACC_NPE_RESPONSE_LOCK_INIT() do{\
    IX_STATUS returnStatus = IX_SUCCESS;\
    IX_ATMDACC_ENSURE (ixAtmdAccPortMgmtInitDone == FALSE, "Initailisation Error");\
    returnStatus = ixOsalMutexInit (&npeResponseLock);\
    IX_ATMDACC_ENSURE (returnStatus == IX_SUCCESS, "Initailisation Error");\
    IX_ATMDACC_NPE_RESPONSE_LOCK_GET ();\
    } while(0)

#define IX_ATMDACC_NPE_RESPONSE_LOCK_GET() do{\
    ixOsalMutexLock (&npeResponseLock, IX_OSAL_WAIT_FOREVER);\
    } while(0)

#define IX_ATMDACC_NPE_RESPONSE_LOCK_RELEASE() do{\
    ixOsalMutexUnlock (&npeResponseLock);\
    } while(0)


#define IX_ATMDACC_NPE_RESPONSE_LOCK_DESTROY() do{\
    ixOsalMutexDestroy (&npeResponseLock);        \
    } while(0)


/*
* Function prototypes.
*/
PRIVATE void ixAtmdAccDummySetupNotifyHandler (unsigned int numPort);
PRIVATE IX_STATUS ixAtmdAccDummyStateChangeHandler (IxAtmLogicalPort port,
                                                    IxAtmdAccPortState state);
PRIVATE BOOL ixAtmdAccDummyStateQuery (IxAtmLogicalPort port);
PRIVATE IX_STATUS ixAtmdAccPortStateQuery (IxAtmLogicalPort port,
                                           IxAtmdAccPortState state,
                                           BOOL *paramError);
PRIVATE IX_STATUS ixAtmdAccPortStateChange (IxAtmLogicalPort port,
                                            IxAtmdAccPortState newState);

PRIVATE IX_STATUS ixAtmdAccUtopiaConfigWrite (const IxAtmdAccUtopiaConfig *utConfig);
PRIVATE IX_STATUS ixAtmdAccUtopiaConfigLoad (void);
PRIVATE void ixAtmdAccUtopiaConfigLoadCallback (IxNpeMhNpeId npeMhNpeId,
                                                  IxNpeMhMessage npeMhMessage);

PRIVATE void ixAtmdAccUtopiaStatusUploadCallback (IxNpeMhNpeId npeMhNpeId,
                                                  IxNpeMhMessage npeMhMessage);
PRIVATE IX_STATUS ixAtmdAccUtopiaStatusUpload (void);
PRIVATE IX_STATUS ixAtmdAccUtopiaStatusRead (IxAtmdAccUtopiaStatus* utStatus);

PRIVATE BOOL ixAtmdAccUtopiaConfigSetParamsValidate (const IxAtmdAccUtopiaConfig
                                                     *utConfig);

PRIVATE void ixAtmdAccNpeStatusReadCallback (IxNpeMhNpeId npeMhNpeId,
					     IxNpeMhMessage npeMhMessage);

PRIVATE void ixAtmdAccUtopiaConfigGenerate(const IxAtmdAccUtopiaConfig *utConfig,
                                           UINT32 *configArrPtr );

PRIVATE void 
ixAtmdAccNpeStatusGenerate(IxAtmdAccUtopiaStatus *statusStruct,
                         UINT32 *status);


                                                     /*
                                                     * Variables Private To This File.
*/
static IxOsalMutex portLock;
static IxOsalMutex npeResponseLock;
static BOOL ixAtmdAccPortMgmtInitDone = FALSE;
static IxAtmdAccPortSetupNotifyHandler setupNotify = ixAtmdAccDummySetupNotifyHandler;
static IxAtmdAccPortStateChangeHandler stateChangeRequest = ixAtmdAccDummyStateChangeHandler;
static IxAtmdAccPortStateQuery             isEnabledQuery = ixAtmdAccDummyStateQuery;
static IxAtmdAccPortStateQuery          isDisableComplete = ixAtmdAccDummyStateQuery;
static unsigned int numTxVcQueues; /**< number of Tx Vc queues in the system */
static unsigned int numberOfPortsConfigured; /**< number of ports in the system */
                                             /*
                                             * The following UINT32 variables are defined as volatile because they are accessed
                                             * from multiple threads. Declaring these as volatile indicates to the compiler that
                                             * these variables should not be placed in registers, which would result in incorrect
                                             * operation of the code.
                                            */
static volatile UINT32 npeRespWordRead = 0;
static volatile UINT32 npeRespOffsetRead = 0;
static BOOL utopiaConfigSetDone = FALSE;
static unsigned int portRequestStats[IX_UTOPIA_MAX_PORTS];
/*
* Function implementations
*/

/* -----------------------------------------
* API functions
* ----------------------------------------- */
PUBLIC IX_STATUS
ixAtmdAccPortEnable(IxAtmLogicalPort port)
{
    if (!ixAtmdAccPortMgmtInitDone)
    {
        return IX_FAIL;
    }
    else
    {
        return ixAtmdAccPortStateChange(port, IX_ATMD_PORT_ENABLED);
    }
}

/* ----------------------------------------------
*/
PUBLIC IX_STATUS
ixAtmdAccPortDisable(IxAtmLogicalPort port)
{
    if (!ixAtmdAccPortMgmtInitDone)
    {
        return IX_FAIL;
    }
    else
    {
        return ixAtmdAccPortStateChange(port, IX_ATMD_PORT_DISABLED);
    }
}

/* ----------------------------------------------
*/
PUBLIC BOOL
ixAtmdAccPortDisableComplete(IxAtmLogicalPort port)
{
    if (!ixAtmdAccPortMgmtInitDone)
    {
        return TRUE;
    }
    else
    {
        IX_STATUS retval;
        BOOL paramError;

        retval = ixAtmdAccPortStateQuery(port, IX_ATMD_PORT_DISABLED, &paramError);

        if ((retval != IX_SUCCESS) || (paramError))
        {
            return FALSE;
        }
    }

    return TRUE;
}


/* ----------------------------------------------
*/
PUBLIC IX_STATUS
ixAtmdAccUtopiaConfigSet (const IxAtmdAccUtopiaConfig *utConfig)
{
    IX_STATUS returnStatus = IX_SUCCESS;

    if (!ixAtmdAccPortMgmtInitDone || utopiaConfigSetDone)
    {
        returnStatus = IX_FAIL;
    }

    if (returnStatus == IX_SUCCESS)
    {
        if (!ixAtmdAccUtopiaConfigSetParamsValidate (utConfig))
        {
            returnStatus = IX_FAIL;
        }
    }

    if (returnStatus == IX_SUCCESS)
    {
        IX_ATMDACC_PORTMGMT_LOCK_GET ();

        returnStatus = ixAtmdAccUtopiaConfigWrite (utConfig);
        if (returnStatus == IX_SUCCESS)
        {
            returnStatus = ixAtmdAccUtopiaConfigLoad ();
        }

        if (returnStatus == IX_SUCCESS)
        {
            utopiaConfigSetDone = TRUE;
            numberOfPortsConfigured = utConfig->utTxConfig.txAddrRange + 1;
            setupNotify(numberOfPortsConfigured);
        }

        IX_ATMDACC_PORTMGMT_LOCK_RELEASE ();
    } /* end of if(returnStatus) */

    return returnStatus;
}



PUBLIC IX_STATUS
ixAtmdAccUtopiaConfigReset (const IxAtmdAccUtopiaConfig *utConfig)
{
        IX_STATUS returnStatus = IX_SUCCESS;
        /* Get the port management lock */
        /* initialise configuration registers and write to NPE-A*/
        IX_ATMDACC_PORTMGMT_LOCK_GET ();
        returnStatus = ixAtmdAccUtopiaConfigWrite (utConfig);
        if (IX_SUCCESS == returnStatus)
        {
            /* Load and get response from NPE-A */
            returnStatus = ixAtmdAccUtopiaConfigLoad ();
        }
        if (IX_SUCCESS == returnStatus)
        {
            utopiaConfigSetDone = FALSE;
        }
        IX_ATMDACC_PORTMGMT_LOCK_RELEASE ();
    return returnStatus ;
}



/* ----------------------------------------------
*/
PUBLIC IX_STATUS
ixAtmdAccUtopiaStatusGet (IxAtmdAccUtopiaStatus* utStatus)
{
    IX_STATUS returnStatus = IX_SUCCESS;

    if (!ixAtmdAccPortMgmtInitDone
        || utStatus == NULL)
    {
        returnStatus = IX_FAIL;
    }
    else
    {
        IX_ATMDACC_PORTMGMT_LOCK_GET ();
        
        /* Copy config & status registers from the Utopia coprocessor registers
        * to NPE memory
        */
        returnStatus = ixAtmdAccUtopiaStatusUpload ();
        
        if (returnStatus == IX_SUCCESS)
        {
            /* Copy the status table from Npe memory to Xscale memory.
            * NOTE: This is a blocking call
            */
            returnStatus = ixAtmdAccUtopiaStatusRead (utStatus);
        }
        
        IX_ATMDACC_PORTMGMT_LOCK_RELEASE ();
    }

    return returnStatus;
}

/* -----------------------------------------
* Internal functions
* ----------------------------------------- */
IX_STATUS
ixAtmdAccPortMgmtInit (void)
{
    IX_STATUS returnStatusVal = IX_SUCCESS;

    if (!ixAtmdAccPortMgmtInitDone)
    {
        utopiaConfigSetDone = FALSE;

        /* initialise internal lock */
        IX_ATMDACC_PORTMGMT_LOCK_INIT ();
        IX_ATMDACC_NPE_RESPONSE_LOCK_INIT ();

        ixAtmdAccPortMgmtInitDone = TRUE;
    } /* end of if(ixAtmdAccPortMgmtInitDone) */
    else
    {
        utopiaConfigSetDone = FALSE;
        returnStatusVal = IX_FAIL;
    } /* end of if-else(ixAtmdAccPortMgmtInitDone) */

    return returnStatusVal;
}



/* ixAtmdAccPortMgmtUninit function */
IX_STATUS
ixAtmdAccPortMgmtUninit (void)
{
    IX_STATUS returnStatusVal = IX_SUCCESS;

    if (ixAtmdAccPortMgmtInitDone)
    {
        /* uninitialise internal lock */
        IX_ATMDACC_NPE_RESPONSE_LOCK_DESTROY ();
        IX_ATMDACC_PORTMGMT_LOCK_DESTROY ();

        ixAtmdAccPortMgmtInitDone = FALSE;
    } /* end of if(ixAtmdAccPortMgmtInitDone) */
    else
    {
        returnStatusVal = IX_FAIL;
    } /* end of if-else(ixAtmdAccPortMgmtInitDone) */
    utopiaConfigSetDone = TRUE;
    return returnStatusVal;
}



/* ----------------------------------------------
*/
IX_STATUS
ixAtmdAccPortMgmtNumTxVcQueuesSet (unsigned int numTxQueues)
{
    IX_STATUS returnStatus = IX_SUCCESS;

    /* Check that the number of transmit VC queues is valid */
    if (numTxQueues > IX_UTOPIA_MAX_PORTS)
    {
        returnStatus = IX_FAIL;
    }
    else
    {
        numTxVcQueues = numTxQueues;
    } /* end of if-else(numTxQueues) */

    return returnStatus;
}

/* ----------------------------------------------
*/
IX_STATUS
ixAtmdAccPortMgmtNumTxVcQueuesGet (unsigned int *numTxQueuesPtr)
{
    IX_STATUS returnStatus = IX_SUCCESS;

    if (numTxQueuesPtr == NULL)
    {
        returnStatus = IX_FAIL;
    }
    else
    {
        *numTxQueuesPtr = numTxVcQueues;
    } /* end of if-else(numTxQueuesPtr) */

    return returnStatus;
}

/* ----------------------------------------------
*/
BOOL
ixAtmdAccPortIsEnabled(IxAtmLogicalPort port)
{
    IX_STATUS retval;
    BOOL paramError;

    retval = ixAtmdAccPortStateQuery(port, IX_ATMD_PORT_ENABLED, &paramError);

    if ((retval != IX_SUCCESS) || (paramError))
    {
        return FALSE;
    }

    return TRUE;
}

/* ----------------------------------------------
*/
void
ixAtmdAccPortStateHandlersRegister(
                                   IxAtmdAccPortSetupNotifyHandler setupNotifyHandler,
                                   IxAtmdAccPortStateChangeHandler stateChangeHandler,
                                   IxAtmdAccPortStateQuery         enabledQuery,
                                   IxAtmdAccPortStateQuery         disabledQuery)
{
    setupNotify        = setupNotifyHandler;
    stateChangeRequest = stateChangeHandler;
    isEnabledQuery     = enabledQuery;
    isDisableComplete  = disabledQuery;
}



void
ixAtmdAccPortStateHandlersUnregister(void)
{
    setupNotify        = ixAtmdAccDummySetupNotifyHandler;
    stateChangeRequest = ixAtmdAccDummyStateChangeHandler;
    isEnabledQuery     = ixAtmdAccDummyStateQuery;
    isDisableComplete  = ixAtmdAccDummyStateQuery;
}



/* ----------------------------------------------
*/
BOOL
ixAtmdAccPortConfigured (IxAtmLogicalPort port)
{
    /* check if the port valid and is configured */
    BOOL result = (port >= IX_UTOPIA_PORT_0) &&
        (port < IX_UTOPIA_MAX_PORTS) &&
        ((unsigned int)port < numberOfPortsConfigured);

    return result;
}

/* ----------------------------------------------
*/
void
ixAtmdAccPortMgmtStatsShow (void)
{
    int port;

    if (ixAtmdAccPortMgmtInitDone)
    {
        printf("\n");
        for (port = 0; port < IX_UTOPIA_MAX_PORTS; port++)
        {
            printf("Port %d : state change requested %u times\n",
                port,
                portRequestStats[port]);
        }
    }
    printf("\n");
}

/* ----------------------------------------------
*/
void
ixAtmdAccPortMgmtStatsReset (void)
{
    ixOsalMemSet(portRequestStats, 0, sizeof(portRequestStats));
}

/* ----------------------------------------------
*/
void
ixAtmdAccUtopiaControlStatsShow (void)
{
    IxAtmdAccUtopiaStatus utStatus;

    if (ixAtmdAccPortMgmtInitDone)
    {
        ixAtmdAccUtopiaStatusGet (&utStatus);

        printf ("Utopia Data\n");
        printf ("           utTxCellCount ....... : %10u\n",
            (unsigned int) utStatus.utTxCellCount);
        printf ("           utTxIdleCellCount ... : %10u\n",
            (unsigned int) utStatus.utTxIdleCellCount);
        printf ("           txFIFO2Underflow .... : %10u\n",
            (unsigned int) utStatus.utTxCellConditionStatus.txFIFO2Underflow);
        printf ("           txFIFO1Underflow .... : %10u\n",
            (unsigned int) utStatus.utTxCellConditionStatus.txFIFO1Underflow);
        printf ("           txFIFO2Overflow ..... : %10u\n",
            (unsigned int) utStatus.utTxCellConditionStatus.txFIFO2Overflow);
        printf ("           txFIFO1Overflow ..... : %10u\n",
            (unsigned int) utStatus.utTxCellConditionStatus.txFIFO1Overflow);
        printf ("           txIdleCellCountOvr .. : %10u\n",
            (unsigned int) utStatus.utTxCellConditionStatus.txIdleCellCountOvr);
        printf ("           txCellCountOvr ...... : %10u\n",
            (unsigned int) utStatus.utTxCellConditionStatus.txCellCountOvr);
        printf ("           utRxCellCount ....... : %10u\n",
            (unsigned int) utStatus.utRxCellCount);
        printf ("           utRxIdleCellCount ... : %10u\n",
            (unsigned int) utStatus.utRxIdleCellCount);
        printf ("           utRxInvalidHECount .. : %10u\n",
            (unsigned int) utStatus.utRxInvalidHECount);
        printf ("           utRxInvalidParCount . : %10u\n",
            (unsigned int) utStatus.utRxInvalidParCount);
        printf ("           utRxInvalidSizeCount  : %10u\n",
            (unsigned int) utStatus.utRxInvalidSizeCount);
        printf ("           rxCellCountOvr ...... : %10u\n",
            (unsigned int) utStatus.utRxCellConditionStatus.rxCellCountOvr);
        printf ("           invalidHecCountOvr .. : %10u\n",
            (unsigned int) utStatus.utRxCellConditionStatus.invalidHecCountOvr);
        printf ("           invalidParCountOvr .. : %10u\n",
            (unsigned int) utStatus.utRxCellConditionStatus.invalidParCountOvr);
        printf ("           invalidSizeCountOvr . : %10u\n",
            (unsigned int) utStatus.utRxCellConditionStatus.invalidSizeCountOvr);
        printf ("           rxIdleCountOvr ...... : %10u\n",
            (unsigned int) utStatus.utRxCellConditionStatus.rxIdleCountOvr);
        printf ("           rxFIFO2Underflow .... : %10u\n",
            (unsigned int) utStatus.utRxCellConditionStatus.rxFIFO2Underflow);
        printf ("           rxFIFO1Underflow .... : %10u\n",
            (unsigned int) utStatus.utRxCellConditionStatus.rxFIFO1Underflow);
        printf ("           rxFIFO2Overflow ..... : %10u\n",
            (unsigned int) utStatus.utRxCellConditionStatus.rxFIFO2Overflow);
        printf ("           rxFIFO1Overflow ..... : %10u\n",
            (unsigned int) utStatus.utRxCellConditionStatus.rxFIFO1Overflow);
    }

    return;
}

/* ----------------------------------------------
*/
void
ixAtmdAccUtopiaControlStatsReset (void)
{
}

/* -----------------------------------------
* Functions visable in this file only
* ----------------------------------------- */
PRIVATE IX_STATUS
ixAtmdAccPortStateChange (IxAtmLogicalPort port, IxAtmdAccPortState newState)
{
    IX_STATUS returnStatus = IX_SUCCESS;

    /* Check parameters */
    if ((port < IX_UTOPIA_PORT_0)
        ||(port >= IX_UTOPIA_MAX_PORTS)
        || !ixAtmdAccPortConfigured(port))
    {
        returnStatus = IX_FAIL;
    }

    if (returnStatus == IX_SUCCESS)
    {
        IX_ATMDACC_PORTMGMT_LOCK_GET();

        /* Notify the interested party */
        returnStatus = stateChangeRequest(port, newState);

        /* update Stats */
        portRequestStats[port]++;

        IX_ATMDACC_PORTMGMT_LOCK_RELEASE ();
    } /* end of if(returnStatus) */
    return returnStatus;
}

/* ----------------------------------------------
*/
PRIVATE IX_STATUS
ixAtmdAccPortStateQuery (IxAtmLogicalPort port,
                         IxAtmdAccPortState state,
                         BOOL *paramError)
{
    IX_STATUS returnStatus = IX_SUCCESS;

    *paramError = FALSE;

    /* Check parameters */
    if ((port < IX_UTOPIA_PORT_0)
        ||(port >= IX_UTOPIA_MAX_PORTS)
        || !ixAtmdAccPortConfigured(port))
    {
        *paramError = TRUE;
        returnStatus = IX_SUCCESS;
    }
    else if ((state != IX_ATMD_PORT_ENABLED) &&
        (state != IX_ATMD_PORT_DISABLED))
    {
        *paramError = TRUE;
        returnStatus = IX_FAIL;
    }
    else if (returnStatus == IX_SUCCESS)
    {
        IX_ATMDACC_PORTMGMT_LOCK_GET();

        if (state == IX_ATMD_PORT_ENABLED)
        {
            if (!isEnabledQuery (port))
            {
                returnStatus = IX_FAIL;
            }
        }
        else
        {
            if (!isDisableComplete (port))
            {
                returnStatus = IX_FAIL;
            }
        } /* end of if-else(state) */

        IX_ATMDACC_PORTMGMT_LOCK_RELEASE();
    } /* end of if(returnStatus) */

    return returnStatus;
}

/* ----------------------------------------------
*/
PRIVATE IX_STATUS
ixAtmdAccDummyStateChangeHandler (IxAtmLogicalPort port,
                                  IxAtmdAccPortState newState)
{
    IX_ATMDACC_ENSURE(0, "PortMgmt No StateChange Handler Registered");
    return IX_FAIL;
}

/* ----------------------------------------------
*/
PRIVATE void
ixAtmdAccDummySetupNotifyHandler (unsigned int numPort)
{
    IX_ATMDACC_ENSURE(0, "PortMgmt No Setup Handler Registered");
}

/* ----------------------------------------------
*/
PRIVATE BOOL
ixAtmdAccDummyStateQuery (IxAtmLogicalPort port)
{
    IX_ATMDACC_ENSURE(0, "PortMgmt No State Check Registered");
    return FALSE;
}

/* ----------------------------------------------
* This function creates the big Endian utopia configration
* required by the NPE. Each 32 bit word represents a utopia
* co-processor register. The corresponding bitfields in the 
* supplied utopia config struct are shifted and ORed together.
* The constants below represent the bit offsets for the fields
* as defined by utopia co-processor hardware.
* These will never change and are used nowhere apart from in 
* this function.
*/
PRIVATE void ixAtmdAccUtopiaConfigGenerate(const IxAtmdAccUtopiaConfig *utConfig,
                                           UINT32 *configArrPtr )
{
        configArrPtr[0] = (UINT32)(  
	(utConfig->utTxConfig.reserved_1    << 31) |      
	(utConfig->utTxConfig.txInterface   << 30) |      
	(utConfig->utTxConfig.txMode        << 29) |      
	(utConfig->utTxConfig.txOctet       << 28) |      
	(utConfig->utTxConfig.txParity      << 27) |      
	(utConfig->utTxConfig.txEvenParity  << 26) |      
	(utConfig->utTxConfig.txHEC         << 25) |      
	(utConfig->utTxConfig.txCOSET       << 24) |      
	(utConfig->utTxConfig.reserved_2    << 23) |      
	(utConfig->utTxConfig.txCellSize    << 16) |      
	(utConfig->utTxConfig.reserved_3    << 13) |      
	(utConfig->utTxConfig.txAddrRange   << 8) |      
	(utConfig->utTxConfig.reserved_4    << 5) |      
	(utConfig->utTxConfig.txPHYAddr )      
        );

        configArrPtr[1] = (UINT32)(  
	(utConfig->utTxStatsConfig.vpi  << 20) |          
	(utConfig->utTxStatsConfig.vci  << 4) |          
	(utConfig->utTxStatsConfig.pti  << 1) |          
	(utConfig->utTxStatsConfig.clp )
        );

        configArrPtr[2] = (UINT32)(  
	(utConfig->utTxDefineIdle.vpi  << 20) |  
	(utConfig->utTxDefineIdle.vci  << 4) |  
	(utConfig->utTxDefineIdle.pti  << 1) |  
	(utConfig->utTxDefineIdle.clp )
        );
   
        configArrPtr[3] = (UINT32)(  
	(utConfig->utTxEnableFields.defineTxIdleGFC  << 31) |  
	(utConfig->utTxEnableFields.defineTxIdlePTI  << 30) |  
	(utConfig->utTxEnableFields.defineTxIdleCLP  << 29) |  
	(utConfig->utTxEnableFields.phyStatsTxEnb    << 28) |  
	(utConfig->utTxEnableFields.vcStatsTxEnb     << 27) |  
	(utConfig->utTxEnableFields.vcStatsTxGFC     << 26) |  
	(utConfig->utTxEnableFields.vcStatsTxPTI     << 25) |  
	(utConfig->utTxEnableFields.vcStatsTxCLP     << 24) |  
	(utConfig->utTxEnableFields.reserved_1       << 21) |  
	(utConfig->utTxEnableFields.txPollStsInt     << 20) |  
	(utConfig->utTxEnableFields.txCellOvrInt     << 19) |  
	(utConfig->utTxEnableFields.txIdleCellOvrInt << 18) |  
	(utConfig->utTxEnableFields.enbIdleCellCnt   << 17) |  
	(utConfig->utTxEnableFields.enbTxCellCnt     << 16) |  
	(utConfig->utTxEnableFields.reserved_2 )
        );

        configArrPtr[4] = (UINT32)(  
	(utConfig->utTxTransTable0.phy0       << 27) |  
	(utConfig->utTxTransTable0.phy1       << 22) |  
	(utConfig->utTxTransTable0.phy2       << 17) |  
	(utConfig->utTxTransTable0.reserved_1 << 16) |  
	(utConfig->utTxTransTable0.phy3       << 11) |  
	(utConfig->utTxTransTable0.phy4       << 6) |  
	(utConfig->utTxTransTable0.phy5       << 1) |  
	(utConfig->utTxTransTable0.reserved_2 )  
        );

        configArrPtr[5] = (UINT32)(  
	(utConfig->utTxTransTable1.phy6       << 27) |  
	(utConfig->utTxTransTable1.phy7       << 22) |  
	(utConfig->utTxTransTable1.phy8       << 17) |  
	(utConfig->utTxTransTable1.reserved_1 << 16) |  
	(utConfig->utTxTransTable1.phy9       << 11) |  
	(utConfig->utTxTransTable1.phy10      << 6) |  
	(utConfig->utTxTransTable1.phy11      << 1) |  
	(utConfig->utTxTransTable1.reserved_2 )  
        );

        configArrPtr[6] = (UINT32)(  
	(utConfig->utTxTransTable2.phy12 << 27) |  
	(utConfig->utTxTransTable2.phy13 << 22) |  
	(utConfig->utTxTransTable2.phy14 << 17) |  
	(utConfig->utTxTransTable2.reserved_1 << 16) |  
	(utConfig->utTxTransTable2.phy15 << 11) |  
	(utConfig->utTxTransTable2.phy16 << 6) |  
	(utConfig->utTxTransTable2.phy17 << 1) |  
	(utConfig->utTxTransTable2.reserved_2 )  
        );

        configArrPtr[7] = (UINT32)(  
	(utConfig->utTxTransTable3.phy18 << 27) |  
	(utConfig->utTxTransTable3.phy19 << 22) |  
	(utConfig->utTxTransTable3.phy20 << 17) |  
	(utConfig->utTxTransTable3.reserved_1 << 16) |  
	(utConfig->utTxTransTable3.phy21 << 11) |  
	(utConfig->utTxTransTable3.phy22 << 6) |  
	(utConfig->utTxTransTable3.phy23 << 1) |  
	(utConfig->utTxTransTable3.reserved_2 )  
        );

        configArrPtr[8] = (UINT32)(  
	(utConfig->utTxTransTable4.phy24 << 27) |  
	(utConfig->utTxTransTable4.phy25 << 22) |  
	(utConfig->utTxTransTable4.phy26 << 17) |  
	(utConfig->utTxTransTable4.reserved_1 << 16) |  
	(utConfig->utTxTransTable4.phy27 << 11) |  
	(utConfig->utTxTransTable4.phy28 << 6) |  
	(utConfig->utTxTransTable4.phy29 << 1) |  
	(utConfig->utTxTransTable4.reserved_2 )
        );

        configArrPtr[9] = (UINT32)(  
	(utConfig->utTxTransTable5.phy30 << 27) |  
	(utConfig->utTxTransTable5.reserved_1 )
        );              

        configArrPtr[10] = (UINT32)(  
	(utConfig->utRxConfig.rxInterface << 31) |  
	(utConfig->utRxConfig.rxMode << 30) |  
	(utConfig->utRxConfig.rxOctet << 29) |  
	(utConfig->utRxConfig.rxParity << 28) |  
	(utConfig->utRxConfig.rxEvenParity << 27) |  
	(utConfig->utRxConfig.rxHEC << 26) |  
	(utConfig->utRxConfig.rxCOSET << 25) |  
	(utConfig->utRxConfig.rxHECpass << 24) |  
	(utConfig->utRxConfig.reserved_1 << 23) |  
	(utConfig->utRxConfig.rxCellSize << 16) |  
	(utConfig->utRxConfig.rxHashEnbGFC << 15) |  
	(utConfig->utRxConfig.rxPreHash << 14) |  
	(utConfig->utRxConfig.reserved_2 << 13) |  
	(utConfig->utRxConfig.rxAddrRange << 8) |  
	(utConfig->utRxConfig.reserved_3 << 5) |  
	(utConfig->utRxConfig.rxPHYAddr )
        );
        
        configArrPtr[11] = (UINT32)(  
	(utConfig->utRxStatsConfig.vpi << 20) |  
	(utConfig->utRxStatsConfig.vci << 4) |  
	(utConfig->utRxStatsConfig.pti << 1) |  
	(utConfig->utRxStatsConfig.clp )
        );

        configArrPtr[12] = (UINT32)(  
	(utConfig->utRxDefineIdle.vpi << 20) |  
	(utConfig->utRxDefineIdle.vci << 4) |  
	(utConfig->utRxDefineIdle.pti << 1) |  
	(utConfig->utRxDefineIdle.clp )
        );

        configArrPtr[13] = (UINT32)(  
	(utConfig->utRxEnableFields.defineRxIdleGFC << 31) |  
	(utConfig->utRxEnableFields.defineRxIdlePTI << 30) |  
	(utConfig->utRxEnableFields.defineRxIdleCLP << 29) |  
	(utConfig->utRxEnableFields.phyStatsRxEnb << 28) |  
	(utConfig->utRxEnableFields.vcStatsRxEnb << 27) |  
	(utConfig->utRxEnableFields.vcStatsRxGFC << 26) |  
	(utConfig->utRxEnableFields.vcStatsRxPTI << 25) |  
	(utConfig->utRxEnableFields.vcStatsRxCLP << 24) |  
	(utConfig->utRxEnableFields.discardHecErr << 23) |  
	(utConfig->utRxEnableFields.discardParErr << 22) |  
	(utConfig->utRxEnableFields.discardIdle << 21) |  
	(utConfig->utRxEnableFields.enbHecErrCnt << 20) |  
	(utConfig->utRxEnableFields.enbParErrCnt << 19) |  
	(utConfig->utRxEnableFields.enbIdleCellCnt << 18) |  
	(utConfig->utRxEnableFields.enbSizeErrCnt << 17) |  
	(utConfig->utRxEnableFields.enbRxCellCnt << 16) |  
	(utConfig->utRxEnableFields.reserved_1 << 13) |  
	(utConfig->utRxEnableFields.rxCellOvrInt << 12) |  
	(utConfig->utRxEnableFields.invalidHecOvrInt << 11) |  
	(utConfig->utRxEnableFields.invalidParOvrInt << 10) |  
	(utConfig->utRxEnableFields.invalidSizeOvrInt << 9) |  
	(utConfig->utRxEnableFields.rxIdleOvrInt << 8) |  
	(utConfig->utRxEnableFields.reserved_2 << 5) |  
	(utConfig->utRxEnableFields.rxAddrMask )
        );

        configArrPtr[14] = (UINT32)(  
	(utConfig->utRxTransTable0.phy0 << 27) |  
	(utConfig->utRxTransTable0.phy1 << 22) |  
	(utConfig->utRxTransTable0.phy2 << 17) |  
	(utConfig->utRxTransTable0.reserved_1 << 16) |  
	(utConfig->utRxTransTable0.phy3 << 11) |  
	(utConfig->utRxTransTable0.phy4 << 6) |  
	(utConfig->utRxTransTable0.phy5 << 1) |  
	(utConfig->utRxTransTable0.reserved_2 )
        );

        configArrPtr[15] = (UINT32)(  
	(utConfig->utRxTransTable1.phy6 << 27) |  
	(utConfig->utRxTransTable1.phy7 << 22) |  
	(utConfig->utRxTransTable1.phy8 << 17) |  
	(utConfig->utRxTransTable1.reserved_1 << 16) |  
	(utConfig->utRxTransTable1.phy9 << 11) |  
	(utConfig->utRxTransTable1.phy10 << 6) |  
	(utConfig->utRxTransTable1.phy11 << 1) |  
	(utConfig->utRxTransTable1.reserved_2 )
        );

        configArrPtr[16] = (UINT32)(  
	(utConfig->utRxTransTable2.phy12 << 27) |  
	(utConfig->utRxTransTable2.phy13 << 22) |  
	(utConfig->utRxTransTable2.phy14 << 17) |  
	(utConfig->utRxTransTable2.reserved_1 << 16) |  
	(utConfig->utRxTransTable2.phy15 << 11) |  
	(utConfig->utRxTransTable2.phy16 << 6) |  
	(utConfig->utRxTransTable2.phy17 << 1) |  
	(utConfig->utRxTransTable2.reserved_2 )
        );

        configArrPtr[17] = (UINT32)(  
	(utConfig->utRxTransTable3.phy18 << 27) |  
	(utConfig->utRxTransTable3.phy19 << 22) |  
	(utConfig->utRxTransTable3.phy20 << 17) |  
	(utConfig->utRxTransTable3.reserved_1 << 16) |  
	(utConfig->utRxTransTable3.phy21 << 11) |  
	(utConfig->utRxTransTable3.phy22 << 6) |  
	(utConfig->utRxTransTable3.phy23 << 1) |  
	(utConfig->utRxTransTable3.reserved_2 )
        );

        configArrPtr[18] = (UINT32)(  
	(utConfig->utRxTransTable4.phy24 << 27) |  
	(utConfig->utRxTransTable4.phy25 << 22) |  
	(utConfig->utRxTransTable4.phy26 << 17) |  
	(utConfig->utRxTransTable4.reserved_1 << 16) |  
	(utConfig->utRxTransTable4.phy27 << 11) |  
	(utConfig->utRxTransTable4.phy28 << 6) |  
	(utConfig->utRxTransTable4.phy29 << 1) |  
	(utConfig->utRxTransTable4.reserved_2 )
        );

        configArrPtr[19] = (UINT32)(  
	(utConfig->utRxTransTable5.phy30 << 27) |  
	(utConfig->utRxTransTable5.reserved_1 )
        );


        configArrPtr[20] = (UINT32)( 
        (utConfig->utSysConfig.reserved_1 << 30) |
        (utConfig->utSysConfig.txEnbFSM << 29) |
        (utConfig->utSysConfig.rxEnbFSM << 28) |
        (utConfig->utSysConfig.disablePins << 27) |
        (utConfig->utSysConfig.tstLoop << 26) |
        (utConfig->utSysConfig.txReset << 25) |
        (utConfig->utSysConfig.rxReset << 24) |
        (utConfig->utSysConfig.reserved_2) 
        );    

}




/* ----------------------------------------------
*/
PRIVATE IX_STATUS
ixAtmdAccUtopiaConfigWrite (const IxAtmdAccUtopiaConfig *utConfig)
{
    IX_STATUS returnStatus = IX_SUCCESS;
    UINT32 npeWords[sizeof (IxAtmdAccUtopiaConfig) / sizeof (UINT32)];
    int numWords = sizeof (IxAtmdAccUtopiaConfig) / sizeof (UINT32);
    int wordCnt;

    IX_ATMDACC_ENSURE (numWords < 64, "out of bound structure");

    ixAtmdAccUtopiaConfigGenerate(utConfig,  npeWords);

    /* Write one word of the config at a time to NPE-A */
    for (wordCnt = 0;
    (wordCnt < numWords) && (returnStatus == IX_SUCCESS);
    wordCnt++)
    {
        returnStatus = ixAtmdAccUtilNpeMsgSend (IX_NPE_A_MSSG_ATM_UTOPIA_CONFIG_WRITE,
            wordCnt * sizeof(UINT32),
            npeWords[wordCnt]);
    } /* end of for(wordCnt) */
    return returnStatus;
}

/* ----------------------------------------------
*/
PRIVATE IX_STATUS
ixAtmdAccUtopiaConfigLoad (void)
{
    IX_STATUS returnStatus = IX_SUCCESS;

    returnStatus = ixAtmdAccUtilNpeMsgWithResponseSend
        (IX_NPE_A_MSSG_ATM_UTOPIA_CONFIG_LOAD,
        NPE_IGNORE,
        NPE_IGNORE,
        ixAtmdAccUtopiaConfigLoadCallback);;

    if (returnStatus == IX_SUCCESS)
    {
        /* Wait for ixAtmdAccUtopiaConfigGetCallback to release */
        IX_ATMDACC_NPE_RESPONSE_LOCK_GET ();
    }

    return returnStatus;
}

/* ----------------------------------------------
*/
PRIVATE void
ixAtmdAccUtopiaConfigLoadCallback (IxNpeMhNpeId npeMhNpeId,
                                   IxNpeMhMessage npeMhMessage)
{
    UINT32 id;
    UINT32 status;

    /* Check NpeId */
    IX_ATMDACC_ENSURE (npeMhNpeId == IX_NPEMH_NPEID_NPEA, "wrong npeMhNpeId");

    /* Check the ID */
    id = npeMhMessage.data[0] & NPE_RESP_ID_MASK;

    IX_ATMDACC_ENSURE (id == NPE_UT_CONFIG_LOAD_EXPECTED_ID, "wrong id");

    /* Check the status */
    status = npeMhMessage.data[0] & NPE_RESP_STATUS_MASK;
    IX_ATMDACC_ENSURE (status == NPE_SUCCESS, "wrong status");

    /* Unblock the config get context */
    IX_ATMDACC_NPE_RESPONSE_LOCK_RELEASE ();
}

/* ----------------------------------------------
*/
PRIVATE IX_STATUS
ixAtmdAccUtopiaStatusUpload (void)
{
    IX_STATUS returnStatus;

    /* Setup message for UtopiaStatus command */
    returnStatus = ixAtmdAccUtilNpeMsgWithResponseSend(
        IX_NPE_A_MSSG_ATM_UTOPIA_STATUS_UPLOAD,
        NPE_IGNORE,
        NPE_IGNORE,
        ixAtmdAccUtopiaStatusUploadCallback);

    if (returnStatus == IX_SUCCESS)
    {
        /* Wait for ixAtmdAccUtopiaConfigGetCallback to release */
        IX_ATMDACC_NPE_RESPONSE_LOCK_GET ();
    }

    return returnStatus;
}

/* ----------------------------------------------
*/
PRIVATE void
ixAtmdAccUtopiaStatusUploadCallback (IxNpeMhNpeId npeMhNpeId,
                                     IxNpeMhMessage npeMhMessage)
{
    UINT32 id;
    UINT32 status;


    /* Check NpeId */
    IX_ATMDACC_ENSURE (npeMhNpeId == IX_NPEMH_NPEID_NPEA, "wrong npeMhNpeId");

    /* Check the ID */
    id = npeMhMessage.data[0] & NPE_RESP_ID_MASK;

    IX_ATMDACC_ENSURE (id == NPE_UT_STATUS_UPLOAD_EXPECTED_ID, "wrong id");

    /* Check the status */
    status = npeMhMessage.data[0] & NPE_RESP_STATUS_MASK;
    IX_ATMDACC_ENSURE (status == NPE_SUCCESS, "wrong status");

    /* Unblock the config get context */
    IX_ATMDACC_NPE_RESPONSE_LOCK_RELEASE ();
}

/* ----------------------------------------------
*/
PRIVATE void
ixAtmdAccNpeStatusReadCallback (IxNpeMhNpeId npeMhNpeId,
                                IxNpeMhMessage npeMhMessage)
{
    UINT32 id;

    /* Check NpeId */
    IX_ATMDACC_ENSURE (npeMhNpeId == IX_NPEMH_NPEID_NPEA, "wrong npeMhNpeId");

    /* Check Id */
    id = npeMhMessage.data[0] & NPE_RESP_ID_MASK;
    IX_ATMDACC_ENSURE (id == NPE_UT_STATUS_READ_EXPECTED_ID, "wrong id");

    npeRespOffsetRead = npeMhMessage.data[0] & NPE_RESP_OFFSET_MASK;
    npeRespWordRead = npeMhMessage.data[1];

    /* Unblock the reading context */
    IX_ATMDACC_NPE_RESPONSE_LOCK_RELEASE ();
}

/* ----------------------------------------------
* This converts the big Endian utopia status
* received by the NPE. Each 32 bit word represents a utopia
* co-processor register. The corresponding fields in the 
* supplied utopia status struct are filled.
* The constants below represent the regsiter and bit offsets for the 
* fields as defined by utopia co-processor hardware.
* These will never change and are used nowhere apart from in 
* this function.
*/
PRIVATE void 
ixAtmdAccNpeStatusGenerate(IxAtmdAccUtopiaStatus *statusStruct,
                          UINT32 *status)
{
    statusStruct->utTxCellCount     = status[0];
    statusStruct->utTxIdleCellCount = status[1];

    /* extract bit fields */
    statusStruct->utTxCellConditionStatus.txFIFO2Underflow =   ((status[2] >> 29) & 1);
    statusStruct->utTxCellConditionStatus.txFIFO1Underflow =   ((status[2] >> 28) & 1);
    statusStruct->utTxCellConditionStatus.txFIFO2Overflow  =   ((status[2] >> 27) & 1);
    statusStruct->utTxCellConditionStatus.txFIFO1Overflow  =   ((status[2] >> 26) & 1);
    statusStruct->utTxCellConditionStatus.txIdleCellCountOvr = ((status[2] >> 25) & 1);
    statusStruct->utTxCellConditionStatus.txCellCountOvr     = ((status[2] >> 24) & 1);
           
    statusStruct->utRxCellCount       = status[3];
    statusStruct->utRxIdleCellCount   = status[4];
    statusStruct->utRxInvalidHECount  = status[5];
    statusStruct->utRxInvalidParCount = status[6];
    statusStruct->utRxInvalidSizeCount = status[7];
    
    /* extract bit fields */
    statusStruct->utRxCellConditionStatus.rxCellCountOvr      = ((status[8] >> 28) & 1); 
    statusStruct->utRxCellConditionStatus.invalidHecCountOvr  = ((status[8] >> 27) & 1); 
    statusStruct->utRxCellConditionStatus.invalidParCountOvr  = ((status[8] >> 26) & 1); 
    statusStruct->utRxCellConditionStatus.invalidSizeCountOvr = ((status[8] >> 25) & 1); 
    statusStruct->utRxCellConditionStatus.rxIdleCountOvr      = ((status[8] >> 24) & 1); 
    statusStruct->utRxCellConditionStatus.rxFIFO2Underflow    = ((status[8] >> 19) & 1); 
    statusStruct->utRxCellConditionStatus.rxFIFO1Underflow    = ((status[8] >> 18) & 1); 
    statusStruct->utRxCellConditionStatus.rxFIFO2Overflow     = ((status[8] >> 17) & 1); 
    statusStruct->utRxCellConditionStatus.rxFIFO1Overflow     = ((status[8] >> 16) & 1); 
}




/* ----------------------------------------------
*/
PRIVATE IX_STATUS
ixAtmdAccUtopiaStatusRead (IxAtmdAccUtopiaStatus *utStatus)
{
    IX_STATUS returnStatus = IX_SUCCESS;
    UINT32  utStatusWords[sizeof(IxAtmdAccUtopiaStatus)/sizeof(UINT32)];
    int numWords = sizeof(IxAtmdAccUtopiaStatus)/sizeof(UINT32);
    int wordCnt;
    UINT32 readOffset;

    IX_ATMDACC_ENSURE (numWords < 64, "out of bound structure");

    ixOsalMemSet (utStatus, 0, sizeof (IxAtmdAccUtopiaStatus));

    /* Read the status table from NPE memory */

    for (wordCnt = 0;
        (wordCnt < numWords) && (returnStatus == IX_SUCCESS);
        wordCnt++)
    {
        /* offset to read */
        readOffset = wordCnt * sizeof(UINT32);
        /* read the word of the status table at this offset */
        returnStatus = ixAtmdAccUtilNpeMsgWithResponseSend(
            IX_NPE_A_MSSG_ATM_UTOPIA_STATUS_READ,
            readOffset,
            NPE_IGNORE,
            ixAtmdAccNpeStatusReadCallback);

        if (returnStatus == IX_SUCCESS)
        {
            /* Wait for ixAtmdAccNpeReadCallback to release */
            IX_ATMDACC_NPE_RESPONSE_LOCK_GET ();

            /* Check the address matches the address from ixAtmdAccNpeReadCallback */
            IX_ATMDACC_ENSURE (npeRespOffsetRead == readOffset, "Read error");

            /* Copy the word read from NPE memory into the utopia status
            * structure. NOTE: npewordRead is set by ixAtmdAccNpeReadCallback
            */
            utStatusWords[wordCnt] = npeRespWordRead;

        } /* end of if(returnStatus) */
    } /* end of for(wordCnt) */
    ixAtmdAccNpeStatusGenerate(utStatus,utStatusWords); 

    return returnStatus;
}

/* ----------------------------------------------
*/
PRIVATE BOOL
ixAtmdAccUtopiaConfigSetParamsValidate (const IxAtmdAccUtopiaConfig *utConfig)
{
    unsigned int numberOfPortsToConfigure = 0;
    BOOL isValid = TRUE;

    if (utConfig == NULL)
    {
        isValid = FALSE;
    }

    if (isValid)
    {
        /* txAddrRange  = (numberOfPorts - 1) */
        numberOfPortsToConfigure = utConfig->utTxConfig.txAddrRange + 1;

        /* Number of Rx ports must be equal the the number of Tx ports */
        if (numberOfPortsToConfigure != (utConfig->utRxConfig.rxAddrRange + (UINT32)1))
        {
            isValid = FALSE;
        }
    } /* end of if(isValid) */

    if (isValid)
    {
    /* Check the number of TxVc Queues is equal to the number of configured
    * ports
    */
        if (numTxVcQueues < numberOfPortsToConfigure)
        {
            isValid = FALSE;;
        }

        if (isValid)
        {
            /* Check that the number of ports is valid */
            if (numberOfPortsToConfigure > IX_UTOPIA_MAX_PORTS)
            {
                isValid = FALSE;
            }
        } /* end of if(isValid) */
    } /* end of if(isValid) */

    return isValid;
}

