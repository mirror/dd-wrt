/**
 **************************************************************************
 * @file asd_cfg.c
 *
 * @description
 *      This file contains OS specific Acceleration System Driver code for 
 *      run-time configuration control of Acceleration Subsystem.
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
 **************************************************************************/
/*
 * The ASD Cfg Table
 * What components modify the table and when?
 *      1) ASD itself, when it loads, it creates table and sets up defaults.
 *             (On Windows, ASD reads values straight from the windows registry)
 *      2) On linux/freebsd, the asd_ctl program at runtime, when it updates 
 *             the table prior to telling ASD to init it's subsystem
 *
 * What components read the table and when?
 *      By subcomponent modules when their init functions are called
 */

/*
 * Objectives
 * ----------
 *  Enable future separation into a separate module from ASD PCI driver
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/errno.h>

#include "cpa.h"
#include "asd_cfg.h"
#include "asd_drv.h"
#include "asd_ctl_drv.h"
#include "asd_cfg_table.h"

/**
 * System Parameter Table Data
 */

static icp_asd_cfg_value_t *asd_cfg_table = NULL;
static CpaBoolean cfg_table_configured = FALSE;

/*
 * Lock for the asd config table to ensure that there will be no concurrent
 * accesses to the table.
 */
static struct mutex asd_config_table_lock;

/*
 * asd_get_default_cfg_value
 * Get default value for this config parameter
 */
static icp_asd_cfg_value_t asd_get_default_cfg_value(int value) {
        int i;

        if(ICP_ASD_CFG_PARAM_TOTAL_PARAMETERS <= value ||
                                ICP_ASD_CFG_PARAM_INVALID >= value) {
                return 0;
        }
        for(i=0;i<ICP_ASD_CFG_PARAM_TOTAL_PARAMETERS;i++) {
                if(value == config_parameter_string_mapping_list[i].
                                cfg_param)
                        return config_parameter_string_mapping_list[i].
                                        cfg_default_param_val;
        }
        return 0;
}
/*
 * asd_cfg_defaults_setup
 * Setup default values in the run-time config table
 */
static void asd_cfg_defaults_setup(void)
{
        int i;

        for(i=0;i<ICP_ASD_CFG_PARAM_TOTAL_PARAMETERS;i++) {
                asd_cfg_table[i] = asd_get_default_cfg_value(i);
        }
}

/*
 * asd_cfg_init
 * Allocate memory for the config table, apply the default values and register
 * the asd_ctl device driver.
 */
int asd_cfg_init(void)
{
        int status = 0;

        if (cfg_table_configured) {
                ASD_ERROR("ASD Cfg Table already configured\n");
                return FAIL;
        }

        /* Allocate memory for table and zero the contents */
        asd_cfg_table =
            kzalloc(ICP_ASD_CFG_PARAM_TOTAL_PARAMETERS *
                    sizeof(icp_asd_cfg_value_t), GFP_KERNEL);
        if (asd_cfg_table == NULL) {
                ASD_ERROR("failed to allocate memory for ASD Cfg Table\n");
                return FAIL;
        }

        /* 
         * Create a mutex lock to be used to guard the asd configuration table 
         */
        mutex_init(&asd_config_table_lock);

        /*
         * Acquire the lock before setting the default values in the table
         */
        if (mutex_lock_interruptible(&asd_config_table_lock)){
                /*
                 * Free the memory previously allocated for the config table
                 */
                kfree(asd_cfg_table);
                return -ERESTARTSYS;
        }

        asd_cfg_defaults_setup();

        cfg_table_configured = TRUE;

        /*
         * Release the mutex
         */
        mutex_unlock(&asd_config_table_lock);

        /*
         * Register the asd ctl charater device driver for dynamic configuration
         * of the system resource variables via the asd_ctl user space program
         */
        status = register_asd_ctl_device_driver();
        if (status) {
                ASD_ERROR("failed to register asd ctrl device driver\n");
                /*
                 * Free the memory previously allocated for the config table
                 */
                kfree(asd_cfg_table);
                cfg_table_configured = FALSE;
                return status;
        }

        return SUCCESS;
}

/*
 * asd_cfg_free
 * Deregister the asd_ctl device driver and free the memory used by the config
 * table.
 */
void asd_cfg_free(void)
{

        /*
         * Unregister the asd ctl character device driver
         */
        unregister_asd_ctl_device_driver();

        /* 
         * Acquire the asd_config_table_lock
         */
        if (mutex_lock_interruptible(&asd_config_table_lock)) {
                return;
        }

        if (asd_cfg_table) {
                kfree(asd_cfg_table);
        }

        cfg_table_configured = FALSE;

        /*
         * Release the asd_config_table_lock
         */
        mutex_unlock(&asd_config_table_lock);
}

/*
 * asd_cfg_param_set
 * Set the specifed parameter value in the config table.
 */
int asd_cfg_param_set(icp_asd_cfg_param_t param,
                      icp_asd_cfg_value_t param_value)
{

        if (cfg_table_configured == FALSE) {
                ASD_ERROR("ASD Cfg Table not configured\n");
                return FAIL;
        }

        if ((param <= ICP_ASD_CFG_PARAM_INVALID) ||
            (param >= ICP_ASD_CFG_PARAM_TOTAL_PARAMETERS)) {
                ASD_ERROR("Invalid parameter %d supplied\n", param);
                return FAIL;
        }
        /* 
         * Acquire the asd_config_table_lock
         */
        if (mutex_lock_interruptible(&asd_config_table_lock)) {
                return -ERESTARTSYS;
        }

        asd_cfg_table[param] = param_value;

        /*
         * Release the asd_config_table_lock
         */
        mutex_unlock(&asd_config_table_lock);

        return SUCCESS;
}

/*
 * asd_cfg_param_get
 * Read the specifed parameter value from the config table.
 */
int asd_cfg_param_get(icp_asd_cfg_param_t param,
                      icp_asd_cfg_value_t * pConfigValue)
{

        if (cfg_table_configured == FALSE) {
                ASD_ERROR("ASD Cfg Table not configured\n");
                return FAIL;
        }

        if ((pConfigValue == NULL) ||
            (param <= ICP_ASD_CFG_PARAM_INVALID) ||
            (param >= ICP_ASD_CFG_PARAM_TOTAL_PARAMETERS)) {

                ASD_ERROR("Invalid parameter %d supplied\n", param);
                return FAIL;
        }

        /* 
         * Acquire the asd_config_table_lock
         */
        if (mutex_lock_interruptible(&asd_config_table_lock)) {
                return -ERESTARTSYS;
        }

        *pConfigValue = asd_cfg_table[param];

        /*
         * Release the asd_config_table_lock
         */
        mutex_unlock(&asd_config_table_lock);

        return SUCCESS;
}
