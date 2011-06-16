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
 * @file cpa.h
 * 
 * @defgroup cpa CPA API
 * 
 * @description
 *      This is the top level API definition. It contains structures, data
 *      types and definitions that are common across the interface.
 * 
 *****************************************************************************/

/**
 *****************************************************************************
 * @defgroup cpa_BaseDataTypes Base Data Types
 * 
 * @ingroup cpa
 * 
 * @description
 *      The base data types for the Intel CPA API.
 * 
 *****************************************************************************/

#ifndef CPA_H
#define CPA_H

#include "cpa_types.h"

/**
 *****************************************************************************
 * @ingroup cpa_BaseDataTypes
 *      Instance handle type
 * 
 * @description
 *      Handle used to uniquely identify an instance.
 *
 * @note
 *      Where only a single instantiation exists this field must be set to
 * @ref CPA_INSTANCE_HANDLE_DEFAULT.
 *
 *****************************************************************************/
typedef void * CpaInstanceHandle;

/**
 *****************************************************************************
 * @ingroup cpa_BaseDataTypes
 *      Default instantiation handle value where there is only a single instance
 * 
 * @description
 *      Used as an instance handle value where only one instance exists.
 *
 *****************************************************************************/
#define CPA_INSTANCE_HANDLE_SINGLE ((CpaInstanceHandle)0)

/**
 *****************************************************************************
 * @ingroup cpa_BaseDataTypes
 *      Flat buffer structure containing a pointer and length member.
 * 
 * @description
 *      A flat buffer structure. The data pointer, pData, is a virtual address 
 *      however the actual data pointed to is required to be in contiguous 
 *      physical memory. It is expected that this buffer handle will be used
 *      when simple, unchained buffers are needed. 
 *
 *****************************************************************************/
typedef struct _CpaFlatBuffer {
    Cpa32U dataLenInBytes;
    /**< Data length specified in bytes*/
	Cpa8U *pData;
    /**< The data pointer is a virtual address, however the actual data pointed
     * to is required to be in contiguous physical memory. */
} CpaFlatBuffer;

/**
 *****************************************************************************
 * @ingroup cpa_BaseDataTypes
 *      Scatter/Gather buffer list containing an array of Simple buffers.
 * 
 * @description
 *      A Scatter/Gather buffer list structure. It is expected that this buffer
 *      structure will be used where more than one flat buffer can be provided
 *      on an particular API.
 *
 *      IMPORTANT - The memory for the pPrivateMetaData member must be allocated 
 *      by the client as contiguous memory.  When allocating memory for 
 *      pPrivateMetaData a call to cpaCyBufferListGetMetaSize MUST be made to 
 *      determine the size of the Meta Data Buffer.  The returned size
 *      (in bytes) may then be passed in a memory allocation routine to allocate 
 *      the pPrivateMetaData memory.
 *
 *****************************************************************************/
typedef struct _CpaBufferList {
    Cpa32U numBuffers;
    /**< Number of pointers */
    CpaFlatBuffer *pBuffers;
    /**< Pointer to an unbounded array containing the number of CpaFlatBuffers
     * defined by numBuffers */
    void *pUserData;
    /**< This is an opaque field that is not read or modified internally. */
    void *pPrivateMetaData;
    /**< Private Meta representation of this buffer List - the memory for this
     * buffer needs to be allocated by the client as contiguous data.
     * The amount of memory required is returned with a call to 
     * cpaCyBufferListGetMetaSize. If cpaCyBufferListGetMetaSize returns a size
     * of zero no memory needs to be allocated, and this parameter can be NULL.
     */    
} CpaBufferList;

/**
 *****************************************************************************
 * @ingroup cpa_BaseDataTypes
 *      API status value type definition
 * @description
 *      This type definition is used for the return values used in all the
 *      API functions.  Common values are #defined, for example see 
 *      @ref CPA_STATUS_SUCCESS, @ref CPA_STATUS_FAIL, etc.
 *****************************************************************************/
typedef Cpa32S CpaStatus;

#define CPA_STATUS_SUCCESS (0)
/**< @ingroup cpa_BaseDataTypes
 *   Success status value. */
#define CPA_STATUS_FAIL (-1)
/**< @ingroup cpa_BaseDataTypes
 *   Fail status value. */
#define CPA_STATUS_RETRY (-2)
/**< @ingroup cpa_BaseDataTypes
 *   Retry status value. */
#define CPA_STATUS_RESOURCE (-3)
/**< @ingroup cpa_BaseDataTypes
 *   The resource that has been requested is unavailable. Refer
 *   to relevant sections of the API for specifics on what the suggested
 *   course of action is. */
#define CPA_STATUS_INVALID_PARAM (-4)
/**< @ingroup cpa_BaseDataTypes
 *   Invalid parameter has been passed in. */
#define CPA_STATUS_FATAL (-5)
/**< @ingroup cpa_BaseDataTypes
 *   A serious error has occurred. Recommended course of action
 *   is to shutdown and restart the component. */

/**
 *****************************************************************************
 * @ingroup cpa_BaseDataTypes
 *      API status string type definition
 * @description
 *      This type definition is used for the generic status text strings 
 *      provided by cpaXxGetStatusText API functions.  Common values are
 *      #defined, for example see @ref CPA_STATUS_STR_SUCCESS, 
 *      @ref CPA_STATUS_FAIL, etc., as well as the maximum size
 *      @ref CPA_STATUS_MAX_STR_LENGTH_IN_BYTES.
 *****************************************************************************/
#define CPA_STATUS_MAX_STR_LENGTH_IN_BYTES (255)
/**< @ingroup cpa_BaseDataTypes
 *   Maximum length of the Overall Status String (including generic and specific 
 *   strings returned by calls to cpaXxGetStatusText */

#define CPA_STATUS_STR_SUCCESS       ("Operation was successful:")
/**< @ingroup cpa_BaseDataTypes
 *   Status string for @ref CPA_STATUS_SUCCESS. */
#define CPA_STATUS_STR_FAIL          ("General or unspecified error occurred:")
/**< @ingroup cpa_BaseDataTypes
 *   Status string for @ref CPA_STATUS_FAIL. */
#define CPA_STATUS_STR_RETRY         ("Recoverable Error occurred:")
/**< @ingroup cpa_BaseDataTypes
 *   Status string for @ref CPA_STATUS_RETRY. */
#define CPA_STATUS_STR_RESOURCE      ("Required resource unavailable:")
/**< @ingroup cpa_BaseDataTypes
 *   Status string for @ref CPA_STATUS_RESOURCE. */
#define CPA_STATUS_STR_INVALID_PARAM ("Invalid Parameter supplied:")
/**< @ingroup cpa_BaseDataTypes
 *   Status string for @ref CPA_STATUS_INVALID_PARAM. */
#define CPA_STATUS_STR_FATAL         ("A Fatal error has occurred:")
/**< @ingroup cpa_BaseDataTypes
 *   Status string for @ref CPA_STATUS_FATAL. */

/**
 *****************************************************************************
 * @ingroup cpa_BaseDataTypes
 *      Instance Types
 * @description
 *      Enumeration of the different instance types.
 *
 *****************************************************************************/
typedef enum _CpaInstanceType
{
    CPA_INSTANCE_TYPE_CRYPTO = 0,
    /**< Cryptograhic instance type. */
    CPA_INSTANCE_TYPE_DATA_COMPRESSION,
    /**< Data compression instance type. */
    CPA_INSTANCE_TYPE_RAID,
    /**< RAID instance type. */
    CPA_INSTANCE_TYPE_XML,
    /**< XML instance type. */
    CPA_INSTANCE_TYPE_REGEX
    /**< Regular Expression instance type. */
}CpaInstanceType;

/**
 *****************************************************************************
 * @ingroup cpa_BaseDataTypes
 *      Instance State
 * @description
 *      Enumeration of the different instance states that are possible.
 *
 *****************************************************************************/
typedef enum _CpaInstanceState
{
    CPA_INSTANCE_STATE_INITIALISED = 0,
    /**< Instance is in the initialized state and ready for use. */
    CPA_INSTANCE_STATE_SHUTDOWN
    /**< Instance is in the shutdown state and not available for use. */
} CpaInstanceState;

/**
 *****************************************************************************
 * @ingroup cpa_BaseDataTypes
 *      Instance Info Max string lengths
 * @description
 *      Definitions of the instance info max string lengths.
 *
 *****************************************************************************/
#define CPA_INSTANCE_MAX_NAME_SIZE_IN_BYTES 64
/**< @ingroup cpa_BaseDataTypes
 * Maximum instance info name string length in bytes */
#define CPA_INSTANCE_MAX_VERSION_SIZE_IN_BYTES 64
/**< @ingroup cpa_BaseDataTypes
 * Maximum instance info version string length in bytes */

/**
 *****************************************************************************
 * @ingroup cpa_BaseDataTypes
 *      Instance Info Structure
 * @description
 *      Structure that contains the information to describe the instance.
 *
 *****************************************************************************/
typedef struct _CpaInstanceInfo {
    CpaInstanceType type;
    /**< Type definition for this instance. */
    CpaInstanceState state;
    /**< Operational state of the instance. */
    Cpa8U name[CPA_INSTANCE_MAX_NAME_SIZE_IN_BYTES];
    /**< Simple text string identifier for the instance. */
    Cpa8U version[CPA_INSTANCE_MAX_VERSION_SIZE_IN_BYTES];
    /**< Version string. There may be multiple versions of the same type of 
     * instance accessible through a particular library. */
} CpaInstanceInfo;

/**
 *****************************************************************************
 * @ingroup cpa_BaseDataTypes
 *      Instance Events
 * @description
 *      Enumeration of the different events that will cause the registered 
 *  Instance notification callback function to be invoked.
 *
 *****************************************************************************/
typedef enum _CpaInstanceEvent
{
    CPA_INSTANCE_EVENT_CREATION = 0,
    /**< Event type that triggers the registered instance notification callback
     * function when an instance is created. */
    CPA_INSTANCE_EVENT_DELETION
    /**< Event type that triggers the registered instance notification callback
     * function when an instance is deleted. */
} CpaInstanceEvent;

#endif /* CPA_H */
