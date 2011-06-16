/***************************************************************************
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
 ***************************************************************************/

/*
 *****************************************************************************
 * Doxygen group definitions
 ****************************************************************************/

/**
 *****************************************************************************
 * @file cpa_cy_common.h
 * 
 * @defgroup cpaCy Cryptographic API.
 * 
 * @ingroup cpa
 *
 * @description
 *      These functions specify the Cryptographic API.
 *
 *****************************************************************************/

/**
 *****************************************************************************
 * @defgroup cpaCyCommon Cryptographic Common API.
 * @file cpa_cy_common.h
 * 
 * @ingroup cpaCy
 * 
 * @description
 *      This file specifies items which are common for both the asymmetric 
 *      (public key cryptography) and the symmetric operations for the
 *      Cryptographic API.
 * 
 *****************************************************************************/
#ifndef CPA_CY_COMMON_H
#define CPA_CY_COMMON_H

#include "cpa.h"

/**
 *****************************************************************************
 * @file cpa_cy_common.h
 * @ingroup cpa_cyCommon
 *      CPA CY Major Version Number
 * @description
 *      The CPA_CY API major version number. This number will be incremented
 *      when significant churn to the API has occurred. The combination of the
 *      major and minor number definitions represent the complete version number
 *      for this interface.
 *
 *****************************************************************************/
#define CPA_CY_API_VERSION_NUM_MAJOR (1)

/**
 *****************************************************************************
 * @file cpa_cy_common.h
 * @ingroup cpa_cyCommon
 *       CPA CY Minor Version Number
 * @description
 *      The CPA_CY API minor version number. This number will be incremented
 *      when minor changes to the API has occurred. The combination of the major
 *      and minor number definitions represent the complete version number for
 *      this interface.
 *
 *****************************************************************************/
#define CPA_CY_API_VERSION_NUM_MINOR (1)

/**
 *****************************************************************************
 * @file cpa_cy_common.h
 * @ingroup cpaCyCommon
 *      Request priority
 * @description
 *      Enumeration of priority of the request to be given to the API. 
 *      Currently two levels - HIGH and NORMAL are supported. HIGH priority
 *      requests will be prioritized on a "best-effort" basis over requests 
 *      that are marked with a NORMAL priority.
 *
 *****************************************************************************/
typedef enum
{
    CPA_CY_PRIORITY_NORMAL = 1, /**< Normal priority */
    CPA_CY_PRIORITY_HIGH /**< High priority */
} CpaCyPriority;

/*****************************************************************************/
/* Callback Definitions                                                      */
/*****************************************************************************/
/**
 *****************************************************************************
 * @file cpa_cy_common.h
 * @ingroup cpaCyCommon
 *      Definition of the crypto generic callback function  
 * 
 * @description
 *      This data structure specifies the prototype for a generic callback
 *      function
 * 
 * @context
 *      This callback function can be executed in a context that DOES NOT
 *      permit sleeping to occur. 
 * 
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param[in] pCallbackTag Opaque value provided by user while making individual
 *                         function call.
 * @param[in] status       Status of the operation. Valid values are  
 *                         CPA_STATUS_SUCCESS and CPA_STATUS_FAIL. 
 * @param[in] pOpData      Opaque Pointer to the operation data that was  
 *                         submitted in the request
 * 
 * @retval
 *      None
 * @pre
 *      Component has been initialized.
 * @post
 *      None
 * @note
 *      None
 * @see
 *      cpaCyKeyGenSsl()
 *
 *****************************************************************************/
typedef void (*CpaCyGenericCbFunc)(void *pCallbackTag,
        CpaStatus status,
        void *pOpData);

/**
 *****************************************************************************
 * @file cpa_cy_common.h
 * @ingroup cpaCyCommon
 *      Definition of generic callback function with an additional output
 *      CpaFlatBuffer parameter.
 * 
 * @description
 *      This data structure specifies the prototype for a generic callback
 *      function which provides an output buffer (of type CpaFlatBuffer). 
 * 
 * @context
 *      This callback function can be executed in a context that DOES NOT
 *      permit sleeping to occur. 
 * 
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param[in] pCallbackTag Opaque value provided by user while making individual
 *                         function call.
 * @param[in] status       Status of the operation. Valid values are  
 *                         CPA_STATUS_SUCCESS and CPA_STATUS_FAIL. 
 * @param[in] pOpData      Opaque Pointer to the operation data that was  
 *                         submitted in the request
 * @param[in] pOut         Pointer to the output buffer provided in the request
 *                         invoking this callback. 
 * 
 * @retval
 *      None
 * @pre
 *      Component has been initialized.
 * @post
 *      None
 * @note
 *      None
 * @see
 *      None
 *
 *****************************************************************************/
typedef void (*CpaCyGenFlatBufCbFunc)(void *pCallbackTag,
        CpaStatus status,
        void *pOpdata,
        CpaFlatBuffer *pOut);

/**
 *****************************************************************************
 * @file cpa_cy_common.h
 * @ingroup cpaCyCommon
 *      Function to return the size of the memory which must be allocated for 
 *      the pPrivateMetaData member of CpaBufferList.
 *      
 * @description
 *      This function is used obtain the size (in bytes) required to allocate 
 *      a buffer descriptor for the pPrivateMetaData member in the 
 *      CpaBufferList the structure.
 *      Should the function return zero then no meta data is required for the
 *      buffer list.
 *
 * @context
 *      This function may be called from any context.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 *
 * @param[in]  instanceHandle      Handle to an instance of this API.
 * @param[in]  numBuffers          The number of pointers in the CpaBufferList.
 *                                 this is the maximum number of CpaFlatBuffers
 *                                 which may be contained in this CpaBufferList.
 * @param[out] pSizeInBytes        Pointer to the size in bytes of memory to be
 *                                 allocated when the client wishes to allocate
 *                                 a cpaFlatBuffer
 * 
 * @retval CPA_STATUS_SUCCESS        Function executed successfully.
 * @retval CPA_STATUS_FAIL           Function failed.
 * @retval CPA_STATUS_INVALID_PARAM  Invalid parameter passed in.
 * 
 * @pre
 *      None.
 * @post
 *      None
 * @note
 *      None
 * @see
 *      cpaCyGetInstances()
 *
 *****************************************************************************/
CpaStatus 
cpaCyBufferListGetMetaSize(const CpaInstanceHandle instanceHandle,
        Cpa32U numBuffers,
        Cpa32U *pSizeInBytes);

/**
 *****************************************************************************
 * @file cpa_cy_common.h
 * @ingroup cpaCyCommon
 *      Function to return a string indicating the specific error that occurred
 *      for a particular instance.
 *      
 * @description
 *      When a function invocation on a particular instance returns an error,
 *      the client can invoke this function to query the instance for a null
 *      terminated string which describes the general error condition, and if
 *      available additional text on the specific error.
 *      The Client MUST allocate CPA_STATUS_MAX_STR_LENGTH_IN_BYTES bytes for 
 *      the buffer string.  
 *
 * @context
 *      This function may be called from any context.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 *
 * @param[in]  instanceHandle   Handle to an instance of this API.
 * @param[in]  errStatus        The error condition that occurred
 * @param[out] pStatusText      Pointer to the string buffer that will be
 *                              updated with a null terminated status text
 *                              string.
 *                              The invoking application MUST allocate this
 *                              buffer to be CPA_STATUS_MAX_STR_LENGTH_IN_BYTES.
 *
 * @retval CPA_STATUS_SUCCESS        Function executed successfully.
 * @retval CPA_STATUS_FAIL           Function failed. Note, In this scenario it 
 *                                   is INVALID to call this function a further
 *                                   time.
 * @retval CPA_STATUS_INVALID_PARAM  Invalid parameter passed in.
 * 
 * @pre
 *      None.
 * @post
 *      None
 * @note
 *      None
 * @see
 *      CpaStatus
 *
 *****************************************************************************/
CpaStatus
cpaCyGetStatusText(const CpaInstanceHandle instanceHandle,
        CpaStatus errStatus,
        Cpa8S *pStatusText);

/*****************************************************************************/
/* Instance Discovery Functions                                              */
/*****************************************************************************/
/**
 *****************************************************************************
 * @file cpa_cy_common.h
 * @ingroup cpaCyCommon
 *      Get the number of instances that are supported by the API
 *      implementation. 
 * 
 * @description
 *
 *     This function will get the number of instances that are supported
 *     by an implementation of the Crytpo API. This number is then used to 
 *     determine the size of the array that must be passed to 
 *     cpaCyGetInstances().
 * 
 * @context
 *      This function MUST NOT be called from an interrupt context as it MAY
 *      sleep. 
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      This function is synchronous and blocking.
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param[out] pNumInstances         Pointer to where the number of 
 *                                   instances will be written.
 * 
 * @retval CPA_STATUS_SUCCESS        Function executed successfully.
 * @retval CPA_STATUS_FAIL           Function failed.
 * @retval CPA_STATUS_INVALID_PARAM  Invalid parameter passed in.
 * 
 * @pre
 *      None
 * @post
 *      None
 * @note
 *      This function operates in a synchronous manner and no asynchronous
 *      callback will be generated
 *
 * @see
 *      cpaCyGetInstances
 * 
 *****************************************************************************/
CpaStatus 
cpaCyGetNumInstances(Cpa16U *pNumInstances);

/**
 *****************************************************************************
 * @file cpa_cy_common.h
 * @ingroup cpaCyCommon
 *      Get the handles to the instances that are supported by the 
 *      API implementation. 
 * 
 * @description
 *
 *      This function will return handles to the instances that are 
 *      supported by an implementation of the Crypto API. These instance
 *      handles can then be used as input parameters with other Crytpo API 
 *      functions. 
 * 
 *      This function will populate an array that has been allocated by the 
 *      caller. The size of this API will have been determined by the 
 *      cpaCyGetNumInstances() function.
 * 
 * @context
 *      This function MUST NOT be called from an interrupt context as it MAY
 *      sleep. 
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      This function is synchronous and blocking.
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param[in]     numInstances       Size of the array.
 * @param[in,out] cyInstances        Pointer to where the instance 
 *                                   handles will be written.
 * 
 * @retval CPA_STATUS_SUCCESS        Function executed successfully.
 * @retval CPA_STATUS_FAIL           Function failed.
 * @retval CPA_STATUS_INVALID_PARAM  Invalid parameter passed in.
 * 
 * @pre
 *      None
 * @post
 *      None
 * @note
 *      This function operates in a synchronous manner and no asynchronous
 *      callback will be generated
 *
 * @see
 *      cpaCyGetNumInstances
 * 
 *****************************************************************************/
CpaStatus 
cpaCyGetInstances(Cpa16U numInstances,
        CpaInstanceHandle *cyInstances);

/**
 *****************************************************************************
 * @file cpa_cy_common.h
 * @ingroup cpaCyCommon
 *      Function to get information on a particular instance.
 *
 * @description
 *      This function will provide instance specific information through a
 *      @ref CpaInstanceInfo structure. 
 *
 * @context
 *      This function will be executed in a context that requires that sleeping
 *      MUST NOT be permitted.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 *
 * @param[in]  instanceHandle       Handle to an instance of this API to be 
 *                                  initialized.
 * @param[out] pInstanceInfo        Pointer to the memory location allocated by
 *                                  the client into which the CpaInstanceInfo
 *                                  structure will be written.
 * 
 * @retval CPA_STATUS_SUCCESS        Function executed successfully.
 * @retval CPA_STATUS_FAIL           Function failed.
 * @retval CPA_STATUS_INVALID_PARAM  Invalid parameter passed in.
 * 
 * @pre
 *      The client has retrieved an instanceHandle from successive calls to
 *      @ref cpaCyGetNumInstances and @ref cpaCyGetInstances.
 * @post
 *      None
 * @note
 *      None
 * @see
 *      cpaCyGetNumInstances,
 *      cpaCyGetInstances,
 *      CpaInstanceInfo
 *
 *****************************************************************************/
CpaStatus 
cpaCyInstanceGetInfo(const CpaInstanceHandle instanceHandle,
        CpaInstanceInfo * pInstanceInfo);

/*****************************************************************************/
/* Instance Notification Functions                                           */
/*****************************************************************************/
/**
 *****************************************************************************
 * @file cpa_cy_common.h
 * @ingroup cpaCyCommon
 *      Callback function for instance notification support.
 *
 * @description
 *      This is the prototype for the instance notification callback function.
 *      The callback function is passed in as a parameter to the 
 *      @ref cpaInstanceSetNotificationCb function.
 *
 * @context
 *      This function will be executed in a context that requires that sleeping
 *      MUST NOT be permitted.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 *
 * @param[in] pCallbackTag     Opaque value provided by user while making
 *                             individual function calls.
 * @param[in] instanceEvent    The event that will trigger this function to
 *                             get invoked.
 * 
 * @retval
 *      None
 * @pre
 *      Component has been initialized and the notification function has been
 *  set via the cpaInstanceSetNotificationCb function.
 * @post
 *      None
 * @note
 *      None
 * @see
 *      cpaInstanceSetNotificationCb(),
 *
 *****************************************************************************/
typedef void (*CpaCyInstanceNotificationCbFunc)(void * pCallbackTag,
        const CpaInstanceEvent instanceEvent);

/**
 *****************************************************************************
 * @file cpa_cy_common.h
 * @ingroup cpaCyCommon
 *      Subscribe for instance notifications.
 *
 * @description
 *      Clients of the CpaCy interface can subscribe for instance notifications
 *      by registering a @ref CpaCyInstanceNotificationCbFunc function.
 *
 * @context
 *      This function will be executed in a context that requires that sleeping
 *      MUST NOT be permitted.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 *
 * @param[in] pInstanceNotificationCb  Instance notification callback
 *                                     function pointer.
 * @param[in] pCallbackTag             Opaque value provided by user while
 *                                     making individual function calls.
 * 
 * @retval CPA_STATUS_SUCCESS          Function executed successfully.
 * @retval CPA_STATUS_FAIL             Function failed.
 * @retval CPA_STATUS_INVALID_PARAM    Invalid parameter passed in.
 * 
 * @pre
 *      Instance has been initialized.
 * @post
 *      None
 * @note
 *      None
 * @see
 *      CpaCyInstanceNotificationCbFunc
 *
 *****************************************************************************/
CpaStatus
cpaCyInstanceSetNotificationCb(
        const CpaCyInstanceNotificationCbFunc pInstanceNotificationCb,
        void *pCallbackTag);

#endif /* CPA_CY_COMMON_H */
