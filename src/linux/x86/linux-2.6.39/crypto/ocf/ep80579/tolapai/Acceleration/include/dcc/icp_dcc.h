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
#if !defined(__ICP_DCC_H__)
#define __ICP_DCC_H__

#if defined(__cplusplus)
extern "C"
{
#endif /* end defined(__cplusplus) */

/**
 *****************************************************************************
 * 
 * @defgroup DCCClient Debug Common Component API
 *
 * @ingroup DCCLibrary
 *
 * @description
 *         This section describes the data structures, typedefs, and functions 
 *         used by the Debug clients. 
 *      
 * 
 *****************************************************************************/


/* Include Header Files */
#include "icp.h"
#include "icp_dcc_common.h"


/**
 *****************************************************************************
 * @ingroup DCCClient
 *    Thread response Status .
 * @description
 *    This value denotes the response status of the specified thread ID.
 * @purpose
 *    During response check, the response status of each thread is returned 
 *    through this structure. 
 *****************************************************************************/
typedef struct icp_dcc_liveness_status_s
{
    icp_dcc_thread_id_t threadId;          /**< Thread ID registered */
    icp_dcc_thread_status_t threadStatus;    /**< ICP_DCC_THREAD_ID_LIVE or
                                                  ICP_DCC_THREAD_ID_DEAD */
} icp_dcc_liveness_status_t;


/**
 *****************************************************************************
 * @ingroup DCCClient
 *    Structure of Package version in buffer returned to Debug Client.
 *
 * @description
 *    This structure provides Package version information in buffer  
 *    provided by the Debug Client.
 *
 * @purpose
 *    When queried for software version, the version information of the 
 *    Package is returned in this format.
 *
 *****************************************************************************/
typedef struct icp_dcc_package_ver_s
{
    uint8_t     packageSize;
    /**< Number of bytes for the package version */
    uint8_t     packageName[ICP_DCC_COMPONENT_NAME_LENGTH];
    /**< Package name as per Version Information structure */
    uint8_t     packageVersion[ICP_DCC_VERSION_STR_LENGTH];
    /**< Package version information in the form of a string */
} icp_dcc_package_ver_t;

/**
 *****************************************************************************
 * @ingroup DCCClient
 *    Structure of Module version in buffer returned to Debug Client.
 *
 * @description
 *    This structure contains the Module version information. This 
 *    information is returned in the buffer provided by the Debug Client.
 *
 * @purpose
 *    When queried for software version, the version information of the
 *    registered Modules are returned in this format.
 *
 *****************************************************************************/
typedef struct icp_dcc_module_ver_s
{
    uint8_t     componentSize;
    /**< Number of bytes for the component version */
    uint32_t    moduleId;
    /**< Module ID */ 
    uint8_t     moduleName[ICP_DCC_COMPONENT_NAME_LENGTH];
    /**< Module name as per Version Information structure */
    uint8_t     moduleVersion[ICP_DCC_VERSION_STR_LENGTH];
    /**< Module version information in the form of a string */
} icp_dcc_module_ver_t;

/**
 *****************************************************************************
 * @ingroup DCCClient
 *    Structure of Version buffer returned to Debug Client.
 *
 * @description
 *    This structure provides Package/Module version information in the
 *    buffer provided by Debug Client.
 *
 * @purpose
 *    When queried for software version, the version information of the
 *    Package, as well as all registered Modules is returned in this format.
 *
 *****************************************************************************/
typedef struct icp_dcc_version_buffer_s
{
    uint32_t    numModules; 
    /**< Count of number of module version fields within the buffer */ 
    icp_dcc_package_ver_t   packageVerInfo;
    /**< Package version information as provided to client */
    icp_dcc_module_ver_t    moduleVerInfo[];
    /**< Module version information as provided to client */
} icp_dcc_version_buffer_t;


/* Version Information */
/**
 *****************************************************************************
 * @ingroup DCCClient
 *    Get the maximum buffer size needed to retrieve Version information of
 *    the Package and all the Components registered with the DCC.
 * 
 * @description
 *    This function provides the DCC Client with buffer size needed to retrieve
 *    Version information of all the Package/Components registered with DCC.
 *    The DCC user should allocate this buffer and pass it to DCC to retrieve 
 *    the Package and Components version information.
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
 * @param  pVersionBufferSize: IN/OUT: DCC Package and Component information
 *
 * @retval  ICP_STATUS_SUCCESS Operation successful
 * @retval  ICP_STATUS_FAIL Operation failed 
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters are NULL
 *
 * @pre
 *      None
 *
 * @post
 *      None
 *
 *****************************************************************************/
icp_status_t 
icp_DccVersionInfoSizeGet (
       uint32_t *pVersionBufferSize );


/**
 *****************************************************************************
 * @ingroup DCCClient
 *        Returns Package and Components version information.
 * 
 * @description
 *         This function provides the caller with version information for the 
 *         Package and each of the Components registered with the DCC.
 *
 * @context
 *      This function runs in the context of the calling function thread.
 *
 * @assumptions
 *      This function should be called with the required amount of buffer 
 *      allocated for the returned information.
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
 * @param  pVerInfo: IN/OUT: The array containing the version information for
 *             the Package and for all the components registered with the DCC.
 *             This is a pointer to the array of "icp_dcc_version_buffer_t".
 * @param  pVersionBufferSize: IN/OUT: 
 *             While calling this API, the user specifies
 *             the amount of memory allocated which can be used by the DCC to 
 *             populate the version information for all the components
 *             registered with the DCC.
 *             The DCC checks if the buffer size specified can accommodate the 
 *             version information retrieved. If so, the DCC fills in the 
 *             component version information for passing back to the user. 
 *             Typically, the size should be the same as that provided when 
 *             the user calls the "icp_DccVersionInfoSizeGet" function. 
 *             In the returned information, the first data corresponds to
 *             Package information and subsequent data corresponds to 
 *             individual component versions.
 *
 * @retval  ICP_STATUS_SUCCESS Operation successful
 * @retval  ICP_STATUS_FAIL Operation failed 
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters are NULL
 *
 * @pre
 *      None
 *
 * @post
 *      None
 *
 * @see icp_DccVersionInfoSizeGet
 * @see icp_dcc_version_buffer_t
 *****************************************************************************/
icp_status_t
icp_DccSoftwareVersionGet (
             uint8_t *pVerInfo,
             uint32_t *pVersionBufferSize );



/* System response */

/* Management Interface */

/**
 ******************************************************************************
 * @ingroup DCCClient
 *        Set the response timeout value for DCC.
 * 
 * @description
 *         This API is used to configure the timeout period (in ms) for 
 *         response monitoring. The DCC waits for a response during this period
 *         before declaring a thread to be alive or dead.
 *         The timeout period should be large enough to allow all threads to 
 *         respond appropriately within this period.
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
 * @param  livenessTimeOut: IN: Timeout value
 *
 * @retval  ICP_STATUS_SUCCESS Operation successful
 * @retval  ICP_STATUS_FAIL Operation failed 
 *
 * @pre
 *      None
 *
 * @post
 *      None
 *
 *****************************************************************************/
icp_status_t
icp_DccLivenessConfigureTimeout (
        uint32_t livenessTimeOut);

/**
 *****************************************************************************
 * @ingroup DCCClient
 *        Get the buffer size to be allocated by the DCC user for getting the 
 *        system response information.
 * 
 * @description
 *         The DCC user uses this API to get the buffer size to be allocated 
 *         for the retrieval of system response information.
 *         The DCC user should free this buffer after the buffer information is
 *         processed. 
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
 *                                 while quering the response status
 *
 * @retval  ICP_STATUS_SUCCESS Operation successful
 * @retval  ICP_STATUS_FAIL Operation failed 
 *
 * @pre
 *      None
 *
 * @post
 *      None
 *
 *****************************************************************************/
icp_status_t
icp_DccLivenessResponseSizeGet (
        uint32_t *pLivenessResponseSize);

/**
 *****************************************************************************
 * @ingroup DCCClient
 *        Verify System response.
 * 
 * @description
 *         The DCC user uses this API to verify response of all threads of 
 *         execution in the system. The user provides the required buffer which
 *         the DCC fills with Response status information. 
 *
 * @context
 *      This function runs in the context of the calling function thread. This
 *      should not be called in interrupt mode.
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
 * @param  pLivenessStatus: IN/OUT: The system threads execution status. This
 *          is a pointer to the array of structure icp_dcc_liveness_status_t.
 * @param  pBufferSize: IN/OUT: The buffer size allocated to return the 
 *                                  system response status.
 *
 * @retval  ICP_STATUS_SUCCESS Operation successful
 * @retval  ICP_STATUS_FAIL Operation failed 
 * @retval  ICP_STATUS_INVALID_PARAM Invalid parameter passed 
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters are NULL
 *
 * @pre
 *      None
 *
 * @post
 *      None
 *
 * @see icp_dcc_liveness_status_t
 * @see icp_DccLivenessResponseSizeGet
 *
 *****************************************************************************/
icp_status_t
icp_DccLivenessVerify (
        uint8_t *pLivenessStatus,
        uint32_t *pBufferSize);


/* DUMP */


/**
 *****************************************************************************
 * @ingroup DCCClient
 *        Get the number of Modules and the maximum buffer size required to 
 *        get a data dump.
 * 
 * @description
 *        This function returns the number of Modules and the maximum size 
 *        of buffer needed for any one dump query transaction.
 *        Before invoking the Data Dump query request, the DCC user gets the 
 *        maximum size of the buffer to allocate and the total number of Modules 
 *        that should be queried for the data dump.
 *
 * @context
 *      This function runs in the context of the calling function thread. This
 *      function should not be called from an interrupt context.
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
 * @param  noOfModules: IN/OUT: Number of Modules registered at DCC for 
 *                            Data Dump
 * @param  pDataDumpSize: IN/OUT: Maximum buffer size required to query any one
 *                            dump Module
 *
 * @retval  ICP_STATUS_SUCCESS Operation successful
 * @retval  ICP_STATUS_FAIL Operation failed 
 * @retval  ICP_STATUS_INVALID_PARAM Invalid parameter passed 
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters are NULL
 *
 * @pre
 *      None
 *
 * @post
 *      None
 *
 * @see 
 *
 *****************************************************************************/
icp_status_t
icp_DccDataDumpInfoGet (
        uint32_t *noOfModules,
        uint32_t *pDataDumpSize);

/**
 *****************************************************************************
 * @ingroup DCCClient
 *        Get the number of Modules and the maximum buffer size required to get 
 *        a data dump.
 * 
 * @description
 *        The User should use this function to dump the data structures of 
 *        Modules registered with the DCC. This function will be called once 
 *        for each registered Module for the data dump information. With each 
 *        call, the user passes to DCC a pre-allocated buffer which is 
 *        passed to the Module to be filled with the dump information.
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
 * @param  moduleId: IN: Opaque index representing the Module being 
 *                          queried for data dump
 * @param  pDataDump: IN/OUT: Data Dump information from the Module
 * @param  pDataDumpSize: IN/OUT: Actual size of dump data present in 
 *                          the buffer
 *
 * @retval  ICP_STATUS_SUCCESS Operation successful
 * @retval  ICP_STATUS_FAIL Operation failed 
 * @retval  ICP_STATUS_INVALID_PARAM Invalid parameter passed 
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters are NULL
 *
 * @pre
 *      None
 *
 * @post
 *      None
 *
 * @see icp_DccDataDumpInfoGet
 *
 *****************************************************************************/
icp_status_t
icp_DccDataDumpGet (
        uint32_t moduleId, 
        uint32_t *pDataDumpSize,
        uint8_t *pDataDump);



/* Client Interface */
/**
 *****************************************************************************
 * @ingroup DCCClient
 *        Prototype of SEN event callback handler to be registered by user.
 * 
 * @description
 *        The DCC user registers the SEN event callback handler with this 
 *        function. When a SEN event occurs, the DCC calls the handler 
 *        registered in this prototype format.
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
 * @param  pSenMsg: IN/OUT: SEN Event Message Details
 *
 * @retval  ICP_STATUS_SUCCESS Operation successful
 * @retval  ICP_STATUS_FAIL Operation failed 
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters are NULL
 *
 * @pre
 *      None
 *
 * @post
 *      None
 *
 * @see 
 *
 *****************************************************************************/
typedef void (*icp_DccSenHandler) (
        icp_dcc_sen_msg_t *pSenMsg);


/**
 *****************************************************************************
 * @ingroup DCCClient
 *        Register the SEN event callback handler.
 * 
 * @description
 *        The DCC user registers the SEN event callback handler with this 
 *        function.
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
 * @retval  ICP_STATUS_SUCCESS Operation successful
 * @retval  ICP_STATUS_FAIL Operation failed 
 *
 * @pre
 *      None
 *
 * @post
 *      None
 *
 * @see icp_DccSenHandler
 *
 *****************************************************************************/
icp_status_t
icp_DccSenHandlerRegister ( icp_DccSenHandler senHandler );


/**
 *****************************************************************************
 * @ingroup DCCClient
 *        Unregister the SEN event callback handler.
 * 
 * @description
 *        The DCC user unregisters the SEN event callback handler.
 *        
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
 * @retval  ICP_STATUS_SUCCESS Operation successful
 * @retval  ICP_STATUS_FAIL Operation failed 
 *
 * @pre
 *      None
 *
 * @post
 *      None
 *
 * @see 
 *
 *****************************************************************************/
icp_status_t
icp_DccSenHandlerUnregister ( void );

#if defined(__cplusplus)
}
#endif /* end defined(__cplusplus) */

#endif /* end !defined(__ICP_DCC_H__) */
