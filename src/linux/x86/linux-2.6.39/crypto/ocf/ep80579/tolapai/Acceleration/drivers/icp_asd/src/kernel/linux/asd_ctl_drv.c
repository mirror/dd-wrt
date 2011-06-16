/**
 **************************************************************************
 * @file asd_ctl_drv.c
 *
 * @description
 *      This file contains the ASD code to register the ASD CTL device driver
 *      and the device drivers associated methods.
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
 **************************************************************************/
#include <linux/version.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/errno.h>
#include <linux/device.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
#include <asm/semaphore.h>
#else
#include <linux/semaphore.h>
#endif

#include "asd_cfg.h"
#include "asd_drv.h"
#include "asd_ctl.h"

/*
 * Defines associated with the ASD_CTL device
 */
#define DEVICE_NAME "icp_asdctl"      /* Character Device Driver Name */
#define MAX_DEVICES 1                 /* Number of Devices to be created */
#define BASE_MINOR_NUM 0              /* Device Minor number */

/*
 * Defines for the ASD_CTL class, class device and cdev structures
 */
static struct cdev asd_ctl_dev;
static struct class *asd_ctl_class;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
static struct class_device *asd_ctl_device;
#define ASD_CTL_DEVICE_CREATE           class_device_create
#define ASD_CTL_DEVICE_DESTROY          class_device_destroy
#else
static struct device *asd_ctl_device;
#define ASD_CTL_DEVICE_CREATE           device_create
#define ASD_CTL_DEVICE_DESTROY          device_destroy
#endif


/*
 * Mutex to be used to guard the ioctl function
 */
static DEFINE_MUTEX(asd_ctl_lock);

/*
 * Major number which will be dynamically allocated
 */
static int major_number = 0;

/*
 * This global variable is used to determine whether or not reconfiguration of
 * the system resource variables are allowed via the asd_ctl program.
 */
static CpaBoolean asd_system_configuration_allowed = TRUE;
/*
 * IOCTL function for the asd_ctl device driver
 */
static int asd_ctl_ioctl(struct inode *inode, struct file *file,
                         unsigned int cmd, unsigned long arg);

static struct file_operations asd_ctl_ops = {
      owner:THIS_MODULE,
      ioctl:asd_ctl_ioctl
};

/*
 * asd_ctl_ioctl
 *
 * IOCTL function for the asd_ctl char device driver. Copies the configuration
 * parameters from user space, updates the ASD Configuration Parameter Table
 * and initialise the subsytem.
 */
static int asd_ctl_ioctl(struct inode *inode,
                         struct file *file, unsigned int cmd, unsigned long arg)
{
        config_parameter_list_t *asd_cfg_param_list=NULL;
        int return_value = 0;
        int index = 0;

        if (cmd == IOCTL_CONFIG_SYS_RESOURCE_PARAMETERS) {
                /* 
                 * Acquire the asd_ctl_lock semaphore
                 */

                if (mutex_lock_interruptible(&asd_ctl_lock)) {
                        return -ERESTARTSYS;
                }

                /* 
                 * Reconfiguration of the ASD Config Table is only allowed
                 * before the Acceleration subsystem has been initialised.
                 * If the subsystem has been initialised, a request to
                 * reconfigure the config parameters will be rejected.
                 */
                if ( (icp_asd_auto_init == FALSE) && 
                     (asd_system_configuration_allowed == TRUE) ) {

                        asd_cfg_param_list = kzalloc(
                                  sizeof(config_parameter_list_t), GFP_KERNEL);

                        if (asd_cfg_param_list == NULL) {
                                ASD_ERROR("failed to allocate memory for the ASD"
                                          " parameter config list\n");
                                mutex_unlock(&asd_ctl_lock);
                                return -EFAULT;
                        }
                        /* 
                         * Copy the config_parameter_list from user address
                         * space.
                         */
                        return_value = copy_from_user(asd_cfg_param_list,
                                              (config_parameter_list_t *)arg,
                                              sizeof (config_parameter_list_t));

                        if (return_value == SUCCESS) {
                                ASD_DEBUG("Num of elements %d\n",
                                          asd_cfg_param_list->number_of_params);

                                /*
                                 * For each parameter in the config parameter 
                                 * list set the value accordingly in the config 
                                 * table.
                                 */
                                for (index = 0;
                                     index < asd_cfg_param_list->number_of_params;
                                     index++) {
                                        ASD_DEBUG("name: %d, value: %d\n",
                                                asd_cfg_param_list->
                                                config_param_list[index].param,
                                                asd_cfg_param_list->
                                                config_param_list[index].value);

                                        return_value =
                                            asd_cfg_param_set
                                            (asd_cfg_param_list->
                                             config_param_list[index].param,
                                             asd_cfg_param_list->
                                             config_param_list[index].value);

                                        if (return_value != SUCCESS) {
                                                ASD_ERROR ("Failed to set "
                                                     "parameter %d \n",
                                                     asd_cfg_param_list->
                                                     config_param_list[index].
                                                     param);
                                        }
                                }

                                /* 
                                 * Initialize the Acceleration Subsystem
                                 */
                                return_value = asd_init_devices();

                                if (return_value) {
                                        ASD_ERROR("failed call to "
                                                  "asd_init_devices\n");
                                        mutex_unlock(&asd_ctl_lock);
                                        return -EIO;
                                }

                                /*
                                 * Set the asd_system_configuration_allowed 
                                 * global variable to FALSE to ensure future
                                 * attempts to configure the system is rejected
                                 */
                                asd_system_configuration_allowed = FALSE;

                                /*
                                 * Free the memory allocated for the parameter
                                 * list table and  release the asd_ctl_lock
                                 * semaphore.
                                 */
                                kfree(asd_cfg_param_list);
                                mutex_unlock(&asd_ctl_lock);

                        } else {
                                ASD_ERROR("ioctl: copy error \n");
                                kfree(asd_cfg_param_list);
                                mutex_unlock(&asd_ctl_lock);
                                return -EFAULT;
                        }
                } else {
                        ASD_ERROR("System already initialised \n");
                        mutex_unlock(&asd_ctl_lock);
                        return -EIO;
                }
        } else {
                ASD_ERROR("Invalid IOCTL command specified for device");
                return -ENOTTY;
        }

        return SUCCESS;
}

/*
 * register_asd_ctl_device_driver
 *
 * Function which dynamically allocates the major number, creates the asd_ctl
 * char device driver and adds it to the system.
 */
int register_asd_ctl_device_driver(void)
{
        int return_value = 0;
        dev_t devid = 0;

        /* 
         * Create a mutex to be used to guard the device driver 
         * ioctl function.
         */
        mutex_init(&asd_ctl_lock);

        /* 
         * Dynamically allocate the Major number for the driver
         */
        return_value = alloc_chrdev_region(&devid, BASE_MINOR_NUM, MAX_DEVICES, DEVICE_NAME);

        if (return_value < 0) {
                ASD_ERROR
                    ("alloc_chrdev_region error - no major number allocated\n");
                return FAIL;
        }

        /*
         * Extract the major number
         */
        major_number = MAJOR(devid);

        /*
         * Create the asd_ctl class
         */
        asd_ctl_class = class_create(THIS_MODULE, DEVICE_NAME);
        if (IS_ERR(asd_ctl_class)) {
                ASD_ERROR("failed call to class_create for device: %s\n",
                           DEVICE_NAME);
                unregister_chrdev_region(MKDEV(major_number, BASE_MINOR_NUM), MAX_DEVICES);
                return FAIL;
        }
        /* 
         * Create the class device.
         * A class device is created so that udev will automatically create the
         * device node.
         */
        asd_ctl_device = ASD_CTL_DEVICE_CREATE(asd_ctl_class, NULL,
                                                   MKDEV(major_number, BASE_MINOR_NUM), NULL,
                                                   DEVICE_NAME);
        if(!asd_ctl_device) {
                ASD_ERROR("failed call to class_device_create for device: %s\n",
                           DEVICE_NAME);
                class_destroy(asd_ctl_class);
                unregister_chrdev_region(MKDEV(major_number, BASE_MINOR_NUM), MAX_DEVICES);
                return FAIL;
        }

        /*
         * Initialise the char device "cdev" structure
         */
        cdev_init(&asd_ctl_dev, &asd_ctl_ops);
        asd_ctl_dev.owner = THIS_MODULE;

        /*
         * Add the asd_ctl char device to the system
         */
        return_value = cdev_add(&asd_ctl_dev, devid, MAX_DEVICES);

        if (return_value < 0) {
                ASD_ERROR("Failed to add device %d\n", return_value);
                ASD_CTL_DEVICE_DESTROY(asd_ctl_class, MKDEV(major_number, BASE_MINOR_NUM));
                class_destroy(asd_ctl_class);
                unregister_chrdev_region(MKDEV(major_number, BASE_MINOR_NUM), MAX_DEVICES);
                return FAIL;
        }

        return SUCCESS;
}

/*
 * unregister_asd_ctl_device_driver
 *
 * Function which removes the asd_ctl char device from the system, deallocates
 * the major number.
 */
void unregister_asd_ctl_device_driver(void)
{

        /*
         * Remove the asd ctl char device from the system
         */
        cdev_del(&asd_ctl_dev);

        /*
         * Remove the asd_ctl class device
         */
        ASD_CTL_DEVICE_DESTROY(asd_ctl_class, MKDEV(major_number, BASE_MINOR_NUM));

        /* 
         * Remove the asd_ctl class
         */
        class_destroy(asd_ctl_class);

        /*
         * Unregister the Major number which was allocated during the
         * registration of the driver.
         */
        unregister_chrdev_region(MKDEV(major_number, BASE_MINOR_NUM), MAX_DEVICES);
}
