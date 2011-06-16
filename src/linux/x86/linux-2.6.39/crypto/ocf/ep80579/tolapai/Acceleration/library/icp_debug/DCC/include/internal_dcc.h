/*******************************************************************************
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
#if !defined(__INTERNAL_DCC_AL_H__)
#define __INTERNAL_DCC_AL_H__

#if defined(__cplusplus)
extern "C"
{
#endif /* end defined(__cplusplus) */

#include "cpa.h"
#include "dcc/icp_dcc_common.h"
#include "dcc/icp_dcc.h"
#include "icp_dcc_al.h"

/**
 *****************************************************************************
 * 
 * @defgroup DCCInternalTypes Debug Common Component Internal Data Structures
 *
 * @ingroup DCCLibraryInternal
 *
 * @description
 *         This file contains the internal data structure definitions of 
 *         Debug Common Component
 *      
 * 
 *****************************************************************************/

#ifndef STATIC
#define STATIC  static
#endif

/*****************************************************************************/

/* Defines for DCC */

#define ICP_DCC_ONE                     1U

/* Maximum value for timeout that user can provide */
#define ICP_DCC_TIMEOUT_LIMIT           20000U

#define ICP_DCC_SHORT_TIMEOUT           100U

/* Package version information */
#ifndef PACKAGE_NAME
#define PACKAGE_NAME            "Security.L"
#endif

#ifndef PACKAGE_VERSION_MAJOR_NUMBER
#define PACKAGE_VERSION_MAJOR_NUMBER          1
#endif

#ifndef PACKAGE_VERSION_MINOR_NUMBER
#define PACKAGE_VERSION_MINOR_NUMBER          1
#endif

#ifndef PACKAGE_VERSION_PATCH_NUMBER
#define PACKAGE_VERSION_PATCH_NUMBER        1
#endif

#ifndef PACKAGE_VERSION_BUILD_NUMBER
#define PACKAGE_VERSION_BUILD_NUMBER        1
#endif


/**
 *****************************************************************************
 * @ingroup DCCInternalTypes
 *    Complete information maintained by DCC about a module/component 
 *
 * @description
 *    This structure provides complete information about a module/component 
 *    that is registered with DCC. This includes dump callback handler, 
 *    version info.  
 *
 * @purpose
 *
 *
 *****************************************************************************/
typedef struct dcc_module_info_s
{
    uint8_t                     moduleName[ICP_DCC_COMPONENT_NAME_LENGTH]; 
    uint8_t                     
        moduleVersion[ICP_DCC_VERSION_STR_LENGTH + ICP_DCC_ONE];
    icp_DccDataDumpHandler      dataDumpHandler;
    const void                  *pPrivData;
    uint32_t                    dataDumpSize;
} dcc_module_info_t;

/**
 *****************************************************************************
 * @ingroup DCCInternalTypes
 *    Complete information maintained by DCC about a thread 
 *
 * @description
 *    This structure provides complete information about a thread 
 *    that is registered with DCC.   
 *
 * @purpose
 *
 *
 *****************************************************************************/
typedef struct dcc_thread_info_s
{
    struct dcc_thread_info_s    *pNext;
    struct dcc_thread_info_s    *pPrev;
    icp_dcc_thread_id_t     varThreadId;
    uint32_t                *pLivenessLocation;
    icp_DccLivenessVerificationHandler
                            livenessVerifier;
    const void              *pPrivData;
    icp_dcc_thread_status_t threadStatus;
    uint32_t                liveCounterValue;
} dcc_thread_info_t;


/**
 *****************************************************************************
 * @ingroup DCCInternalTypes
 *    DCC Internal Lock Types 
 *
 * @description
 *    These defines provide an agnostic way of lock usage for DCC.  
 *
 * @purpose
 *    
 *
 *****************************************************************************/
#if !defined(ICP_DCC_USE_SEMAPHORE)
 
#define ICP_DCC_LOCK_TYPE            IxOsalSpinLock
#define ICP_DCC_LOCK_T(lock)         IxOsalSpinLock lock
                                                                                 
#define ICP_DCC_LOCK_INIT(lock) \
                                 if(ixOsalSpinLockInit(&lock, 0)!=IX_SUCCESS)\
                                 {\
                                     ixOsalLog(IX_OSAL_LOG_LVL_ERROR,\
                                             IX_OSAL_LOG_DEV_STDERR,\
                                             "SpinLockInit Failed\n",\
                                             0,0,0,0,0,0);\
                                     return ICP_STATUS_FAIL;\
                                 }

#define ICP_DCC_LOCK_FINI(lock) \
                                 if(ixOsalSpinLockDestroy(&lock)!=IX_SUCCESS)\
                                 {\
                                     ixOsalLog(IX_OSAL_LOG_LVL_ERROR,\
                                             IX_OSAL_LOG_DEV_STDERR,\
                                             "SpinLockDestroy Failed\n",\
                                             0,0,0,0,0,0);\
                                     return ICP_STATUS_FAIL;\
                                 }


#define ICP_DCC_LOCK(lock)      \
                                 if(ixOsalSpinLockLock(&lock)!=IX_SUCCESS)\
                                 {\
                                     ixOsalLog(IX_OSAL_LOG_LVL_ERROR,\
                                             IX_OSAL_LOG_DEV_STDERR,\
                                             "SpinLockLock Failed\n",\
                                             0,0,0,0,0,0);\
                                     return ICP_STATUS_FAIL;\
                                 }

#define ICP_DCC_UNLOCK(lock)    \
                                 if(ixOsalSpinLockUnlock(&lock)!=IX_SUCCESS)\
                                 {\
                                     ixOsalLog(IX_OSAL_LOG_LVL_ERROR,\
                                             IX_OSAL_LOG_DEV_STDERR,\
                                             "SpinLockUnlock Failed\n",\
                                             0,0,0,0,0,0);\
                                     return ICP_STATUS_FAIL;\
                                 }

#else /* Use semaphores */

#define ICP_DCC_LOCK_TYPE            IxOsalSemaphore
#define ICP_DCC_LOCK_T(lock)         IxOsalSemaphore lock
                                                                                 
#define ICP_DCC_LOCK_INIT(lock) \
                                if(ixOsalSemaphoreInit(&lock, 1)!=IX_SUCCESS)\
                                {\
                                     ixOsalLog(IX_OSAL_LOG_LVL_ERROR,\
                                             IX_OSAL_LOG_DEV_STDERR,\
                                             "SemInit Failed\n",\
                                             0,0,0,0,0,0);\
                                     return ICP_STATUS_FAIL;\
                                 }

#define ICP_DCC_LOCK_FINI(lock) \
                                if(ixOsalSemaphoreDestroy(&lock)!=IX_SUCCESS)\
                                {\
                                     ixOsalLog(IX_OSAL_LOG_LVL_ERROR,\
                                             IX_OSAL_LOG_DEV_STDERR,\
                                             "SemDestroy Failed\n",\
                                             0,0,0,0,0,0);\
                                     return ICP_STATUS_FAIL;\
                                 }


#define ICP_DCC_LOCK(lock)      \
                                if(ixOsalSemaphoreWait(\
                                   &lock,IX_OSAL_WAIT_FOREVER)!=IX_SUCCESS)\
                                {\
                                     ixOsalLog(IX_OSAL_LOG_LVL_ERROR,\
                                             IX_OSAL_LOG_DEV_STDERR,\
                                             "SemWait Failed\n",\
                                             0,0,0,0,0,0);\
                                     return ICP_STATUS_FAIL;\
                                 }

#define ICP_DCC_TRYLOCK(lock)   \
                                 if(ixOsalSemaphoreTrywait(&lock)!=IX_SUCCESS)\
                                 {\
                                     ixOsalLog(IX_OSAL_LOG_LVL_ERROR,\
                                             IX_OSAL_LOG_DEV_STDERR,\
                                             "SemTrywait Failed\n",\
                                             0,0,0,0,0,0);\
                                     return ICP_STATUS_FAIL;\
                                 }

#define ICP_DCC_UNLOCK(lock)    \
                                 if(ixOsalSemaphorePost(&lock)!=IX_SUCCESS)\
                                 {\
                                     ixOsalLog(IX_OSAL_LOG_LVL_ERROR,\
                                             IX_OSAL_LOG_DEV_STDERR,\
                                             "SemUnlock Failed\n",\
                                             0,0,0,0,0,0);\
                                     return ICP_STATUS_FAIL;\
                                 }

#endif

/* Utility function to convert integer to string */
#define ICP_DCC_INT_TO_STR(intValue,str)  \
            snprintf((char*)str,\
                    (ICP_DCC_MODULE_ID_STR_LENGTH+ICP_DCC_ONE), \
                    "%d",intValue)

/* Utility function to create package version string from the major, minor 
   patch and build versions. This is used while formatting version info 
   before passing it on to Debug Client 
 */
#define ICP_DCC_CREATE_PKG_VER_STRING(str,majVer,minVer,patchVer,buildVer)  \
    snprintf((char*)str,\
            (ICP_DCC_VERSION_STR_LENGTH+ICP_DCC_ONE),\
            "%d.%d.%d-%04d",majVer,minVer,patchVer,buildVer)

/* Utility function to create version string from the major, minor and patch 
   versions. This is used while formatting version info before passing it on
   to Debug Client 
 */
#define ICP_DCC_CREATE_VER_STRING(str,majVer,minVer,patchVer)  \
    snprintf((char*)str,\
            (ICP_DCC_VERSION_STR_LENGTH+ICP_DCC_ONE),\
            "%d.%d.%03d",majVer,minVer,patchVer)

/** Internal Function declarations */

CpaStatus DccInit ( void );

uint8_t  DccInitialized ( void );

CpaStatus DccShutdown ( void );

uint32_t DccTotalModulesGet ( void );

uint32_t DccTotalThreadsGet ( void );

uint32_t DccMaxDataDumpSizeGet ( void );

uint32_t DccVersionInfoSizeGet ( void );


/* Version Information */
icp_status_t DccVersionGet (
        uint8_t *pVerInfo, uint32_t *pBufferSize);

icp_status_t DccComponentVersionSet (
        icp_dcc_ver_info_t *pVersionInfo, uint32_t *pModuleId);


/* System Liveness */
uint32_t DccLivenessTimeoutGet ( void );

icp_status_t DccLivenessTimeoutSet ( uint32_t livenessTimeout );

icp_status_t DccThreadInfoLivenessLocationGet (
        const dcc_thread_info_t *pThreadInfo, 
        uint32_t **pLivenessLocation);

icp_status_t DccLivenessLocationSet (
         icp_dcc_thread_id_t *pThreadId, uint32_t *pLivenessLocation);

icp_status_t DccThreadInfoLivenessVerifierGet (
        const dcc_thread_info_t *pThreadInfo, 
        icp_DccLivenessVerificationHandler *pLivenessVerifier, 
        const void **ppUsrPrivData );

icp_status_t DccLivenessVerifierSet (
        icp_dcc_thread_id_t *pThreadId, 
        icp_DccLivenessVerificationHandler livenessVerifier,
        const void *pUsrPrivData);

icp_status_t DccLivenessVerify ( 
        uint8_t *pLivenessStatus, uint32_t *pBufferSize);

icp_status_t DccThreadInfoLiveCounterValueGet (
        const dcc_thread_info_t *pThreadInfo, uint32_t *pValue);

icp_status_t DccThreadInfoLiveCounterValueSet (
        dcc_thread_info_t *pThreadInfo, uint32_t value);

icp_status_t DccThreadInfoStatusGet (
        const dcc_thread_info_t *pThreadInfo, 
        icp_dcc_thread_status_t *pThreadStatus);

icp_status_t DccThreadInfoStatusSet (
        dcc_thread_info_t *pThreadInfo, 
        icp_dcc_thread_status_t threadStatus);


/* Data Dump */  
icp_status_t DccDataDumpHandlerGet (
                uint32_t moduleId, 
                uint32_t *pDumpSize, 
                icp_DccDataDumpHandler *pDumpHandler,
                const void **ppUsrPrivData);

icp_status_t DccDataDumpHandlerSet (
                uint32_t moduleId, 
                uint32_t dumpSize, 
                icp_DccDataDumpHandler dumpHandler,
                const void *pUsrPrivData);

/* SEN */
icp_status_t DccSenHandlerGet ( icp_DccSenHandler *pSenHandler );
       
icp_status_t DccSenHandlerSet ( icp_DccSenHandler senHandler );


/* Routines to maintain threadInfo structure */
icp_status_t DccFindThreadInfo (
        icp_dcc_thread_id_t *pThreadId,
        dcc_thread_info_t **pThreadInfo);

icp_status_t DccAddThreadInfo (
        icp_dcc_thread_id_t *pThreadId,
        dcc_thread_info_t **pThreadInfo);

icp_status_t DccRemoveThreadInfo (dcc_thread_info_t *pThreadInfo);


#if defined(__cplusplus)
}
#endif /* end defined(__cplusplus) */

#endif /* end !defined(__ICP_DCC_AL_H__) */
