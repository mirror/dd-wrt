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
 ******************************************************************************
 * @file icp_dcc.c
 *
 * @defgroup DCC Debug Client  API
 *
 * @ingroup DCCClient
 *
 * @description
 *      This file contains the types and functions relating to
 *      the interfaces provided for debug clients for 
 *      using information from Debug Core Component
 *
 *****************************************************************************/


/*****************************************************************************/


/*****************************************************************************/
/* Include Header Files */
#include "icp_dcc_al.h"

#include "dcc/icp_dcc.h"

#include "internal_dcc.h"

#include "IxOsal.h"



/* Version Information */

/**
 ******************************************************************************
 * @ingroup DCCClient
 *    Get the maximum buffer size to be allocated to retriev the Version info 
 *    of all the Package, Components registered with DCC 
 * 
 * @description
 *    This function provides DCC Client with buffer size required to retrieve 
 *    Version info of all the Package/Components registered with DCC. 
 *    DCC User should allocate this buffer and pass it to DCC to retrieve the
 *    required package/component version information
 *
 * @context
 *      This function runs in the context of the calling function thread.
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
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @param  pVersionBufferSize: IN/OUT: DCC package and component information
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_FAIL Operation Failed 
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters is null
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 *****************************************************************************/
icp_status_t
icp_DccVersionInfoSizeGet (
       uint32_t *pVersionBufferSize )
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

    if ( pVersionBufferSize == NULL )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_NULL_PARAM;
    }

    *pVersionBufferSize = DccVersionInfoSizeGet ();
    
    return ICP_STATUS_SUCCESS;
}

/**
 ******************************************************************************
 * @ingroup DCCClient
 *        Returns the package and registered components version information.
 * 
 * @description
 *         This function will provide the caller, the version information
 *         of the package and of each of the component registered with DCC.
 *
 * @context
 *      This function runs in the context of the calling function thread.
 *
 * @assumptions
 *      This function should be called with the required amount of buffer 
 *      allocated to contain the returned information.
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is synchronous and blocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @param  pVerInfo: IN/OUT: The array containing the version information 
 *             of the package and of all the components registered with DCC
 * @param  pVersionBufferSize: IN/OUT: 
 *             While calling this API, the user will specify 
 *             the amount of memory allocated which can be used by DCC to 
 *             populate with the version information of all the components
 *             registered with DCC.
 *             DCC will check if the buffer size meets the size of version
 *             information it has and if it does, then it will fill in the 
 *             components version information and pass it to the user.
 *             Ideally, the size should be same as the one DCC provided user
 *             in the "icp_DccVersionInfoSizeGet" function call.
 *             In the returned information, the first data will correspond to
 *             package information and the subsequent ones to individual 
 *             component versions.
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_FAIL Operation Failed 
 * @retval  ICP_STATUS_NULL_PARAM parameter is null
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 * @see icp_dcc_get_system_info
 *****************************************************************************/
icp_status_t
icp_DccSoftwareVersionGet (
             uint8_t *pVerInfo,
             uint32_t *pVersionBufferSize )
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

    if ( pVerInfo == NULL || pVersionBufferSize == NULL )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_NULL_PARAM;
    }

    return DccVersionGet ( pVerInfo, pVersionBufferSize );
}


/* System Liveness */

/* Management Interface */

/**
 ******************************************************************************
 * @ingroup DCCClient
 *        Set the Liveness timeout value for DCC 
 * 
 * @description
 *         This API is used to configure the timeout period (in ms) for the 
 *         liveness monitoring. DCC will wait for this period for  
 *         response and declare a thread to be live or dead. The timeout 
 *         period should large enough so that all threads can respond 
 *         appropriately within this period
 *
 * @context
 *      This function runs in the context of the calling function thread.
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
 * @param  livenessTimeOut: IN: The timeout value
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_FAIL Operation Failed 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 *****************************************************************************/
icp_status_t
icp_DccLivenessConfigureTimeout (
        uint32_t livenessTimeOut)
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

    DccLivenessTimeoutSet ( livenessTimeOut );

    return ICP_STATUS_SUCCESS;
}

/**
 ******************************************************************************
 * @ingroup DCCClient
 *        Get the buffer size to be allocated by DCC user for getting the 
 *        system liveness information
 * 
 * @description
 *         DCC user will use this API to get the buffer size to be allocated to
 *         retrieve the system liveness information.
 *         DCC user should free this buffer once it completes processing it.
 *
 * @context
 *      This function runs in the context of the calling function thread.
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
 * @param  pLivenessResponseSize: IN/OUT: The buffer size to be allocated 
 *                                 while quering the liveness status
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_FAIL Operation Failed 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 *****************************************************************************/
icp_status_t
icp_DccLivenessResponseSizeGet (
        uint32_t *pLivenessResponseSize)
{
    uint32_t  totalThreads = 0;

    if ( DccInitialized() != TRUE )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: DCC Not Initialized!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_FAIL;
    }

    if (pLivenessResponseSize == NULL )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: NULL Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_NULL_PARAM;
    }

    totalThreads = DccTotalThreadsGet ();

    *pLivenessResponseSize = totalThreads * sizeof(icp_dcc_liveness_status_t);

    return ICP_STATUS_SUCCESS;
}

/**
 ******************************************************************************
 * @ingroup DCCClient
 *        Verify System Liveness
 * 
 * @description
 *         DCC user will use this API to verify the liveness of all threads of 
 *         execution in the system. User will provide DCC the required buffer 
 *         to fill the Liveness status. 
 *
 * @context
 *      This function runs in the context of the calling function thread. This
 *      should not be called in interrupt mode
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
 * @param  pLivenessStatus: IN/OUT: The system threads execution status
 * @param  pBufferSize: IN/OUT: The buffer size allocated to return the 
 *                                  system liveness status
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_FAIL Operation Failed 
 * @retval  ICP_STATUS_INVALID_PARAM Invalid parameter passed 
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters is null
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 * @see icp_dcc_get_liveness_thread_info
 *
 *****************************************************************************/
icp_status_t
icp_DccLivenessVerify (
        uint8_t *pLivenessStatus,
        uint32_t *pBufferSize)
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

    if (pLivenessStatus == NULL || pBufferSize == NULL )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: NULL Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_NULL_PARAM;
    }

    return DccLivenessVerify(pLivenessStatus, pBufferSize);
}


/* DUMP */


/**
 ******************************************************************************
 * @ingroup DCCClient
 *        Get the number of modules and the max buffer size required to get 
 *        data dump
 * 
 * @description
 *        This function will return the number of modules and the maximum size
 *        of buffer needed for any one dump query transaction. Before invoking 
 *        the Data Dump query request, DCC user will get the max size of the
 *        buffer to allocate and the total number of modules which should
 *        be queried for the data dump.
 *
 * @context
 *      This function runs in the context of the calling function thread. This 
 *      function should not be called from interrupt context.
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
 *      No
 *
 * @threadSafe
 *      Yes
 *
 * @param  noOfModules: IN/OUT: Number of modules registered at DCC for Data 
 *                             Dump
 * @param  pDataDumpSize: IN/OUT: maximum buffer size required to query any one
 *                            dump module
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_FAIL Operation Failed 
 * @retval  ICP_STATUS_INVALID_PARAM Invalid parameter passed 
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters is null
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
icp_DccDataDumpInfoGet (
        uint32_t *noOfModules,
        uint32_t *pDataDumpSize)
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

    if ( noOfModules == NULL || pDataDumpSize == NULL )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_NULL_PARAM;
    }

    *noOfModules = DccTotalModulesGet ();
    *pDataDumpSize = DccMaxDataDumpSizeGet ();

    return ICP_STATUS_SUCCESS;
}

/**
 ******************************************************************************
 * @ingroup DCCClient
 *        Get the number of modules and the max buffer size required to get 
 *        data dump
 * 
 * @description
 *        User should use this function to dump the data structures registered 
 *        with the DCC. This function will be called once for each registered 
 *        module for the data dump information. With each call, User will pass 
 *        DCC a pre-allocated buffer which will be passed to the module to be 
 *        filled up with the dump information.
 *
 * @context
 *      This function runs in the context of the calling function thread.
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
 *      No
 *
 * @threadSafe
 *      Yes
 *
 * @param  moduleId: IN: The opaque index representing the module being 
 *                          queried for data dump
 * @param  pDataDump: IN/OUT: Data Dump information from the module
 * @param  pDataDumpSize: IN/OUT: The actual size of dump data present in the 
 *                           buffer.
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_FAIL Operation Failed 
 * @retval  ICP_STATUS_INVALID_PARAM Invalid parameter passed 
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters is null
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 * @see icp_dcc_get_dump_info
 *
 *****************************************************************************/
icp_status_t
icp_DccDataDumpGet (
        uint32_t moduleId, 
        uint32_t *pDataDumpSize,
        uint8_t *pDataDump)
{
    icp_status_t            err = ICP_STATUS_SUCCESS;
    uint32_t                dumpSize = 0;
    icp_DccDataDumpHandler  pDataDumpHandler;
    const void              *pUsrPrivData = NULL;

    if ( DccInitialized() != TRUE )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: DCC Not Initialized!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_FAIL;
    }

    /* check for null parameters */
    if ( pDataDumpSize == NULL || pDataDump == NULL )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_NULL_PARAM;
    }

    /* Check for invalid moduleId parameter (this is the index) */
    if ( moduleId == 0 || moduleId > DccTotalModulesGet () )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Invalid value for Parameter - moduleId!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_INVALID_PARAM;
    }

    /* Get dumpHandler / dumpSize */
    err = DccDataDumpHandlerGet ( moduleId, &dumpSize, &pDataDumpHandler,
                                  &pUsrPrivData);

    if ( err != ICP_STATUS_SUCCESS)
    {
        return err;
    }

    /* Ensure pDataDumpSize is greater than or equal to the dump size 
     * as registered for the particular moduleId
     */
    if ( *pDataDumpSize < dumpSize )
    {
        return ICP_STATUS_FAIL;
    }

    /* If no handler registered, return error since no dump will be 
       available */
    if ( pDataDumpHandler == NULL )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: No Dump Handler Registered!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        *pDataDumpSize = 0;
        return ICP_STATUS_SUCCESS;
    }

    /* Invoke the dump handler to get the required data dump */
    err = pDataDumpHandler ( (void *)pDataDump, pDataDumpSize, pUsrPrivData ); 
    if ( err != ICP_STATUS_SUCCESS )
    {
        return err;
    }

    return ICP_STATUS_SUCCESS;
}


/* Client Interface */

/**
 ******************************************************************************
 * @ingroup DCCClient
 *        Register the SEN event callback handler
 * 
 * @description
 *        DCC user will register the SEN event callback handler of this 
 *        prototype.
 *
 * @context
 *      This function runs in the context of the calling function thread.
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
 *      No
 *
 * @threadSafe
 *      Yes
 *
 * @param  senHandler IN: SEN Handler callback to be registered
 *        
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_FAIL Operation Failed 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 * @see icp_DccSenHandler
 *
 *****************************************************************************/
icp_status_t
icp_DccSenHandlerRegister ( icp_DccSenHandler senHandler )
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

    if ( senHandler == NULL )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_NULL_PARAM;
    }

    return DccSenHandlerSet ( senHandler );
}


/**
 ******************************************************************************
 * @ingroup DCCClient
 *        Unregister the SEN event callback handler
 * 
 * @description
 *        DCC user will unregister the SEN event callback handler with this 
 *        prototype.
 *
 * @context
 *      This function runs in the context of the calling function thread.
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
 *      No
 *
 * @threadSafe
 *      Yes
 *
 * @param  
 *        
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_FAIL Operation Failed 
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
icp_DccSenHandlerUnregister ( void )
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

    return DccSenHandlerSet ( NULL );
}


/*****************************************************************************
 * end of file icp_dcc.c 
 *****************************************************************************/
