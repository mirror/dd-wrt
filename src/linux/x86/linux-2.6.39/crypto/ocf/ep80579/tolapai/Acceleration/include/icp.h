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
 * @file icp.h
 * 
 * @defgroup icp Acceleration API
 * 
 * @description
 *      This is the top level API definition. It contains structures, data
 *      types and definitions that are common across the other interfaces.
 * 
 *****************************************************************************/

/**
 *****************************************************************************
 * @defgroup icp_BaseDataTypes Base Data Types
 * 
 * @ingroup icp
 * 
 * @description
 *      The base data types for the Intel Acceleration API.
 * 
 *****************************************************************************/

#ifndef ICP_H
#define ICP_H

#include "icp_osal_types.h"

/**
 *****************************************************************************
 * @ingroup icp_BaseDataTypes
 *      NULL definition
 *
 * @description
 *      This define is used to identify a NULL value. 
 * 
 *****************************************************************************/
#ifndef NULL
#define NULL (0)
#endif

#ifndef TRUE
#define TRUE (1)
/**< @ingroup icp_BaseDataTypes
 *   True value definition */
#endif
#ifndef FALSE
#define FALSE (0)
/**< @ingroup icp_BaseDataTypes
 *   False value definition */
#endif

/**
 *****************************************************************************
 * @ingroup icp_BaseDataTypes
 *      Boolean type 
 *
 * @description
 *      Functions in this API use this type for Boolean variables that take 
 *      true or false values. 
 * 
 *****************************************************************************/
typedef enum 
{
    ICP_FALSE = FALSE,      /**< False value */
    ICP_TRUE = TRUE,        /**< True value */
} icp_boolean_t;

/**
 *****************************************************************************
 * @ingroup icp_BaseDataTypes
 *      User provided representation of the context that is returned unmodified.
 *
 * @description
 *      This type defines an opaque user context passed in by the user when 
 *      the callback handler is registered. It is returned to the user when
 *      the callback function is invoked. This value is not modified or used by
 *      the acceleration components. It may be used to allow the application to
 *      associate a completion callback call with a specific instance of the 
 *      original function call.
 * 
 *****************************************************************************/
typedef void * icp_user_context_t;   

/**
 *****************************************************************************
 * @ingroup icp_BaseDataTypes
 *      User provided correlator value that is returned unmodified.
 *
 * @description
 *      This type defines an opaque value provided by the user while making 
 *      an API function call. The value is returned to the user when the 
 *      callback function is invoked. This value is not modified or used 
 *      internally by the components. It may be used by the client to help
 *      correlate a particular instance of a function call to a particular 
 *      callback. 
 * 
 *****************************************************************************/
typedef void * icp_correlator_t;

/**
 *****************************************************************************
 * @ingroup icp_BaseDataTypes
 *      Generic handle to items in the Acceleration API
 *
 * @description
 *      This type is a handle that uniquely identifies items in the 
 *      acceleration API.
 * @note
 *      To mark a handle as invalid use @ref ICP_INVALID_HANDLE.
 * 
 * 
 *****************************************************************************/
typedef uint32_t icp_handle_t;

/**
 *****************************************************************************
 * @ingroup icp_BaseDataTypes
 *      Invalid handle
 *
 * @description
 *      This define is used to identify a handle as invalid. 
 * @note
 *      To mark a buffer handle as invalid use the 
 *      @ref ICP_INVALID_BUFFER_HANDLE.
 * 
 *****************************************************************************/
#define ICP_INVALID_HANDLE ((icp_handle_t)0)

/**
 *****************************************************************************
 * @ingroup icp_BaseDataTypes
 *      Accelerator handle type
 * 
 * @description
 *      Handle used to uniquely identify an acceleration device instance.
 *
 * @note
 *      Where only a single accelerator exists on the Silicon variant, this
 *      field must be set to @ref ICP_ACCEL_HANDLE_DEFAULT.
 *
 *****************************************************************************/
typedef void * icp_accel_handle_t;

/**
 *****************************************************************************
 * @ingroup icp_BaseDataTypes
 *      Default acceleration handle value
 * 
 * @description
 *      Used as an acceleration handle value where only one acceleration device
 *      exists on silicon.
 *
 *****************************************************************************/
#define ICP_ACCEL_HANDLE_DEFAULT ((icp_accel_handle_t)0)

/**
 *****************************************************************************
 * @ingroup icp_BaseDataTypes
 *      Handle to registered callback function
 *
 * @description
 *      This type is a handle that uniquely identifies a registered callback 
 *      function.   
 * 
 *****************************************************************************/
typedef icp_handle_t icp_callback_handle_t;  

/**
 *****************************************************************************
 * @ingroup icp_BaseDataTypes
 *      Handle to registered event handler function.
 *
 * @description
 *      This type is a handle that uniquely identifies a registered event 
 *      handler function.
 * 
 *****************************************************************************/
typedef icp_handle_t icp_event_func_handle_t;  

/**
 *****************************************************************************
 * @ingroup icp_BaseDataTypes
 *     Buffer Handle 
 * 
 * @description
 *      This type uniquely identifies a buffer handle for this API.
 *
 * @purpose
 *      The intention is to present an abstraction that hides from the
 *      clients of the API any of the private, implementation-specific
 *      aspects of the buffer format used by implementations of this
 *      API.  Functions will be provided to translate between this
 *      buffer format and those buffer formats used by clients
 *      (e.g. OS-specific buffer formats such as sk_buffs, mbufs,
 *      etc.), as well as functions to get key data about the buffer,
 *      i.e. the length, pointer to the data contained in the buffer,
 *      etc.  The abstraction also supports the concept of buffer
 *      chaining.
 * @note
 *      The buffer translation module can be used to perform conversions 
 *      between various OS buffer formats and the icp_buffer_handle_t.
 *      Please refer to the buffer translation module documentation for more
 *      information on supported data buffer conversions.
 *      To define an invalid buffer handle use the 
 *      @ref ICP_INVALID_BUFFER_HANDLE define. 
 *
 *****************************************************************************/
typedef uint64_t icp_buffer_handle_t;

/**
 *****************************************************************************
 * @ingroup icp_BaseDataTypes
 *      Invalid buffer handle
 *
 * @description
 *      This define is used to identify a buffer handle as invalid. 
 * 
 *****************************************************************************/
#define ICP_INVALID_BUFFER_HANDLE ((icp_buffer_handle_t)0)

/**
 *****************************************************************************
 * @ingroup icp_BaseDataTypes
 *      Flat buffer structure containing a pointer and length member.
 * 
 * @description
 *      A flat buffer structure. The data pointer, pData, is a virtual address 
 *      however the actual data pointed to is required to be in contiguous 
 *      physical memory. It is expected that this buffer handle will be used
 *      when simple, unchained buffers are needed. The icp_buffer_handle_t 
 *      defined in icp_buffer.h describes more fully featured buffers that
 *      may be used when chaining or mapping from OS specific buffers is 
 *      required. 
 *
 *****************************************************************************/
typedef struct icp_flat_buffer_s
{   
    uint8_t *pData;
    /**< The data pointer is a virtual address, however the actual data pointed
     * to is required to be in contiguous physical memory. */
    uint32_t dataLenInBytes;
    /**< Data length specified in bytes*/
    void * clientBufferHandle;
    /**< This is an opaque field that is not read or modified internally. An
     * example usage scenario for this structure member is for the client to 
     * assign this to be the pointer for the start of the buffer in which the
     * data resides. Subsequently it may be used to recover the start of the 
     * data buffer. */
} icp_flat_buffer_t;


/**
 *****************************************************************************
 * @ingroup icp_BaseDataTypes
 *      Acceleration API status value type definition
 * @description
 *      This type definition is used for the return values used in all the
 *      acceleration API functions.  Common values are #defined, for example
 *      see @ref ICP_STATUS_SUCCESS, @ref ICP_STATUS_FAIL, etc.
 *****************************************************************************/
typedef uint32_t icp_status_t;

#define ICP_STATUS_SUCCESS (0)
/**< @ingroup icp_BaseDataTypes
 *   Success status value. */
#define ICP_STATUS_FAIL (1)
/**< @ingroup icp_BaseDataTypes
 *   Fail status value. */
#define ICP_STATUS_RETRY (2)
/**< @ingroup icp_BaseDataTypes
 *   Retry status value. */
#define ICP_STATUS_RESOURCE (3)
/**< @ingroup icp_BaseDataTypes
 *   The resource that has been requested is unavailable status value. Refer
 *   to relevant sections of the API for specifics on what the suggested
 *   course of action is. */
#define ICP_STATUS_INVALID_PARAM (4)
/**< @ingroup icp_BaseDataTypes
 *   Invalid parameter has been passed in status value. */
#define ICP_STATUS_FATAL (5)
/**< @ingroup icp_BaseDataTypes
 *   A serious error has occurred status value. Recommended course of action
 *   is to shutdown and restart the component. */
#define ICP_STATUS_UNDERFLOW (6)
/**< @ingroup icp_BaseDataTypes
 *   Underflow error status value - the client is under submitting data.
 *   This status value will be deprecated in a subsequent release of this
 *   interface. */
#define ICP_STATUS_OVERFLOW (7)
/**< @ingroup icp_BaseDataTypes
 *   Overflow error status value - the client is over submitting data.  This
 *   status value will be deprecated in a subsequent release of this
 *   interface. */
#define ICP_STATUS_NULL_PARAM (8)
/**< @ingroup icp_BaseDataTypes
 *   One or more parameters is null status value.  This status value will be
 *   deprecated in a subsequent release of this interface. */
#define ICP_STATUS_MUTEX (9)
/**< @ingroup icp_BaseDataTypes
 *   Failure with a mutex operation status value.  This status value will be
 *   deprecated in a subsequent release of this interface. */
#define ICP_STATUS_ALREADY_REGISTERED (10)
/**< @ingroup icp_BaseDataTypes
 *   An attempt was made to register an item, for example a callback, with
 *   the same value as an existing registered value.  This status value will
 *   be deprecated in a subsequent release of this interface. */
#define ICP_STATUS_INVALID_HANDLE (11)
/**< @ingroup icp_BaseDataTypes
 *   An invalid handle was passed in status value.  This status value will
 *   be deprecated in a subsequent release of this interface. */
#define ICP_STATUS_NOT_SUPPORTED (12)
/**< @ingroup icp_BaseDataTypes
 *   Operation not supported in the current implementation status value.
 *   This status value will be deprecated in a subsequent release of this
 *   interface. */

/**
 *****************************************************************************
 * @ingroup icp_BaseDataTypes
 *      Iterator type  
 *
 * @description
 *      This type is used in this API, when the same function is repeatedly
 *      called to get values from a table or other data structure. 
 *
 *      In the first call, the iterator is set to ICP_ITERATOR_FIRST. The API
 *      returns an updated value for the iterator which must be passed into
 *      the next call until the API returns an iterator value of 
 *      ICP_ITERATOR_LAST
 * 
 *****************************************************************************/
typedef uint32_t icp_iterator_t;

#define ICP_ITERATOR_FIRST  ((icp_iterator_t)0)
/**< @ingroup icp_BaseDataTypes
 *   Use this define to access the first element of a table or other data
 *   structure. */

#define ICP_ITERATOR_LAST   ((icp_iterator_t) (-1))
/**< @ingroup icp_BaseDataTypes
 *   Use this define to indicate the last iteration of a table or other data
 *   structure */

#endif /* ICP_H */
