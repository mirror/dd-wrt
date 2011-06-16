/*******************************************************************************
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
* @defgroup mil internal
*
* @ingroup mil_internal
*
* @description
*    This file contains the API definitions that the
*	kernel level code uses to communicate.
*****************************************************************************/
#ifndef _MIL_INTERNAL_H
#define _MIL_INTERNAL_H

#if defined(__linux) && defined (__KERNEL__)
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mempool.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>

#define bzero(ptr, len) (memset((ptr), '\0', (len)), (void *) 0)

#elif defined (__freebsd) && defined(_KERNEL) 
#include <sys/types.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/systm.h>

#else
#error "Unsupported OS: Unable to include the proper OS specific headers"
#endif /* defined(__linux__) && defined (__KERNEL__) and freebsd branch */

#include "icp_dcc_cfg.h"
#include "mil_dataTypes.h"

#define MIL_KERNEL_FAILURE_VALUE (-1) /**< Return value for kernel 
                                        module failure*/


/**
*****************************************************************************
* @ingroup mil_internal
*
*
* @description
*      This enumerated type described the status of MIL,
*	whether it is enabled or diabled.
*
*
*
*****************************************************************************/
typedef enum mil_enable_status_s
{
    MIL_ENABLE =0,
    MIL_DISABLE
}mil_enable_status_t;


/**
*****************************************************************************
* @ingroup mil_internal
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

mil_error_status_t GetDumpData(void *arg);

/**
*****************************************************************************
* @ingroup mil_internal
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
mil_error_status_t GetVersionInfoAll(void * arg);
/**
*****************************************************************************
* @ingroup mil_internal
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
*
*****************************************************************************/
mil_error_status_t ConfigureLivenessTimeout(void * arg);
/**
*****************************************************************************
* @ingroup mil_internal
*  	system health check
*
* @description
*  This function will get the system health check
information from DCC
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
mil_error_status_t LivenessVerify(void * arg);
/**
*****************************************************************************
* @ingroup mil_internal
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
mil_error_status_t DebugEnable(void);
/**
*****************************************************************************
* @ingroup mil_internal
*  	Disable MIL
*
* @description
*  This function will disable the MIL, and
stops other APIs from getting information.
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
mil_error_status_t DebugDisable(void);
/**
*****************************************************************************
* @ingroup mil_internal
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
mil_error_status_t GetVersionInfo(void *arg);


/**
*****************************************************************************
* @ingroup MILKernelInterface
* This API will take the same argument as copyin
* and call copyin with those arguments. 
*
* @description
*       Copies data from user to kernel space
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
* @retval  MIL_SUCCESS if everything goes fine, 
*           or else will return MIL_COPY_FAILED error
* @pre
*      None.
*
* @post
*      None.
*
*****************************************************************************/
mil_error_status_t mil_from_user(void *kernel_addr,
                                 void *user_addr,
                                 size_t size_of_buffer);


/**
*****************************************************************************
* @ingroup MILKernelInterface
* This API will take the same argument as copyout
* and call copyout with those arguments. 
*
* @description
*       Copies data from kernel to user space
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
*      This function is synchronous and non-blocking.
*
* @reentrant
*      No
*
* @threadSafe
*      No
*
* @retval  MIL_SUCCESS if everything goes fine, 
*           or else will return MIL_COPY_FAILED error
* @pre
*      None.
*
* @post
*      None.
*
*****************************************************************************/
mil_error_status_t mil_to_user(void *user_addr,
                               void *kernel_addr,
                               size_t size_of_buffer);


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
mil_error_status_t GetFileName(void * arg);


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
mil_error_status_t SetFileName(void *arg);


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
mil_error_status_t SendCommand(u_long mil_command, void * arg);


/**
*****************************************************************************
* @ingroup MILKernelInternal
*   Sen Handler
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
void MilSenHandler ( icp_dcc_sen_msg_t *pSenMsg );

#endif/*MIL_INTERNAL_H*/
