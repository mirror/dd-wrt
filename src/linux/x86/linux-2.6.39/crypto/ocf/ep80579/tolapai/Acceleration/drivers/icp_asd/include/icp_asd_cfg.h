/**************************************************************************
 * ASD:
 *      Public Header File for Run-Time System Resource Variable 
 *         Enumeration.
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
 * @file icp_asd_cfg.h
 * 
 * @defgroup icp_AsdCfg Acceleration System Driver Configuration Interface.
 * 
 * @ingroup icp_Asd
 *
 * @description
 *      This is the top level header file for the run-time system configuration
 *      parameters. This interface may be used by components of this API to
 *      access the supported run-time configuration parameters.
 *
 *****************************************************************************/

#ifndef ICP_ASD_CFG_H
#define ICP_ASD_CFG_H 1

#include "cpa.h"

/**
 *****************************************************************************
 * @ingroup icp_AsdCfg
 *      Configuration Parameters
 *
 * @description
 *      Configuration parameters for the various sub-components of the API
 *
 *****************************************************************************/
typedef enum {
        /* Invalid Config Parameter */
        ICP_ASD_CFG_PARAM_INVALID = 0,
	
	    ICP_ASD_CFG_PARAM_NUM_SYMM_CONCURRENT_REQ,
        /* Number of asymmetric concurrent requests */
        ICP_ASD_CFG_PARAM_NUM_ASYM_CONCURRENT_REQ,
        /* Size of a buffer used to preallocate the LAC random numbers in a
         * cache to use in synchronous mode
         */
        ICP_ASD_CFG_PARAM_LAC_RANDOM_CACHE_SIZE,
        /* Enable interrupt coalescing on the Lookaside eagle tail ring.*/
        ICP_ASD_CFG_PARAM_ET_RING_LOOKASIDE_INTERRUPT_COALESCING_ENABLE,

        /* Frequency of coalesced interrupt for the Lookaside eagle tail
         * ring in nanoseconds (Note: The resolution of this timer is SKU
         * dependent with a minimum accuracy of 5 nanoseconds).
         */
        ICP_ASD_CFG_PARAM_ET_RING_LOOKASIDE_COALESCE_TIMER_NS,

        /* Enable MSI interrupts for the Ring Controller */
        ICP_ASD_CFG_PARAM_ET_RING_MSI_INTERRUPT_ENABLE,

        /* Enable Fast Interrupts for the Ring Controller - FreeBSD only */
        ICP_ASD_CFG_PARAM_ET_RING_FAST_INTERRUPT_ENABLE,

        /* Add new system resource parameters here */

        /* Subcomponent parameters here - not dynamically modifiable */
        /* Address of the UOF firmware*/
        ICP_ASD_CFG_PARAM_UOF_ADDRESS,
        /* Size of the UOF firmware */
        ICP_ASD_CFG_PARAM_UOF_SIZE_BYTES,
        /* Address of the MMP firmware */
        ICP_ASD_CFG_PARAM_MMP_ADDRESS,
        /* Size of the MMP firmware */
        ICP_ASD_CFG_PARAM_MMP_SIZE_BYTES,
        /* NCDRAM address */
        ICP_ASD_CFG_PARAM_NCDRAM_ADDRESS,
        /* NCDRAM Size*/
        ICP_ASD_CFG_PARAM_NCDRAM_SIZE_BYTES,
        /* CDRAM Address */
        ICP_ASD_CFG_PARAM_CDRAM_ADDRESS,
        /* CDRAM Size */
        ICP_ASD_CFG_PARAM_CDRAM_SIZE_BYTES,

        /* Total Number of Configuration Parameters */
        ICP_ASD_CFG_PARAM_TOTAL_PARAMETERS /* !!! Do NOT change this param !!!*/
} icp_asd_cfg_param_t;

/**
 *****************************************************************************
 * @ingroup icp_AsdCfg
 *      Defintion of the type of the configuration parameter value.
 *
 * @description
 *      This type defines the ASD run-time configuration parameter value.
 * 
 *****************************************************************************/
typedef unsigned long icp_asd_cfg_value_t;

 /**
 *****************************************************************************
 * @ingroup icp_AsdCfg
 *      Definition of callback function for determining run-time configuration
 *      parameters.
 * 
 * @description
 *      This callback function prototype will be supplied to init functions by
 *      the initialising component during a call to icp_AsdCfg<sub-comp>Init.
 *      This may subsequently invoke this callback function in order to aquire
 *      run-time configuration parameters.
 * 
 * @context
 *      This function will be executed in a context that requires that sleeping
 *      must be permitted.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param configParam         IN  This input parameter is used to identify which
 *                                configuration item the value is being 
 *                                requested for.
 * @param pConfigValue        OUT Pointer to the memory into which the 
 *                                configuration parameter value will be written.
 *
 * @retval CPA_STATUS_SUCCESS       Function executed successfully.
 * @retval CPA_STATUS_FAIL          Function failed.
 * 
 * @pre
 *      Acceleration Services unit has been initialised.
 * @post
 *      None
 * @note
 *      None
 * @see
 *      icp_AsdCfgLacInit()
 * 
 *****************************************************************************/
 
typedef CpaStatus (*icp_asd_cfg_param_get_cb_func_t)(
    icp_asd_cfg_param_t configParam,
    icp_asd_cfg_value_t *pConfigValue);


/*
 * Prototype for common OS agnostic interface to return a run-time config
 * parameter.
 */
extern CpaStatus icp_AsdCfgParamGet(
                                       icp_asd_cfg_param_t configParam, 
                                       icp_asd_cfg_value_t *pConfigValue);



#endif /* ICP_ASD_CFG_H */
