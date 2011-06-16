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
 * @file internal_dcc.c
 *
 * @defgroup DCC Internal Types and Functions
 *
 * @ingroup DCCInternal
 *
 * @description
 *      This file contains the types and functions relating to
 *      the internal data structures maintained by DCC 
 *
 *****************************************************************************/


/*****************************************************************************/


/*****************************************************************************/
/* Include Header Files */
#include "cpa.h"
#include "dcc/icp_dcc.h"
#include "icp_dcc_al.h"

#include "IxOsal.h"

#include "internal_dcc.h"


/*****************************************************************************/

/* Data maintained by DCC */

/* Maintain Initialization status of DCC */
STATIC uint8_t          dccInitialized = FALSE;

/* To keep count of total modules registered with DCC at any point of time */
STATIC uint32_t         totalModulesRegistered = 0;

/* To keep count of total threads registered with DCC at any point of time */
STATIC uint32_t         totalThreadsRegistered = 0;

STATIC ICP_DCC_LOCK_T(dccLock); 

STATIC dcc_module_info_t    *dccModuleInfo[ICP_DCC_MAX_MODULES] = { 0 }; 

STATIC dcc_thread_info_t    *dccThreadInfo = NULL;

STATIC icp_DccSenHandler    dccSenHandler = NULL;

/* DCC maintains the timeout provided by Debug clients */
STATIC uint32_t             dccLivenessTimeout = 0;

/* DCC maintains the maximum dataDump size required to dump data */
STATIC uint32_t             dccMaxDataDumpSize = 0;

/* DCC maintains total version info size which is required when 
   VersionInfoSizeGet is invoked client. For every component verison that is 
   registered, the moduleId length (stringified format) as well as version 
   size (stringified), component name length.
 */
STATIC uint32_t             dccVersionInfoSize = 0;

/* spinlock to synchronize access to counter for number of modules using 
 * DCC init
 */
IxOsalSpinLock dccInitSpinLock; 

icp_status_t icp_DccInitilizeModule ( void );
icp_status_t icp_DccFinilizeModule ( void );

/*****************************************************************************/

/**
 * function to initialize DCC
 */
CpaStatus
DccInit ( void )
{
    size_t    pkgNameLen = 0;

    ICP_DCC_LOCK_INIT(dccLock);

    /* Set List pointer to NULL, set all variables maintained by DCC to
       their defaults */
    dccThreadInfo = NULL;

    totalModulesRegistered = 0;
    totalModulesRegistered = 0;

    dccSenHandler = NULL;

    dccLivenessTimeout = 0;
    dccMaxDataDumpSize = 0;
    dccVersionInfoSize = 0;

    /* Register Product Version. To be decided on how to get package info */
    /* Allocate space for new component's data */

    dccModuleInfo[0] = (dcc_module_info_t *) 
                                ixOsalMemAlloc ( sizeof(dcc_module_info_t) );

    ixOsalMemSet ( dccModuleInfo[0], 
                    0, sizeof(dcc_module_info_t));

    /* Set version info in moduleInfo structure */
    pkgNameLen = strlen (PACKAGE_NAME);
    /* Assumption is that the user provides pkg Name and Version string
       properly 
     */
    ixOsalMemCopy ( 
            (void *)dccModuleInfo[0]->moduleName,
            (void *)PACKAGE_NAME,
            pkgNameLen );

    /* Convert the version info available into string format */
    ICP_DCC_CREATE_PKG_VER_STRING(
            dccModuleInfo[0]->moduleVersion,
            PACKAGE_VERSION_MAJOR_NUMBER,
            PACKAGE_VERSION_MINOR_NUMBER,
            PACKAGE_VERSION_PATCH_NUMBER,
            PACKAGE_VERSION_BUILD_NUMBER);

    /* Package will be the first one to register. Set dumpHandler/Size to 0 */
    dccModuleInfo[0]->dataDumpSize = 0;
    dccModuleInfo[0]->dataDumpHandler = NULL;
    dccModuleInfo[0]->pPrivData = NULL;

    /* Update VersionInfoSize with size of package version info */
    dccVersionInfoSize = sizeof(icp_dcc_package_ver_t);

    dccInitialized = TRUE;

    return CPA_STATUS_SUCCESS;
}

/**
 * function to shutdown DCC
 */
CpaStatus
DccShutdown ( void )
{
    uint32_t i = 0;
    dcc_thread_info_t *pThreadInfo = dccThreadInfo;

    /* free of all memory allocated to store moduleInfo */
    for ( i = 0; i <= totalModulesRegistered; i++ )
    {
        if ( dccModuleInfo[i] != NULL )
        {
            ixOsalMemFree ( (void *)dccModuleInfo[i] );
            dccModuleInfo[i] = NULL;
        }
    }

    if ( pThreadInfo != NULL )
    {
        /* Free all memory allocated to hold threadInfo */
        while ( pThreadInfo != NULL )
        {
            pThreadInfo->pPrev = NULL;
            dccThreadInfo = pThreadInfo->pNext;

            pThreadInfo->pNext = pThreadInfo->pPrev = NULL;
            ixOsalMemFree ( (void *)pThreadInfo );
        
            pThreadInfo = dccThreadInfo;
        }
    }

    /* Set SEN Handler to NULL */
    dccSenHandler = NULL;

    /* Set all information maintained by DCC to defaults */
    totalModulesRegistered = 0;
    totalThreadsRegistered = 0;

    dccLivenessTimeout = 0;
    dccMaxDataDumpSize = 0;
    dccVersionInfoSize = 0;

    /* Set Flag to False */
    dccInitialized = FALSE;

    ICP_DCC_LOCK_FINI(dccLock);

    return CPA_STATUS_SUCCESS;
}


/******************************************************************************
 * 
 * Version Information 
 *
 *****************************************************************************/

/**
 * Function to Get Component version
 */
icp_status_t
DccVersionGet (uint8_t *pVerInfo, uint32_t *pBufferSize)
{
    uint32_t    moduleId = ICP_DCC_ONE; /* 0 reserved for product ver */
    uint32_t    index = 0;
    uint32_t    offset = 0;
    uint8_t     *pModVersion = NULL;
    uint8_t     *pModName = NULL;


    /* Use struct type pointer to update the buffer with package version 
       info */ 
    icp_dcc_version_buffer_t *pVersionBuffer = 
                                    (icp_dcc_version_buffer_t *) pVerInfo;

    if ( pVersionBuffer == NULL )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_NULL_PARAM;
    }

    ICP_DCC_LOCK(dccLock);

    pModName = dccModuleInfo[0]->moduleName;

    pModVersion = dccModuleInfo[0]->moduleVersion;

    ICP_DCC_UNLOCK(dccLock);

    /* update the number of modules registered with DCC details */
    pVersionBuffer->numModules = DccTotalModulesGet ();

    offset = sizeof(uint32_t);   /* accounting for numModules member 
                                     in VersionBuffer structure */

    /* Update the buffer with package version information */
    /* Package version length */
    pVersionBuffer->packageVerInfo.packageSize = sizeof(icp_dcc_package_ver_t);

    /* Copy package name into buffer */
    ixOsalMemCopy ( (void *)pVersionBuffer->packageVerInfo.packageName,
                                (void *)pModName,
                                ICP_DCC_COMPONENT_NAME_LENGTH);

    /* Copy version into buffer */
    ixOsalMemCopy ( (void *)pVersionBuffer->packageVerInfo.packageVersion,
                              (void *)pModVersion,
                              ICP_DCC_VERSION_STR_LENGTH );

    /* Update Valid size in buffer */
    offset += sizeof(icp_dcc_package_ver_t);

    /* Fill buffer with version of each component registered */
    for ( index = 0; moduleId <= DccTotalModulesGet (); moduleId++ )
    {
        /* Package version length */
        pVersionBuffer->moduleVerInfo[index].componentSize = 
            sizeof(icp_dcc_module_ver_t);

        /* Copy moduleID String into buffer */
        pVersionBuffer->moduleVerInfo[index].moduleId = moduleId;

        /* Copy component name into buffer */
        ICP_DCC_LOCK(dccLock);
        pModName = dccModuleInfo[moduleId]->moduleName;
        pModVersion = dccModuleInfo[moduleId]->moduleVersion;
        ICP_DCC_UNLOCK(dccLock);

        ixOsalMemCopy ( 
                (void *)pVersionBuffer->moduleVerInfo[index].moduleName,
                (void *)pModName,
                ICP_DCC_COMPONENT_NAME_LENGTH );

        /* Copy version into buffer */
        ixOsalMemCopy ( 
                (void *)pVersionBuffer->moduleVerInfo[index].moduleVersion,
                (void *)pModVersion,
                ICP_DCC_VERSION_STR_LENGTH );

    
        offset += sizeof(icp_dcc_module_ver_t);
        index++;
    }

    /* Update bufferSize with lenght of valid data in the buffer */
    *pBufferSize = offset; 

    return ICP_STATUS_SUCCESS;
}

/**
 * Function to Set Component Version
 */
icp_status_t
DccComponentVersionSet (icp_dcc_ver_info_t *pVersionInfo, uint32_t *pModuleId)
{
    dcc_module_info_t   *pNewModule = NULL;

    if ( DccTotalModulesGet () == ICP_DCC_MAX_MODULES )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Exceeded MAX_MODULES. "\
                "Cannot register any new module!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_RESOURCE;
    }


    /* Allocate space for new component's data */

    pNewModule = (dcc_module_info_t *) 
                                ixOsalMemAlloc ( sizeof(dcc_module_info_t) );

    ixOsalMemSet ( pNewModule, 0, sizeof(dcc_module_info_t));


    /* Set version info in moduleInfo structure */
    ixOsalMemCopy ( 
            (void *)pNewModule->moduleName,
            (void *)pVersionInfo->name,
            ICP_DCC_COMPONENT_NAME_LENGTH );

    /* Convert the version info available into string format */
    ICP_DCC_CREATE_VER_STRING(
            pNewModule->moduleVersion,
            pVersionInfo->majorVersion,
            pVersionInfo->minorVersion,
            pVersionInfo->patchVersion);

    /* Update global with the lengths from this module */
    dccVersionInfoSize += sizeof(icp_dcc_module_ver_t);

    ICP_DCC_LOCK(dccLock);

    ++totalModulesRegistered;
    dccModuleInfo[totalModulesRegistered] = pNewModule; 
    *pModuleId = totalModulesRegistered;

    ICP_DCC_UNLOCK(dccLock);

    return ICP_STATUS_SUCCESS;
}


/******************************************************************************
 * 
 * System Liveness
 *
 *****************************************************************************/

/**
 * function to Set Liveness location 
 */
icp_status_t
DccThreadInfoLivenessLocationGet (
        const dcc_thread_info_t *pThreadInfo, uint32_t **pLivenessLocation )
{
#ifdef ICP_DCC_DEBUG
    if ( pLivenessLocation == NULL || pThreadInfo == NULL )
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
        return ICP_STATUS_NULL_PARAM;
    }
#endif

    ICP_DCC_LOCK(dccLock);
    *pLivenessLocation = pThreadInfo->pLivenessLocation;
    ICP_DCC_UNLOCK(dccLock);

    return ICP_STATUS_SUCCESS;
}

/**
 * function to Set Liveness location 
 */
icp_status_t
DccLivenessLocationSet (
        icp_dcc_thread_id_t *pThreadId, uint32_t *pLivenessLocation )
{
    dcc_thread_info_t   *pThreadInfo = (dcc_thread_info_t *){ 0 };

#ifdef ICP_DCC_DEBUG
    if ( pThreadId == NULL )
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
        return ICP_STATUS_NULL_PARAM;
    }
#endif

    DccFindThreadInfo (pThreadId, &pThreadInfo);

    /* If this is an unregister call */
    if ( pLivenessLocation == NULL )
    {
        ixOsalLog (IX_OSAL_LOG_LVL_DEBUG3, IX_OSAL_LOG_DEV_STDOUT,
                "Function: %s: Unregister call\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
        /* Nothing more to be done. Return with status */
        return DccRemoveThreadInfo ( pThreadInfo );
    }

    ixOsalLog (IX_OSAL_LOG_LVL_DEBUG3, IX_OSAL_LOG_DEV_STDOUT,
                "Function: %s: Register call\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);

    /* If no info found and this is not an unregister call */
    if ( pThreadInfo == NULL )
    {
        pThreadInfo = (dcc_thread_info_t *){ 0 };
        if ( (DccAddThreadInfo (pThreadId, &pThreadInfo)) 
                != ICP_STATUS_SUCCESS )
        {
#ifdef ICP_DCC_DEBUG
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                    "Function: %s: DccAddThreadInfo Failed!\n",
                    (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
            return ICP_STATUS_FAIL;
        }
    }

    ICP_DCC_LOCK(dccLock);

    /* Register the liveness location for the module */
    pThreadInfo->pLivenessLocation =  (uint32_t *)pLivenessLocation;

    /* Set the callback handler field to zero since both cannot 
       exist simultaneously */
    pThreadInfo->livenessVerifier = NULL;
    pThreadInfo->pPrivData = NULL;

    ICP_DCC_UNLOCK(dccLock);

    return ICP_STATUS_SUCCESS;
}

/**
 * function to Get liveness verification handler 
 */
icp_status_t
DccThreadInfoLivenessVerifierGet (
        const dcc_thread_info_t *pThreadInfo,
        icp_DccLivenessVerificationHandler *pLivenessVerifier,
        const void **ppUsrPrivData)
{
#ifdef ICP_DCC_DEBUG
    if ( pLivenessVerifier == NULL || pThreadInfo == NULL 
            || ppUsrPrivData == NULL )
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
        return ICP_STATUS_NULL_PARAM;
    }
#endif

    ICP_DCC_LOCK(dccLock);
    *pLivenessVerifier = pThreadInfo->livenessVerifier;
    *ppUsrPrivData = pThreadInfo->pPrivData;
    ICP_DCC_UNLOCK(dccLock);

    return ICP_STATUS_SUCCESS;
}


/**
 * function to Set liveness verification handler 
 */
icp_status_t
DccLivenessVerifierSet (
        icp_dcc_thread_id_t *pThreadId,
        icp_DccLivenessVerificationHandler livenessVerifier,
        const void *pUsrPrivData )
{
    dcc_thread_info_t   *pThreadInfo = (dcc_thread_info_t *){ 0 };

#ifdef ICP_DCC_DEBUG
    if ( pThreadId == NULL )
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
        return ICP_STATUS_NULL_PARAM;
    }
#endif

    DccFindThreadInfo (pThreadId, &pThreadInfo);

    /* If this is an unregister call */
    if ( livenessVerifier == NULL )
    {
        /* Nothing more to be done. Return with status */
        return DccRemoveThreadInfo ( pThreadInfo );
    }

    /* If no info found and this is not an unregister call */
    if ( pThreadInfo == NULL )
    {
        pThreadInfo = (dcc_thread_info_t *){ 0 };
        if ( (DccAddThreadInfo (pThreadId, &pThreadInfo)) 
                != ICP_STATUS_SUCCESS )
        {
#ifdef ICP_DCC_DEBUG
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                    "Function: %s: DccAddThreadInfo Failed!\n",
                    (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
            return ICP_STATUS_FAIL;
        }
    }

    ICP_DCC_LOCK(dccLock);

    /* Register the callback handler for the module */
    pThreadInfo->livenessVerifier = livenessVerifier;

    /* Store the private data pointer provide by user */
    pThreadInfo->pPrivData = pUsrPrivData;

    /* Set the liveness locaiton field to zero since both cannot 
       exist simultaneously */
    pThreadInfo->pLivenessLocation = NULL;

    ICP_DCC_UNLOCK(dccLock);

    return ICP_STATUS_SUCCESS;
}


/**
 * function to Get liveness Counter Value 
 */
icp_status_t
DccThreadInfoLiveCounterValueGet (
        const dcc_thread_info_t *pThreadInfo, uint32_t *pValue
    )
{
#ifdef ICP_DCC_DEBUG
    if ( pThreadInfo == NULL || pValue == NULL )
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
        return ICP_STATUS_NULL_PARAM;
    }
#endif

    ICP_DCC_LOCK(dccLock);
    *pValue = pThreadInfo->liveCounterValue;
    ICP_DCC_UNLOCK(dccLock);

    return ICP_STATUS_SUCCESS;
}

/**
 * function to Set liveness Counter Value 
 */
icp_status_t
DccThreadInfoLiveCounterValueSet (
        dcc_thread_info_t *pThreadInfo, uint32_t value
    )
{
#ifdef ICP_DCC_DEBUG
    if (pThreadInfo == NULL )
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
        return ICP_STATUS_NULL_PARAM;
    }
#endif

    ICP_DCC_LOCK(dccLock);
    pThreadInfo->liveCounterValue = value;
    ICP_DCC_UNLOCK(dccLock);

    return ICP_STATUS_SUCCESS;
}

/**
 * function to Get liveness Status 
 */
icp_status_t
DccThreadInfoStatusGet (
        const dcc_thread_info_t *pThreadInfo, 
        icp_dcc_thread_status_t *pThreadStatus
    )
{
#ifdef ICP_DCC_DEBUG
    if ( pThreadInfo == NULL || pThreadStatus == NULL )
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
        return ICP_STATUS_NULL_PARAM;
    }
#endif

    ICP_DCC_LOCK(dccLock);
    *pThreadStatus = pThreadInfo->threadStatus;
    ICP_DCC_UNLOCK(dccLock);

    return ICP_STATUS_SUCCESS;
}

/**
 * function to Set liveness Status 
 */
icp_status_t
DccThreadInfoStatusSet (
        dcc_thread_info_t *pThreadInfo, icp_dcc_thread_status_t threadStatus
    )
{
#ifdef ICP_DCC_DEBUG
    if (pThreadInfo == NULL )
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
        return ICP_STATUS_NULL_PARAM;
    }
#endif

    ICP_DCC_LOCK(dccLock);
    pThreadInfo->threadStatus = threadStatus;
    ICP_DCC_UNLOCK(dccLock);

    return ICP_STATUS_SUCCESS;
}


/**
 * function to Get liveness timeout 
 */
uint32_t
DccLivenessTimeoutGet ( void )
{
    uint32_t timeout = 0;

    ICP_DCC_LOCK(dccLock);
    timeout = dccLivenessTimeout;
    ICP_DCC_UNLOCK(dccLock);

    return timeout;
}

/**
 * function to Get liveness timeout 
 */
icp_status_t 
DccLivenessTimeoutSet ( uint32_t livenessTimeout )
{
    ICP_DCC_LOCK(dccLock);
    dccLivenessTimeout = livenessTimeout;
    ICP_DCC_UNLOCK(dccLock);

    return ICP_STATUS_SUCCESS;
}

/**
 * LivenessVerify
 */
icp_status_t
DccLivenessVerify (
        uint8_t *pLivenessStatus,
        uint32_t *pBufferSize)
{
    uint32_t                    timeout = 0;
    uint32_t                    i = 0;
    dcc_thread_info_t           *pThreadInfo = dccThreadInfo;
    uint8_t                     *pThreadIdStr = NULL;
    uint32_t                    firstLiveValue = 0;
    uint32_t                    *pLocation = NULL; 
    icp_DccLivenessVerificationHandler 
                                pLivenessVerifier = NULL;
    const void                  *pPrivData = NULL;
    uint32_t                    offset = 0;
    icp_dcc_thread_status_t     threadStatus = ICP_DCC_THREAD_ID_DEAD;  
    icp_dcc_liveness_status_t   (*pStatusBuffer)[ICP_DCC_MAX_THREADS] = NULL;

    if ( pThreadInfo == NULL )
    {
        return ICP_STATUS_FAIL;
    }

    pStatusBuffer = 
        (icp_dcc_liveness_status_t (*)[ICP_DCC_MAX_THREADS])pLivenessStatus;

    if ( pStatusBuffer == NULL )
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
        return  ICP_STATUS_NULL_PARAM;
    }


    /* Get liveness of all threads which have registered a callback */
    for ( i = 0, pThreadInfo = dccThreadInfo;
            pThreadInfo != NULL;
              pThreadInfo = pThreadInfo->pNext )
    {
        /* This always returns success */
        DccThreadInfoLivenessVerifierGet (
                pThreadInfo, &pLivenessVerifier, &pPrivData );

        if ( pLivenessVerifier != NULL )   
        {
            /* Invoke callback */
            if (  ( pLivenessVerifier (&pThreadInfo->varThreadId,
                         &threadStatus, pPrivData )) != ICP_STATUS_SUCCESS )
            {
                ixOsalLog (IX_OSAL_LOG_LVL_DEBUG3, IX_OSAL_LOG_DEV_STDOUT,
                    "Function: %s: Verifier returned error! Assuming Dead\n",
                    (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);

                /* Set this to some new enum like ICP_DCC_THREADID_FAIL */
                threadStatus = ICP_DCC_THREAD_ID_DEAD;
            }

            /* This always returns success */
            DccThreadInfoStatusSet ( 
                    pThreadInfo, threadStatus );
        }
        else
        {
            /* Always returns success */
            DccThreadInfoLivenessLocationGet ( 
                 pThreadInfo, &pLocation );
            
            if ( pLocation != NULL)
            {
                /* Always returns success. Return value ignored */
                DccThreadInfoLiveCounterValueSet (
                    pThreadInfo, *pLocation );
            }
            else 
            {
                /* ERROR: This condition should not arise since there is no
                 node maintained in the threadInfo list with NULL Verifier
                 as well as NULL location*/
                /* Code should NEVER reach here */
                return ICP_STATUS_FAIL;
            }
        }

        i++;
        if ( i > ICP_DCC_MAX_THREADS )
        {
            /* some problem in the threadInfo str. Break from loop */
            break;
        }
    }

    timeout = DccLivenessTimeoutGet();
    if ( timeout > ICP_DCC_TIMEOUT_LIMIT )
    {
        return ICP_STATUS_FAIL;
    }

    /* Sleep for 'timeout' milliseconds */
    ixOsalSleep (timeout);

    /* This iteration is only for those threads that have locaiton
       registered. Check counter value to find thread Status.
       Status of those which have registered callbacks is already 
       evaluated and stored in respective node in dccThreadInfo list.
     */  
    for ( i = 0, pThreadInfo = dccThreadInfo;
            pThreadInfo != NULL;
              pThreadInfo = pThreadInfo->pNext )
    {
        /* Always returns success */
        DccThreadInfoLivenessLocationGet ( 
                 pThreadInfo, &pLocation );
            
        if ( pLocation != NULL)
        {
            /* Always returns success */
            DccThreadInfoLiveCounterValueGet (
                    pThreadInfo, &firstLiveValue );

            if ( firstLiveValue != *pLocation )
            {
                threadStatus = ICP_DCC_THREAD_ID_LIVE;
            }
            else
            {
                /* Sleep for a short duration and double check status */
                timeout = ICP_DCC_SHORT_TIMEOUT;
                ixOsalSleep ( timeout );
                
                if ( firstLiveValue != *pLocation )
                {
                    threadStatus = ICP_DCC_THREAD_ID_LIVE;
                }
                else
                {
                    threadStatus = ICP_DCC_THREAD_ID_DEAD;
                }
            }

            /* Always returns success */
            DccThreadInfoStatusSet ( 
                 pThreadInfo, threadStatus );
        }

        /* Get status of all threads and populate to user provided buffer */
        if ( *pBufferSize < 
                (offset + ICP_DCC_THREAD_ID_LENGTH + 
                                sizeof(icp_dcc_thread_status_t) ) )
        {
            return ICP_STATUS_FAIL;
        }

        ICP_DCC_LOCK(dccLock);
        pThreadIdStr = pThreadInfo->varThreadId.threadIdString;
        ICP_DCC_UNLOCK(dccLock);

        ixOsalMemCopy (
                (void *)(*pStatusBuffer)[i].threadId.threadIdString,
                (void *)pThreadIdStr,
                ICP_DCC_THREAD_ID_LENGTH );

        DccThreadInfoStatusGet (pThreadInfo, &threadStatus);

        ICP_DCC_LOCK(dccLock);
        (*pStatusBuffer)[i].threadStatus = threadStatus;
        ICP_DCC_UNLOCK(dccLock);

        /* Move offset within buffer */
        offset += sizeof(icp_dcc_liveness_status_t);

        i++;    
        if ( i >= ICP_DCC_MAX_THREADS )
        {
            /* some problem in the threadInfo str. Break from loop */
            break;
        }
    }

    /* Set the valid data size as offset */
    *pBufferSize = offset;

    return ICP_STATUS_SUCCESS;

}

/******************************************************************************
 * 
 * Data dump
 *
 *****************************************************************************/

/**
 * Function to Get Dump Handler
 */
icp_status_t
DccDataDumpHandlerGet ( 
        uint32_t moduleId, 
        uint32_t *pDumpSize, 
        icp_DccDataDumpHandler *pDumpHandler,
        const void **ppUsrPrivData
    )
{
    if (pDumpHandler == NULL || pDumpSize == NULL || ppUsrPrivData == NULL )
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Null Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
        return ICP_STATUS_NULL_PARAM;
    }

    /* Critical section */
    ICP_DCC_LOCK(dccLock);

    /* If registered already, warn user in debug mode */
    if (dccModuleInfo[moduleId]->dataDumpHandler == NULL)
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: No Handler Registered!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        /* Not required to return from here. We fall through till the end. */
    }

    /* Return the registered callback handler for the module */
    *pDumpHandler = dccModuleInfo[moduleId]->dataDumpHandler;
    *pDumpSize = dccModuleInfo[moduleId]->dataDumpSize; 
    *ppUsrPrivData = dccModuleInfo[moduleId]->pPrivData;

    ICP_DCC_UNLOCK(dccLock);

    return ICP_STATUS_SUCCESS;
}

/**
 * Function to Set Dump Handler
 */
icp_status_t
DccDataDumpHandlerSet (
        uint32_t moduleId, 
        uint32_t dumpSize, 
        icp_DccDataDumpHandler dumpHandler,
        const void *pUsrPrivData
    )
{
    uint32_t        i = 0;
    uint32_t        maxDumpSize = 0;

    /* If this is a call to unregister, set to NULL and update
       dccMaxDataDumpSize */
    if ( dumpHandler == NULL )
    {
        /* Critical section */
        ICP_DCC_LOCK(dccLock);
        
        if ( dccMaxDataDumpSize == dccModuleInfo[moduleId]->dataDumpSize )
        {
            /* Set this dumpSize to zero so that the same value is 
               not returned */
            dccModuleInfo[moduleId]->dataDumpSize = 0;

            /* Search for the next max dump Size. Start from index 1 of array
               since package version information is registered at index 0*/
            for ( ++i; i <= totalModulesRegistered; i++ )
            {
                if ( dccModuleInfo[i]->dataDumpSize > maxDumpSize )
                {
                    maxDumpSize = dccModuleInfo[i]->dataDumpSize;
                }
            }
    
            /* Set the global info for maximum data dump size */
            dccMaxDataDumpSize = maxDumpSize;
        }
               
        /* Set handler and size to NULL and zero to unregister */ 
        dccModuleInfo[moduleId]->dataDumpHandler = NULL;
        dccModuleInfo[moduleId]->pPrivData = NULL;
        dccModuleInfo[moduleId]->dataDumpSize = 0;

        /* Unregister completed. Return control */
        ICP_DCC_UNLOCK(dccLock);
        return ICP_STATUS_SUCCESS;
    }

    /* Call for HandlerRegister */

    ICP_DCC_LOCK(dccLock);

    /* If registered already, warn user in debug mode */
    if (dccModuleInfo[moduleId]->dataDumpHandler != NULL)
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Callback already registered!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        ICP_DCC_UNLOCK(dccLock);
        return ICP_STATUS_FAIL;
    }

    /* Register the callback handler for the module */
    dccModuleInfo[moduleId]->dataDumpHandler = dumpHandler;
    dccModuleInfo[moduleId]->pPrivData = pUsrPrivData;
    dccModuleInfo[moduleId]->dataDumpSize = dumpSize;

    if ( dumpSize > dccMaxDataDumpSize )
    {
        dccMaxDataDumpSize = dumpSize;
    }

    ICP_DCC_UNLOCK(dccLock);

    return ICP_STATUS_SUCCESS;
}


/**
 * function to get the registered SEN handler 
 */
icp_status_t 
DccSenHandlerGet ( icp_DccSenHandler *pSenHandler)
{
    ICP_DCC_LOCK(dccLock);
    *pSenHandler = dccSenHandler;
    ICP_DCC_UNLOCK(dccLock);

    return ICP_STATUS_SUCCESS;
}

/**
 * function to set the SEN handler with  DCC 
 */
icp_status_t 
DccSenHandlerSet ( icp_DccSenHandler senHandler)
{
    ICP_DCC_LOCK(dccLock);
    dccSenHandler = senHandler;
    ICP_DCC_UNLOCK(dccLock);

    return ICP_STATUS_SUCCESS;
}


/*****************************************************************************
 * Utility functions 
 ****************************************************************************/

icp_status_t 
DccFindThreadInfo (
        icp_dcc_thread_id_t *pThreadId, 
        dcc_thread_info_t **ppThreadInfo
    )
{
    dcc_thread_info_t   *pThrd = NULL;

#ifdef ICP_DCC_DEBUG
    if ( pThreadId == NULL )
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: NULL Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
        return ICP_STATUS_NULL_PARAM;
    }
#endif

    /* Critical section */
    ICP_DCC_LOCK(dccLock);

    for (pThrd = dccThreadInfo; 
            pThrd != NULL; 
                pThrd = pThrd->pNext)
    {
        if ( strncmp ( 
                    (char*)pThreadId->threadIdString, 
                    (char*)pThrd->varThreadId.threadIdString, 
                    ICP_DCC_THREAD_ID_LENGTH ) == 0 )
        {
            break;
        }
    }

    /* If we came here through break, then some valid node has been found.
       Else, we are here after complete for loop, hence pThrd is NULL */
    *ppThreadInfo = pThrd;

    ICP_DCC_UNLOCK(dccLock);

    return ICP_STATUS_SUCCESS;
}

/**
 * Add a new node to threadInfo linked list 
 */
icp_status_t 
DccAddThreadInfo (
        icp_dcc_thread_id_t *pThreadId, 
        dcc_thread_info_t **ppThreadInfo)
{
    if ( DccTotalThreadsGet() == ICP_DCC_MAX_THREADS )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Exceeded MAX_THREADS. "\
                "Cannot register any new thread!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_RESOURCE;
    }

#ifdef ICP_DCC_DEBUG
    if ( pThreadId == NULL || ppThreadInfo == NULL )
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: NULL Input Parameter!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
        return ICP_STATUS_NULL_PARAM;
    }
#endif

    /* Allocate a node for this thread and populate the location */
    *ppThreadInfo = (dcc_thread_info_t *)
                        ixOsalMemAlloc ( sizeof(dcc_thread_info_t) );
    
    if ( *ppThreadInfo == NULL )
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Could not allocate memory!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
        return ICP_STATUS_RESOURCE;
    }

    ixOsalMemCopy ( 
            (void *)(*ppThreadInfo)->varThreadId.threadIdString,
            (void *)pThreadId->threadIdString,
            ICP_DCC_THREAD_ID_LENGTH );

    (*ppThreadInfo)->pLivenessLocation = NULL;
    (*ppThreadInfo)->livenessVerifier = NULL;
    (*ppThreadInfo)->pPrivData = NULL;
    (*ppThreadInfo)->pPrev = NULL;
    (*ppThreadInfo)->pNext = NULL;
    (*ppThreadInfo)->threadStatus = ICP_DCC_THREAD_ID_DEAD;
    (*ppThreadInfo)->liveCounterValue = 0;

    /* Critical section */
    ICP_DCC_LOCK(dccLock);

    /* Add new node to start of the linked list */
    (*ppThreadInfo)->pNext = dccThreadInfo;
    
    if (dccThreadInfo != NULL )
    {
        dccThreadInfo->pPrev = *ppThreadInfo;
    }
    dccThreadInfo = *ppThreadInfo;

    ++totalThreadsRegistered;

    ICP_DCC_UNLOCK(dccLock);

    return ICP_STATUS_SUCCESS;
}

/**
 * Delete a node from threadInfo linked list 
 */
icp_status_t
DccRemoveThreadInfo (dcc_thread_info_t *pThreadInfo)
{
    if ( pThreadInfo == NULL )
    {
#ifdef ICP_DCC_DEBUG
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "Function: %s: Non existent node!\n",
                (uint32_t)__FUNCTION__, 0, 0, 0, 0, 0);
#endif
        return ICP_STATUS_INVALID_PARAM;
    }

    ICP_DCC_LOCK(dccLock);

    if ( (pThreadInfo->pPrev != NULL) && (pThreadInfo->pNext != NULL) )
    {
        /* Node is in middle of the list */
        pThreadInfo->pPrev->pNext = pThreadInfo->pNext;
        pThreadInfo->pNext->pPrev = pThreadInfo->pPrev;
    }
    else if ( pThreadInfo->pPrev == NULL && pThreadInfo->pNext == NULL )
    {
        /* This is the only node in the list */
        dccThreadInfo = NULL;

    }
    else if ( pThreadInfo->pPrev == NULL )
    {
        /* This is first node in the list */
        dccThreadInfo = pThreadInfo->pNext;
        pThreadInfo->pNext->pPrev = NULL;
    }
    else
    {
        /* This is the last node in the list */
        pThreadInfo->pPrev->pNext = NULL;
    }

    --totalThreadsRegistered;

    ICP_DCC_UNLOCK(dccLock);
   
    if ( pThreadInfo != NULL )
    {
        ixOsalMemFree ( (void *)pThreadInfo );
        pThreadInfo = NULL;
    }

    return ICP_STATUS_SUCCESS;
}

/*****************************************************************************/

/**
 * function to Get total no. of components registered
 */
uint32_t
DccTotalModulesGet ( void )
{
    uint32_t totalModules = 0;

    ICP_DCC_LOCK(dccLock);
    totalModules = totalModulesRegistered;
    ICP_DCC_UNLOCK(dccLock);

    return totalModules;
}

/**
 * function to Get total no. of threads registered
 */
uint32_t
DccTotalThreadsGet ( void )
{
    uint32_t totalThreads = 0;

    ICP_DCC_LOCK(dccLock);
    totalThreads = totalThreadsRegistered;
    ICP_DCC_UNLOCK(dccLock);

    return totalThreads;
}

/**
 * function to Get maximum data dump size
 */
uint32_t
DccMaxDataDumpSizeGet ( void )
{
    uint32_t maxSize = 0;

    ICP_DCC_LOCK(dccLock);
    maxSize = dccMaxDataDumpSize;
    ICP_DCC_UNLOCK(dccLock);

    return maxSize;
}

/**
 * function to GetVersionInfoSize 
 */
uint32_t
DccVersionInfoSizeGet ( void )
{
    uint32_t size = 0;

    ICP_DCC_LOCK(dccLock);
    size = dccVersionInfoSize;
    ICP_DCC_UNLOCK(dccLock);

    return size;
}

uint8_t
DccInitialized ( void )
{
    /* return the status of flag */
    return dccInitialized;
}

icp_status_t
icp_DccInitilizeModule ( void )
{
    IX_STATUS osal_err = IX_SUCCESS;
    icp_status_t retval = ICP_STATUS_SUCCESS;

    osal_err = ixOsalSpinLockInit( &dccInitSpinLock, TYPE_IGNORE);
    if (IX_SUCCESS != osal_err)
    {
        ixOsalLog(IX_OSAL_LOG_LVL_ERROR,\
                IX_OSAL_LOG_DEV_STDERR,\
                "DccSpinLockInit Failed\n",\
                0,0,0,0,0,0);
        retval = ICP_STATUS_FAIL;
    }
    return retval;
}

icp_status_t
icp_DccFinilizeModule ( void )
{
    IX_STATUS osal_err = IX_SUCCESS;
    icp_status_t retval = ICP_STATUS_SUCCESS;

    osal_err = ixOsalSpinLockDestroy( &dccInitSpinLock);
    if (IX_SUCCESS != osal_err)
    {
        ixOsalLog(IX_OSAL_LOG_LVL_ERROR,\
                IX_OSAL_LOG_DEV_STDERR,\
                "DccSpinLockDestroy Failed\n",\
                0,0,0,0,0,0);
        retval = ICP_STATUS_FAIL;
    }
    return retval;
}



