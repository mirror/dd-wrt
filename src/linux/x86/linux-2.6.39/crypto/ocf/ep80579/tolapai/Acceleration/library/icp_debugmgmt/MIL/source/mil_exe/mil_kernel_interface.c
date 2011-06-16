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
 * @file mil_kernel_interface.c
 *
 * @defgroup MIL
 *
 * @ingroup MILKernelInterface
 *
 * @description
 *      This file contains the functions to get the data
 *      from the user interface part and then send it to the kernel
 *      module in the desired format
 *
 *****************************************************************************/

#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include "mil_dataTypes.h"

#define MIL_FAILURE_VALUE -1

/**
 *****************************************************************************
 * @ingroup MILKernelInterface
 *  Open the character device
 *
 * @description
 *  This function will get called when the character
	device is to be opened.
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
 * @param devname IN character device name to be opened
 *
 * @return integer which is device_handler number
 *
 * @pre
 *      device must be present
 *
 * @post
 *      None.
 *
 *****************************************************************************/

int
mil_open_device(char* devname)
{
    int device_handle = open ( devname , O_RDWR );

    if ( device_handle < 0 )
    {
        MIL_USER_DEBUG_PRINT("[MIL_us:mil_open_device] "\
                            "error opening device\n");
        return MIL_FAILURE_VALUE;
    }
    else
    {
        MIL_USER_DEBUG_PRINT("[MIL_us:mil_open_device] "\
                            "opened device with handle (%d)\n", \
                            device_handle);
        return device_handle;
    }
};

/**
 *****************************************************************************
 * @ingroup MILKernelInterface
 *  	Close the character device
 *
 * @description
 *  This function will get called when the character
	device is to be closed.
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
 * @param device_handler The handler to the device
	which needs to be closed
 *
 * @return 0 if success
	-1 if failure
 *
 * @pre
 *      device must be present and open
 *
 * @post
 *      None.
 *
 *****************************************************************************/

mil_error_status_t
close_device( int device_handler)
{
    int err = 0;
    if ( device_handler <= 0)
    {
        MIL_USER_DEBUG_PRINT("[MIL_us:close_device] "\
                            "invalid device handler.\n");
        return MIL_FAILURE;
    }
    err = close( device_handler);
    if ( err != 0 )
    {
        MIL_USER_DEBUG_PRINT("[MIL_us:close_device] close failed.\n");
        return MIL_FAILURE;
    }
    return MIL_SUCCESS;
};

/**
 *****************************************************************************
 * @ingroup MILKernelInterface
 *  	ioctl with the character device
 *
 * @description
 *  This function will do the ioctl with the device driver
	when it gets called .
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
 * @param device_handler IN character device handler to be used
 * @param mil_command the buffer to be used to pass to the kernel
 *
 * @return MIL_SUCCESS for success
	or appropriate mil_error_status_t
 *
 * @pre
 *      device must be present and open
 *
 * @post
 *      None.
 *
 *****************************************************************************/
mil_error_status_t
ioctl_device( int device_handler, u_long mil_command, void* arg)
{
    int status = 0;

    MIL_USER_DEBUG_PRINT("[MIL_us:ioctl_device] ioctl( %d, %lu, %p)\n", \
        device_handler, mil_command, arg);

    status = ioctl ( device_handler ,
                     mil_command ,
                     arg );

    MIL_USER_DEBUG_PRINT("[MIL_us:ioctl_device] ioctl return = %d\n", status);
    MIL_USER_DEBUG_PRINT("[MIL_us:ioctl_device] ioctl errno = %d\n", errno);
    if (status != MIL_SUCCESS )
    {
        MIL_USER_DEBUG_PRINT("[MIL_us:ioctl_device] ioctl call failed.\n");
        return (mil_error_status_t)status ;
    }
    return MIL_SUCCESS;

}

/**
 *****************************************************************************
 * @ingroup MILKernelInterface
 *  	talk with the user interface module
 *
 * @description
 *  This function will get called bu user interface module
	to talk with the kernel module. This function then
	opens the device, do the ioctl and then close the
	device.
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
 * @param mil_command IN the buffer to be passed to the kernel driver
 *
 * @return appropriate mil_error_status_t
 *
 * @pre
 *      device must be present
 *
 * @post
 *      None.
 *
 *****************************************************************************/
mil_error_status_t
call_ioctl( u_long mil_command, void* arg)
{
    int device_handle = 0;
    mil_error_status_t err = MIL_SUCCESS;
    device_handle = mil_open_device ( MIL_DEVICE_NAME );
    if ( device_handle <= 0 )
    {
        return MIL_DRIVER_DEVICE_OPEN_ERROR;
    }
    else
    {
        MIL_USER_DEBUG_PRINT("[MIL_us:call_ioctl] device opened successfully "\
                               "with the handle %d\n" , \
                               device_handle );
    }

    MIL_USER_DEBUG_PRINT("[MIL_us:call_ioctl] ioctl_device( "\
                        "cmd = %lu, arg = %p)\n", mil_command, arg);

    err = ioctl_device ( device_handle , mil_command, arg);
    if ( err != MIL_SUCCESS )
    {
        MIL_USER_DEBUG_PRINT("[MIL_us:call_ioctl] ioctl_device failed "\
                            "(err = %d)\n", err );
        close_device( device_handle);
        return err;
    }
    MIL_USER_DEBUG_PRINT("[MIL_us:call_ioctl] ioctl_device succeded "\
                        "(return = %d)\n", err );

    err = close_device( device_handle);
#ifdef MIL_DEBUG
    if ( err != MIL_SUCCESS )
    {
        return MIL_DRIVER_DEVICE_CLOSE_ERROR;
    }
#endif
    return MIL_SUCCESS;
}
