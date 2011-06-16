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
* @file mil_linux_kernel.c
*
* @defgroup MIL
*
* @ingroup MILKernelInterface
*
* @description
*      This file contains the functions to get the data
*      from the user level and send it to
*	the internal APIs to proceed
*
*
*****************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>

#include "icp_dcc_cfg.h"
#include "dcc/icp_dcc.h"
#include "mil_internal.h"

static int mil_open_device_count = 0;/**<number of open
                                         connection for this device*/
static int major_number = 0; /**<Registration id for the device*/

static struct mutex mil_usage_counter_mutex;


MODULE_LICENSE("Dual BSD/GPL");

/**
*****************************************************************************
* @ingroup MILKernelInterface
*  Open character device
*
* @description
*  This function will get called when the character
*	device gets opened
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
* @retval  integer as defined by the complier
*
* @pre
*      Character device must be present
*
* @post
*      None.
*
*****************************************************************************/
int mil_open_device( struct inode *inode, struct file *filp)
{
    int status = 0;
    *inode = *inode;
    *filp = *filp;

    mutex_lock(&mil_usage_counter_mutex);
    MIL_KERNEL_DEBUG_PRINT ("[MIL_ks:mil_open_device] mutex_lock\n");
    if ( mil_open_device_count > 0 )
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:mil_open_device] "\
                            "device is already opened.\n" );
        status = MIL_KERNEL_FAILURE_VALUE;
    }
    else
    {
        mil_open_device_count++;
        MIL_KERNEL_DEBUG_PRINT ("[MIL_ks:mil_open_device] "\
                        "device is opened with %d \n", mil_open_device_count);
    }
    mutex_unlock(&mil_usage_counter_mutex);
    MIL_KERNEL_DEBUG_PRINT ("[MIL_ks:mil_open_device] mutex_unlock\n");
    return 0;
}

/**
*****************************************************************************
* @ingroup MILKernelInterface
*  Close character device
*
* @description
*  This function will get called when the character
*	device gets closed
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
* @retval  integer as defined by the complier
*
* @pre
*      Character device must be present and opened
*
* @post
*      None.
*
*****************************************************************************/

int
mil_release_device (struct inode *inode, struct file *filp)
{
    int status = 0;
    *inode = *inode;
    *filp = *filp;

    mutex_lock(&mil_usage_counter_mutex);
    MIL_KERNEL_DEBUG_PRINT("[MIL_ks:mil_release_device] mutex_lock\n");
    if ( mil_open_device_count > 0 )
    {
        mil_open_device_count--;
    
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:mil_release_device] device is closed "\
                                "with %d\n", mil_open_device_count );
    }
    else
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:mil_release_device] device is closed");
        status = MIL_KERNEL_FAILURE_VALUE;
    }
    mutex_unlock(&mil_usage_counter_mutex);
    MIL_KERNEL_DEBUG_PRINT("[MIL_ks:mil_release_device] mutex_unlock\n");
    return status;
}

/**
*****************************************************************************
* @ingroup MILKernelInterface
*  ioctl with character device
*
* @description
*  This function will get called when the character
*	device is opened for ioctl
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
* @retval  integer as defined by the complier
*
* @pre
*      Character device must be present
*
* @post
*      None.
*
*****************************************************************************/
int mil_ioctl_device (
    struct inode *inode ,
    struct file *filp,
    unsigned int cmd ,
    unsigned long arg )
{
    mil_error_status_t sent_status = MIL_SUCCESS;
    int ret_val = 0;

    /*Workaround to eliminate Parasoft warnings*/
    inode = inode;
    filp = filp;

    MIL_KERNEL_DEBUG_PRINT("\n[MIL_ks:mil_ioctl_device] "\
                            "cmd = %d, arg = %p, *arg = %p\n", 
                            cmd, (void *) arg, (void *) *((int32_t *) arg));
    switch(cmd)
    {
    case IOCTL_CMD_MIL_DEBUG_ENABLE:
    case IOCTL_CMD_MIL_DEBUG_DISABLE:
    case IOCTL_CMD_MIL_GET_VERSION_SIZE:
    case IOCTL_CMD_MIL_FILENAME_SET:
    case IOCTL_CMD_MIL_FILENAME_GET:
    case IOCTL_CMD_MIL_VERSION_INFO_GET_SIZE:
    case IOCTL_CMD_MIL_DEBUG_VERSION_GET_ALL:
    case IOCTL_CMD_MIL_LIVENESS_CONFIGURE_TIMEOUT:
    case IOCTL_CMD_MIL_LIVENESS_GET_SIZE:
    case IOCTL_CMD_MIL_LIVENESS_VERIFY:
    case IOCTL_CMD_MIL_DATA_DUMP_GET_SIZE:
    case IOCTL_CMD_MIL_DATA_DUMP_GET_DATA:
    case IOCTL_CMD_MIL_DISPLAY_HELP :
    case IOCTL_CMD_MIL_INTERNAL_DEBUG_ENABLE:
    case IOCTL_CMD_MIL_INTERNAL_DATA_DUMP_GET:
    case IOCTL_CMD_MIL_INTERNAL_VERSION_GET:
    case IOCTL_CMD_MIL_INTERNAL_LIVENESS_VERIFY:
    case IOCTL_CMD_MIL_INVALID_COMMAND:
        MIL_KERNEL_DEBUG_PRINT ("[MIL_ks:mil_ioctl_device] command "\
                                "recognized\n"); 
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:mil_ioctl_device] "\
                                "SendCommand(%ud, %p)\n", cmd, (void *) arg);
        sent_status = SendCommand(cmd, (void *)arg);
        if (sent_status != MIL_SUCCESS)
            {
            MIL_KERNEL_DEBUG_PRINT("[MIL_ks:mil_ioctl_device] "\
                        "SendCommand(%ud, ...) failed with err = %d\n",\
                        cmd, (int) sent_status); 
            ret_val = -EFAULT;
            }
        break;
    default:
        MIL_KERNEL_DEBUG_PRINT ("[MIL_ks:mil_ioctl_device] " \
                                "command NOT recognized\n");
        ret_val = -ENOTTY;
        break;
    }
    MIL_KERNEL_DEBUG_PRINT ("[MIL_ks:mil_ioctl_device] return value = %d\n\n",
                           ret_val);
    return ret_val;
}


/**
*****************************************************************************
* @ingroup MILKernelInterface
*  This API will take the same argument as copy_from_user \
   and call copy_from_user with those arguments. \
   
*
* @description
*  This function will get called when the character
*   device is opened for ioctl
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
* @retval  Will return MIL_SUCCESS if everything goes fine, \
            or else will return MIL_COPY_FAILED error
*
* @pre
*      None.
*
* @post
*      None.
*
*****************************************************************************/
mil_error_status_t
mil_from_user (void * buffer,
               void * arg,
               size_t size)
{
    unsigned long ioctl_error = 0UL;
    
    ioctl_error = copy_from_user((char *)buffer, (void *) arg, size);
    /*If (ioctl_error !=0) return MIL_COPY_FAILED*/
    MIL_NOTEQUAL_TEST_AND_RETURN (ioctl_error, 0, MIL_COPY_FAILED)
    return MIL_SUCCESS;
}

 
/**
*****************************************************************************
* @ingroup MILKernelInterface
* This API will take the same argument as copy_to_user
* and call copy_to_user with those arguments. 
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
mil_error_status_t
mil_to_user(void * arg ,
            void *buffer ,
            size_t size)
{
    unsigned long ioctl_error = 0UL;
    
    ioctl_error = copy_to_user((void *)arg, (char *)buffer, size);
    MIL_NOTEQUAL_TEST_AND_RETURN (ioctl_error, 0, MIL_COPY_FAILED)
    return MIL_SUCCESS;
}

/*
  Standard structure for file operations
*/
static struct
            file_operations mil_operations =
    {
    open :
        mil_open_device,
    release :
        mil_release_device,
    ioctl :
        mil_ioctl_device,
    };

/**
*****************************************************************************
* @ingroup MILKernelInterface
*  initialization of module
*
* @description
*  This function will get called when the module
is inserted using insmod. The main purpose is
to register the character device.
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
*      This function is synchronous and blocking.
*
* @reentrant
*      No
*
* @threadSafe
*      Yes
*
* @retval  integer as defined by the complier
*
* @pre
*      Character device must be present
*
* @post
*      None.
*
*****************************************************************************/
int mil_init ( void )
{
    MIL_KERNEL_PRINT("\n[MIL_ks:mil_init] *** loading MIL kernel module (" \
                MIL_STATIC_VERSION " - " __DATE__ " - " __TIME__")\n\n" );

    mutex_init(&mil_usage_counter_mutex);
    MIL_KERNEL_DEBUG_PRINT ("[MIL_ks:mil_init] mutex_init\n");


    /*First register the device*/
    major_number = register_chrdev (
                       0 ,
                       MIL_DEVICE_NAME ,
                       &mil_operations );

    MIL_KERNEL_DEBUG_PRINT ("[MIL_ks:mil_init] MIL major_number is %d.\n", \
                            major_number);
    if ( major_number < 0 )
    {
        MIL_KERNEL_DEBUG_PRINT("[MIL_ks:mil_init] "\
                                "could not register the driver device.\n");
        return MIL_KERNEL_FAILURE_VALUE;
    }
    return 0;
}


/**
*****************************************************************************
* @ingroup MILKernelInterface
*  removal of module
*
* @description
*  This function will get called when the module
is removed using rmmod. The main purpose is
to unregister the character device.
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
* @retval  integer as defined by the complier
*
* @pre
*      Character device must be present
*
* @post
*      None.
******************************************************************/
void mil_fini ( void )
{
    MIL_KERNEL_PRINT("\n[MIL_ks:mil_fini] *** unloading "\
                    "MIL kernel module\n\n");
    /*Unregister the device */
    unregister_chrdev (
                 major_number ,
                 MIL_DEVICE_NAME );
}

module_init(mil_init);
module_exit(mil_fini);

