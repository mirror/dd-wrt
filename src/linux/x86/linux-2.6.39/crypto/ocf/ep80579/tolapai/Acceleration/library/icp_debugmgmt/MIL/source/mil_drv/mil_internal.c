/******************************************************************************
 *
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
* @file mil_internal.c
*
* @defgroup MIL
*
* @ingroup MILKernelInternal
*
* @description
*      This file contains the functions to get the data
from the DCC and write it to log file in
the desired format
*
*****************************************************************************/

#include "dcc/icp_dcc.h"
#include "icp_dcc_cfg.h"
#include "icp_dcc_al.h"
#include "mil_internal.h"

#define WRITE_TO_SYSLOG(format...)  MIL_KERNEL_DEBUG_PRINT(format)

#ifndef DEFAULT_LOG_FILE_NAME
#define DEFAULT_LOG_FILE_NAME "/var/log/icp_debugmgmt.log"
#endif 

static
mil_enable_status_t
mil_initialized = MIL_DISABLE; /**<MIL enable status variable*/

static
uint8_t logFilename[MAX_FILENAME_SIZE] = DEFAULT_LOG_FILE_NAME;


#if defined(__linux) && defined (__KERNEL__)

#define     MIL_ALLOC_NON_CONTIGUOUS_MEM(size)   vmalloc(size)
#define     MIL_FREE_MEM(addr)    vfree( addr )
#define     MIL_MEMSET(addr, value, size)  memset(addr, value, size)

#elif defined (__freebsd) && defined(_KERNEL) 

#define MIL_ALLOC_NON_CONTIGUOUS_MEM(size) \
            malloc(size, M_TEMP, (M_WAITOK | M_ZERO))
#define MIL_FREE_MEM(addr)    free( addr, M_TEMP)
#define MIL_MEMSET(addr, value, size) \
            (memset((addr), (value), (size)), (void *) 0)

#else
#error "Unsupported OS: Unable to specify MIL_ALLOC_MEM, MIL_FREE_MEM"
#endif /* defined(__linux__) && defined (__KERNEL__) and freebsd elif branch */

mil_error_status_t GetDataDumpSize(void* arg);
mil_error_status_t GetVersionInfoSize(void* arg);
mil_error_status_t GetLivenessSize(void* arg);

/**
*****************************************************************************
* @ingroup MILKernelInternal
*  	Sen Handler
*
* @description
*  This function is responsible for writing the
SEN information in syslog in desired format.
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
*      No
*
* @threadSafe
*      Yes
*
* @param pSenMsg IN the message to be written
in encoded format
*
* @pre
*      MIL must be enabled.
*
* @post
*      None.
*
*****************************************************************************/

void
MilSenHandler (
    icp_dcc_sen_msg_t *pSenMsg )
{
    uint32_t    i = 0;
    
    /* Print all data provided in senMsg */
    WRITE_TO_SYSLOG("\nicp_debugmgmt:SEN:" );
    WRITE_TO_SYSLOG("%lld;" , pSenMsg->timestamp );
    WRITE_TO_SYSLOG("%u;" , ( uint32_t )( pSenMsg->senPriority ));
    WRITE_TO_SYSLOG("%u;" , pSenMsg->moduleId);
    WRITE_TO_SYSLOG("%u;" , pSenMsg->eventId );
#ifdef MIL_DEBUG
    MIL_KERNEL_DEBUG_PRINT ( "\n EventInfoSize: %d" , pSenMsg->eventInfoSize );
#endif /*MIL_DEBUG*/
    /*Print the message*/
    for ( i = 0 ; i < pSenMsg->eventInfoSize ; i++ )
    {
        WRITE_TO_SYSLOG( "%c" , pSenMsg->eventInfo[i] );
    }
    WRITE_TO_SYSLOG( "\n" );
    return;
}

/**
*****************************************************************************
* @ingroup MILKernelInternal
*  Get the data dump
*
* @description
*  This function will get the data dump from DCC
and put it in syslog in desired format.
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
*      No
*
* @threadSafe
*      Yes
*
*
* @retval  MIL_SUCCESS If successful
*
* @retval  MIL_ERROR A valid mil_error_status_t for failure
*
* @pre
*      MIL must be enabled.
*
* @post
*      None.
*
*****************************************************************************/

mil_error_status_t
GetDumpData(void* arg)
{
    uint32_t    index = 0;
    uint8_t     *pDataDump = NULL;
    uint32_t    noOfModules = 0;
    uint32_t    maxDumpSize = 0;
    uint32_t    moduleDumpSize = 0;
    mil_error_status_t status = MIL_SUCCESS;

    if ( mil_initialized != MIL_ENABLE )
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetDumpData] MIL is not enabled.\n");
        return MIL_NOT_ENABLED_ERROR;
    }

    /*get the numer of modules and max dump size*/
    status = mil_from_user(&index, (void *) *((int32_t *) arg), 
                sizeof(uint32_t));
    MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetDumpData] index is %d\n", index);
    MIL_NOTEQUAL_TEST_AND_RETURN(status, MIL_SUCCESS, status);

    if ( icp_DccDataDumpInfoGet ( &noOfModules , &maxDumpSize )
            != ICP_STATUS_SUCCESS)
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetDumpData] "\
        "icp_DccDataDumpInfoGet Failed!\n" );
        return MIL_DATA_DUMP_INFO_ERROR;
    }

    pDataDump = (uint8_t *) MIL_ALLOC_NON_CONTIGUOUS_MEM(\
                                maxDumpSize + sizeof(uint32_t));
    if (pDataDump == NULL)
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetDumpData] "\
                "MIL_ALLOC_NON_CONTIGUOUS_MEM failed!\n");
        return MIL_MEMORY_ALLOC_FAIL;
    }

    /* Invoke data dump for each module and put it in syslog */
    MIL_MEMSET( pDataDump , 0 , (maxDumpSize + sizeof(uint32_t)));
    moduleDumpSize = maxDumpSize;
    if ( icp_DccDataDumpGet ( index ,
                              &moduleDumpSize ,
                              pDataDump + sizeof(uint32_t))
            != ICP_STATUS_SUCCESS )
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetDumpData] icp_DccDataDumpGet " \
                                "failed for module index = %d\n", index);
        MIL_FREE_MEM( pDataDump );
        pDataDump = NULL;
        return MIL_DATA_DUMP_GET_ERROR;
    }
    memcpy(pDataDump, &moduleDumpSize, sizeof(uint32_t));
    MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetDumpData] module dump size is %u\n", 
                            moduleDumpSize);
    status = mil_to_user((void *) *((int32_t *) arg), pDataDump, 
                         moduleDumpSize + sizeof(uint32_t));
    MIL_FREE_MEM( pDataDump );
    pDataDump = NULL;
    return status;
}


/**
*****************************************************************************
* @ingroup MILKernelInternal
*  Get the version information
*
* @description
*  This function will get the version information from DCC
and put it in syslog in desired format.
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
*      No
*
* @threadSafe
*      Yes
*
*
* @retval  MIL_SUCCESS If successful
*
* @retval  MIL_ERROR A valid mil_error_status_t for failure
*
* @pre
*      MIL must be enabled.
*
* @post
*      None.
*
*****************************************************************************/
mil_error_status_t
GetVersionInfoAll(void* arg )
{
    uint32_t MILVersionInfoBufferSize = 0;
    icp_dcc_version_buffer_t *pVerInfo = NULL; /**<Buffer to read data */
    mil_error_status_t status = MIL_SUCCESS;

    if ( mil_initialized != MIL_ENABLE )
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetVersionInfoAll] MIL is " \
                "not enabled. Can not complete the task.\n");
        return MIL_NOT_ENABLED_ERROR;
    }

    /*Get the version size from the dcc*/
    if ( icp_DccVersionInfoSizeGet ( &MILVersionInfoBufferSize )
            != ICP_STATUS_SUCCESS )
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetVersionInfoAll] "\
                "can not get version info size from DCC\n");
        return MIL_VERSION_SIZE_GET_ERROR;
    }
    MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetVersionInfoAll] "\
        "the version info buffer size is %d\n", MILVersionInfoBufferSize);

    /*Allocate this much of buffer*/
    pVerInfo = ( icp_dcc_version_buffer_t * ) MIL_ALLOC_NON_CONTIGUOUS_MEM
               ( MILVersionInfoBufferSize );
    if ( pVerInfo == NULL )
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetVersionInfoAll] "\
                                "MIL_ALLOC_NON_CONTIGUOUS_MEM failed.\n");
        return MIL_MEMORY_ALLOC_FAIL;
    }
    /*get the version information in the buffer*/
    if ( icp_DccSoftwareVersionGet ( (uint8_t *) pVerInfo ,
                                     &MILVersionInfoBufferSize )
            != ICP_STATUS_SUCCESS )
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetVersionInfoAll] "\
                             "Can not get the version data from the DCC\n");

        MIL_FREE_MEM( pVerInfo );
        pVerInfo = NULL;
        return MIL_VERSION_DATA_GET_ERROR;
    }
    status = mil_to_user((void *) *((int32_t *) arg), \
                pVerInfo, MILVersionInfoBufferSize);
    MIL_FREE_MEM( pVerInfo);
    pVerInfo = NULL;
    return status;
}

/**
*****************************************************************************
* @ingroup MILKernelInternal
*  configure the timeout for system health check
*
* @description
*  This function will call the DCC function to set the
*	timeout for system helath check, and put the
*	information in syslog in desired format.
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
*      No
*
* @threadSafe
*      Yes
*
*
* @retval  MIL_SUCCESS If successful
*
* @retval  MIL_ERROR A valid mil_error_status_t for failure
*
* @param timeout IN The timeout value that needs to be set
* @pre
*      MIL must be enabled.
*
* @post
*      None.
**************************************************************************/
mil_error_status_t
ConfigureLivenessTimeout(void* arg)
{

    uint32_t timeout = MIN_TIMEOUT_VALUE;
    mil_error_status_t status = MIL_SUCCESS;
    
    
    /*Check for the enable status*/
    if ( mil_initialized != MIL_ENABLE )
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:ConfigureLivenessTimeout] "\
                                "MIL is not enabled.\n");
        return MIL_NOT_ENABLED_ERROR;
    }

    status = mil_from_user(&timeout,(void *) *((int32_t *) arg), 
                sizeof(uint32_t));
    MIL_NOTEQUAL_TEST_AND_RETURN (status, MIL_SUCCESS, status);

#ifdef MIL_DEBUG
    if ( ( timeout < MIN_TIMEOUT_VALUE ) || ( timeout > MAX_TIMEOUT_VALUE ) )
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:ConfigureLivenessTimeout] "\
                                "Timeout value out of range.\n" );
        return MIL_FAILURE;
    }
#endif/*MIL_DEBUG*/

    /*configure the timeout by calling dcc API*/
    status = icp_DccLivenessConfigureTimeout( timeout);
    if (ICP_STATUS_SUCCESS != status)
    {

        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:ConfigureLivenessTimeout] "\
                            "icp_DccLivenessConfigureTimeout failed (%d).\n",\
                            status);

        return MIL_CONFIGURE_LIVENESS_TIMEOUT_ERROR;
    }
    /*Log the information*/
    MIL_KERNEL_DEBUG_PRINT("[MIL_ks:ConfigureLivenessTimeout] Liveness "\
                            "timout configured with value %d.\n", timeout);

    return MIL_SUCCESS;
}


/**
*****************************************************************************
* @ingroup MILKernelInternal
*  	system health check
*
* @description
*  This function will get the system health check information from DCC
and put it in syslog in desired format.
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
*      No
*
* @threadSafe
*      Yes
*
*
* @retval  MIL_SUCCESS If successful
*
* @retval  MIL_ERROR A valid mil_error_status_t for failure
*
* @pre
*      MIL must be enabled.
*
* @post
*      None.
*
*****************************************************************************/
mil_error_status_t
LivenessVerify(void* arg)
{
    uint32_t LivenessResponseSize = 0;
    mil_error_status_t status = MIL_SUCCESS;
    icp_dcc_liveness_status_t *pLivenessStatusBuffer = NULL;


    /*check for MIL enable*/
    if ( mil_initialized != MIL_ENABLE )
    {

        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:LivenessVerify] "\
                                "MIL is not enabled.\n");
        return MIL_NOT_ENABLED_ERROR;
    }

    status = mil_from_user(&LivenessResponseSize, \
                (void *) *((int32_t *) arg), sizeof(uint32_t));
    MIL_NOTEQUAL_TEST_AND_RETURN (status, MIL_SUCCESS, status);
    MIL_KERNEL_DEBUG_PRINT("[MIL_ks:LivenessVerify] liveness response size "\
                            "is %d.\n", LivenessResponseSize);

    /*allocate memory for the buffer*/  
    pLivenessStatusBuffer = (icp_dcc_liveness_status_t *) \
                    MIL_ALLOC_NON_CONTIGUOUS_MEM( LivenessResponseSize);
    MIL_DEBUG_POSITIVE_TEST_AND_RETURN( pLivenessStatusBuffer, 
                                         NULL,
                                         MIL_MEMORY_ALLOC_FAIL);
    /*Get the thread status info from the dcc*/
    status = icp_DccLivenessVerify( (uint8_t *) pLivenessStatusBuffer, \
                                  &LivenessResponseSize);
    if (ICP_STATUS_SUCCESS != status)
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:LivenessVerify] " \
                                "icp_DccLivenessVerify failed (%d).\n", \
                                status);
        MIL_FREE_MEM( pLivenessStatusBuffer );
        pLivenessStatusBuffer = NULL;
        return MIL_LIVENESS_VERIFY_ERROR;
    }
    status = mil_to_user((void *) *((int32_t *) arg), pLivenessStatusBuffer,
                         LivenessResponseSize);
    MIL_FREE_MEM( pLivenessStatusBuffer );
    pLivenessStatusBuffer = NULL;
    return status;
}

/**
*****************************************************************************
* @ingroup MILKernelInternal
*  	Enable the debug
*
* @description
*  This function will enable the MIL, so that
other APIs can run.
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
*      No
*
* @threadSafe
*      Yes
*
*
* @retval  MIL_SUCCESS If successful
*
* @retval  MIL_ERROR A valid mil_error_status_t for failure
*
* @pre
*      MIL must be enabled.
*
* @post
*      None.
*
*****************************************************************************/
mil_error_status_t
DebugEnable ( void )
{
    icp_status_t status = ICP_STATUS_SUCCESS;
    
    /*check the mil enable flag. if set return error.*/
    if ( mil_initialized == MIL_ENABLE )
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:DebugEnable] "\
                                "MIL is already enabled.\n");
        return MIL_REPEAT_ENABLE_ERROR;
    }

    /*Register the sen handler*/
    status = icp_DccSenHandlerRegister ( MilSenHandler);
    if (ICP_STATUS_SUCCESS != status)
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:DebugEnable] SenHandlerRegister "\
                                "failed (%d).\n", status);
        return MIL_SEN_HANDLER_REGISTER_ERROR;
    }

    /*Register Timeout*/
    status = icp_DccLivenessConfigureTimeout (LIVENESS_DEFAULT_TIMEOUT );
    if ( ICP_STATUS_SUCCESS  != status)
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:DebugEnable] registering liveness "\
                                "timeout failed (%d).\n", status);
        icp_DccSenHandlerUnregister ();
        return MIL_CONFIGURE_LIVENESS_TIMEOUT_ERROR;
    }
    /* if all the operation above end up ok enable MIL*/
    mil_initialized = MIL_ENABLE;
    return MIL_SUCCESS;
}

/**
*****************************************************************************
* @ingroup MILKernelInternal
*  	Disable MIL
*
* @description
*  This function will disable the MIL, and
* stops other APIs from getting information.
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
*      No
*
* @threadSafe
*      Yes
*
*
* @retval  MIL_SUCCESS If successful
*
* @retval  MIL_ERROR A valid mil_error_status_t for failure
*
* @pre
*      MIL must be enabled.
*
* @post
*      None.
*
*****************************************************************************/
mil_error_status_t
DebugDisable ( void )
{
    mil_error_status_t status = MIL_SUCCESS;

    /*Chekc if the flag is reset. If yes return error.*/
    if ( mil_initialized != MIL_ENABLE )
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:DebugDisable] "\
                                "MIL is not enabled.\n");
        return MIL_NOT_ENABLED_ERROR;
    }
    /*Unregister the sen Handle*/
    status = icp_DccSenHandlerUnregister();
    if (ICP_STATUS_SUCCESS != status)
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:DebugDisable] SenHandlerUnregister "\
                                "failed (%d).\n", status);
        mil_initialized = MIL_DISABLE;
        return MIL_SEN_HANDLER_UNREGISTER_ERROR;
    }

    /*reset the mil disable flag*/
    mil_initialized = MIL_DISABLE;
    return MIL_SUCCESS;
}


/**
*****************************************************************************
* @ingroup MILKernelInternal
* API to get the data dump size
*
* @description
*  This function will get the data dump size from DCC
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
*      No
*
* @threadSafe
*      Yes
*
*
* @retval  MIL_SUCCESS If successful
*
* @retval  MIL_ERROR A valid mil_error_status_t for failure
*
* @pre
*      MIL must be enabled.
*
* @post
*      None.
*
*****************************************************************************/
mil_error_status_t GetDataDumpSize(void* arg)
{
    DataDumpSizeBuffer_t DumpSizeBuffer = {0};
    mil_error_status_t status = MIL_SUCCESS;
    
    if(mil_initialized != MIL_ENABLE)
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetDataDumpSize] MIL not enabled \n");
        return MIL_NOT_ENABLED_ERROR;
    }

    if ( icp_DccDataDumpInfoGet ( &DumpSizeBuffer.noOfModule , 
                                  &DumpSizeBuffer.maxDumpSize )
            != ICP_STATUS_SUCCESS)
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetDataDumpSize] " \
                                "icp_DccDataDumpInfoGet failed.\n");
        return MIL_DATA_DUMP_INFO_ERROR;
    } 
    status = mil_to_user((void *) *((int32_t *) arg), 
                         &DumpSizeBuffer, 
                         sizeof(DataDumpSizeBuffer_t));
    return status;
}


/**
*****************************************************************************
* @ingroup MILKernelInternal
* API to get version info size
*
* @description
*  This function will get version info size from DCC
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
*      No
*
* @threadSafe
*      Yes
*
*
* @retval  MIL_SUCCESS If successful
*
* @retval  MIL_ERROR A valid mil_error_status_t for failure
*
* @pre
*      MIL must be enabled.
*
* @post
*      None.
*
*****************************************************************************/
mil_error_status_t GetVersionInfoSize(void* arg)
{
    int32_t MILVersionInfoBufferSize = 0;
    mil_error_status_t status = MIL_SUCCESS;
    icp_status_t icp_status = ICP_STATUS_SUCCESS;
    
    if ( mil_initialized != MIL_ENABLE )
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetVersionInfoSize] "\
                                "MIL not enabled.\n");
        return MIL_NOT_ENABLED_ERROR;
    }

    icp_status = icp_DccVersionInfoSizeGet ( &MILVersionInfoBufferSize );
    if (icp_status != ICP_STATUS_SUCCESS) 
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetVersionInfoSize] "\
                                "icp_DccVersionInfoSizeGet failed.\n");
        return MIL_FAILURE;
    }
    MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetVersionInfoSize] dump_size = %d\n", \
                             MILVersionInfoBufferSize);
    MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetVersionInfoSize] arg = %p (%p, %x)\n",\
                            (void *) *((int32_t *) arg), arg, \
                            *((int32_t *) arg));
    status = mil_to_user((void *) *((int32_t *) arg),
                         (void *) &MILVersionInfoBufferSize,
                         sizeof(int32_t *));
    return status;
}


/**
*****************************************************************************
* @ingroup MILKernelInternal
* API to get liveness size
*
* @description
*  This function will get liveness size from DCC
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
*      No
*
* @threadSafe
*      Yes
*
*
* @retval  MIL_SUCCESS If successful
*
* @retval  MIL_ERROR A valid mil_error_status_t for failure
*
* @pre
*      MIL must be enabled.
*
* @post
*      None.
*
*****************************************************************************/
mil_error_status_t GetLivenessSize(void* arg)
{
    uint32_t LivenessResponseSize = 0;
    mil_error_status_t status = MIL_SUCCESS;

    if(mil_initialized != MIL_ENABLE)
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetLivenessSize] "\
                                "MIL is not enabled.\n");
        return MIL_NOT_ENABLED_ERROR;
    }
    if ( icp_DccLivenessResponseSizeGet ( &LivenessResponseSize )
            != ICP_STATUS_SUCCESS )
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetLivenessSize] "\
                                "icp_DccLivenessResponseSizeGet failed!\n");
        return MIL_LIVENESS_SIZE_GET_ERROR;
    }
    MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetLivenessSize] liveness info size "\
                            "is %d\n", LivenessResponseSize);
    status = mil_to_user((void *) *((int32_t *) arg), &LivenessResponseSize,
                         sizeof(uint32_t));
    return status;
}


/**
*****************************************************************************
* @ingroup MILKernelInternal
* proccesses commands from the ioctl calls
*
* @description
*  This function proccesses commands from the ioctl calls
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
*      No
*
* @threadSafe
*      Yes
*
*
* @retval  MIL_SUCCESS If successful
*
* @retval  MIL_ERROR A valid mil_error_status_t for failure
*
* @pre
*      None.
* @post
*      None.
*
*****************************************************************************/
mil_error_status_t
SendCommand(u_long mil_command, void* arg)
{
    mil_error_status_t sent_status = MIL_SUCCESS;

    MIL_KERNEL_DEBUG_PRINT("[MIL_ks:SendCommand] SendCommand(%lu, %p)\n", \
                            mil_command, arg);
    
    switch ( mil_command)
    {
    case IOCTL_CMD_MIL_INTERNAL_DEBUG_ENABLE:
        sent_status = DebugEnable ();
        break;

    case IOCTL_CMD_MIL_DEBUG_DISABLE:
        sent_status = DebugDisable ();
        break;

    case IOCTL_CMD_MIL_VERSION_INFO_GET_SIZE:
        sent_status = GetVersionInfoSize(arg);
        break;

    case IOCTL_CMD_MIL_INTERNAL_VERSION_GET:
        sent_status = GetVersionInfoAll (arg);
        break;

    case IOCTL_CMD_MIL_LIVENESS_CONFIGURE_TIMEOUT:
        sent_status = ConfigureLivenessTimeout (arg);
        break;

    case IOCTL_CMD_MIL_INTERNAL_LIVENESS_VERIFY:
        sent_status = LivenessVerify (arg);
        break;

    case IOCTL_CMD_MIL_INTERNAL_DATA_DUMP_GET:
        sent_status = GetDumpData (arg);
        break;

    case IOCTL_CMD_MIL_FILENAME_GET:
        sent_status = GetFileName(arg);
        break;

    case IOCTL_CMD_MIL_FILENAME_SET:
        sent_status = SetFileName(arg);
        break;

    case IOCTL_CMD_MIL_LIVENESS_GET_SIZE:
        sent_status = GetLivenessSize(arg);
        break;

    case IOCTL_CMD_MIL_DATA_DUMP_GET_SIZE:
        sent_status = GetDataDumpSize(arg);
        break;

    default:
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:SendCommand] unrecognized command\n");
        sent_status = MIL_UNREACHABLE_CODE;
        break;
    }
    MIL_KERNEL_DEBUG_PRINT("[MIL_ks:SendCommand] return = %d\n", sent_status);
return sent_status;
}


/**
*****************************************************************************
* @ingroup MILKernelInternal
* API to set the log filename received from the user
*
* @description
*  This function will set the log filename received from the user
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
*      This function is synchronous and nonblocking.
*
* @reentrant
*      No
*
* @threadSafe
*      No
*
*
* @retval  MIL_SUCCESS If successful
*
* @retval  MIL_ERROR A valid mil_error_status_t for failure
*
* @pre
*      None.
*
* @post
*      None.
*
*****************************************************************************/
mil_error_status_t
SetFileName(void* arg)
{
    mil_error_status_t status = MIL_SUCCESS;
    uint32_t *ptemp_file_name =(int32_t *) &logFilename;

    MIL_KERNEL_DEBUG_PRINT("[MIL_ks:SetFileName] file = %s\n", \
                            (char *) ptemp_file_name);
    MIL_KERNEL_DEBUG_PRINT("[MIL_ks:SetFileName] *arg = %p\n", \
                            (void *) *((int32_t *) arg));
    
    MIL_MEMSET(ptemp_file_name, '\0', MAX_FILENAME_SIZE);
    status = mil_from_user((void *) ptemp_file_name, \
                            (void *) *((int32_t *) arg), MAX_FILENAME_SIZE);

    MIL_KERNEL_DEBUG_PRINT("[MIL_ks:SetFileName] copy from user: %d\n",status);
    MIL_KERNEL_DEBUG_PRINT("[MIL_ks:SetFileName] filename = %s\n", \
                            (char *) logFilename);
    return  status;
}


/**
*****************************************************************************
* @ingroup MILKernelInternal
* API to get the log filename
*
* @description
*  This function will get the log filename
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
*      This function is synchronous and nonblocking.
*
* @reentrant
*      No
*
* @threadSafe
*      No
*
*
* @retval  MIL_SUCCESS If successful
*
* @retval  MIL_ERROR A valid mil_error_status_t for failure
*
* @pre
*      None.
*
* @post
*      None.
*
*****************************************************************************/
mil_error_status_t
GetFileName(void* arg)
{
    mil_error_status_t status = MIL_SUCCESS;
    uint8_t *ptemp_file_name = &logFilename[0];

    MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetFileName] file = %s\n",ptemp_file_name);
    MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetFileName] arg = %p\n", \
                            (void *) *((int32_t *) arg));

    status = mil_to_user((void *) *((int32_t *) arg), \
                        (void *) ptemp_file_name, MAX_FILENAME_SIZE);

    MIL_KERNEL_DEBUG_PRINT("[MIL_ks:GetFileName] copy to user: %d\n", status);
    return status;
}

