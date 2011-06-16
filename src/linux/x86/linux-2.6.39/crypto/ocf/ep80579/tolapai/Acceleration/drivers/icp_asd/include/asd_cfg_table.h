/**************************************************************************
 * ASD:
 *      Public Header File for Run-Time System Resource Variable Table
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
 **************************************************************************/

/*
 ****************************************************************************
 * Doxygen group definitions
 ****************************************************************************/

/**
 *****************************************************************************
 * @file asd_cfg_table.h
 * 
 * @defgroup icp_AsdCfg Acceleration System Driver Configuration Interface and
 *      default parameter values
 * 
 * @ingroup icp_Asd
 *
 * @description
 *      The table in this file gives the default values for the Acceleration 
 *      System Driver. The table is used in the user space program to make
 *      sure parameter names are correct. It is also used in the kernel module
 *      to load default parameter values.
 *****************************************************************************/
/*
 * Max parameter name string length.
 */
#define MAX_PARAM_NAME_STR_LEN  64

/*
 *Max parameter value string length.
 */
#define MAX_PARAM_VALUE_STR_LEN  32

/*
 * The following typedef structure is used to map a parameter string to it's
 * icp_asd_cfg_param_t configuration parameter
 */
typedef struct config_parameter_string_mapping_s{
    char param_string[MAX_PARAM_NAME_STR_LEN];
    int cfg_default_param_val;
    icp_asd_cfg_param_t cfg_param;
} config_parameter_string_mapping_t;

/* 
 * Define the config_parameter_string_mapping_list of type 
 * config_parameter_string_mapping_t and initialise it with the valid
 * "Parameter String, Configuration Parameter" pairs. Default parameter values
 * are used here. These parameters are configurable from the user space program.
 */
config_parameter_string_mapping_t 
    config_parameter_string_mapping_list[ICP_ASD_CFG_PARAM_TOTAL_PARAMETERS] = {
    /*
     * Fastpath Configuration Parameters
     * Default values, taken originally from the MemoryAnalysis.xls
     */

    /*
     * Lookaside Crypto Configuration Parameters
     */
    {"NUM_CONCURRENT_LAC_SYMMETRIC_REQUESTS", 768,
     ICP_ASD_CFG_PARAM_NUM_SYMM_CONCURRENT_REQ},
    {"NUM_CONCURRENT_LAC_ASYMMETRIC_REQUESTS", 512,
     ICP_ASD_CFG_PARAM_NUM_ASYM_CONCURRENT_REQ},
    {"LAC_RANDOM_CACHE_SIZE", 131070, ICP_ASD_CFG_PARAM_LAC_RANDOM_CACHE_SIZE},

    /* 
     * Interrupt Coalescing Configuration Parameters
     */
    {"ET_RING_LOOKASIDE_INTERRUPT_COALESCING_ENABLE", 1,
     ICP_ASD_CFG_PARAM_ET_RING_LOOKASIDE_INTERRUPT_COALESCING_ENABLE},
    {"ET_RING_LOOKASIDE_COALESCE_TIMER_NS", 10000,
     ICP_ASD_CFG_PARAM_ET_RING_LOOKASIDE_COALESCE_TIMER_NS},

    /*
     * Enable MSI Interrupt Configuration Parameter
     */
    {"ET_RING_MSI_INTERRUPT_ENABLE", 0,
     ICP_ASD_CFG_PARAM_ET_RING_MSI_INTERRUPT_ENABLE},

    /*
     * Enable Filtered Interrupts on FreeBSD
     */
    {"ET_RING_FAST_INTERRUPT_ENABLE", 0,
     ICP_ASD_CFG_PARAM_ET_RING_FAST_INTERRUPT_ENABLE},
};
