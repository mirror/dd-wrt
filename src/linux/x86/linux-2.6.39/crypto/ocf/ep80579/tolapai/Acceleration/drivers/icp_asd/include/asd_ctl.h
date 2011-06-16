/**
 **************************************************************************
 * @file asd_ctl.h
 *
 * @description
 *      This is the header file for the ASD CTL user space program.
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
#ifndef ASD_CTL_H
#define ASD_CTL_H 1

#if defined (__linux__)
#include <linux/ioctl.h>
#elif defined (__freebsd)
#include <sys/ioccom.h>
#endif

#include "icp_asd_cfg.h"


#define STATIC static
#define MAIN main
#define CONFIG_FILE "/etc/icp_asd.conf"

/*
 * Configuration parameter structure
 * Consists of the parameter of type icp_asd_cfg_param_t and the
 * parameter value
 */
typedef struct config_parameter_s{
    icp_asd_cfg_param_t param;
    icp_asd_cfg_value_t value;

}config_parameter_t;

/* 
 * Configuration parameter list
 * Consists of a list of parameter names and values and the number of 
 * elements in the list.
 */
typedef struct  config_parameter_list_s{

    config_parameter_t config_param_list[ICP_ASD_CFG_PARAM_TOTAL_PARAMETERS];
    int number_of_params;

}config_parameter_list_t;

/*
 * Max line size in the icp_asd.conf which will be read
 */
#define MAX_LINE_SIZE 160

/* 
 * Define the IOCTL number for use between the asd_ctl and the kernel
 */

#define ASD_CTL_IOC_MAGIC 'a'
#define IOCTL_CONFIG_SYS_RESOURCE_PARAMETERS _IOW(ASD_CTL_IOC_MAGIC, 0, config_parameter_list_t)

#endif /*#ifndef ASD_CTL_H*/
