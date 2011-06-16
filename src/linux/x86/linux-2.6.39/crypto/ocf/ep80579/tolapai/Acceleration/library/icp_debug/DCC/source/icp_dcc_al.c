/******************************************************************************
 * 
 * @par
 * This file is provided under a dual BSD/GPLv2 license.  When using or 
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program; if not, write to the Free Software 
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution 
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE 
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: Security.L.1.0.3-98
 *
 *****************************************************************************/

/**
 *****************************************************************************
 * @file icp_dcc_al.c
 *
 * @defgroup DCC Acceleration Software  API
 *
 * @ingroup DCCAL
 *
 * @description
 *      This file contains the types and functions relating to
 *      the interfaces provided for acceleration software units for 
 *      registering various debug information with the Debug Core Component
 *
 *****************************************************************************/


/*****************************************************************************/

/*****************************************************************************/
/* Include Header Files */
#include "cpa.h"
#include "icp_dcc_al.h"
#include "dcc/icp_dcc.h"
#include "internal_dcc.h"
#include "icp_dcc_cfg.h"

#include "IxOsal.h"

#define ZERO_MOD_COUNT  (0)
#define SHUTDOWN_TRESHOLD  (2)

extern IxOsalSpinLock dccInitSpinLock;
static UINT32  initDccModulesCounter = 0;


/* DCC Library Initialize & Shutdown Functions */

/**
 *****************************************************************************
 * @ingroup DCCAL
 *      Initialize DCC Library
 * 
 * @description
 *      This function will initialize the DCC Library. It will create the 
 *      required tables and setup the default debug functionality handlers
 *
 * @context
 *      This function may sleep and therefore may not be called from interrupt
 *      context.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is synchronous and blocking.
 *      
 * @reentrant
 *      No
 *
 * @threadSafe
 *      No
 *
 * @param 
 *     
 *     
 * @retval  CPA_STATUS_SUCCESS If successful
 * @retval  CPA_STATUS_RESOURCE Error related to system resource
 * @retval  CPA_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 *****************************************************************************/
CpaStatus
icp_AsdCfgDccInit (
        CpaInstanceHandle instanceHandle,
        icp_asd_cfg_param_get_cb_func_t getCfgParamFunc
    )
{
IX_STATUS osal_err = IX_SUCCESS;
CpaStatus result = CPA_STATUS_SUCCESS;

/* Workaround until ASD parameters are available and usable */
instanceHandle = instanceHandle;
getCfgParamFunc = getCfgParamFunc;

osal_err = ixOsalSpinLockLock( &dccInitSpinLock);
if (IX_SUCCESS != osal_err)
    {
    ixOsalLog(IX_OSAL_LOG_LVL_ERROR,\
        IX_OSAL_LOG_DEV_STDERR,\
        "DccInit: DccSpinLockLock Failed\n",\
        0,0,0,0,0,0);
    result = CPA_STATUS_RESOURCE;
    }
else
    {
    if ( DccInitialized() == TRUE )
        {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: DCC Already Initialized!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        initDccModulesCounter++;
        result = CPA_STATUS_SUCCESS;
        }
    else
        {
        /* Initialize all internal data structures maintained by DCC */
        result = DccInit ();
        if (CPA_STATUS_SUCCESS == result)
            {
            initDccModulesCounter++;
            }
        else
            {
            ixOsalLog(IX_OSAL_LOG_LVL_ERROR,\
                IX_OSAL_LOG_DEV_STDERR,\
                "DccInit Failed\n",\
                0,0,0,0,0,0);
            }
        }
    }
osal_err = ixOsalSpinLockUnlock( &dccInitSpinLock);
if (IX_SUCCESS != osal_err)
    {
    ixOsalLog(IX_OSAL_LOG_LVL_ERROR,\
        IX_OSAL_LOG_DEV_STDERR,\
        "DccInit: DccSpinLockUnlock Failed\n",\
        0,0,0,0,0,0);
    result = CPA_STATUS_RESOURCE;
    }
return result;
}


/**
 *****************************************************************************
 * @ingroup DCCAL
 *      Shutdown DCC Library
 * 
 * @description
 *      This function will shutdown the DCC Library. It will delete all the 
 *      created tables and free the allocated memory
 *
 * @context
 *      This function may sleep and therefore may not be called from interrupt
 *      context.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is synchronous and blocking.
 *      
 * @reentrant
 *      No
 *
 * @threadSafe
 *      No
 *
 * @param  
 *  
 *     
 * @retval  CPA_STATUS_SUCCESS If successful
 * @retval  CPA_STATUS_RESOURCE Error related to system resource
 * @retval  CPA_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      DCC should be initialized before calling this function
 *
 * @post
 *      None.
 *
 *****************************************************************************/
CpaStatus
icp_AsdCfgDccShutdown ( CpaInstanceHandle instanceHandle )
{
IX_STATUS osal_err = IX_SUCCESS;
CpaStatus result = CPA_STATUS_SUCCESS;

/* Workaround until ASD parameters are available and usable */
instanceHandle = instanceHandle;

osal_err = ixOsalSpinLockLock( &dccInitSpinLock);
if (IX_SUCCESS != osal_err)
    {
    ixOsalLog(IX_OSAL_LOG_LVL_ERROR,\
        IX_OSAL_LOG_DEV_STDERR,\
        "DccShutdown: DccSpinLockLock Failed\n",\
        0,0,0,0,0,0);
    result = CPA_STATUS_RESOURCE;
    }
else
    {
    if ( DccInitialized() != TRUE )
        {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: DCC Not Initialized!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        if (ZERO_MOD_COUNT < initDccModulesCounter)
            {
            ixOsalLog(IX_OSAL_LOG_LVL_ERROR,\
                IX_OSAL_LOG_DEV_STDERR,\
                "initDccModCounter is non 0 but DccInitialized() == FALSE\n",\
                0,0,0,0,0,0);
            }
        initDccModulesCounter = 0;
        result = CPA_STATUS_SUCCESS;
        }
    else
        {
        if (SHUTDOWN_TRESHOLD > initDccModulesCounter)
            {
            /* Cleanup all internal data structures maintained 
             * by DCC and shutdown */
            result = DccShutdown();
            if (CPA_STATUS_SUCCESS == result)
                {
                initDccModulesCounter--;
                if (0 < initDccModulesCounter)
                    {
                    ixOsalLog(IX_OSAL_LOG_LVL_ERROR,\
                        IX_OSAL_LOG_DEV_STDERR,\
                        "initDccModCounter is non 0 after DccShutdown()\n",\
                        0,0,0,0,0,0);
                    }
                initDccModulesCounter = 0;
                }
            else
                {
                ixOsalLog(IX_OSAL_LOG_LVL_ERROR,\
                    IX_OSAL_LOG_DEV_STDERR,\
                    "DccShutdown Failed\n",\
                    0,0,0,0,0,0);
                }
            }
        else
            {
            initDccModulesCounter--;
            }
        }
    }
osal_err = ixOsalSpinLockUnlock( &dccInitSpinLock);
if (IX_SUCCESS != osal_err)
    {
    ixOsalLog(IX_OSAL_LOG_LVL_ERROR,\
        IX_OSAL_LOG_DEV_STDERR,\
        "DccShutdown: DccSpinLockUnlock Failed\n",\
        0,0,0,0,0,0);
    result = CPA_STATUS_RESOURCE;
    }
return result;
}

/**
 *****************************************************************************
 * @ingroup icp_Dcc
 *      Starts the DCC library.
 * 
 *      This function is a place holder.
 *
 * @context
 *      Any context
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is nonblocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @retval  CPA_STATUS_SUCCESS        No error 
 *
 * @pre
 *      None.
 *
 *****************************************************************************/
CpaStatus 
icp_AsdCfgDccStart ( CpaInstanceHandle instanceHandle )
{
    /* Workaround until ASD parameters are available and usable */
    instanceHandle = instanceHandle;

    if ( DccInitialized() != TRUE )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: DCC Not Initialized!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return CPA_STATUS_FAIL;
    }
    return CPA_STATUS_SUCCESS;
}

/**
 *****************************************************************************
 * @ingroup icp_Dcc
 *      Stops the DCC library.
 * 
 *      This function is a place holder.
 *
 * @context
 *      Any context
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is nonblocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @retval  CPA_STATUS_SUCCESS        No error 
 *
 * @pre
 *      None.
 *
 * @post
 *      None.
 *
 *****************************************************************************/
CpaStatus 
icp_AsdCfgDccStop( CpaInstanceHandle instanceHandle )
{
    /* Workaround until ASD parameters are available and usable */
    instanceHandle = instanceHandle;

    if ( DccInitialized() != TRUE )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: DCC Not Initialized!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return CPA_STATUS_FAIL;
    }
    return CPA_STATUS_SUCCESS;
}



/*===========================================================================*/

/* Product & Component Version */

/* Acceleration Software Interface */

/**
 *****************************************************************************
 * @ingroup DCCAL
 *      Register the Software Version Information with DCC
 * 
 * @description
 *      This function is used to register the Software version information 
 *      with DCC.
 *
 * @context
 *      This function does not sleep and therefore may be called from 
 *      interrupt context.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is non-blocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @param  pSoftwareVersion: IN: Pointer to storage containing the software 
 *                               version information to be registered with DCC.
 * @param  pModuleId: OUT: Module ID returned by the DCC for the 
 *                         package/component registering its version info   
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters is null
 * @retval  ICP_STATUS_RESOURCE Error related to system resource
 * @retval  ICP_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 *****************************************************************************/
icp_status_t
icp_DccComponentVersionRegister (
        icp_dcc_ver_info_t *pSoftwareVersion, 
        uint32_t *pModuleId 
    )
{
    if ( DccInitialized() != TRUE )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: DCC Not Initialized!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_FAIL;
    }

    /* Validate input parameters */
    if ( pSoftwareVersion == NULL || pModuleId == NULL )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_NULL_PARAM;

    }

    return DccComponentVersionSet ( pSoftwareVersion, pModuleId );
}


/*===========================================================================*/

/* Liveness */

/**
 *****************************************************************************
 * @ingroup DCCAL
 *      Register Acceleration Software Thread Liveness 
 *      Verification Memory Location
 * 
 * @description
 *      This API will be used by the Acceleration Software components to 
 *      register the memory location location of the threads with DCC. 
 *      For ASU, corresponding access layer components will allocate memory 
 *      and patch it to the PPS and then will register the location with DCC. 
 *
 * @context
 *      This function may sleep and therefore may not be called from interrupt
 *      context.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is blocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @param  pThreadId: IN: The thread id whose liveness is being verified
 * @param  pLivenessLocation: IN: The thread id Liveness Update location
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_INVALID_PARAM Invalid parameter passed 
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters is null
 * @retval  ICP_STATUS_RESOURCE Error related to system resource
 * @retval  ICP_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 * @see 
 *
 *****************************************************************************/
icp_status_t
icp_DccLivenessLocationRegister (
        icp_dcc_thread_id_t *pThreadId,
        uint32_t *pLivenessLocation
    )
{
    if ( DccInitialized() != TRUE )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: DCC Not Initialized!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_FAIL;
    }

    /* Validate input parameters */
    if ( pLivenessLocation == NULL || pThreadId == NULL )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_NULL_PARAM;
    }

    /* Check if there is any dump callback handler already registered */
    return DccLivenessLocationSet (pThreadId, pLivenessLocation);
}


/**
 *****************************************************************************
 * @ingroup DCCAL
 *        Un-Register Acceleration Software Layer & ASU Thread Liveness 
 *        Verification Memory Location
 * 
 * @description
 *        This API will be used by the access layer components to un-register 
 *        the memory location of each thread of execution with the DCC.
 *
 * @context
 *      This function may sleep and therefore may not be called from interrupt
 *      context.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is blocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @param  pThreadId: IN: The thread id whose liveness location is to be 
 *                        unregistered  
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_INVALID_PARAM Invalid parameter passed 
 * @retval  ICP_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 * @see 
 *
 *****************************************************************************/
icp_status_t
icp_DccLivenessLocationUnregister (
        icp_dcc_thread_id_t *pThreadId
    )
{
    if ( DccInitialized() != TRUE )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: DCC Not Initialized!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_FAIL;
    }

    /* Validate input parameters */
    if ( pThreadId == NULL )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_NULL_PARAM;
    }


    /* Unregister liveness location by setting it to NULL */
    return DccLivenessLocationSet (pThreadId, NULL);
}


/**
 *****************************************************************************
 * @ingroup DCCAL
 *      Register Thread Liveness Verification Handler
 * 
 * @description
 *      This API will be used by the Acceleration Software components to 
 *      register the livenes verification handler firmware.
 *
 * @context
 *      This function may sleep and therefore may not be called from interrupt
 *      context.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is blocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @param  pThreadId: IN: The thread id whose liveness is being verified
 * @param  pLivenessVerificationHandler: IN: callback of thread to verify 
 *                liveness
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_INVALID_PARAM Invalid parameter passed 
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters is null
 * @retval  ICP_STATUS_RESOURCE Error related to system resource
 * @retval  ICP_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 * @see icp_DccLivenessVerify @ icp_dcc.h  
 *
 *****************************************************************************/
icp_status_t
icp_DccLivenessVerificationHandlerRegister (
        icp_dcc_thread_id_t *pThreadId,
        icp_DccLivenessVerificationHandler livenessVerificationHandler,
        const void *pUsrPrivData
    )
{
    if ( DccInitialized() != TRUE )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: DCC Not Initialized!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_FAIL;
    }

    /* Validate input parameters */
    if ( pThreadId == NULL || livenessVerificationHandler == NULL )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_NULL_PARAM;
    }

    /* Register verification handler with DCC */
    return DccLivenessVerifierSet (pThreadId, 
                                   livenessVerificationHandler,
                                   pUsrPrivData);
}


/**
 ******************************************************************************
 * @ingroup DCCAL
 *      Un-Register Thread Liveness Verification Handler 
 * 
 * @description
 *      This API will be used by the Acceleration Software components to 
 *      un-register liveness verification handler for firmware.
 *
 * @context
 *      This function may sleep and therefore may not be called from interrupt
 *      context.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is blocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @param  pThreadId: IN: The thread id whose liveness callback is unregistered
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_INVALID_PARAM Invalid parameter passed 
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters is null
 * @retval  ICP_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 * @see icp_DccLivenessVerify @ icp_dcc.h
 *
 *****************************************************************************/
icp_status_t
icp_DccLivenessVerificationHandlerUnregister (
        icp_dcc_thread_id_t *pThreadId
    )
{
    if ( DccInitialized() != TRUE )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: DCC Not Initialized!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_FAIL;
    }

    /* Validate input parameters --> Nothing to be done */
    if ( pThreadId == NULL )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_NULL_PARAM;
    }


    /* Unregister liveness verification handler by setting it to NULL */
    return DccLivenessVerifierSet (pThreadId, NULL, NULL);
}


/*===========================================================================*/

/* DUMP */

/**
 *****************************************************************************
 * @ingroup DCCAL
 *        Register Acceleration Software Module Data dump handler with DCC
 * 
 * @description
 *        Acceleration Software Modules register their data dump handler with
 *        DCC using this function.
 *
 * @context
 *      This function may sleep and therefore may not be called from interrupt
 *      context.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is blocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @param  moduleId: IN: Module ID of the access layer module registering the
 *                       dump handler.
 * @param  pDataDumpSize: IN: The actual size of dump data present in the 
 *                       buffer.
 * @param  dataDumpHandler: IN: Data dump handler
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_INVALID_PARAM Invalid parameter passed 
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters is null
 * @retval  ICP_STATUS_RESOURCE Error related to system resource
 * @retval  ICP_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 * @see  icp_DccDataDumpHandler 
 *
 *****************************************************************************/
icp_status_t
icp_DccDataDumpHandlerRegister (
        uint32_t moduleId,
        uint32_t dataDumpSize,
        icp_DccDataDumpHandler dataDumpHandler, 
        const void *pUsrPrivData
    )
{
    if ( DccInitialized() != TRUE )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: DCC Not Initialized!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_FAIL;
    }

    /* Validate input parameters */
    if ( moduleId > DccTotalModulesGet() )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Invalid value for Parameter - moduleId!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_INVALID_PARAM;
    }

    if ( dataDumpHandler == NULL )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_NULL_PARAM;
    }

    /* Check if there is any dump callback handler already registered */
    return DccDataDumpHandlerSet (
            moduleId, dataDumpSize, dataDumpHandler, pUsrPrivData);
}


/**
 *****************************************************************************
 * @ingroup DCCAL
 *        Un-Register the Data dump handler registered with DCC
 * 
 * @description
 *        Acceleration Software Modules un-register the registered data dump 
 *        handler from DCC using this function.
 *
 *
 * @context
 *      This function may sleep and therefore may not be called from interrupt
 *      context.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is blocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @param  moduleId: IN: Module ID of the acceleration software module 
 *                       unregistering the data dump handler.
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_INVALID_PARAM Invalid parameter passed 
 * @retval  ICP_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 *****************************************************************************/
icp_status_t
icp_DccDataDumpHandlerUnregister (
        uint32_t moduleId
    )
{
    if ( DccInitialized() != TRUE )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: DCC Not Initialized!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_FAIL;
    }

    /* Validate input parameters */
    if ( moduleId > DccTotalModulesGet() )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Invalid value for Parameter - moduleId!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_INVALID_PARAM;
    }

    /* Check if there is any dump callback handler already registered */
    return DccDataDumpHandlerSet (moduleId, 0, NULL, NULL);
}


/*===========================================================================*/

/* SEN */

/**
 *****************************************************************************
 * @ingroup DCCAL
 *      Notify DCC of Software Error Notification (SEN) event
 * 
 * @description
 *      This API is used by the Acceleration Software to notify any Software 
 *      Error Notification (referred to as SEN hereafter) messages to DCC. 
 *      This function will call the SEN event handler. If no SEN event handler
 *      is registered, then only Logger API is called.
 *
 * @context
 *      This function runs in the context of the calling function. This can be
 *      called from interrupt context also.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is blocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @param  pSenMsg: IN: Details of the occured Software Error 
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters is null
 * @retval  ICP_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 *****************************************************************************/
icp_status_t
icp_DccSenNotify (
        icp_dcc_sen_msg_t *pSenMsg
    )
{
    uint32_t            i = 0;
    icp_DccSenHandler   pSenHandler = NULL;
    
    if ( DccInitialized() != TRUE )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: DCC Not Initialized!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_FAIL;
    }


    /* Validate input parameters */
    if ( pSenMsg == NULL )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_NULL_PARAM;
    }

    /* This will fail only if LOCK APIs fail which is unlikely. 
     * Hence, the error check is within DEBUG flag
     */
    if ( DccSenHandlerGet ( &pSenHandler ) != ICP_STATUS_SUCCESS )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: DccSenHandlerGet Failed!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_FAIL;
    }
    
    if ( pSenHandler != NULL )
    {
        /* senHandler is function pointer returning void. */
        pSenHandler ( pSenMsg );
    }

#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_DEBUG3, IX_OSAL_LOG_DEV_STDOUT,
                "Function: %s: No SEN Handler registered!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        /* Use Standards OSAL Logging API to dump SEN Msg */
        ixOsalStdLog ("\nDCC SEN:");
        ixOsalStdLog ("%lld;", pSenMsg->timestamp);
        ixOsalStdLog ("%d;", pSenMsg->senPriority);
        ixOsalStdLog ("%d;", pSenMsg->moduleId);
        ixOsalStdLog ("%d;", pSenMsg->eventId);
#ifdef ICP_DCC_DEBUG
        ixOsalStdLog ("\n\tEventInfoSize: %d", pSenMsg->eventInfoSize);
#endif

        if ( pSenMsg->eventInfoSize > ICP_DCC_MAX_MSG_SIZE )
        {
#ifdef ICP_DCC_DEBUG
            ixOsalLog ( IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
                    "Message too long. May be truncated\n",
                    0, 0, 0, 0, 0, 0);
#endif
        }

        for ( i = 0; i < pSenMsg->eventInfoSize; i++ )
        {
            if( i >= ICP_DCC_MAX_MSG_SIZE )
            {
                break;
            }
            ixOsalStdLog ("%c", pSenMsg->eventInfo[i]);
        }
        ixOsalStdLog ("\n");

    return ICP_STATUS_SUCCESS;
}

/*****************************************************************************
 * end of icp_dcc_al.c
 *****************************************************************************/
