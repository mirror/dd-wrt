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
*****************************************************************************
*
* @defgroup mil dataTypes
*
* @ingroup mildataTypes
*
* @description
*    This file contains the error and the commands that
* 	will be shared within the kernel and user mode
*
*****************************************************************************/


#ifndef _MIL_DATATYPES_H_
#define _MIL_DATATYPES_H

#if !(defined (__KERNEL__ ) || defined(_KERNEL))
#include <inttypes.h>
#endif

#if defined(__linux)
#include <linux/ioctl.h>
#endif

#if defined(__freebsd) 
#include <sys/ioccom.h>
#endif

#ifndef MIL_STATIC_VERSION
#define MIL_STATIC_VERSION "MIL.2.1.0"
#endif /*MIL_STATIC_VERSION*/


#ifndef MIL_DEVICE_NAME

#if defined(__linux) 

#define MIL_DEVICE_NAME "/dev/mil_driver"

#elif defined(__freebsd) 

#define MIL_DEVICE_NAME_FREEBSD_CREATE  "mil_driver"
#define MIL_DEVICE_NAME "/dev/mil_driver"

#else
#error "Unsupported OS: Unable to define MIL_DEVICE_NAME macro"
#endif /* defined(__linux) and #elif defined(__freebsd) */

#endif /* #ifndef (MIL_DEVICE_NAME) */

#ifndef LIVENESS_DEFAULT_TIMEOUT
#define LIVENESS_DEFAULT_TIMEOUT 500
#endif /*LIVENESS_DEFAULT_TIMEOUT*/

#ifndef MIN_TIMEOUT_VALUE
#define MIN_TIMEOUT_VALUE 100
#endif /*MIN_TIMEOUT_VALUE*/

#ifndef MAX_TIMEOUT_VALUE
#define MAX_TIMEOUT_VALUE 5000
#endif /*MAX_TIMEOUT_VALUE*/

#ifndef MIL_SECURE_NUMBER
#define MIL_SECURE_NUMBER 0xCF
#endif /*MIL_SECURE_NUMBER*/

#define MAX_FILENAME_SIZE 256/*<Maximum char length of filename*/

#ifdef MIL_DEBUG
/*macros which will be usefull only in debug mode*/

#if defined(_KERNEL) || defined(__KERNEL__)

#if defined(__linux) && defined(__KERNEL__)

#define MIL_KERNEL_PRINT(args...) printk(args)
#define MIL_KERNEL_DEBUG_PRINT(args...) printk(args)

#elif defined(__freebsd) && defined(_KERNEL)

#define MIL_KERNEL_PRINT(args...) printf(args)
#define MIL_KERNEL_DEBUG_PRINT(args...) printf(args)

#else
#error "Unsupported OS: Unable to define MIL_KERNEL_DEBUG_PRINT macro"
#endif  /* defined(__linux) && defined(__KERNEL__) and #elif freebsd branch */

#else /* defined(_KERNEL) || defined(__KERNEL__) */
#define MIL_KERNEL_DEBUG_PRINT(args...) /* define empty macro just in case it
                                         is used somewhere in the user code */

#endif /* defined(_KERNEL) || defined(__KERNEL__) */

#define MIL_USER_DEBUG_PRINT(args...) printf(args)

#define MIL_DEBUG_POSITIVE_TEST_AND_RETURN(_a, _b, _c) \
if (_a == _b) \
{ \
   return (_c); \
}

#define MIL_DEBUG_NEGATIVE_TEST_AND_RETURN(_a, _b, _c) \
if(_a != _b) \
{ \
   return (_c); \
}

#else /* defined MIL_DEBUG */

#if defined(_KERNEL) || defined(__KERNEL__)

#if defined(__linux) && defined(__KERNEL__)
#define MIL_KERNEL_PRINT(args...) printk(args)

#elif defined(__freebsd) && defined(_KERNEL)
#define MIL_KERNEL_PRINT(args...) printf(args)

#else
#error "Unsupported OS: Unable to define MIL_KERNEL_PRINT macro"
#endif /* defined(__linux) && defined(__KERNEL__) and #elif freebsd branch */

#endif /* defined(_KERNEL) || defined(__KERNEL__) */

#define MIL_KERNEL_DEBUG_PRINT(args...) /* define empty macro just in case it
                                           is used somewhere in the user code*/
#define MIL_USER_DEBUG_PRINT(args...) 

#define MIL_DEBUG_POSITIVE_TEST_AND_RETURN(_a, _b, _c) 
#define MIL_DEBUG_NEGATIVE_TEST_AND_RETURN(_a, _b, _c)
#endif /* defined(MIL_DEBUG) */

/*These macros will be used in non debug mode
specially for testing the status of ioctl calls and
dcc calls*/

#define MIL_EQUAL_TEST_AND_RETURN(_a, _b, _c) \
if(_a == _b) \
{ \
return (_c); \
}\

#define MIL_NOTEQUAL_TEST_AND_RETURN(_a, _b, _c) \
if(_a != _b) \
{ \
return (_c); \
}\
 
/**
*****************************************************************************
* @ingroup mildataTypes
*
*
* @description
*      This enumerated type describes the errors which
*	are shared between kernel and the user mode.
*	Kernel level code returns these errors and
* 	use level then prints appropriate message.
*
*
*
*****************************************************************************/

typedef enum mil_error_status_s
{
    MIL_SUCCESS = 0,
    MIL_FAILURE,
    MIL_UNREACHABLE_CODE,
    /*Erros at Kernel Space*/
    MIL_NOT_ENABLED_ERROR,
    MIL_REPEAT_ENABLE_ERROR,
    MIL_VERSION_SIZE_GET_ERROR,
    MIL_VERSION_DATA_GET_ERROR,
    MIL_DATA_DUMP_INFO_ERROR,
    MIL_DATA_DUMP_GET_ERROR,
    MIL_MEMORY_ALLOC_FAIL,
    MIL_CONFIGURE_LIVENESS_TIMEOUT_ERROR,
    MIL_LIVENESS_SIZE_GET_ERROR,
    MIL_LIVENESS_VERIFY_ERROR,
    MIL_SEN_HANDLER_REGISTER_ERROR,
    MIL_SEN_HANDLER_UNREGISTER_ERROR,
    MIL_COPY_FAILED,
    /*Errors at user space*/
    MIL_DRIVER_DEVICE_IOCTL_ERROR,
    MIL_DRIVER_DEVICE_OPEN_ERROR,
    MIL_DRIVER_DEVICE_CLOSE_ERROR,
    MIL_FILENAME_SIZE_IS_BIG,
    MIL_FILE_ERROR, 
    MIL_UNKNOWN_ERROR
}mil_error_status_t;

/**
*****************************************************************************
* @ingroup mildataTypes
*
*
* @description
*      This enumerated type describes the commands which
*      are shared between kernel and the user mode.
*      User level code sends these commands and
*      kernel level then take appropriate action.
*      Internal are the commands not visible to the user.
*
*
*
*****************************************************************************/

typedef enum mil_user_commands_s
{
    MIL_DEBUG_ENABLE = 1,
    MIL_DEBUG_DISABLE,
    MIL_GET_VERSION_SIZE,
    MIL_FILENAME_SET,
    MIL_FILENAME_GET,
    MIL_VERSION_INFO_GET_SIZE,
    MIL_DEBUG_VERSION_GET_ALL,
    MIL_LIVENESS_CONFIGURE_TIMEOUT,
    MIL_LIVENESS_GET_SIZE,
    MIL_LIVENESS_VERIFY,
    MIL_DATA_DUMP_GET_SIZE,
    MIL_DATA_DUMP_GET_DATA,
    MIL_DISPLAY_HELP,
    MIL_INTERNAL_DEBUG_ENABLE,
    MIL_INTERNAL_DATA_DUMP_GET,
    MIL_INTERNAL_VERSION_GET,
    MIL_INTERNAL_LIVENESS_VERIFY,
    MIL_INVALID_COMMAND
} mil_commands_t;


#if (defined(__linux) || defined(__freebsd))

#define MIL_CTL_IO_MAGIC    0xCC

#define IOCTL_CMD_MIL_DEBUG_ENABLE \
            _IO(MIL_CTL_IO_MAGIC, MIL_DEBUG_ENABLE)

#define IOCTL_CMD_MIL_DEBUG_DISABLE \
            _IO(MIL_CTL_IO_MAGIC, MIL_DEBUG_DISABLE)

#define IOCTL_CMD_MIL_GET_VERSION_SIZE \
            _IOWR(MIL_CTL_IO_MAGIC, MIL_GET_VERSION_SIZE, int32_t)

#define IOCTL_CMD_MIL_FILENAME_SET \
            _IOWR(MIL_CTL_IO_MAGIC, MIL_FILENAME_SET, int32_t)

#define IOCTL_CMD_MIL_FILENAME_GET \
            _IOWR(MIL_CTL_IO_MAGIC, MIL_FILENAME_GET, int32_t)

#define IOCTL_CMD_MIL_VERSION_INFO_GET_SIZE \
            _IOWR(MIL_CTL_IO_MAGIC, MIL_VERSION_INFO_GET_SIZE, int32_t)

#define IOCTL_CMD_MIL_DEBUG_VERSION_GET_ALL \
            _IOWR(MIL_CTL_IO_MAGIC, MIL_DEBUG_VERSION_GET_ALL, int32_t)

#define IOCTL_CMD_MIL_LIVENESS_CONFIGURE_TIMEOUT \
            _IOWR(MIL_CTL_IO_MAGIC, MIL_LIVENESS_CONFIGURE_TIMEOUT, int32_t)

#define IOCTL_CMD_MIL_LIVENESS_GET_SIZE \
            _IOWR(MIL_CTL_IO_MAGIC, MIL_LIVENESS_GET_SIZE, int32_t)

#define IOCTL_CMD_MIL_LIVENESS_VERIFY \
            _IOWR(MIL_CTL_IO_MAGIC, MIL_LIVENESS_VERIFY, int32_t)

#define IOCTL_CMD_MIL_DATA_DUMP_GET_SIZE \
            _IOWR(MIL_CTL_IO_MAGIC, MIL_DATA_DUMP_GET_SIZE, int32_t)

#define IOCTL_CMD_MIL_DATA_DUMP_GET_DATA \
            _IOWR(MIL_CTL_IO_MAGIC, MIL_DATA_DUMP_GET_DATA, int32_t)

#define IOCTL_CMD_MIL_DISPLAY_HELP  \
            _IO(MIL_CTL_IO_MAGIC, MIL_DISPLAY_HELP)

#define IOCTL_CMD_MIL_INTERNAL_DEBUG_ENABLE \
            _IO(MIL_CTL_IO_MAGIC, MIL_INTERNAL_DEBUG_ENABLE)

#define IOCTL_CMD_MIL_INTERNAL_DATA_DUMP_GET \
            _IOWR(MIL_CTL_IO_MAGIC, MIL_INTERNAL_DATA_DUMP_GET, int32_t)
                
#define IOCTL_CMD_MIL_INTERNAL_VERSION_GET \
            _IOWR(MIL_CTL_IO_MAGIC, MIL_INTERNAL_VERSION_GET, int32_t)
                
#define IOCTL_CMD_MIL_INTERNAL_LIVENESS_VERIFY \
            _IOWR(MIL_CTL_IO_MAGIC, MIL_INTERNAL_LIVENESS_VERIFY, int32_t)
                
#define IOCTL_CMD_MIL_INVALID_COMMAND \
            _IO(MIL_CTL_IO_MAGIC, MIL_INVALID_COMMAND)

#else
#error "Unsupported OS: Unable to define IOCTL macros"

#endif /* (defined(__linux) || defined(__freebsd) */


/**
*****************************************************************************
* @ingroup mildataTypes
*
* @description
*    This structure defines the fields for buffer that
*    is passed between the user and the kernel mode
*    while getting the max dump size and the no of modules 
*
*
*****************************************************************************/
typedef struct DataDumpSizeBuffer_s
{
    uint32_t noOfModule;
    uint32_t maxDumpSize; 
} DataDumpSizeBuffer_t;

#endif /*ifdef MIL_DATATYPES*/
